import unreal

mat = unreal.load_asset("/Game/_MyTest/Materials/M_HeightNoise2.M_HeightNoise2")
unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)

# 直接放大 WP 的微小变化：WP.x * 0.3 + 0.5
wp = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionWorldPosition, -600, 0)

# WP.x 提取：DotProduct(WP, (1,0))
# 直接用 WP (Vector3) * Constant(0.3) 取 x 分量
amp = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -800, 100)
amp.set_editor_property("r", 0.3)

mul = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -400, 50)
unreal.MaterialEditingLibrary.connect_material_expressions(wp, "", mul, "a")
unreal.MaterialEditingLibrary.connect_material_expressions(amp, "", mul, "b")

center_c = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -600, 200)
center_c.set_editor_property("r", 0.5)

add = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionAdd, -200, 100)
unreal.MaterialEditingLibrary.connect_material_expressions(mul, "", add, "a")
unreal.MaterialEditingLibrary.connect_material_expressions(center_c, "", add, "b")

# Append(R, G)
append = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionAppendVector, 0, 100)
unreal.MaterialEditingLibrary.connect_material_expressions(add, "", append, "a")
unreal.MaterialEditingLibrary.connect_material_expressions(add, "", append, "b")
unreal.MaterialEditingLibrary.connect_material_property(append, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

unreal.MaterialEditingLibrary.recompile_material(mat)

# 导入
landscape = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() == 'Landscape':
        landscape = a
        break
proxy = unreal.LandscapeProxy.cast(landscape)
components = landscape.get_components_by_class(unreal.LandscapeComponent)
sb = [(c.section_base_x, c.section_base_y) for c in components]
xs = sorted(set(b[0] for b in sb))
ss = xs[1] - xs[0]
rt_size = (max(xs) - min(xs)) + ss + 1

world = unreal.EditorLevelLibrary.get_editor_world()
rt = unreal.RenderingLibrary.create_render_target2d(world, rt_size, rt_size, unreal.TextureRenderTargetFormat.RTF_RGBA16F)
unreal.RenderingLibrary.draw_material_to_render_target(world, rt, mat)
result = proxy.landscape_import_heightmap_from_render_target(rt, True)
print(f"import: {result}")
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
