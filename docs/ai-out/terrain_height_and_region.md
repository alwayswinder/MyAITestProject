# 程序化地形建设 — 起伏地形 + 不规则活动区域

## 环境要求

- UE 5.x 项目，已启用 Python 脚本插件
- 地形 Landscape 使用材质 `MI_LandscapeMasterMaterial`（包含图层参数 `"Grass"` 和 `"Stone"`）
- LayerInfo 资产存在：
  - `/Game/Orasot_Bundle/Maps/M_Global_Overview_sharedassets/Grass_LayerInfo.Grass_LayerInfo`（layer_name=`"Grass"`）
  - `/Game/Orasot_Bundle/Maps/5_Bioms_ShaderAssets/Stone_LayerInfo.Stone_LayerInfo`（layer_name=`"Stone"`）

> 如果项目不同，只需修改 `LANDSCAPE_MATERIAL` 和 `GRASS_LAYER`/`STONE_LAYER` 参数路径，其余逻辑通用。

## 方案概览

```
Step 1: 地形高度起伏
  Python 生成多层 hash-based value noise
  → 编码为 RG16 TGA (R=高8位, G=低8位)
  → import_file_as_texture2d 导入为瞬态纹理
  → 材质 TextureSample → Emissive (Unlit)
  → draw_material_to_render_target
  → landscape_import_heightmap_from_render_target(import_height_from_rg_channel=True)

Step 2: 不规则活动区域 + 自然过渡
  2D FBM 噪声场（取代角度正弦波）生成有机海岸线形状
  → signed distance + smoothstep 连续渐变（非二值 0/255）
  → 噪声调制过渡带宽度（有的地方陡峭，有的地方平缓）
  → 写入 8-bit 灰度 TGA (0=纯石头, 128=半过渡, 255=纯草地)
  → import_file_as_texture2d 导入
  → 材质 TextureSample → Emissive
  → 绘制到 R8 RT
  → landscape_import_weightmap_from_render_target("Grass") 为正相
  → OneMinus 反相材质 → landscape_import_weightmap_from_render_target("Stone")
  → Grass + Stone 权重始终为 1.0

Step 3: 岛屿内部 5 区域分割
  Lloyd 松弛（3 轮）平衡面积 + 噪声调制 Voronoi 产生不规则边界
  → 每像素计算到 5 个站点的噪声调制距离，取最近（winner）和次近（runner-up）
  → smoothstep 软过渡（120px）消除硬边界
  → 写入 5 个 8-bit 灰度 TGA + Stone TGA
  → import_file_as_texture2d × 6 → R8 RT → landscape_import_weightmap_from_render_target × 6
  → 区域层：Grass / Grass Biom 4 / Grass Biom 3 / Grass Biom 2 / DesertSand
  → Stone = 1 - island_mask（岛屿外石头覆盖）
```

---

## 核心概念

### RG16 高度编码

UE5 地形高度使用 **16-bit 精度**，数据以 R+G 双通道编码：

```
16-bit 高度值 (0~65535)
  → 高8位 → R 通道（landscape 读取为 high byte）
  → 低8位 → G 通道（landscape 读取为 low byte）
  → landscape_import_heightmap_from_render_target(import_height_from_rg_channel=True)
```

只用单通道（R only）只有 256 级高度，地形会呈现块状阶梯。

### 权重图导入

```
landscape.landscape_import_weightmap_from_render_target(rt, layer_name)
```

- RT 格式：`RTF_R8`（单通道 8-bit）
- 像素值 0 = 权重 0，像素值 255 = 权重 1
- `layer_name` 须匹配地形材质中 LayerBlend 节点的参数名

---

## 执行参数

| 参数 | 默认值 | 含义 |
|------|--------|------|
| `W` / `H` | 由地形分辨率决定 | 地形高度图/权重图分辨率（本项目为 2160） |
| `BASE_CELL` | 1024 | 噪声基础特征大小（越大起伏越舒展） |
| `OCTAVES` | 4 | 噪声细节层数（越少越平滑） |
| `AMPLITUDE` | 0.35 | 起伏幅度（1.0=全范围，0.35=温和起伏） |
| `BASE_RADIUS` | 800 | 岛屿基础半径（控制活动区域大小） |
| `EDGE_MARGIN` | 200 | 岛屿距地形边缘最小缓冲（像素） |
| `BOUNDARY_AMP` | 250 | 2D 噪声边界位移幅度（越大形状越不规则） |
| `BOUNDARY_OCTAVES` | 5 | 边界噪声 FBM 层数 |
| `BOUNDARY_CELL` | 300 | 边界噪声基础特征大小 |
| `TRANSITION_BASE` | 160 | 基础过渡带宽（像素，越宽越柔和） |
| `TRANSITION_NOISE_AMP` | 60 | 过渡带宽度噪声调制（有的地方陡峭有的平缓） |
| `DETAIL_CELL` | 50 | 海岸线细节噪声特征大小 |
| `DETAIL_AMP` | 20 | 海岸线细节噪声幅度 |
| `REGION_SEED` | 12345 | 区域种子点随机种子（固定可复现） |
| `NOISE_AMP`（Step 3） | 80 | 区域边界噪声幅度（越大越不规则） |
| `NOISE_CELL`（Step 3） | 50 | 区域边界噪声特征尺寸 |
| `NOISE_OCTAVES`（Step 3） | 3 | 区域边界噪声 FBM 层数 |
| `REGION_TRANSITION` | 120 | 区域间过渡带宽度（像素，越宽越柔和） |

