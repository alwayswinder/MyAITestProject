# 程序化关卡搭建 — 据点 + 弯曲路径 + 树木

## 环境要求

- UE 5.x 项目，已启用 Python 脚本插件
- 地形 Landscape 使用材质 `M_Landscape`，包含图层参数 `"1"`, `"2"`, `"3"`, `"4"`, `"RemoveFoliage"`
- 图层资产存在: `2_LayerInfo`, `3_LayerInfo`
- 树木网格: `/Game/Orasot_Bundle/LowPolyForestVol2/StaticMeshes/Environment/SM_Env_Pine_Tree_1`

> 如果项目不同，只需要修改 `TREE_MESH` 路径和图层参数名，其余逻辑通用。

## 方案逻辑

```
1. 清空旧数据（删树 + 清零图层权重）
2. 随机生成 N 个据点坐标（仅逻辑点位，不画图层）
3. 用 MST + 额外边生成路径拓扑
4. 用贝塞尔曲线生成弯曲路径数据（一次生成，画 RT 和种树共用）
5. 将路径绘制到 RenderTarget → 导入地形 Layer
6. 在据点和路径之外的空间网格中填充树木
```

## 执行参数

| 参数 | 默认值 | 含义 |
|------|--------|------|
| `RANDOM_SEED` | `int(time.time())` | 每次运行随机；如需复现可固定为某值 |
| `NUM_NODES` | 30 | 据点数量 |
| `NODE_RADIUS_WORLD` | 500 | 据点半径(units)，仅用于树木排除 |
| `MIN_NODE_DISTANCE` | 2000 | 据点间最小间距 |
| `MARGIN` | 300 | 据点距地形边缘最小距离 |
| `PATH_WIDTH_WORLD` | 250 | 路径宽度(units) |
| `PATH_CURVINESS` | 0.25 | 弯曲度(0=直线, 0.5=强弯曲) |
| `EXTRA_EDGE_RATIO` | 0.35 | MST 之外额外添加路径的比例 |
| `LAYER_3_NAME` | `"3"` | 地形材质中路径对应的图层参数名 |
| `LAYER_WEIGHT` | 0.5 | 路径绘制权重(0~1) |
| `TREE_DENSITY` | 2500 | 树木总数 |
| `TREE_MIN_SPACING` | 300 | 树木间最小间距 |
| `BUFFER_WORLD` | 300 | 树木与路径/据点的间隔 |

## 完整脚本（复制后直接运行）

