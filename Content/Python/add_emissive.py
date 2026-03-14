
import unreal

material_path = "/Game/Materials/M_ColorAdjustable"
material = unreal.load_asset(material_path)

if material:
    print(f"正在处理材质: {material_path}")
    
    # 创建自发光颜色参数
    emissive_color = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionVectorParameter,
        node_pos_x=-400,
        node_pos_y=600
    )
    emissive_color.set_editor_property("parameter_name", "EmissiveColor")
    emissive_color.set_editor_property("default_value", unreal.LinearColor(0.0, 0.0, 0.0, 1.0))
    
    # 创建自发光强度参数
    emissive_strength = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionScalarParameter,
        node_pos_x=-200,
        node_pos_y=600
    )
    emissive_strength.set_editor_property("parameter_name", "EmissiveStrength")
    emissive_strength.set_editor_property("default_value", 0.0)
    
    # 相乘：颜色 * 强度
    multiply = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionMultiply,
        node_pos_x=0,
        node_pos_y=600
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(emissive_color, "", multiply, "A")
    unreal.MaterialEditingLibrary.connect_material_expressions(emissive_strength, "", multiply, "B")
    
    # 连接到自发光输出
    unreal.MaterialEditingLibrary.connect_material_property(
        multiply, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
    )
    
    # 编译并保存
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(material_path, False)
    print(f"{material_path} 更新完成！已添加自发光参数！")
else:
    print("无法加载材质！")