---

## Step 1：地形起伏

### 完整脚本

```python
import math, os, unreal

W = H = 2160  # 须匹配地形实际分辨率（本项目 17×17 组件，每组件 127 quads → 2160）

# ============================================================
# 噪声函数
# ============================================================
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
    v10 = hash_noise(ix + 1, iy, seed)
    v01 = hash_noise(ix, iy + 1, seed)
    v11 = hash_noise(ix + 1, iy + 1, seed)
    return lerp(lerp(v00, v10, sx), lerp(v01, v11, sx), sy)

# ============================================================
# 可调参数
# ============================================================
BASE_CELL = 1024   # 基础特征大小（越大越舒展）
OCTAVES = 4        # 细节层数（越少越平滑）
AMPLITUDE = 0.35   # 起伏幅度 (0~1)，0.35=温和起伏
OUT_TGA = "python_noise_height.tga"
# ============================================================

print("=== Step 1: Generating noise heightmap ===")
heights = [0] * (W * H)
for y in range(H):
    if y % 512 == 0:
        print(f"  Row {y}/{H}")
    for x in range(W):
        val = 0.0
        amp_n = 1.0
        freq = 1.0
        max_amp = 0.0
        for octave in range(OCTAVES):
            cell = BASE_CELL / freq
            val += amp_n * value_noise(x, y, cell, octave * 1337)
            max_amp += amp_n
            amp_n *= 0.5
            freq *= 2.0
        val = val / max_amp
        # 以 32768 为中心压缩到 AMPLITUDE 范围
        h = int(32768 + (val - 0.5) * 65535 * AMPLITUDE)
        h = max(0, min(65535, h))
        heights[y * W + x] = h

# 写入 RG16 TGA（R=高8位, G=低8位）
with open(OUT_TGA, "wb") as f:
    f.write(bytearray([0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0]))
    f.write(bytearray([W & 0xFF, (W >> 8) & 0xFF, H & 0xFF, (H >> 8) & 0xFF]))
    f.write(bytearray([32, 0x20]))
    for val in heights:
        hi = (val >> 8) & 0xFF
        lo = val & 0xFF
        f.write(bytearray([0, lo, hi, 255]))
    f.write(bytearray([0, 0, 0, 0, 0, 0, 0, 0]))
    f.write(b"TRUEVISION-XFILE.\0")

print(f"TGA written: {OUT_TGA}")

# ============================================================
# 导入地形（必须在同一执行上下文，防止瞬态纹理被 GC）
# ============================================================
world = unreal.EditorLevelLibrary.get_editor_world()

# 加载 TGA 为纹理
tex = unreal.RenderingLibrary.import_file_as_texture2d(world, OUT_TGA)
tex.set_editor_property("srgb", False)
tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DISPLACEMENTMAP)
tex.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
tex.modify(True)

# 创建材质（引用纹理，防止 GC）
mat_path = "/Game/_MyTest/Materials/M_NoiseHeightmap.M_NoiseHeightmap"
if unreal.EditorAssetLibrary.does_asset_exist(mat_path):
    mat = unreal.load_asset(name=mat_path)
else:
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

# 渲染到 RT
rt = unreal.RenderingLibrary.create_render_target2d(
    world, W, H,
    unreal.TextureRenderTargetFormat.RTF_RGBA16F,
    clear_color=unreal.LinearColor(0, 0, 0, 1),
    auto_generate_mip_maps=False
)
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)

# 找地形
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == "Landscape":
        landscape = actor
        break
assert landscape, "No Landscape found!"

# 导入高度图（RG 双通道模式）
result = landscape.landscape_import_heightmap_from_render_target(
    rt, import_height_from_rg_channel=True, edit_layer_index=0
)
assert result, "Heightmap import failed!"

landscape.force_layers_full_update()
landscape.modify()
unreal.EditorLevelLibrary.save_current_level()

# 清理
os.remove(OUT_TGA)
print("=== Step 1 complete! ===")
```

### 调整指南

| 参数 | 增大效果 | 减小效果 |
|------|---------|---------|
| `BASE_CELL` | 起伏更舒展 | 起伏更细碎 |
| `OCTAVES` | 细节更丰富 | 地形更平滑 |
| `AMPLITUDE` | 起伏更剧烈 | 起伏更温和 |

---

## Step 2：不规则活动区域（岛屿）+ 自然过渡

### 设计思路

第一版使用角度正弦波扰动 + 二值 mask（0/255），产生硬边界和几何感形状。
改进版采用以下策略实现有机形状和自然过渡：

1. **2D FBM 噪声取代角度正弦波** —— 真正的有机海岸线形状，无几何重复感
2. **smoothstep 渐变替代二值阈值** —— 连续灰度值（0~255）产生平滑权重过渡
3. **噪声调制过渡带宽度** —— 有的地方过渡陡峭，有的地方平缓，更自然
4. **性能优化** —— 边界噪声在 1/4 分辨率预计算后双线性上采样，运算量降为 1/16

```
权重值编码：
  255 = 纯草地（Grass=1.0, Stone=0.0）
  128 = 半过渡（Grass=0.5, Stone=0.5）
    0 = 纯石头（Grass=0.0, Stone=1.0）

反相材质（OneMinus）自动保证 Grass + Stone ≡ 1.0
```

### 完整脚本