```python
import unreal
import random
import math
import time

# ============================================================
# 配置区 — 按需修改
# ============================================================
RANDOM_SEED = int(time.time())     # 固定为某值可复现
NUM_NODES = 30
NODE_RADIUS_WORLD = 500
PATH_WIDTH_WORLD = 250
EXTRA_EDGE_RATIO = 0.35
MIN_NODE_DISTANCE = 2000
PATH_CURVINESS = 0.25
LAYER_3_NAME = "3"
LAYER_WEIGHT = 0.5
MARGIN = 300
TREE_MESH = "/Game/Orasot_Bundle/LowPolyForestVol2/StaticMeshes/Environment/SM_Env_Pine_Tree_1.SM_Env_Pine_Tree_1"
TREE_DENSITY = 2500
TREE_MIN_SPACING = 300
BUFFER_WORLD = 300

# ============================================================
# Step 1: 获取地形 & 计算分辨率
# ============================================================
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == 'Landscape':
        landscape = actor
        break
assert landscape, "场景中没有 Landscape!"
proxy = unreal.LandscapeProxy.cast(landscape)

# 关键：计算地形权重图分辨率
# landscape_import_weightmap_from_render_target 是 1:1 像素映射，RT 必须精确匹配
components = landscape.get_components_by_class(unreal.LandscapeComponent)
section_bases = [(comp.section_base_x, comp.section_base_y) for comp in components]
xs = sorted(set(b[0] for b in section_bases))
section_size = xs[1] - xs[0]
total_quads = (max(xs) - min(xs)) + section_size
RT_SIZE = total_quads + 1
print(f"权重图分辨率: {RT_SIZE}x{RT_SIZE}")

# 地形世界范围
origin, extent = proxy.get_actor_bounds(False)
world_min_x = origin.x - extent.x
world_max_x = origin.x + extent.x
world_min_y = origin.y - extent.y
world_max_y = origin.y + extent.y
world_width = world_max_x - world_min_x
world_height = world_max_y - world_min_y

def world_to_px(wx, wy):
    """世界坐标 → 像素坐标（用于 RT 绘制）"""
    return ((wx - world_min_x) / world_width * (RT_SIZE - 1),
            (wy - world_min_y) / world_height * (RT_SIZE - 1))

# ============================================================
# Step 2: 清空旧数据
# ============================================================
all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
tree_actors = [a for a in all_actors if a.get_folder_path() == "Trees"]
for actor in tree_actors:
    unreal.EditorLevelLibrary.destroy_actor(actor)
print(f"删除旧树: {len(tree_actors)}")

world = unreal.EditorLevelLibrary.get_editor_world()
rt_black = unreal.RenderingLibrary.create_render_target2d(
    world, RT_SIZE, RT_SIZE, unreal.TextureRenderTargetFormat.RTF_R8
)
unreal.RenderingLibrary.clear_render_target2d(world, rt_black, unreal.LinearColor(0, 0, 0, 1))
# 清零之前用过的图层
for layer_name in ["2", "3"]:
    proxy.landscape_import_weightmap_from_render_target(rt_black, layer_name)
print("图层已清零")

# ============================================================
# Step 3: 生成据点坐标（仅逻辑，不画图层）
# ============================================================
random.seed(RANDOM_SEED)
nodes_world = []
attempts = 0
while len(nodes_world) < NUM_NODES and attempts < 8000:
    x = random.uniform(world_min_x + MARGIN, world_max_x - MARGIN)
    y = random.uniform(world_min_y + MARGIN, world_max_y - MARGIN)
    valid = all(math.sqrt((x-nx)**2 + (y-ny)**2) >= MIN_NODE_DISTANCE
                for nx, ny in nodes_world)
    if valid:
        nodes_world.append((x, y))
    attempts += 1
print(f"据点: {len(nodes_world)} 个")

nodes_px = [world_to_px(wx, wy) for wx, wy in nodes_world]

# ============================================================
# Step 4: Kruskal MST + 额外边
# ============================================================
edges_all = []
for i in range(len(nodes_px)):
    for j in range(i+1, len(nodes_px)):
        d = math.sqrt((nodes_px[i][0]-nodes_px[j][0])**2 + (nodes_px[i][1]-nodes_px[j][1])**2)
        edges_all.append((d, i, j))
edges_all.sort()

parent = list(range(len(nodes_px)))
def find(x):
    while parent[x] != x:
        parent[x] = parent[parent[x]]
        x = parent[x]
    return x
def union(a, b):
    a, b = find(a), find(b)
    if a != b:
        parent[a] = b
        return True
    return False

selected = []
for d, i, j in edges_all:
    if union(i, j):
        selected.append((i, j))
    if len(selected) == len(nodes_px) - 1:
        break

extra_n = int(len(selected) * EXTRA_EDGE_RATIO)
sel_set = set((min(i,j), max(i,j)) for i,j in selected)
remaining = [(d, i, j) for d, i, j in edges_all if (min(i,j), max(i,j)) not in sel_set]
random.shuffle(remaining)
for _, i, j in remaining[:extra_n]:
    selected.append((i, j))
print(f"路径: {len(selected)} 条 (MST {NUM_NODES-1} + 额外 {len(selected)-NUM_NODES+1})")

# ============================================================
# Step 5: 贝塞尔曲线 — 一次生成，两处复用
# ============================================================
def bezier(p0, p1, p2, p3, n):
    pts = []
    for t_i in range(n + 1):
        t = t_i / n
        u = 1 - t
        x = u**3*p0[0] + 3*u**2*t*p1[0] + 3*u*t**2*p2[0] + t**3*p3[0]
        y = u**3*p0[1] + 3*u**2*t*p1[1] + 3*u*t**2*p2[1] + t**3*p3[1]
        pts.append((x, y))
    return pts

path_world = []   # 用于树木排除
path_pixel = []   # 用于绘制 RT

for i, j in selected:
    swx, swy = nodes_world[i]
    ewx, ewy = nodes_world[j]
    dx, dy = ewx - swx, ewy - swy
    length = math.hypot(dx, dy)
    nx, ny = -dy/length, dx/length if length > 0.001 else (0, 0)
    o1 = random.uniform(-PATH_CURVINESS, PATH_CURVINESS) * length
    o2 = random.uniform(-PATH_CURVINESS, PATH_CURVINESS) * length
    cp1 = (swx + dx*0.33 + nx*o1, swy + dy*0.33 + ny*o1)
    cp2 = (swx + dx*0.66 + nx*o2, swy + dy*0.66 + ny*o2)
    pts_w = bezier((swx, swy), cp1, cp2, (ewx, ewy), 201)
    path_world.append(pts_w)
    path_pixel.append([world_to_px(px, py) for px, py in pts_w])

# ============================================================
# Step 6: 绘制路径 → 导入 Layer 3
# ============================================================
color = unreal.LinearColor(LAYER_WEIGHT, LAYER_WEIGHT, LAYER_WEIGHT, 1)
rt = unreal.RenderingLibrary.create_render_target2d(
    world, RT_SIZE, RT_SIZE, unreal.TextureRenderTargetFormat.RTF_R8
)
unreal.RenderingLibrary.clear_render_target2d(world, rt, unreal.LinearColor(0,0,0,1))
canvas, _, ctx = unreal.RenderingLibrary.begin_draw_canvas_to_render_target(world, rt)

for pts in path_pixel:
    for k in range(len(pts) - 1):
        canvas.draw_line(unreal.Vector2D(pts[k][0], pts[k][1]),
                         unreal.Vector2D(pts[k+1][0], pts[k+1][1]), 2.5, color)
unreal.RenderingLibrary.end_draw_canvas_to_render_target(world, ctx)

ok = proxy.landscape_import_weightmap_from_render_target(rt, LAYER_3_NAME)
assert ok, "Layer 3 导入失败！检查图层名是否正确"
print("路径绘制完成")

# ============================================================
# Step 7: 种树（用空间网格加速）
# ============================================================
CELL = TREE_MIN_SPACING
node_buf = NODE_RADIUS_WORLD + BUFFER_WORLD
path_buf = PATH_WIDTH_WORLD / 2 + BUFFER_WORLD

tm = 800
gx0, gx1 = int((world_min_x + tm) // CELL), int((world_max_x - tm) // CELL)
gy0, gy1 = int((world_min_y + tm) // CELL), int((world_max_y - tm) // CELL)

excluded = set()

def mark_exclusion(cx, cy, buf):
    r = int(buf // CELL) + 1
    for dx in range(-r, r + 1):
        for dy in range(-r, r + 1):
            gx, gy = cx + dx, cy + dy
            if gx0 <= gx <= gx1 and gy0 <= gy <= gy1:
                wx = gx * CELL + CELL / 2
                wy = gy * CELL + CELL / 2
                if (wx - cx*CELL - CELL/2)**2 + (wy - cy*CELL - CELL/2)**2 < buf**2:
                    # 简化：用网格中心近似
                    pass
                if (wx - (cx*CELL + CELL/2))**2 + (wy - (cy*CELL + CELL/2))**2 < buf**2:
                    pass

# 更精确的排除标记
for nx, ny in nodes_world:
    r = int(node_buf // CELL) + 1
    cx, cy = int(nx // CELL), int(ny // CELL)
    for dx in range(-r, r + 1):
        for dy in range(-r, r + 1):
            gx, gy = cx + dx, cy + dy
            if gx0 <= gx <= gx1 and gy0 <= gy <= gy1:
                gcx = gx * CELL + CELL / 2
                gcy = gy * CELL + CELL / 2
                if (gcx - nx)**2 + (gcy - ny)**2 < node_buf**2:
                    excluded.add((gx, gy))

for pts in path_world:
    for px, py in pts:
        r = int(path_buf // CELL) + 1
        cx, cy = int(px // CELL), int(py // CELL)
        for dx in range(-r, r + 1):
            for dy in range(-r, r + 1):
                gx, gy = cx + dx, cy + dy
                if gx0 <= gx <= gx1 and gy0 <= gy <= gy1:
                    gcx = gx * CELL + CELL / 2
                    gcy = gy * CELL + CELL / 2
                    if (gcx - px)**2 + (gcy - py)**2 < path_buf**2:
                        excluded.add((gx, gy))

available = [(gx, gy) for gx in range(gx0, gx1 + 1)
             for gy in range(gy0, gy1 + 1) if (gx, gy) not in excluded]
random.seed(RANDOM_SEED + 100)
random.shuffle(available)

trees = []
for gx, gy in available:
    if len(trees) >= TREE_DENSITY:
        break
    x = gx * CELL + random.uniform(20, CELL - 20)
    y = gy * CELL + random.uniform(20, CELL - 20)
    trees.append((x, y))

mesh = unreal.load_asset(TREE_MESH)
for x, y in trees:
    yaw = random.uniform(0, 360)
    s = random.uniform(0.8, 1.3)
    a = unreal.EditorLevelLibrary.spawn_actor_from_object(
        mesh, unreal.Vector(x, y, 0), unreal.Rotator(0, 0, yaw))
    if a:
        a.set_actor_scale3d(unreal.Vector(s, s, s))
        a.set_folder_path("Trees")

# 保存
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
print(f"\n=== 完成 ===")
print(f"据点: {len(nodes_world)} 个（仅逻辑，未绘制）")
print(f"路径: {len(selected)} 条 (Layer {LAYER_3_NAME}, 权重{LAYER_WEIGHT})")
print(f"树木: {len(trees)} 棵")
```

