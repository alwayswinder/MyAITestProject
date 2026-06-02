import unreal

mat = unreal.load_asset("/Game/_MyTest/Materials/M_HeightNoise2.M_HeightNoise2")
unreal.MaterialEditingLibrary.delete_all_material_expressions(mat)

# 测试 A: UV*100 → Noise (如果连接生效，应该有极密的凹凸)
tc = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionTextureCoordinate, -800, 0)
c100 = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionConstant, -1000, 100)
c100.set_editor_property("r", 100.0)
mul = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionMultiply, -600, 50)
noise = unreal.MaterialEditingLibrary.create_material_expression(mat, unreal.MaterialExpressionNoise, -400, 0)
noise.set_editor_property("scale", 1.0)

unreal.MaterialEditingLibrary.connect_material_expressions(tc, "", mul, "a")
unreal.MaterialEditingLibrary.connect_material_expressions(c100, "", mul, "b")

result = unreal.MaterialEditingLibrary.connect_material_expressions(mul, "", noise, "World Position")

# 直接输出 Noise 到 Emissive（R通道），不经过编码
noise.set_editor_property("scale", 1.0)
unreal.MaterialEditingLibrary.connect_material_property(noise, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
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

# 用 False 测试（不使用 RG 解码，直接读 R 通道）
ok = proxy.landscape_import_heightmap_from_render_target(rt, False)
with open("E:/Unreal/Projects/_FormGit/MyAITestProject/uv_test_result.txt", "w") as f:
    f.write(f"UV->World Position: {result}\nimport: {ok}")

unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