```python
import math, random, os, unreal

W = H = 2160  # 权重图分辨率（需匹配地形实际分辨率）
cx, cy = W // 2, H // 2
BASE_RADIUS = 800
EDGE_MARGIN = 200

random.seed()
SEED = random.randint(0, 99999)

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
    fx, fy = x / cell_size, y / cell_size
    ix, iy = int(math.floor(fx)), int(math.floor(fy))
    sx = smoothstep(fx - ix)
    sy = smoothstep(fy - iy)
    return lerp(
        lerp(hash_noise(ix, iy, seed), hash_noise(ix+1, iy, seed), sx),
        lerp(hash_noise(ix, iy+1, seed), hash_noise(ix+1, iy+1, seed), sx), sy)

def fbm_noise(x, y, octaves, base_cell, seed=0, lacunarity=2.0, gain=0.5):
    """Fractal Brownian Motion — 多层有机噪声"""
    val = 0.0
    amp = 1.0
    freq = 1.0
    max_val = 0.0
    for i in range(octaves):
        val += amp * value_noise(x, y, base_cell / freq, seed + i * 1337)
        max_val += amp
        amp *= gain
        freq *= lacunarity
    return val / max_val

def bilinear_sample(grid, x, y):
    """双线性采样 2D 网格"""
    w, h = len(grid[0]), len(grid)
    x = max(0, min(w - 1.0001, x))
    y = max(0, min(h - 1.0001, y))
    ix, iy = int(x), int(y)
    fx, fy = x - ix, y - iy
    ix1, iy1 = min(ix + 1, w - 1), min(iy + 1, h - 1)
    return lerp(
        lerp(grid[iy][ix], grid[iy][ix1], fx),
        lerp(grid[iy1][ix], grid[iy1][ix1], fx), fy)

# ============================================================
# 可调参数
# ============================================================
BOUNDARY_OCTAVES = 5      # 边界噪声细节层数
BOUNDARY_CELL = 300       # 边界噪声基础特征大小（越小越曲折）
BOUNDARY_AMP = 250        # 边界位移幅度（越大岛屿形状越不规则）
TRANSITION_BASE = 160     # 基础过渡带宽（像素）
TRANSITION_NOISE_AMP = 60 # 过渡带宽度噪声调制幅度
DETAIL_CELL = 50          # 海岸线细节噪声特征大小
DETAIL_AMP = 20           # 海岸线细节噪声幅度
# ============================================================

print("=== Step 2: Generating organic island mask ===")

# Phase 1: 低分辨率预计算边界位移场（1/4 分辨率，大幅减少运算量）
LOWRES = W // 4
print(f"Pre-computing boundary noise at {LOWRES}x{LOWRES}...")
displacement_low = [[0.0] * LOWRES for _ in range(LOWRES)]
for y in range(LOWRES):
    for x in range(LOWRES):
        fx, fy = x * 4, y * 4
        displacement_low[y][x] = (
            fbm_noise(fx, fy, BOUNDARY_OCTAVES, BOUNDARY_CELL, SEED) - 0.5
        ) * 2 * BOUNDARY_AMP

# Phase 2: 全分辨率生成 mask（双线性上采样 + 细节噪声 + smoothstep 过渡）
print("Generating full-res mask...")
mask = bytearray(W * H)
for y in range(H):
    if y % 540 == 0:
        print(f"  Row {y}/{H}")
    for x in range(W):
        dx, dy = x - cx, y - cy
        dist = math.hypot(dx, dy)

        # 2D 有机边界位移（从低分辨率场双线性采样）
        displacement = bilinear_sample(displacement_low, x / 4.0, y / 4.0)

        # 小尺度海岸线细节
        detail = (value_noise(x, y, DETAIL_CELL, SEED + 9999) - 0.5) * 2 * DETAIL_AMP

        effective_radius = BASE_RADIUS + displacement + detail

        # 边缘惩罚：靠近地形边缘强制为石头
        edge_dist = min(x, y, W - 1 - x, H - 1 - y)
        penalty = (1 - edge_dist / EDGE_MARGIN) * 400 if edge_dist < EDGE_MARGIN else 0

        # 噪声调制过渡带宽度（有的地方陡峭，有的地方平缓）
        tw_noise = value_noise(x, y, 200, SEED + 55555)
        transition_w = TRANSITION_BASE + (tw_noise - 0.5) * 2 * TRANSITION_NOISE_AMP
        transition_w = max(50, transition_w)

        # 有符号距离 → smoothstep 连续渐变
        signed_dist = effective_radius - (dist + penalty)
        half_w = transition_w / 2.0
        t = (signed_dist + half_w) / transition_w
        t = max(0.0, min(1.0, t))
        mask[y * W + x] = int(smoothstep(t) * 255)

# 统计
grass_weight = sum(b for b in mask) / 255.0
print(f"Grass avg weight: {grass_weight / (W*H) * 100:.1f}%")

# 写入 8-bit 灰度 TGA
TGA_PATH = "province_mask.tga"
with open(TGA_PATH, "wb") as f:
    f.write(bytearray([0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0]))  # type=3 = grayscale
    f.write(bytearray([W & 0xFF, (W >> 8) & 0xFF, H & 0xFF, (H >> 8) & 0xFF]))
    f.write(bytearray([8, 0x20]))  # 8-bit, top-left origin
    f.write(mask)
    f.write(bytearray([0, 0, 0, 0, 0, 0, 0, 0]))
    f.write(b"TRUEVISION-XFILE.\0")

print(f"TGA written: {TGA_PATH}")

# ============================================================
# 导入到 UE5 地形（须在同一执行上下文，防止瞬态纹理被 GC）
# ============================================================
world = unreal.EditorLevelLibrary.get_editor_world()

# 加载 mask 纹理
tex = unreal.RenderingLibrary.import_file_as_texture2d(world, TGA_PATH)
tex.set_editor_property("srgb", False)
tex.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
tex.modify(True)

# 创建正相材质（mask → Grass 权重）
mat = unreal.load_asset(name="/Game/_MyTest/Materials/M_ProvinceMask.M_ProvinceMask")
if not mat:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset("M_ProvinceMask", "/Game/_MyTest/Materials",
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

# 渲染 Grass layer
rt = unreal.RenderingLibrary.create_render_target2d(
    world, W, H,
    unreal.TextureRenderTargetFormat.RTF_R8,
    clear_color=unreal.LinearColor(0, 0, 0, 1),
    auto_generate_mip_maps=False
)
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)

# 创建反相材质（1 - mask → Stone 权重）
mat_inv = unreal.load_asset(name="/Game/_MyTest/Materials/M_ProvinceMask_Inverted.M_ProvinceMask_Inverted")
if not mat_inv:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat_inv = tools.create_asset("M_ProvinceMask_Inverted", "/Game/_MyTest/Materials",
                                  unreal.Material, unreal.MaterialFactoryNew())
mat_inv.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
mel.delete_all_material_expressions(mat_inv)
tc2 = mel.create_material_expression(mat_inv, unreal.MaterialExpressionTextureCoordinate, -600, 0)
ts2 = mel.create_material_expression(mat_inv, unreal.MaterialExpressionTextureSample, -400, 0)
ts2.set_editor_property("texture", tex)
om = mel.create_material_expression(mat_inv, unreal.MaterialExpressionOneMinus, -200, 0)
mel.connect_material_expressions(tc2, "", ts2, "")
mel.connect_material_expressions(ts2, "", om, "")
mel.connect_material_property(om, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
mel.recompile_material(mat_inv)

# 渲染 Stone layer
rt2 = unreal.RenderingLibrary.create_render_target2d(
    world, W, H,
    unreal.TextureRenderTargetFormat.RTF_R8,
    clear_color=unreal.LinearColor(0, 0, 0, 1),
    auto_generate_mip_maps=False
)
unreal.RenderingLibrary.draw_material_to_render_target(world, rt2, mat_inv)

# 找地形
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == "Landscape":
        landscape = actor
        break
assert landscape, "No Landscape found!"

# 导入权重图
landscape.landscape_import_weightmap_from_render_target(rt, "Grass")
landscape.landscape_import_weightmap_from_render_target(rt2, "Stone")

landscape.force_layers_full_update()
landscape.modify()
unreal.EditorLevelLibrary.save_current_level()

# 清理
os.remove(TGA_PATH)
print("=== Step 2 complete! ===")
```