## 其他 AI 复刻指南

### 如果你的项目不同，需要修改：

| 你的差异 | 修改位置 |
|---------|---------|
| 没有 `Orasot_Bundle` 资源 | 将 `TREE_MESH` 换成你项目中的静态网格 |
| 地形图层名不是 `"2"`, `"3"` | 修改 `LAYER_3_NAME` 和 Step 2 中清空的图层名列表 |
| 想据点也画图层 | 取消 Step 6 中据点 RT 绘制的注释 |
| 地形分辨率不同 | `RT_SIZE` 会自动计算，无需手动改 |
| 树木太少/太多 | 调 `TREE_DENSITY` 和 `TREE_MIN_SPACING` |
| 树长到路上了 | 增大 `BUFFER_WORLD`（当前 300）或 `PATH_SEGMENTS` |
| 据点太集中 | 减小 `MARGIN`（当前 300）或增大 `MIN_NODE_DISTANCE` |

### 复现验证清单

运行脚本后检查：

- [ ] 关卡中所有旧树已被删除
- [ ] 地形 Layer 2 权重无显示（据点未画）
- [ ] 地形 Layer 3 有弯曲路径（权重半透明）
- [ ] 树木数量接近 `TREE_DENSITY` 设定值
- [ ] 树木没有长在路径上
- [ ] 树木没有长在据点范围内
- [ ] 树木随机分布、缩放自然

