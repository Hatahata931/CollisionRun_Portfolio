//-----------------------------------------------------------
//  [ stageDataSerializer.cpp ]
//  ステージ配置データをJSON形式で保存・読み込み
//-----------------------------------------------------------
#include "main.h"
#include "stageDataSerializer.h"

#include <exception>
#include <fstream>
#include <iomanip>
#include <stdexcept>
#include <utility>

#include "external/nlohmann/json.hpp"

namespace
{
	using json = nlohmann::json;

	//現在保存するJSON形式のバージョン
	constexpr int CurrentVersion = 3;

	//Vector3をJSON配列に変換
	json VectorToJSON(const Vector3& value)
	{
		return json::array({ value.x, value.y, value.z });
	}

	//JSONをVector3に変換
	Vector3 JSONToVector(const json& value, const Vector3& defaultValue)
	{
		if (!value.is_array() || value.size() != 3)
			return defaultValue;

		return {
			value[0].get<float>(),
			value[1].get<float>(),
			value[2].get<float>()
		};
	}

	//旧形式のtypeまたはassetNameを新形式のAssetIdとして読み込む
	std::string GetLegacyAssetId(const json& objectJSON)
	{
		const std::string assetName = objectJSON.value("assetName", std::string());
		if (!assetName.empty())
			return assetName;

		return objectJSON.value("type", std::string("Crate"));
	}

	//呼び出し元がメッセージを要求した場合だけ結果を設定
	void SetError(std::string* errorMessage, const std::string& message)
	{
		if (errorMessage)
			*errorMessage = message;
	}
}

bool StageDataSerializer::Save(const char* filePath, const std::vector<StageObjectData>& objects, std::string* errorMessage)
{
	//保存先パスが空の場合は処理しない
	if (!filePath || filePath[0] == '\0')
	{
		SetError(errorMessage, "Save path is empty.");
		return false;
	}

	//既定の保存先フォルダがなければ作成する
	CreateDirectoryA("asset", nullptr);
	CreateDirectoryA("asset\\stage", nullptr);

	try
	{
		//バージョンと配置物一覧を持つJSON全体を作成
		json root;
		root["version"] = CurrentVersion;
		root["objects"] = json::array();

		//各配置データをJSONオブジェクトへ変換して配列に追加
		for (const StageObjectData& object : objects)
		{
			json objectJSON;
			objectJSON["assetId"] = object.AssetId;
			objectJSON["position"] = VectorToJSON(object.Position);
			objectJSON["rotation"] = VectorToJSON(object.Rotation);
			objectJSON["scale"] = VectorToJSON(object.Scale);
			if (object.RotationVelocity.x != 0.0f ||
				object.RotationVelocity.y != 0.0f ||
				object.RotationVelocity.z != 0.0f)
			{
				objectJSON["rotationVelocity"] = VectorToJSON(object.RotationVelocity);
			}
			root["objects"].push_back(objectJSON);
		}

		//指定されたパスを保存先として開く
		std::ofstream file(filePath);
		if (!file)
		{
			SetError(errorMessage, "Failed to open save file.");
			return false;
		}

		//4文字分インデントして人が読みやすい形式で書き込む
		file << std::setw(4) << root;
		SetError(errorMessage, "Saved.");
		return true;
	}
	catch (const std::exception& exception)
	{
		SetError(errorMessage, exception.what());
		return false;
	}
}

bool StageDataSerializer::Load(const char* filePath, std::vector<StageObjectData>& objects, std::string* errorMessage)
{
	//読み込み元パスが空の場合は処理しない
	if (!filePath || filePath[0] == '\0')
	{
		SetError(errorMessage, "Load path is empty.");
		return false;
	}

	try
	{
		std::ifstream file(filePath);
		if (!file)
		{
			SetError(errorMessage, "Failed to open load file.");
			return false;
		}

		//ファイル全体をJSONとして読み込む
		json root;
		file >> root;

		//versionがない旧データはversion1として扱う
		const int version = root.value("version", 1);
		if (version < 1 || version > CurrentVersion)
		{
			SetError(errorMessage, "Unsupported stage data version.");
			return false;
		}

		//配置物一覧が存在し、配列になっているか確認
		if (!root.contains("objects") || !root["objects"].is_array())
		{
			SetError(errorMessage, "Invalid stage data.");
			return false;
		}

		//途中で失敗しても呼び出し元のデータを壊さないよう一時リストへ読み込む
		std::vector<StageObjectData> loadedObjects;
		for (const json& objectJSON : root["objects"])
		{
			StageObjectData object;
			//新形式はassetId、旧形式はtypeまたはassetNameからIDを取得
			object.AssetId = version >= 2
				? objectJSON.value("assetId", std::string())
				: GetLegacyAssetId(objectJSON);

			if (object.AssetId.empty())
				throw std::runtime_error("Stage object assetId is empty.");

			object.Position = JSONToVector(objectJSON.value("position", json::array()), object.Position);
			object.Rotation = JSONToVector(objectJSON.value("rotation", json::array()), object.Rotation);
			object.Scale = JSONToVector(objectJSON.value("scale", json::array()), object.Scale);
			if (version >= 3)
			{
				object.RotationVelocity = JSONToVector(
					objectJSON.value("rotationVelocity", json::array()),
					object.RotationVelocity);
			}
			loadedObjects.push_back(object);
		}

		//全件の読み込みに成功してから呼び出し元へ反映
		objects = std::move(loadedObjects);
		SetError(errorMessage, "Loaded.");
		return true;
	}
	catch (const std::exception& exception)
	{
		SetError(errorMessage, exception.what());
		return false;
	}
}
