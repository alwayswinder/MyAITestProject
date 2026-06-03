现在我们来制定一个程序化的关卡搭建方案，
首先确定一些重要据点的位置，大概20个，据点范围半径500范围圆形；
然后用路径连通各个据点，希望路径不能笔直，要有弯曲变化；
据点使用/Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/LowPoly2_Showcase_sharedassets/2_LayerInfo.2_LayerInfo'；
路径使用/Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/LowPoly2_Showcase_sharedassets/3_LayerInfo.3_LayerInfo'；


现在尝试用tree的资产/Script/Engine.StaticMesh'/Game/Orasot_Bundle/LowPolyForestVol2/StaticMeshes/Environment/SM_Env_Pine_Tree_1.SM_Env_Pine_Tree_1'
填充路和据点之外的区域


现在我想实现一个设计，
第一步：你在当前地形上划出一块区域，要求占据当前地形绝大部分面积，区域形状不规则，像真实地图中的省份划分一样
区域内使用/Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/M_Global_Overview_sharedassets/Grass_LayerInfo.Grass_LayerInfo'作为基底layer；
区域外使用/Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/5_Bioms_ShaderAssets/Stone_LayerInfo.Stone_LayerInfo'作为基底


将当前的实现记录下来重新写入一个新的文档，用于之后完全复刻，
第一步，给地形一个noise起伏变化，
第二步，在中心位置随机出不规则区域作为游戏真正的活动区域，
保存在E:\Unreal\Projects\_FormGit\MyAITestProject\docs\ai-out

现在我们需要将上一步得到的中心不规则区域分割为5个不同区域，
我希望每个区域一样是不规则边界，像省内内部的不同地区一样，面积可以有差异，但不能过大；
有什么好的方案吗？

先思考能不能做到，
然后思考是在上一步基础上实现简单，还是第一步就直接划分出五个区域简单。
1.草地区域，基底本身的layer
2.沙漠区域 /Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/M_Global_Overview_sharedassets/DesertSand_LayerInfo.DesertSand_LayerInfo'
3.红色区域 /Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/M_Global_Overview_sharedassets/Grass_Biom_3_LayerInfo.Grass_Biom_3_LayerInfo'
4.污染区域 /Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/M_Global_Overview_sharedassets/Grass_Biom_4_LayerInfo.Grass_Biom_4_LayerInfo'
5.森林区域 /Script/Landscape.LandscapeLayerInfoObject'/Game/Orasot_Bundle/Maps/M_Global_Overview_sharedassets/Grass_Biom_2_LayerInfo.Grass_Biom_2_LayerInfo'


草地 Grass
黑色 Grass Biom 4
红色 Grass Biom 3
深绿 Grass Biom 2
沙漠 DesertSand