### 常见失败原因

| 现象 | 原因 |
|------|------|
| `landscape_import_weightmap_from_render_target` 返回 False | 图层名 `LAYER_3_NAME` 与地形材质中的参数名不匹配 |
| RT 导入后内容挤在角落 | `RT_SIZE` 计算错误，没有匹配地形权重图分辨率 |
| 树全部歪倒 | `Rotator(0, yaw, 0)` 写错，正确是 `Rotator(0, 0, yaw)` |
| 树木种在路径上 | `path_world` 数据与绘制 RT 所用数据不一致。必须一次生成 |
| 树木数量远小于预期 | `TREE_MIN_SPACING` 过大或 `BUFFER_WORLD` 过大，可用网格太少 |
| 种树极慢（超时） | 使用了 O(n²) 拒绝采样。改为网格方法 |

## 坐标系说明

```
世界坐标 (landscape bounds)
  X: [world_min_x, world_max_x]  ≈ [-12600, 12600]
  Y: [world_min_y, world_max_y]  ≈ [-12675, 12600]

像素坐标 (RenderTarget)
  X: [0, RT_SIZE)  ≈ [0, 253)
  Y: [0, RT_SIZE)  ≈ [0, 253)

转换:  world_to_px(wx, wy)  →  线性映射
```

## API 参考

