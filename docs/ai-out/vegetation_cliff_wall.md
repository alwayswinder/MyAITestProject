# 程序化植被生成 — Step 1：小岛围墙（悬崖环绕）

## 环境

- UE 5.x + Python 脚本插件
- 关卡：`/Game/_MyTest/Map/M_AI_LevelCreate`
- 地形：24×24 组件，每组件 63 quads → 1513×1513 分辨率
- 地形缩放：100，总世界尺寸 151200×151200
- 左下角世界坐标：(-75600, -75600, 0)

## 前置条件

已完成三步地形建设（见 `terrain_height_and_region.md`）：
1. 地形高度起伏（FBM noise → RG16 高度图）
2. 不规则岛屿 + Grass/Stone 权重图
3. 5 区域 Lloyd + 噪声 Voronoi 分割

## 需求

`SM_Cliff_3` 作为岛屿边缘围墙，环绕整个小岛。

| 参数 | 值 |
|------|-----|
| 网格 | `/Game/.../Biom_Bamboo/StaticMeshes/SM_Cliff_3.SM_Cliff_3` |
| 缩放 | 2 倍 |
| 中心对齐 | 网格 bounds 中心与地表平齐 |
| yaw | 完全随机 (0-360°) |
| 间距 | 沿 3D 边缘均匀 ~1337 单位 |
| 数量 | ~300 |

## 网格信息

```
SM_Cliff_3:
  bounds size:   1094 × 1080 × 1413
  bounds center: (102, 123, -3.4)
  bottom_z_offset: 709.66（pivot 到底部距离）
  pivot 距底部: 710 → 2倍后 1419
  pivot 距顶部: 703 → 2倍后 1406
  bounds 中心 Z: -3.35（相对 pivot）→ 2倍后 -6.71
```

## 放置策略

### 问题：角度均匀采样导致陡坡空隙

按角度均匀采样边缘点时，2D 角度对应的 3D 边缘长度在陡坡处更长，导致同一角度步长下陡坡区域悬崖间距变大，出现视觉空隙。

**2D 周长约 352,000** vs **3D 边缘总长约 401,021**（差约 14%，由地形起伏造成）。

### 解决：3D 路径均匀采样

```
Step 1: 密集角度采样 (720个)
  → 沿角度射线搜索 island_mask 穿过 0.5 的位置
  → 双线性插值获得亚像素精确边缘点

Step 2: Trace 地表高度
  → 对每个边缘点进行 line_trace（50000 → -50000）
  → 获取地表 Z 值，构建 3D 边缘路径

Step 3: 沿 3D 路径均匀采样
  → 计算路径总长 / 300 = 目标间距 (~1337)
  → 沿路径每 1337 单位放置一个悬崖
  → 陡坡区域自动获得更多悬崖，缓坡更少

Step 4: 放置网格
  → spawn_z = surface_z + 3.355 * 2（中心与地表平齐）
  → yaw = random(0, 360)
  → Rotator(roll=0, pitch=0, yaw=random_yaw)
```

## 关键代码

### Rotator 参数顺序

UE5 Python API 中 `unreal.Rotator` 的参数顺序为 **`(roll, pitch, yaw)`**：

```python
# 正确
rot = unreal.Rotator(0, 0, yaw)  # roll=0, pitch=0, yaw=yaw

# 错误
rot = unreal.Rotator(0, yaw, 0)  # roll=0, pitch=yaw, yaw=0 → 悬崖躺倒！
```

### 避坑：trace 互相遮挡

必须 **先 trace 预计算所有地表 Z，再 spawn**。如果在 spawn 循环中 trace，后放的悬崖会 trace 到之前已放置的悬崖上，导致 Z 值异常（飘在空中）。

```python
# ❌ 错误：trace 和 spawn 混在一起
for pos in edge_positions:
    z = trace(pos)       # 可能打到已放置的悬崖
    spawn(pos, z)

# ✅ 正确：先全部 trace，再全部 spawn
surface_zs = [trace(p) for p in edge_positions]  # 无任何悬崖干扰
for pos, z in zip(edge_positions, surface_zs):
    spawn(pos, z)
```

## 参数调整记录

| 轮次 | 数量 | yaw | 对齐方式 | 间距策略 | 结果 |
|------|------|-----|---------|---------|------|
| 1 | 150 | 朝外 | 底部对齐 | 角度均匀 | 密度不够 |
| 2 | 150 | 朝外 + 中心对齐 | 中心对齐 | 角度均匀 | Rotator 参数错→修复 |
| 3 | 150 | 朝外 | 中心对齐 | 角度均匀 | 密了但 yaw 太整齐 |
| 4 | 150 | ±20° 随机 | 中心对齐 | 角度均匀 | 陡坡处有空隙 |
| 5 | 300 | ±20° 随机 | 中心对齐 | 角度均匀 | trace 互相遮挡→预计算修复 |
| 6 | 300 | ±20° 随机 | 中心对齐 | 角度均匀 | 密度够了但陡坡仍有空隙 |
| 7 | 301 | ±20° 随机 | 中心对齐 | 3D 路径均匀 | 间距一致 |
| 8 | **301** | **完全随机 0-360°** | **中心对齐** | **3D 路径均匀** | ✅ 最终版 |

