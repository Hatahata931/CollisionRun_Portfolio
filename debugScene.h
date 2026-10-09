//-----------------------------------------------------------
//  [ debugScene.h ]
//  デバッグ用シーン
//-----------------------------------------------------------
#pragma once

#include "stageBase.h"

class DebugScene : public StageBase
{
private:
	static constexpr const char* StageObjectDataPath = "asset\\stage\\debug_scene_editor.json";

public:
	void Init() override;
	void Uninit() override;
	void Update() override;

	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};