### 调整指南

| 参数 | 增大效果 | 减小效果 |
|------|---------|---------|
| `BOUNDARY_AMP` | 岛屿形状更不规则、曲折 | 更接近圆形 |
| `BOUNDARY_CELL` | 边界大尺度弯曲更舒展 | 更细碎 |
| `BOUNDARY_OCTAVES` | 边界细节层次更丰富 | 更平滑 |
| `TRANSITION_BASE` | 草地→石头过渡更宽、更柔和 | 过渡更窄、更锐利 |
| `TRANSITION_NOISE_AMP` | 过渡带宽窄变化更明显 | 过渡带宽度更均匀 |
| `DETAIL_AMP` | 海岸线小尺度锯齿更强 | 边缘更平滑 |
| `DETAIL_CELL` | 海岸线细节特征更大 | 细节更细密 |

---

## 权重图分辨率计算

如果项目中的地形分辨率不同，需先计算权重图分辨率：

```python
landscape = next(a for a in unreal.EditorLevelLibrary.get_all_level_actors() 
                 if a.get_class().get_name() == "Landscape")
components = landscape.get_components_by_class(unreal.LandscapeComponent)
section_bases = [
    (comp.get_editor_property("section_base_x"),
     comp.get_editor_property("section_base_y"))
    for comp in components
]
xs = sorted(set(b[0] for b in section_bases))
section_size = xs[1] - xs[0]
total_quads = (max(xs) - min(xs)) + section_size
RT_SIZE = total_quads + 1
print(f"Weightmap resolution: {RT_SIZE}x{RT_SIZE}")
```

将 Step 2 中的 `W = H = 2160` 替换为实际计算出的 `RT_SIZE`。

---

## 常见陷阱

### 陷阱1：瞬态纹理被垃圾回收

```python
# ❌ 错误：分段执行
tex = import_file_as_texture2d(...)
# ... 其他代码（GC 可能回收 tex）
create_material_with(tex)  # tex 已失效

# ✅ 正确：导入后立即创建材质引用
tex = import_file_as_texture2d(...)
ts.set_editor_property("texture", tex)  # 材质持有引用，防止 GC
```

### 陷阱2：不要用 Canvas 绘制高度数据

```python
# ❌ 错误：Canvas.draw_texture 会应用 sRGB/gamma 校正
# ✅ 正确：draw_material_to_render_target 保持线性值
```

### 陷阱3：高度图 RT 分辨率须匹配地形

地形高度图分辨率须与噪声输出分辨率一致。本项目为 2160×2160（17×17 组件，每组件 127 quads）。

