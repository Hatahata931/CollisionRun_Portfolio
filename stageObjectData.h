//-----------------------------------------------------------
//  [ stageObjectData.h ]
//  ステージ配置オブジェクトのデータ
//-----------------------------------------------------------
#pragma once

#include <string>

#include "vector3.h"

//ステージエディターで保存する1オブジェクト分の配置データ
struct StageObjectData
{
	std::string AssetId = "Crate"; //StageObjectCatalogと結び付ける保存用ID
	Vector3 Position{ 0.0f, 1.0f, 0.0f }; //配置位置
	Vector3 Rotation{ 0.0f, 0.0f, 0.0f }; //回転角度(ラジアン)
	Vector3 Scale{ 1.0f, 1.0f, 1.0f }; //拡大率
	Vector3 RotationVelocity{ 0.0f, 0.0f, 0.0f }; //1フレームごとの回転量(ラジアン)
};