| API | 用途 |
|-----|------|
| `LandscapeProxy.landscape_import_weightmap_from_render_target(rt, layer_name)` | 将 RT 权重图导入地形图层 |
| `RenderingLibrary.create_render_target2d(world, w, h, format)` | 创建 RT |
| `RenderingLibrary.clear_render_target2d(world, rt, color)` | 清除 RT |
| `RenderingLibrary.begin/end_draw_canvas_to_render_target()` | Canvas 绘制 |
| `Canvas.draw_line(pos_a, pos_b, thickness, color)` | 画线（抗锯齿） |
| `EditorLevelLibrary.spawn_actor_from_object(mesh, location, rotation)` | 放置 Actor |
| `EditorLevelLibrary.destroy_actor(actor)` | 删除 Actor |

## 执行记录

| 日期 | 种子 | 据点 | 路径 | 树木 | 耗时 |
|------|------|------|------|------|------|
| 2026-06-02 | 1780391390 | 30 | 39 | 2500 | ~49s |

---

# 附录：地形高度修改

## 核心概念

### RG16 编码原理

UE5 地形高度使用 **16-bit 精度**，数据以 R+G 双通道编码存储在纹理中：

```
16-bit 高度值 (0~65535)
  → 高8位 → R 通道 (landscape 读取为 high byte)
  → 低8位 → G 通道 (landscape 读取为 low byte)
  → landscape_import_heightmap_from_render_target(import_height_from_rg_channel=True)
```

如果只用单通道（R only），只有 256 级高度，地形会呈现**块状**阶梯。

### 关键 API

```python
# 导入高度（RG双通道模式）
landscape.landscape_import_heightmap_from_render_target(
    rt, 
    import_height_from_rg_channel=True,  # 关键！R=高8位, G=低8位
    edit_layer_index=0
)

# 导出高度验证
landscape.landscape_export_heightmap_to_render_target(
    rt,
    export_height_into_rg_channel=True,
    export_landscape_proxies=False
)
```

---

## 方案一：材质生成噪声 → 地形（推荐，GPU 实时）

参考材质：`/Game/_MyTest/Land/M_HeightNoise2.M_HeightNoise2`

### 材质连线

```
WorldPosition → Add → Noise(VoronoiALU, scale=0.0005, levels=10) 
    → Multiply(×13107=6553.5×2) 
    → PackRG8Height("Height Int -32k to 32k") 
    → Emissive Color
```

关键节点：
- **PackRG8Height** 函数：`/Landmass/Landscape/BlueprintBrushes/MF/PackRG8Height.PackRG8Height`
  - 输入：高度值（范围 -32768~32767）
  - 输出：XY（编码后的 RG 通道）
- **Offset 参数**：VectorParameter(R=200,G=100,B=100)，用于偏移 Noise 采样位置

### 执行脚本