### 陷阱4：不要用 AssetImportTask 导入纹理

```python
# ❌ 会导致 UE 编辑器崩溃！
task = unreal.AssetImportTask()
tools.import_asset_tasks([task])

# ✅ 使用 import_file_as_texture2d（瞬态但稳定）
tex = unreal.RenderingLibrary.import_file_as_texture2d(world, path)
```

### 陷阱5：纹理压缩会破坏高度数据

```python
tex.set_editor_property("srgb", False)
tex.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_DISPLACEMENTMAP)
tex.set_editor_property("filter", unreal.TextureFilter.TF_NEAREST)
```

---

## API 参考

| API | 用途 |
|-----|------|
| `landscape_import_heightmap_from_render_target(rt, import_height_from_rg_channel, edit_layer_index)` | 从 RT 导入高度图 |
| `import_height_from_rg_channel=True` | RG 双通道 16-bit 模式（必须） |
| `landscape_import_weightmap_from_render_target(rt, layer_name)` | 导入权重图 |
| `draw_material_to_render_target(world, rt, material)` | 材质渲染到 RT（保持线性值） |
| `import_file_as_texture2d(world, path)` | 导入图片为瞬态纹理 |
| `MaterialExpressionOneMinus` | 1-x 节点，用于权重反相 |
| `TextureRenderTargetFormat.RTF_R8` | 单通道 8-bit RT（权重图用） |
| `TextureRenderTargetFormat.RTF_RGBA16F` | 四通道 16-bit float RT（高度图用） |

---

## 复现验证清单

运行脚本后检查：

- [ ] **Step 1**：地形有明显起伏，平滑无块状
- [ ] **Step 1**：`landscape_import_heightmap_from_render_target` 返回 True
- [ ] **Step 2**：草地（Grass）在内部，石头（Stone）在外环绕
- [ ] **Step 2**：岛屿边界为连续渐变过渡（非硬边界），过渡带宽度自然变化
- [ ] **Step 2**：草地不接触地形边缘（有纯石头缓冲带）
- [ ] **Step 2**：两个 `landscape_import_weightmap_from_render_target` 均返回 True
- [ ] 草地平均权重约 44%（受随机种子轻微波动）
- [ ] **Step 2**：Grass + Stone 权重在任意像素均为 1.0（OneMinus 反相保证）
- [ ] **Step 3**：5 个区域层 + Stone 层 `landscape_import_weightmap_from_render_target` 全部返回 True
- [ ] **Step 3**：区域边界有机弯曲无直线，过渡带与外圈海岸线同样柔和
- [ ] **Step 3**：各区域面积比 < 2x（当前 1.7x），岛屿外全部为 Stone
- [ ] **Step 3**：岛屿内部每像素至少一个区域权重 > 0

## Step 3：岛屿内部 5 区域分割

### 设计思路

将 Step 2 生成的岛屿进一步分割为 5 个不规则区域，类似国家内部的省份划分。采用 **Lloyd 松弛 + 噪声调制 Voronoi 图** 算法。

**为什么不用其他方案：**
- 纯 Voronoi：直线边界几何感太强，面积差异可达 5x+
- 分水岭算法：边界沿高度走向，无法控制区域数
- 切割线：产生楔形/矩形区域，不自然
- BSP 分割：过于规整

**Lloyd + 噪声 Voronoi 的优势：**
1. **Lloyd 迭代**：每轮将种子点移到区域质心，3 轮即可将面积差异控制在 1.7x 以内
2. **FBM 噪声扭曲距离**：每个种子点有独立的噪声偏移量，5 条边界各有独特的不规则弯曲
3. **smoothstep 软过渡**：区域间 60~120px 渐变，消除硬边锯齿感

**权重编码：**
```
每像素 → 计算到 5 个站点的噪声调制距离 → 取最近（winner）和次近（runner-up）
  winner 区域权重 = smoothstep(runner_up_dist - winner_dist) × island_mask
  runner-up 区域权重 = (1 - winner_weight) × island_mask（仅在过渡带内非零）
  其余区域权重 = 0

Stone = 1 - island_mask（保持 Step 2 的岛屿外石头覆盖）
```

### 完整脚本

