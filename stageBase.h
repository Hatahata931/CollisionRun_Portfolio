//-----------------------------------------------------------
//  [ stageBase.h ]
//  ステージ共通の配置ヘルパーとエディター用データ管理
//-----------------------------------------------------------
#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "scene.h"
#include "stageObjectData.h"
#include "vector3.h"

class Box;
class GameObject;

//JSONで管理する配置データと生成済みオブジェクトの組
struct StageObjectInstance
{
	StageObjectData Data; //JSONへ保存する配置データ
	GameObject* Object = nullptr; //シーン上の実体 取得済みアイテムなどはDataを残してnullptrになる
};

class StageBase : public Scene
{
private:
	std::vector<StageObjectInstance> m_StageObjects; //エディターとJSONで管理する配置オブジェクト

	Box* CreateBox(Vector3 position, Vector3 scale);

protected:
	static constexpr const char* MechanicalStoneTexturePath = "asset\\texture\\mechanical_stone_block.png";

	//それぞれモデル・テクスチャを指定してBoxを生成
	Box* CreateBoxModel(Vector3 position, Vector3 scale, const char* model, Vector3 rotation = { 0.0f, 0.0f, 0.0f });
	Box* CreateBoxTexture(Vector3 position, Vector3 scale, const char* texture, Vector3 rotation = { 0.0f, 0.0f, 0.0f });

	//管理中の実体がシーンから削除される直前にポインタを切り離す
	void OnGameObjectDestroying(GameObject* object) override;

public:
	//StageBaseを継承するゲームステージではポーズ有効
	bool CanPause() const override { return true; }

	//このシーンが使用するエディター配置データの保存先を取得
	virtual const char* GetStageObjectDataPath() const = 0;

	GameObject* CreateStageObject(const StageObjectData& data);
	bool UpdateStageObject(size_t index, const StageObjectData& data);
	bool DeleteStageObject(size_t index);
	void ClearStageObjects();
	bool SaveStageObjects(const char* filePath, std::string* errorMessage = nullptr) const;
	bool LoadStageObjects(const char* filePath, std::string* errorMessage = nullptr);

	const std::vector<StageObjectInstance>& GetStageObjects() const { return m_StageObjects; }
};
