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
  Python 生成6层hash-based value noise
  → 编码为 RG16 TGA (R=高8位, G=低8位)
  → import_file_as_texture2d 导入为瞬态纹理
  → 材质 TextureSample → Emissive (Unlit)
  → draw_material_to_render_target
  → landscape_import_heightmap_from_render_target(import_height_from_rg_channel=True)

Step 2: 不规则活动区域
  极坐标 + 正弦扰动生成连续闭合岛屿形状
  → 写入 8-bit TGA (0=石头, 255=草地)
  → import_file_as_texture2d 导入
  → 材质 TextureSample → Emissive
  → 绘制到 R8 RT
  → landscape_import_weightmap_from_render_target("Grass") 为正相
  → landscape_import_weightmap_from_render_target("Stone") 为反相
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
landscape_proxy.landscape_import_weightmap_from_render_target(rt, layer_name)
```

- RT 格式：`RTF_R8`（单通道 8-bit）
- 像素值 0 = 权重 0，像素值 255 = 权重 1
- `layer_name` 须匹配地形材质中 LayerBlend 节点的参数名

---

## 执行参数

| 参数 | 默认值 | 含义 |
|------|--------|------|
| `W` / `H` | 2048 | 地形高度图分辨率 |
| `BASE_CELL` | 1024 | 噪声基础特征大小（越大起伏越舒展） |
| `OCTAVES` | 4 | 噪声细节层数（越少越平滑） |
| `AMPLITUDE` | 0.35 | 起伏幅度（1.0=全范围，0.35=温和起伏） |
| `BASE_RADIUS` | 800 | 岛屿基础半径（控制活动区域大小） |
| `EDGE_MARGIN` | 200 | 岛屿距地形边缘最小缓冲（像素） |
| `WAVES` | 随机生成 | 4 层正弦波，每次运行随机振幅/频率/相位 |

---

## Step 1：地形起伏

### 完整脚本

```python
import math, os, unreal

W = H = 2048

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

## Step 2：不规则活动区域（岛屿）

### 完整脚本

```python
import math, random, os, unreal

W = H = 2160  # 权重图分辨率（需匹配地形实际分辨率）

# ============================================================
# 生成不规则岛屿 mask
# ============================================================
cx, cy = W // 2, H // 2         # 岛屿中心
BASE_RADIUS = 800                # 基础半径
EDGE_MARGIN = 200                # 边缘缓冲（像素）

# 随机化参数
random.seed()
NOISE_SEED = random.randint(0, 99999)
WAVES = [
    (random.uniform(120, 220), random.randint(2, 5), random.random() * 6.28),
    (random.uniform(60, 120), random.randint(5, 10), random.random() * 6.28),
    (random.uniform(30, 60), random.randint(10, 18), random.random() * 6.28),
    (random.uniform(10, 30), random.randint(20, 30), random.random() * 6.28),
]

def value_noise(x, y, cs, seed):
    h = seed + int(x / cs) * 374761393 + int(y / cs) * 668265263
    h = (h ^ (h >> 13)) * 1274126177
    h = h ^ (h >> 16)
    return (h & 0x7fffffff) / 0x7fffffff

print("=== Step 2: Generating island mask ===")
mask = bytearray(W * H)
for y in range(H):
    if y % 360 == 0:
        print(f"  Row {y}/{H}")
    for x in range(W):
        dx, dy = x - cx, y - cy
        dist = math.hypot(dx, dy)
        angle = math.atan2(dy, dx)

        # 岛屿半径 = 基础半径 + 正弦扰动（产生不规则形状）
        radius = BASE_RADIUS
        for amp, freq, phase in WAVES:
            radius += amp * math.sin(angle * freq + phase)

        # 海岸线噪声扰动
        noise = (value_noise(x, y, 60, NOISE_SEED) - 0.5) * 30

        # 边缘惩罚：靠近边缘的距离被推远，保证纯石头缓冲带
        edge_dist = min(x, y, W - 1 - x, H - 1 - y)
        penalty = (1 - edge_dist / EDGE_MARGIN) * 500 if edge_dist < EDGE_MARGIN else 0

        mask[y * W + x] = 255 if dist + penalty < radius + noise else 0

# 统计覆盖率
white = sum(1 for b in mask if b > 128)
print(f"草地(岛屿): {white / W / H * 100:.1f}%  |  石头(海水): {100 - white / W / H * 100:.1f}%")

# 写入 8-bit TGA
TGA_PATH = "province_mask.tga"
with open(TGA_PATH, "wb") as f:
    f.write(bytearray([0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0]))  # type=3 = grayscale
    f.write(bytearray([W & 0xFF, (W >> 8) & 0xFF, H & 0xFF, (H >> 8) & 0xFF]))
    f.write(bytearray([8, 0x20]))  # 8-bit, top-left origin
    f.write(mask)
    f.write(bytearray([0, 0, 0, 0, 0, 0, 0, 0]))
    f.write(b"TRUEVISION-XFILE.\0")

# ============================================================
# 导入到 UE5 地形
# ============================================================
world = unreal.EditorLevelLibrary.get_editor_world()

# 加载 mask 纹理
tex = unreal.RenderingLibrary.import_file_as_texture2d(world, TGA_PATH)
tex.set_editor_property("srgb", False)
tex.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
tex.modify(True)

# 创建正相材质（mask → Grass）
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

# 渲染 Grass layer (mask 直接作为权重)
rt = unreal.RenderingLibrary.create_render_target2d(
    world, W, H,
    unreal.TextureRenderTargetFormat.RTF_R8,
    clear_color=unreal.LinearColor(0, 0, 0, 1),
    auto_generate_mip_maps=False
)
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)

# 创建反相材质（1 - mask → Stone）
mat_inv = unreal.load_asset(name="/Game/_MyTest/Materials/M_ProvinceMask_Inverted.M_ProvinceMask_Inverted")
if not mat_inv:
    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat_inv = tools.create_asset("M_ProvinceMask_Inverted", "/Game/_MyTest/Materials",
                                  unreal.Material, unreal.MaterialFactoryNew())
mel.delete_all_material_expressions(mat_inv)
tc2 = mel.create_material_expression(mat_inv, unreal.MaterialExpressionTextureCoordinate, -600, 0)
ts2 = mel.create_material_expression(mat_inv, unreal.MaterialExpressionTextureSample, -400, 0)
ts2.set_editor_property("texture", tex)
om = mel.create_material_expression(mat_inv, unreal.MaterialExpressionOneMinus, -200, 0)
mel.connect_material_expressions(tc2, "", ts2, "")
mel.connect_material_expressions(ts2, "", om, "")
mel.connect_material_property(om, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
mel.recompile_material(mat_inv)

# 渲染 Stone layer (1 - mask)
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
proxy = unreal.LandscapeProxy.cast(landscape)

# 导入权重图
proxy.landscape_import_weightmap_from_render_target(rt, "Grass")
proxy.landscape_import_weightmap_from_render_target(rt2, "Stone")

proxy.force_layers_full_update()
proxy.modify()
unreal.EditorLevelLibrary.save_current_level()

# 清理
os.remove(TGA_PATH)
print("=== Step 2 complete! ===")
```

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

地形高度图分辨率（2048x2048）必须与噪声输出分辨率一致。

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
- [ ] **Step 2**：岛屿边界不规则且连续（非模糊过渡）
- [ ] **Step 2**：草地不接触地形边缘（有纯石头缓冲带）
- [ ] **Step 2**：两个 `landscape_import_weightmap_from_render_target` 均返回 True
- [ ] 覆盖率：草地约 44%，石头约 56%（受随机参数轻微波动）
