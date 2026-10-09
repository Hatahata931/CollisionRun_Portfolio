//-----------------------------------------------------------
//  [ stageObjectFactory.h ]
//  カタログ定義と配置データからステージオブジェクトを生成
//-----------------------------------------------------------
#pragma once

class GameObject;
class Scene;
struct StageObjectData;

class StageObjectFactory
{
public:
	//カタログ定義と配置データから指定したシーンへオブジェクトを生成
	static GameObject* Create(Scene* scene, const StageObjectData& data);
	//既存の配置オブジェクトへ編集可能なデータを反映
	static bool ApplyData(GameObject* object, const StageObjectData& data);
};