```python
import unreal

W, H = 2048, 2048  # 必须匹配地形高度图分辨率
world = unreal.EditorLevelLibrary.get_editor_world()

# 1. 找地形
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == "Landscape":
        landscape = actor
        break
assert landscape, "没找到地形"

# 2. 加载材质
mat = unreal.load_asset(name="/Game/_MyTest/Land/M_HeightNoise2.M_HeightNoise2")
assert mat, "没找到材质"

# 3. 创建 RT
rt = unreal.RenderingLibrary.create_render_target2d(
    world_context_object=world, width=W, height=H,
    format=unreal.TextureRenderTargetFormat.RTF_RGBA16F,
    clear_color=unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
    auto_generate_mip_maps=False
)

# 4. 材质渲染到 RT
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)

# 5. 写入地形（RG 双通道模式）
result = landscape.landscape_import_heightmap_from_render_target(
    rt, import_height_from_rg_channel=True, edit_layer_index=0
)

if result:
    landscape.force_layers_full_update()
    landscape.modify()
    unreal.EditorLevelLibrary.save_current_level()
    print("完成")
```

### 调整起伏强度

修改材质中 Multiply 节点的值（当前 `6553.5 × 2 = 13107`）：
- 增大 → 起伏更剧烈
- 减小 → 起伏更平缓

### 调整噪声频率

修改 Noise 节点的参数：
- `Scale`：增大 → 噪声更舒展（特征更大）
- `Levels`：减少 → 细节更少，地形更平滑

---

## 方案二：Python 生成噪声 → TGA → 地形

### 完整流程

```
Python 生成噪声 (6-octave value noise, hash-based)
  → 每像素 16-bit 高度值
  → 编码为 TGA (BGRA32, R=高8位, G=低8位)
  → import_file_as_texture2d (瞬态纹理)
  → 材质 (TextureSample → Emissive, Unlit)
  → draw_material_to_render_target
  → landscape_import_heightmap_from_render_target(import_height_from_rg_channel=True)
```

### 执行脚本

> 注意：完整脚本分为两步，因为 noise 生成较慢（~50s），
> 且 `import_file_as_texture2d` 创建的是瞬态纹理，材质和 RT 操作必须在同一次执行中完成。

#### 第1步：生成噪声 + 写入 TGA

```python
import math, os

W = H = 2048

def hash_noise(x, y, seed=0):
    h = seed + x * 374761393 + y * 668265263
    h = (h ^ (h >> 13)) * 1274126177
    h = h ^ (h >> 16)
    return (h & 0x7fffffff) / 0x7fffffff

def smoothstep(t):
    return t * t * (3 - 2 * t)

def lerp(a, b, t):
    return a + (b - a) * t

def value_noise(x, y, cell_size, seed=0):
    fx = x / cell_size
    fy = y / cell_size
    ix = int(math.floor(fx))
    iy = int(math.floor(fy))
    frac_x = fx - ix
    frac_y = fy - iy
    sx = smoothstep(frac_x)
    sy = smoothstep(frac_y)
    v00 = hash_noise(ix, iy, seed)
    v10 = hash_noise(ix+1, iy, seed)
    v01 = hash_noise(ix, iy+1, seed)
    v11 = hash_noise(ix+1, iy+1, seed)
    return lerp(lerp(v00, v10, sx), lerp(v01, v11, sx), sy)

# === 可调参数 ===
BASE_CELL = 1024    # 基础特征大小（越大越舒展）
OCTAVES = 4         # 细节层数（越少越平滑）
AMPLITUDE = 0.35    # 起伏幅度 (0~1)，1.0=全范围(0~65535)，0.35=温和起伏
OUT_TGA = "python_noise_height.tga"  # 输出到项目根目录
# ================

heights = [0] * (W * H)
for y in range(H):
    if y % 512 == 0:
        print(f"Row {y}/{H}")
    for x in range(W):
        val = 0.0
        amp = 1.0
        freq = 1.0
        max_amp = 0.0
        for octave in range(OCTAVES):
            cell = BASE_CELL / freq
            val += amp * value_noise(x, y, cell, octave * 1337)
            max_amp += amp
            amp *= 0.5
            freq *= 2.0
        val = val / max_amp
        heights[y * W + x] = int(val * 65535)

# 写入 TGA（RG16 编码：R=高8位, G=低8位）
with open(OUT_TGA, "wb") as f:
    f.write(bytearray([0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0]))
    f.write(bytearray([W & 0xFF, (W >> 8) & 0xFF, H & 0xFF, (H >> 8) & 0xFF]))
    f.write(bytearray([32, 0x20]))  # 32-bit, top-left origin
    for val in heights:
        hi = (val >> 8) & 0xFF
        lo = val & 0xFF
        # TGA 像素顺序: B, G, R, A
        f.write(bytearray([0, lo, hi, 255]))
    f.write(bytearray([0, 0, 0, 0, 0, 0, 0, 0]))
    f.write(b"TRUEVISION-XFILE.\0")

print(f"TGA 已生成: {os.path.abspath(OUT_TGA)}")
```