```python
import math, random, os

W = H = 2160
cx, cy = W // 2, H // 2
LOWRES = W // 4  # 540，噪声场预计算分辨率

# ============================================================
# 噪声函数（复用 Step 2）
# ============================================================
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
    fx, fy = x / cell_size, y / cell_size
    ix, iy = int(math.floor(fx)), int(math.floor(fy))
    sx = smoothstep(fx - ix)
    sy = smoothstep(fy - iy)
    return lerp(
        lerp(hash_noise(ix, iy, seed), hash_noise(ix+1, iy, seed), sx),
        lerp(hash_noise(ix, iy+1, seed), hash_noise(ix+1, iy+1, seed), sx), sy)

def fbm_noise(x, y, octaves, base_cell, seed=0):
    val = 0.0; amp = 1.0; freq = 1.0; max_val = 0.0
    for i in range(octaves):
        val += amp * value_noise(x, y, base_cell / freq, seed + i * 1337)
        max_val += amp; amp *= 0.5; freq *= 2.0
    return val / max_val

def bilinear_sample(grid, x, y):
    w, h = len(grid[0]), len(grid)
    x = max(0, min(w - 1.0001, x)); y = max(0, min(h - 1.0001, y))
    ix, iy = int(x), int(y); fx, fy = x - ix, y - iy
    ix1, iy1 = min(ix + 1, w - 1), min(iy + 1, h - 1)
    return lerp(lerp(grid[iy][ix], grid[iy][ix1], fx), lerp(grid[iy1][ix], grid[iy1][ix1], fx), fy)

def upsample(low_grid, scale):
    """低分辨率网格双线性上采样到全分辨率"""
    lw, lh = len(low_grid[0]), len(low_grid)
    result = [0.0] * (lw * scale * lh * scale)
    for y in range(lh * scale):
        for x in range(lw * scale):
            result[y * lw * scale + x] = bilinear_sample(low_grid, x / scale, y / scale)
    return result

def write_tga_grayscale(path, data, w, h):
    with open(path, "wb") as f:
        f.write(bytearray([0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0]))
        f.write(bytearray([w & 0xFF, (w >> 8) & 0xFF, h & 0xFF, (h >> 8) & 0xFF]))
        f.write(bytearray([8, 0x20]))
        f.write(data)
        f.write(bytearray([0, 0, 0, 0, 0, 0, 0, 0]))
        f.write(b"TRUEVISION-XFILE.\0")

# ============================================================
# 可调参数
# ============================================================
# 岛屿 mask 参数（与 Step 2 一致）
ISLAND_SEED = 42000       # 固定种子保证可复现
BASE_RADIUS = 800
EDGE_MARGIN = 200
BOUNDARY_OCTAVES = 5
BOUNDARY_CELL = 300
BOUNDARY_AMP = 250
TRANSITION_BASE = 160
TRANSITION_NOISE_AMP = 60
DETAIL_CELL = 50
DETAIL_AMP = 20

# 区域分割参数
NUM_REGIONS = 5
REGION_SEED = 12345        # 区域种子点随机种子
NOISE_AMP = 80             # 区域边界噪声幅度（越大边界越不规则）
NOISE_CELL = 50            # 边界噪声特征尺寸
NOISE_OCTAVES = 3          # 边界噪声 FBM 层数
REGION_TRANSITION = 120    # 区域间过渡带宽度（像素，越大越柔和）
# ============================================================

print(f"REGION_TRANSITION = {REGION_TRANSITION}")

# ============================================================
# Phase 0: 重建岛屿 mask（低分辨率 + 上采样，速度快）
# ============================================================
print("=== Phase 0: Island mask (lowres + upsample) ===")
# 边界位移场：540×540
disp_low = [[0.0] * LOWRES for _ in range(LOWRES)]
for y in range(LOWRES):
    for x in range(LOWRES):
        fx, fy = x * 4, y * 4
        disp_low[y][x] = (
            fbm_noise(fx, fy, BOUNDARY_OCTAVES, BOUNDARY_CELL, ISLAND_SEED) - 0.5
        ) * 2 * BOUNDARY_AMP

# 岛屿 mask：540×540
island_low = [0.0] * (LOWRES * LOWRES)
for y in range(LOWRES):
    for x in range(LOWRES):
        fx, fy = x * 4, y * 4
        dx, dy = fx - cx, fy - cy
        dist = math.hypot(dx, dy)
        displacement = disp_low[y][x]
        detail = (value_noise(fx, fy, DETAIL_CELL, ISLAND_SEED + 9999) - 0.5) * 2 * DETAIL_AMP
        effective_radius = BASE_RADIUS + displacement + detail
        edge_dist = min(fx, fy, W - 1 - fx, H - 1 - fy)
        penalty = (1 - edge_dist / EDGE_MARGIN) * 400 if edge_dist < EDGE_MARGIN else 0
        tw_noise = value_noise(fx, fy, 200, ISLAND_SEED + 55555)
        transition_w = max(50, TRANSITION_BASE + (tw_noise - 0.5) * 2 * TRANSITION_NOISE_AMP)
        signed_dist = effective_radius - (dist + penalty)
        t = max(0.0, min(1.0, (signed_dist + transition_w / 2.0) / transition_w))
        island_low[y * LOWRES + x] = smoothstep(t)

# 上采样到 2160×2160
island_low_2d = [[island_low[y * LOWRES + x] for x in range(LOWRES)] for y in range(LOWRES)]
island_mask = upsample(island_low_2d, 4)
print(f"Island avg weight: {sum(island_mask) / (W*H) * 100:.1f}%")

# ============================================================
# Phase 1: 区域噪声场预计算（540×540）
# ============================================================
print("=== Phase 1: Region noise field ===")
noise_field = [[0.0] * LOWRES for _ in range(LOWRES)]
for y in range(LOWRES):
    for x in range(LOWRES):
        noise_field[y][x] = fbm_noise(x * 4, y * 4, NOISE_OCTAVES, NOISE_CELL, REGION_SEED)

# ============================================================
# Phase 2: 采样初始种子点（岛屿内部，间距 > 200px）
# ============================================================
print("=== Phase 2: Sampling seed sites ===")
import random as rnd
rnd.seed(REGION_SEED)
sites = []
step = 16
for _ in range(NUM_REGIONS):
    candidates = []
    for y in range(EDGE_MARGIN, H - EDGE_MARGIN, step):
        for x in range(EDGE_MARGIN, W - EDGE_MARGIN, step):
            if island_mask[y * W + x] > 0.7:
                if not sites:
                    candidates.append((x, y))
                else:
                    min_d = min(math.hypot(x - s[0], y - s[1]) for s in sites)
                    if min_d > 200:
                        candidates.append((x, y))
    sites.append(rnd.choice(candidates) if candidates else (cx + rnd.randint(-200, 200), cy + rnd.randint(-200, 200)))
    print(f"  Site {len(sites)-1}: {sites[-1]}")

# ============================================================
# Phase 3: Lloyd 松弛（3 轮，stride=4 加速）
# ============================================================
print("=== Phase 3: Lloyd relaxation ===")
for iteration in range(3):
    print(f"  Iteration {iteration+1}/3")
    sums_x = [0.0] * NUM_REGIONS
    sums_y = [0.0] * NUM_REGIONS
    sums_w = [0.0] * NUM_REGIONS
    
    for y in range(0, H, 4):
        for x in range(0, W, 4):
            w = island_mask[y * W + x]
            if w < 0.3:
                continue
            best_dist = float('inf')
            best_site = -1
            for i in range(NUM_REGIONS):
                dx, dy = x - sites[i][0], y - sites[i][1]
                euclidean = math.sqrt(dx*dx + dy*dy)
                nx = (x + i * 713 + REGION_SEED) % (LOWRES * 4)
                ny = (y + i * 371 + REGION_SEED) % (LOWRES * 4)
                noise = bilinear_sample(noise_field, nx / 4.0, ny / 4.0)
                modulated = euclidean + (noise - 0.5) * 2.0 * NOISE_AMP
                if modulated < best_dist:
                    best_dist = modulated
                    best_site = i
            sums_x[best_site] += x * w
            sums_y[best_site] += y * w
            sums_w[best_site] += w
    
    new_sites = []
    total_w = sum(sums_w)
    for i in range(NUM_REGIONS):
        if sums_w[i] > 0:
            nsx = max(EDGE_MARGIN, min(W - 1 - EDGE_MARGIN, int(sums_x[i] / sums_w[i])))
            nsy = max(EDGE_MARGIN, min(H - 1 - EDGE_MARGIN, int(sums_y[i] / sums_w[i])))
            new_sites.append((nsx, nsy))
            print(f"    Site {i}: ({nsx}, {nsy}) area ~{sums_w[i]/total_w*100:.1f}%")
        else:
            new_sites.append(sites[i])
    sites = new_sites

print(f"Final sites: {sites}")

# ============================================================
# Phase 4: 单次遍历生成全部 5 区域 + Stone 权重图
# ============================================================
print("=== Phase 4: Generating weightmaps (single pass) ===")
HALF_W = REGION_TRANSITION / 2.0
region_data = [bytearray(W * H) for _ in range(NUM_REGIONS)]
stone_data = bytearray(W * H)

for y in range(H):
    if y % 540 == 0:
        print(f"  Row {y}/{H}")
    for x in range(W):
        idx = y * W + x
        island_w = island_mask[idx]
        stone_data[idx] = int((1.0 - island_w) * 255)
        
        if island_w < 0.001:
            for r in range(NUM_REGIONS):
                region_data[r][idx] = 0
            continue
        
        # 计算到 5 个站点的噪声调制距离
        dists = []
        for i in range(NUM_REGIONS):
            dx, dy = x - sites[i][0], y - sites[i][1]
            euclidean = math.sqrt(dx*dx + dy*dy)
            nx = (x + i * 713 + REGION_SEED) % (LOWRES * 4)
            ny = (y + i * 371 + REGION_SEED) % (LOWRES * 4)
            noise = bilinear_sample(noise_field, nx / 4.0, ny / 4.0)
            dists.append(euclidean + (noise - 0.5) * 2.0 * NOISE_AMP)
        
        # 最近（winner）和次近（runner-up）
        sorted_idx = sorted(range(NUM_REGIONS), key=lambda i: dists[i])
        winner, runner_up = sorted_idx[0], sorted_idx[1]
        
        # smoothstep 软过渡
        signed_dist = dists[runner_up] - dists[winner]
        t = max(0.0, min(1.0, (signed_dist + HALF_W) / REGION_TRANSITION))
        winner_weight = smoothstep(t)
        
        for r in range(NUM_REGIONS):
            if r == winner:
                region_data[r][idx] = int(winner_weight * island_w * 255)
            elif r == runner_up and signed_dist < HALF_W:
                region_data[r][idx] = int((1.0 - winner_weight) * island_w * 255)
            else:
                region_data[r][idx] = 0

# ============================================================
# Phase 5: 写入 TGA + UE 导入
# ============================================================
print("=== Phase 5: Writing TGAs ===")
region_layers = ["Grass", "Grass_Biom_4", "Grass_Biom_3", "Grass_Biom_2", "DesertSand"]
for r in range(NUM_REGIONS):
    tga_path = f"region_{r}.tga"
    write_tga_grayscale(tga_path, region_data[r], W, H)
    print(f"  {tga_path} ({region_layers[r]}): total weight = {sum(region_data[r]) / 255:.1f}")

write_tga_grayscale("region_stone.tga", stone_data, W, H)
print("  region_stone.tga")

# --- UE 导入（须在同一上下文执行）---
import unreal
world = unreal.EditorLevelLibrary.get_editor_world()

landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == "Landscape":
        landscape = actor; break
assert landscape, "No Landscape found!"

layers = [
    ("region_0.tga", "Grass"), ("region_1.tga", "Grass Biom 4"),
    ("region_2.tga", "Grass Biom 3"), ("region_3.tga", "Grass Biom 2"),
    ("region_4.tga", "DesertSand"), ("region_stone.tga", "Stone"),
]

mel = unreal.MaterialEditingLibrary
tools = unreal.AssetToolsHelpers.get_asset_tools()

for tga_path, layer_name in layers:
    tex = unreal.RenderingLibrary.import_file_as_texture2d(world, tga_path)
    tex.set_editor_property("srgb", False)
    tex.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
    tex.modify(True)
    
    mat_name = f"M_RegionImport_{layer_name.replace(' ', '_')}"
    mat_path = f"/Game/_MyTest/Materials/{mat_name}.{mat_name}"
    mat = unreal.load_asset(name=mat_path) if unreal.EditorAssetLibrary.does_asset_exist(mat_path) else None
    if not mat:
        mat = tools.create_asset(mat_name, "/Game/_MyTest/Materials",
                                  unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    mel.delete_all_material_expressions(mat)
    tc = mel.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -400, 0)
    ts = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -200, 0)
    ts.set_editor_property("texture", tex)
    mel.connect_material_expressions(tc, "", ts, "")
    mel.connect_material_property(ts, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    
    rt = unreal.RenderingLibrary.create_render_target2d(
        world, W, H, unreal.TextureRenderTargetFormat.RTF_R8,
        clear_color=unreal.LinearColor(0, 0, 0, 1), auto_generate_mip_maps=False)
    unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)
    
    result = landscape.landscape_import_weightmap_from_render_target(rt, layer_name)
    print(f"  {layer_name}: {result}")
    assert result, f"Failed: {layer_name}"

landscape.force_layers_full_update()
landscape.modify()
unreal.EditorLevelLibrary.save_current_level()

# 清理临时文件
import os
for tga_path, _ in layers:
    if os.path.exists(tga_path): os.remove(tga_path)
print("=== Step 3 complete! ===")
```

