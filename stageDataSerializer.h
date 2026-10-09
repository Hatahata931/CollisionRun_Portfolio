//-----------------------------------------------------------
//  [ stageDataSerializer.h ]
//  ステージ配置データをJSON形式で保存・読み込み
//-----------------------------------------------------------
#pragma once

#include <string>
#include <vector>

#include "stageObjectData.h"

class StageDataSerializer
{
public:
	static bool Save(const char* filePath, const std::vector<StageObjectData>& objects, std::string* errorMessage = nullptr);
	static bool Load(const char* filePath, std::vector<StageObjectData>& objects, std::string* errorMessage = nullptr);
};
