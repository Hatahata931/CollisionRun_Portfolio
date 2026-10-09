//-----------------------------------------------------------
//  [ stageBase.cpp ]
//  ステージ共通の配置ヘルパーとエディター用データ管理
//-----------------------------------------------------------
#include "main.h"
#include "stageBase.h"

#include <vector>

#include "box.h"
#include "gameObject.h"
#include "stageDataSerializer.h"
#include "stageObjectCatalog.h"
#include "stageObjectFactory.h"

Box* StageBase::CreateBox(Vector3 position, Vector3 scale)
{
	auto box = AddGameObject<Box>(Scene::OBJECT3D);
	box->SetPosition(position);
	box->SetScale(scale);

	return box;
}

//モデルを指定してBoxを生成
Box* StageBase::CreateBoxModel(Vector3 position, Vector3 scale, const char* model, Vector3 rotation)
{
	auto box = CreateBox(position, scale);
	box->SetRotation(rotation);
	box->SetModel(model);

	return box;
}

//テクスチャを指定してBoxを生成
Box* StageBase::CreateBoxTexture(Vector3 position, Vector3 scale, const char* texture, Vector3 rotation)
{
	auto box = CreateBox(position, scale);
	box->SetRotation(rotation);
	box->SetTexture(texture);

	return box;
}

//管理中の実体がシーンから削除される直前にポインタを切り離す
void StageBase::OnGameObjectDestroying(GameObject* object)
{
	for (StageObjectInstance& instance : m_StageObjects)
	{
		if (instance.Object == object)
		{
			//配置データは保存・再生成に必要なため残し、実体のみ無効化する
			instance.Object = nullptr;
			break;
		}
	}
}

GameObject* StageBase::CreateStageObject(const StageObjectData& data)
{
	GameObject* object = StageObjectFactory::Create(this, data);
	if (!object)
		return nullptr;

	m_StageObjects.push_back({ data, object });
	return object;
}

bool StageBase::UpdateStageObject(size_t index, const StageObjectData& data)
{
	//対象の番号が有効か確認
	if (index >= m_StageObjects.size())
		return false;

	StageObjectInstance& instance = m_StageObjects[index];

	//種類が変わる場合はモデルや判定も変わるため作り直す
	if (instance.Data.AssetId != data.AssetId)
	{
		GameObject* newObject = StageObjectFactory::Create(this, data);
		if (!newObject)
			return false;

		if (instance.Object)
			instance.Object->SetDestroy();
		instance.Object = newObject;
	}
	//取得などで実体だけ削除済みの場合は、保存データから再生成
	else if (!instance.Object)
	{
		instance.Object = StageObjectFactory::Create(this, data);
		if (!instance.Object)
			return false;
	}
	//種類が変わらない場合は既存の実体へ編集内容を反映
	else
	{
		if (!StageObjectFactory::ApplyData(instance.Object, data))
			return false;
	}

	instance.Data = data;
	return true;
}

bool StageBase::DeleteStageObject(size_t index)
{
	//対象の番号が有効か確認
	if (index >= m_StageObjects.size())
		return false;

	if (m_StageObjects[index].Object)
		m_StageObjects[index].Object->SetDestroy();

	m_StageObjects.erase(m_StageObjects.begin() + index);
	return true;
}

void StageBase::ClearStageObjects()
{
	for (StageObjectInstance& instance : m_StageObjects)
	{
		if (instance.Object)
			instance.Object->SetDestroy();
	}

	m_StageObjects.clear();
}

bool StageBase::SaveStageObjects(const char* filePath, std::string* errorMessage) const
{
	std::vector<StageObjectData> objects;
	objects.reserve(m_StageObjects.size());

	//実体のポインタは保存せず、再生成に必要な配置データだけを取り出す
	for (const StageObjectInstance& instance : m_StageObjects)
		objects.push_back(instance.Data);

	return StageDataSerializer::Save(filePath, objects, errorMessage);
}

bool StageBase::LoadStageObjects(const char* filePath, std::string* errorMessage)
{
	//読み込みと検証が終わるまで現在の配置を保持
	std::vector<StageObjectData> objects;
	if (!StageDataSerializer::Load(filePath, objects, errorMessage))
		return false;

	//未登録モデルが含まれていても現在の配置を失わないよう削除前に検証
	for (const StageObjectData& data : objects)
	{
		if (!StageObjectCatalog::Find(data.AssetId))
		{
			if (errorMessage)
				*errorMessage = "Unknown stage asset: " + data.AssetId;
			return false;
		}
	}

	//JSONの解析と全AssetIdの検証が成功してから現在の配置を削除
	ClearStageObjects();

	for (const StageObjectData& data : objects)
	{
		if (!CreateStageObject(data))
		{
			if (errorMessage)
				*errorMessage = "Failed to create a stage object.";
			return false;
		}
	}

	return true;
}