### 调整指南

| 参数 | 增大效果 | 减小效果 |
|------|---------|---------|
| `NOISE_AMP` | 区域边界更不规则、曲折 | 更接近圆形 Voronoi 格 |
| `NOISE_CELL` | 边界大尺度弯曲更舒展 | 更细碎 |
| `NOISE_OCTAVES` | 边界细节层次更丰富 | 更平滑 |
| `REGION_TRANSITION` | 区域间过渡更宽、更柔和 | 更接近硬边界 |
| 种子点数量 | 区域数更多 | 区域数更少 |

### 区域 → 图层映射

| 区域 | Layer Name | 备注 |
|------|-----------|------|
| Region 0 | Grass | 草地 |
| Region 1 | Grass Biom 4 | 黑色 |
| Region 2 | Grass Biom 3 | 红色 |
| Region 3 | Grass Biom 2 | 深绿 |
| Region 4 | DesertSand | 沙漠 |
| 岛屿外部 | Stone | 石头（1 - island_mask） |

---

## 执行记录

### 2026-06-03 — 初始执行

**环境：**
- 关卡：`/Game/_MyTest/Map/M_AI_LevelCreate`
- 地形：17×17 组件，每组件 127 quads，分辨率 2160×2160
- 材质：`MI_LandscapeMasterMaterial`（含 Grass、Grass Biom 2/3/4、DesertSand、Stone 图层参数）