#### 第2步：导入 TGA 并应用到地形

```python
import unreal, os

W = H = 2048
world = unreal.EditorLevelLibrary.get_editor_world()

# 1. 导入 TGA（瞬态纹理，必须紧接着创建材质）
tga_path = os.path.join(unreal.Paths.project_dir(), "python_noise_height.tga")
tex = unreal.RenderingLibrary.import_file_as_texture2d(world, tga_path)
tex.set_editor_property("srgb", False)
tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DISPLACEMENTMAP)
tex.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
tex.modify(True)

# 2. 创建/更新材质（必须在导入后立即执行，防止 GC）
mat_path = "/Game/_MyTest/Materials/M_NoiseHeightmap.M_NoiseHeightmap"
mat = unreal.load_asset(name=mat_path)  # 事先创建好的材质
if not mat:
    # 首次需要创建
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset("M_NoiseHeightmap", "/Game/_MyTest/Materials", 
                              unreal.Material, unreal.MaterialFactoryNew())
mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)

mel = unreal.MaterialEditingLibrary
mel.delete_all_material_expressions(mat)
tc = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -400, 0)
ts = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -200, 0)
ts.set_editor_property("texture", tex)
mel.connect_material_expressions(tc, "", ts, "")
mel.connect_material_property(ts, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
mel.recompile_material(mat)
unreal.EditorAssetLibrary.save_asset(mat_path, only_if_is_dirty=False)

# 3. 渲染到 RT
rt = unreal.RenderingLibrary.create_render_target2d(
    world_context_object=world, width=W, height=H,
    format=unreal.TextureRenderTargetFormat.RTF_RGBA16F,
    clear_color=unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
    auto_generate_mip_maps=False
)
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)

# 4. 找地形
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == "Landscape":
        landscape = actor
        break
assert landscape, "没找到地形"

# 5. 导入高度（RG 双通道模式）
result = landscape.landscape_import_heightmap_from_render_target(
    rt, import_height_from_rg_channel=True, edit_layer_index=0
)
assert result, "高度导入失败！"

landscape.force_layers_full_update()
landscape.modify()
unreal.EditorLevelLibrary.save_current_level()
print("完成！")
```

---

## 方案三：外部 16-bit PNG 高度图 → 地形

如需导入外部 16-bit PNG 高度图（如 Splat_04.PNG），核心思路同方案二：

1. 将 PNG 转换为 RG16 TGA（R=高8位, G=低8位）
2. 用 `import_file_as_texture2d` 导入
3. 材质 passthrough → RT → 地形

关键区别：不需要自己生成噪声，而是从 PNG 读取像素数据。

> 注意：直接导入 16-bit PNG 到 UE 不会保留 16-bit 精度（UE 的 PNG 导入器会转换为 8-bit）。
> 必须先用 Python 读取 16-bit PNG，编码为 RG16 TGA 再导入。

---

## 常见陷阱

### 陷阱1：纹理是瞬态的，会被 GC

```python
# ❌ 错误：分段执行
tex = import_file_as_texture2d(...)  # 瞬态纹理
# ... 其他代码（GC 可能回收 tex）
create_material(tex)  # tex 可能已失效

# ✅ 正确：导入后立即创建材质引用
tex = import_file_as_texture2d(...)
ts = MaterialExpressionTextureSample()
ts.set_editor_property("texture", tex)  # 材质持有引用，防止 GC
```

### 陷阱2：不要用 Canvas 绘制高度数据

