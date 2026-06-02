import unreal

# 获取地形
landscape = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() == 'Landscape':
        landscape = a
        break
proxy = unreal.LandscapeProxy.cast(landscape)

origin, extent = proxy.get_actor_bounds(False)
print(f"地形中心: ({origin.x:.1f}, {origin.y:.1f}, {origin.z:.1f})")
print(f"地形范围: Z: {extent.z:.1f}")
print(f"地表高度范围: {origin.z - extent.z:.1f} ~ {origin.z + extent.z:.1f}")

# 尝试读取某个位置的权重/高度
components = landscape.get_components_by_class(unreal.LandscapeComponent)
print(f"LandscapeComponent 数: {len(components)}")

# 用编辑层API看看
try:
    layer_info = landscape.get_editor_property("landscape_edit_layer_infos")
    print(f"edit layers: {layer_info}")
except:
    print("no edit layer info")
