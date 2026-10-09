//-----------------------------------------------------------
//  [ title.h ]
//  タイトル画面
//-----------------------------------------------------------
#pragma once

#include "stageBase.h"

class Title : public StageBase
{
private:
	static constexpr const char* StageObjectDataPath = "asset\\stage\\title_editor.json"; //エディター配置データの保存先

	void CreateSceneObjects();

public:
	void Init() override;
	void Update() override;

	bool CanPause() const override { return false; }
	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};
