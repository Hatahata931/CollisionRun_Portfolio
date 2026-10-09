//-----------------------------------------------------------
//  [ stage1.h ]
//  ステージ1
//-----------------------------------------------------------
#pragma once

#include "stageBase.h"

class Stage1 : public StageBase
{
private:
	static constexpr const char* StageObjectDataPath = "asset\\stage\\stage1_editor.json"; //エディター配置データの保存先
	static constexpr float PlayerFallLimitY = -10.0f; //落下判定になる高さ

	void CreateGimmicks();
	void CreatePlayer();

public:
	void Init() override;
	void Update() override;

	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};