## 最终结果

- 悬崖数量：301
- 3D 间距：平均 1321，标准差 82，最大 1337
- yaw：完全随机 0-360°
- 缩放：2 倍
- 对齐：网格中心与地表平齐
- 朝向：无统一方向，自然散落

## 完整脚本

```python
import math, random, unreal

W = H = 1513; cx, cy = W // 2, H // 2
LOWRES = int(math.ceil(W / 4.0))

# ---- 噪声函数 ----
def hash_noise(x, y, seed=0):
    h = seed + x * 374761393 + y * 668265263
    h = (h ^ (h >> 13)) * 1274126177; h = h ^ (h >> 16)
    return (h & 0x7fffffff) / 0x7fffffff

def smoothstep(t): return t * t * (3 - 2 * t)
def lerp(a, b, t): return a + (b - a) * t

def value_noise(x, y, cell_size, seed=0):
    fx, fy = x/cell_size, y/cell_size
    ix, iy = int(math.floor(fx)), int(math.floor(fy))
    sx, sy = smoothstep(fx-ix), smoothstep(fy-iy)
    return lerp(lerp(hash_noise(ix,iy,seed), hash_noise(ix+1,iy,seed),sx),
                lerp(hash_noise(ix,iy+1,seed), hash_noise(ix+1,iy+1,seed),sx), sy)

def fbm_noise(x, y, octaves, base_cell, seed=0):
    val=0.0; amp=1.0; freq=1.0; max_val=0.0
    for i in range(octaves):
        val+=amp*value_noise(x,y,base_cell/freq,seed+i*1337)
        max_val+=amp; amp*=0.5; freq*=2.0
    return val/max_val

def bilinear_sample(grid, x, y):
    w,h=len(grid[0]),len(grid)
    x=max(0,min(w-1.0001,x)); y=max(0,min(h-1.0001,y))
    ix,iy=int(x),int(y); fx,fy=x-ix,y-iy
    ix1,iy1=min(ix+1,w-1),min(iy+1,h-1)
    return lerp(lerp(grid[iy][ix],grid[iy][ix1],fx),lerp(grid[iy1][ix],grid[iy1][ix1],fx),fy)

# ---- 岛屿 mask 参数（与地形建设一致）----
BASE_RADIUS=560; EDGE_MARGIN=140; ISLAND_SEED=42000
BOUNDARY_OCTAVES=5; BOUNDARY_CELL=300; BOUNDARY_AMP=175
TRANSITION_BASE=112; TRANSITION_NOISE_AMP=42; DETAIL_CELL=50; DETAIL_AMP=14

# ---- 生成岛屿 mask ----
disp_low=[[0.0]*LOWRES for _ in range(LOWRES)]
for y in range(LOWRES):
    for x in range(LOWRES):
        fx,fy=x*4,y*4
        disp_low[y][x]=(fbm_noise(fx,fy,BOUNDARY_OCTAVES,BOUNDARY_CELL,ISLAND_SEED)-0.5)*2*BOUNDARY_AMP

island_mask=[0.0]*(W*H)
for y in range(H):
    for x in range(W):
        dx,dy=x-cx,y-cy; dist=math.hypot(dx,dy)
        displacement=bilinear_sample(disp_low,x/4.0,y/4.0)
        detail=(value_noise(x,y,DETAIL_CELL,ISLAND_SEED+9999)-0.5)*2*DETAIL_AMP
        effective_radius=BASE_RADIUS+displacement+detail
        edge_dist=min(x,y,W-1-x,H-1-y)
        penalty=(1-edge_dist/EDGE_MARGIN)*400 if edge_dist<EDGE_MARGIN else 0
        tw_noise=value_noise(x,y,200,ISLAND_SEED+55555)
        transition_w=max(50,TRANSITION_BASE+(tw_noise-0.5)*2*TRANSITION_NOISE_AMP)
        signed_dist=effective_radius-(dist+penalty)
        t=max(0,min(1,(signed_dist+transition_w/2)/transition_w))
        island_mask[y*W+x]=smoothstep(t)

# ---- 密集采样 720 个边缘点 ----
LANDSCAPE_MIN=-75600.0; WORLD_SIZE=151200.0
def px_to_world(px,py): return (LANDSCAPE_MIN+(px/W)*WORLD_SIZE, LANDSCAPE_MIN+(py/H)*WORLD_SIZE)

dense_xy=[]; center_wx,center_wy=px_to_world(cx,cy)
for i in range(720):
    theta=2*math.pi*i/720
    for r in range(1,int(W*0.7)):
        px=int(cx+r*math.cos(theta)); py=int(cy+r*math.sin(theta))
        if px<0 or px>=W or py<0 or py>=H: break
        if island_mask[py*W+px]<0.5:
            prev_r=r-1
            if prev_r>=0:
                ppx=int(cx+prev_r*math.cos(theta)); ppy=int(cy+prev_r*math.sin(theta))
                if 0<=ppx<W and 0<=ppy<H:
                    prev_mask=island_mask[ppy*W+ppx]
                    if prev_mask>=0.5:
                        frac=(0.5-prev_mask)/(island_mask[py*W+px]-prev_mask)
                        epx=ppx+frac*(px-ppx); epy=ppy+frac*(py-ppy)
                        wx,wy=px_to_world(epx,epy)
                        dense_xy.append((wx,wy))
            break

# ---- Trace 地表高度（预计算，避免互相遮挡）----
world=unreal.EditorLevelLibrary.get_editor_world()
trace_results=[]
for wx,wy in dense_xy:
    result=unreal.SystemLibrary.line_trace_single(
        world,unreal.Vector(wx,wy,50000),unreal.Vector(wx,wy,-50000),
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1,False,[],unreal.DrawDebugTrace.NONE,
        False,unreal.LinearColor(1,0,0,1),unreal.LinearColor(0,1,0,1),0)
    z=result.to_tuple()[4].z
    trace_results.append((wx,wy,z))

# ---- 沿 3D 路径均匀采样 ----
path_lengths=[]
total_len=0.0
for i in range(len(trace_results)):
    x1,y1,z1=trace_results[i]
    x2,y2,z2=trace_results[(i+1)%len(trace_results)]
    d=math.sqrt((x2-x1)**2+(y2-y1)**2+(z2-z1)**2)
    path_lengths.append(d); total_len+=d

SPACING=total_len/300
final_positions=[]
accum=0.0; target=0.0
for i in range(len(trace_results)):
    accum+=path_lengths[i]
    while accum>=target:
        frac=1.0-(accum-target)/path_lengths[i] if path_lengths[i]>0 else 0
        frac=max(0,min(1,frac))
        nxt=(i+1)%len(trace_results)
        x=lerp(trace_results[i][0],trace_results[nxt][0],frac)
        y=lerp(trace_results[i][1],trace_results[nxt][1],frac)
        z=lerp(trace_results[i][2],trace_results[nxt][2],frac)
        final_positions.append((x,y,z))
        target+=SPACING

# ---- 放置网格 ----
CLIFF_PATH="/Game/Orasot_Bundle/Stylized_Landscape_5_Bioms/Biom_Bamboo/StaticMeshes/SM_Cliff_3.SM_Cliff_3"
CLIFF_SCALE=2.0
CENTER_Z_OFFSET=3.355

random.seed(9999)
mesh=unreal.load_asset(name=CLIFF_PATH)
actor_class=unreal.StaticMeshActor.static_class()

for i,(wx,wy,surface_z) in enumerate(final_positions):
    yaw=random.uniform(0,360)
    spawn_z=surface_z+CENTER_Z_OFFSET*CLIFF_SCALE
    rot=unreal.Rotator(0,0,yaw)  # (roll, pitch, yaw)
    loc=unreal.Vector(wx,wy,spawn_z)
    
    actor=unreal.EditorLevelLibrary.spawn_actor_from_class(actor_class,loc,rot)
    if actor:
        actor.set_actor_label(f"Cliff_{i:03d}")
        mc=actor.get_component_by_class(unreal.StaticMeshComponent)
        mc.set_editor_property("static_mesh",mesh)
        mc.set_editor_property("relative_scale3d",unreal.Vector(CLIFF_SCALE,CLIFF_SCALE,CLIFF_SCALE))
        actor.modify()

unreal.EditorLevelLibrary.save_current_level()
```

## 常见陷阱

| 陷阱 | 说明 | 解决 |
|------|------|------|
| Rotator 参数顺序 | `unreal.Rotator(roll, pitch, yaw)` 不是 `(pitch, yaw, roll)` | 写真随机 yaw：`Rotator(0, 0, yaw)` |
| trace 互相遮挡 | spawn 后再 trace 会打到已放置的网格 | 先预计算所有地表 Z，再 spawn |
| 陡坡空隙 | 角度均匀采样在陡坡处 3D 间距变大 | 按 3D 路径距离均匀采样 |
| 整齐排列 | 所有悬崖朝一个方向太假 | yaw 完全随机 0-360° |