```python
# ❌ 错误：Canvas.draw_texture 会应用 sRGB/gamma 校正，破坏高度数据
# ✅ 正确：draw_material_to_render_target 保持线性值
```

### 陷阱3：RT 分辨率必须匹配地形

地形高度图分辨率必须精确匹配，否则数据会拉伸/挤压。可通过以下方式获取地形分辨率：

```python
components = landscape.get_components_by_class(unreal.LandscapeComponent)
section_bases = [(comp.section_base_x, comp.section_base_y) for comp in components]
xs = sorted(set(b[0] for b in section_bases))
section_size = xs[1] - xs[0]
total_quads = (max(xs) - min(xs)) + section_size
RT_SIZE = total_quads + 1
```

### 陷阱4：不要用 AssetImportTask 导入纹理

```python
# ❌ 会导致 UE 编辑器崩溃！
task = unreal.AssetImportTask()
task.set_editor_property("filename", tga_path)
...
tools.import_asset_tasks([task])

# ✅ 使用 import_file_as_texture2d（瞬态但稳定）
tex = unreal.RenderingLibrary.import_file_as_texture2d(world, tga_path)
```

### 陷阱5：纹理压缩会破坏高度数据

```python
tex.set_editor_property("srgb", False)
tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DISPLACEMENTMAP)
tex.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
```

---

## API 参考（地形高度）

| API | 用途 |
|-----|------|
| `landscape_import_heightmap_from_render_target(rt, import_height_from_rg_channel, edit_layer_index)` | 从 RT 导入高度图 |
| `import_height_from_rg_channel=True` | RG 双通道 16-bit 模式（必须） |
| `import_height_from_rg_channel=False` | 单通道 8-bit 模式（不要用，会块状） |
| `landscape_export_heightmap_to_render_target(rt, export_height_into_rg_channel, export_landscape_proxies)` | 导出高度图用于验证 |
| `draw_material_to_render_target(world, rt, material)` | 材质渲染到 RT（保持线性值） |
| `import_file_as_texture2d(world, path)` | 导入图片为瞬态纹理 |
| `PackRG8Height` 材质函数 | 编码高度为 RG16（位于 Landmass 插件） |
| `MaterialFunctionCall.material_function` | 设置函数调用目标 |

---

## 参数速查（方案二）

| `AMPLITUDE` | Z 范围（约） | 地形效果 |
|:-----------:|:------------:|----------|
| 1.0 | -25000 ~ 25000 | 剧烈起伏，有极端高峰和低谷 |
| 0.7 | -17000 ~ 17000 | 明显起伏，适合山地 |
| 0.5 | -11000 ~ 11000 | 中等起伏，适合丘陵 |
| **0.35** | **-6000 ~ 5300** | **温和起伏，适合漫步型地形** |
| 0.2 | -3000 ~ 3000 | 微缓坡地，接近平面 |

| `BASE_CELL` | 特征大小 | 地形效果 |
|:-----------:|:--------:|----------|
| 256 | 小 | 细碎频繁起伏，类似小丘 |
| 512 | 中 | 正常起伏 |
| **1024** | **大** | **舒展开阔，类似大丘陵** |
| 2048 | 极大 | 整个地形只有几个起伏 |

| `OCTAVES` | 细节层次 | 地形效果 |
|:---------:|:--------:|----------|
| 6 | 丰富 | 主特征上有大量小细节 |
| **4** | **适中** | **主特征清晰，细节恰到好处** |
| 2 | 少 | 非常平滑，几乎无细节 |

**当前运行推荐值：** `BASE_CELL=1024, OCTAVES=4, AMPLITUDE=0.35`

---

## 复现验证清单

运行脚本后检查：

- [ ] 地形有明显起伏（非平面）
- [ ] 起伏平滑无块状（非阶梯状）
- [ ] RT 验证时 R 和 G 通道均有非零变化值
- [ ] `landscape_import_heightmap_from_render_target` 返回 True
- [ ] 地形 Z 范围与世界范围比例协调（不极端不过平）
