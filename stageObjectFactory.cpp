//-----------------------------------------------------------
//  [ stageObjectFactory.cpp ]
//  カタログ定義と配置データからステージオブジェクトを生成
//-----------------------------------------------------------
#include "main.h"
#include "stageObjectFactory.h"

#include "box.h"
#include "breakPlatform.h"
#include "checkpoint.h"
#include "heart.h"
#include "jumpPad.h"
#include "scene.h"
#include "spike.h"
#include "stageObjectCatalog.h"
#include "stageObjectData.h"
#include "stageProp.h"
#include "star.h"
#include "terrainBlock.h"

//カタログ定義と配置データから指定したシーンへオブジェクトを生成
GameObject* StageObjectFactory::Create(Scene* scene, const StageObjectData& data)
{
	if (!scene)
		return nullptr;

	//保存用IDから生成に必要なカタログ定義を取得
	const StageObjectDefinition* definition = StageObjectCatalog::Find(data.AssetId);
	if (!definition)
		return nullptr;

	//異なる種類の配置オブジェクトを共通して扱うためGameObjectポインタで保持
	GameObject* object = nullptr;

	//カタログで指定された種類に応じて生成するクラスを切り替える
	switch (definition->Kind)
	{
	case StageObjectKind::Box:
	{
		auto box = scene->AddGameObject<Box>(Scene::OBJECT3D);
		box->SetModel(definition->ModelPath);
		//有効なパスがある場合テクスチャを設定
		if (definition->BoxTexturePath && definition->BoxTexturePath[0] != '\0')
			box->SetTexture(definition->BoxTexturePath);
		box->SetColliderHalfSize(definition->ColliderHalfSize);
		box->SetColliderCenterOffset(definition->ColliderCenterOffset);
		object = box;
		break;
	}
	case StageObjectKind::Prop:
	{
		auto prop = scene->AddGameObject<StageProp>(Scene::OBJECT3D);
		prop->SetModel(definition->ModelPath);
		object = prop;
		break;
	}
	case StageObjectKind::Terrain:
	{
		const TerrainTextureInfo& textures = definition->TerrainTextures;
		if (!textures.TopTexturePath || !textures.UpperSideTexturePath ||
			!textures.RepeatingSideTexturePath || !textures.BottomTexturePath)
			return nullptr;

		auto terrain = scene->AddGameObject<TerrainBlock>(Scene::OBJECT3D);
		terrain->SetTextures(textures.TopTexturePath, textures.UpperSideTexturePath,
			textures.RepeatingSideTexturePath, textures.BottomTexturePath);
		object = terrain;
		break;
	}
	case StageObjectKind::BreakPlatform:
		object = scene->AddGameObject<BreakPlatform>(Scene::OBJECT3D);
		break;
	case StageObjectKind::Checkpoint:
		object = scene->AddGameObject<Checkpoint>(Scene::OBJECT3D);
		break;
	case StageObjectKind::JumpPad:
		object = scene->AddGameObject<JumpPad>(Scene::OBJECT3D);
		break;
	case StageObjectKind::Spike:
		object = scene->AddGameObject<Spike>(Scene::OBJECT3D);
		break;
	case StageObjectKind::Star:
		object = scene->AddGameObject<Star>(Scene::OBJECT3D);
		break;
	case StageObjectKind::Heart:
		object = scene->AddGameObject<Heart>(Scene::OBJECT3D);
		break;
	default:
		return nullptr;
	}

	if (!ApplyData(object, data))
	{
		object->SetDestroy();
		return nullptr;
	}

	return object;
}

//既存の配置オブジェクトへ編集可能なデータを反映
bool StageObjectFactory::ApplyData(GameObject* object, const StageObjectData& data)
{
	if (!object)
		return false;

	const StageObjectDefinition* definition = StageObjectCatalog::Find(data.AssetId);
	if (!definition)
		return false;

	object->SetPosition(data.Position);
	object->SetRotation(data.Rotation);
	object->SetScale(data.Scale);

	if (definition->Kind == StageObjectKind::Box)
	{
		auto* box = dynamic_cast<Box*>(object);
		if (!box)
			return false;

		box->SetRotationVelocity(data.RotationVelocity);
	}

	return true;
}
