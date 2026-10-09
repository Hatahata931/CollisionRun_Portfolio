//-----------------------------------------------------------
//  [ stage3.h ]
//  ステージ3
//-----------------------------------------------------------
#pragma once

#include "stageBase.h"

class Stage3 : public StageBase
{
private:
	static constexpr const char* StageObjectDataPath = "asset\\stage\\stage3_editor.json"; //エディター配置データの保存先
	static constexpr float PlayerFallLimitY = -6.0f; //落下判定になる高さ

	void CreateGimmicks();
	void CreatePlayer();

public:
	void Init() override;
	void Update() override;

	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};