**Step 1 执行结果：** ✅ 通过
- 参数：BASE_CELL=1024, OCTAVES=4, AMPLITUDE=0.35
- `landscape_import_heightmap_from_render_target` → True
- 执行时间：噪声生成 ~35s + UE 导入 ~3s

**Step 2 执行结果：** ✅ 通过（经三轮迭代）
- 第一版：角度正弦波 + 二值 mask → 硬边界，几何感强 ❌
- 第二版：2D FBM 噪声 + smoothstep 80px 固定过渡带 → 仍不够自然 ❌
- 第三版：2D FBM 噪声（1/4 低分辨率预计算 + 双线性上采样）+ smoothstep 160±60px 噪声调制过渡带 → 效果满意 ✅
- 参数：BOUNDARY_AMP=250, BOUNDARY_OCTAVES=5, BOUNDARY_CELL=300, TRANSITION_BASE=160, TRANSITION_NOISE_AMP=60, DETAIL_CELL=50, DETAIL_AMP=20
- 草地平均权重：44.7%，石头平均权重：55.3%
- `landscape_import_weightmap_from_render_target("Grass")` → True
- `landscape_import_weightmap_from_render_target("Stone")` → True
- 执行时间：mask 生成 ~27s + UE 导入 ~5s

**Step 3 执行结果：** ✅ 通过（经三轮过渡带宽度迭代）
- 第一版：REGION_TRANSITION=12px → 边界太硬 ❌
- 第二版：REGION_TRANSITION=60px → 有改善但仍不够柔和 ❌
- 第三版：REGION_TRANSITION=120px → 与外圈海岸线过渡一致，效果满意 ✅
- 参数：NOISE_AMP=80, NOISE_CELL=50, NOISE_OCTAVES=3, REGION_TRANSITION=120, ISLAND_SEED=42000, REGION_SEED=12345
- 固定种子点：(559,975), (888,1520), (1138,477), (1139,864), (1542,1187)
- 区域面积分布：Grass 20.1%, Grass Biom 4 21.7%, Grass Biom 3 17.6%, Grass Biom 2 15.1%, DesertSand 25.5%
- 面积比（最大/最小）：1.7x
- 全部 6 层 `landscape_import_weightmap_from_render_target` → True
- 执行时间：计算 ~32s + UE 导入 ~10s
