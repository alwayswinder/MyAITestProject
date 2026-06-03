import unreal

W, H = 2048, 2048
world = unreal.EditorLevelLibrary.get_editor_world()

# 找地形
landscape = None
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if actor.get_class().get_name() == "Landscape":
        landscape = actor
        break

if not landscape:
    print("没找到地形")
    exit()

# 加载噪声材质
mat = unreal.load_asset(name="/Game/_MyTest/Land/M_HeightNoise2.M_HeightNoise2")
if not mat:
    print("没找到材质")
    exit()

# 创建RT（和地形分辨率一致）
rt = unreal.RenderingLibrary.create_render_target2d(
    world_context_object=world, width=W, height=H,
    format=unreal.TextureRenderTargetFormat.RTF_RGBA16F,
    clear_color=unreal.LinearColor(0.0, 0.0, 0.0, 1.0),
    auto_generate_mip_maps=False
)

# 材质画到RT（不做任何处理）
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)

# 直接写入地形
result = landscape.landscape_import_heightmap_from_render_target(
    rt, import_height_from_rg_channel=True, edit_layer_index=0
)

if result:
    landscape.force_layers_full_update()
    landscape.modify()
    unreal.EditorLevelLibrary.save_current_level()
    print("完成")
