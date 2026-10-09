//-----------------------------------------------------------
//  [ stageSelect.h ]
//  ステージ選択画面
//-----------------------------------------------------------
#pragma once

#include <array>
#include "stageBase.h"
#include "stageId.h"

class GameObject;

class StageSelect : public StageBase
{
private:
	//1つのステージカードを構成する要素
	struct StageCard
	{
		StageId Id = StageId::Stage1;
		GameObject* Panel = nullptr; //カード背景
		GameObject* Preview = nullptr; //ステージ画像
		GameObject* Title = nullptr; //「ステージ1」などの文字
		GameObject* Difficulty = nullptr;
	};

	static constexpr const char* StageObjectDataPath = "asset\\stage\\stage_select_editor.json";
	static constexpr int CardCount = 3;

	//カードの位置・大きさ
	static constexpr float FirstCardCenterX = 285.0f;
	static constexpr float CardSpacing = 355.0f;
	static constexpr float CardCenterY = 380.0f;
	static constexpr float CardWidth = 320.0f;
	static constexpr float CardHeight = 420.0f;

	//ステージ画像の位置・大きさ
	static constexpr float PreviewCenterY = 350.0f;
	static constexpr float PreviewWidth = 280.0f;
	static constexpr float PreviewHeight = 180.0f;

	//文字の位置・大きさ
	static constexpr float TitleCenterY = 215.0f;
	static constexpr float TitleWidth = 220.0f;
	static constexpr float TitleHeight = 58.0f;

	//星の位置・大きさ
	static constexpr float DifficultyCenterY = 515.0f;
	static constexpr float DifficultyWidth = 275.0f;
	static constexpr float DifficultyHeight = 94.0f;

	//選択中の大きさ
	static constexpr float SelectedScale = 1.06f;
	static constexpr float SelectionFrameMargin = 6.0f; //選択枠をカードより少し大きくするための余白

	std::array<StageCard, CardCount> m_Cards{};
	GameObject* m_SelectionFrame = nullptr;
	int m_SelectedIndex = 0;
	int m_PreviousMouseX = 0;
	int m_PreviousMouseY = 0;

	//画面座標を中心とする2D画像を生成
	GameObject* CreateFixedImage(const char* texturePath, Vector3 position, Vector3 scale);
	void CreateSceneObjects();
	void CreateMenu();
	void SetSelectedIndex(int index);
	void UpdateCardVisuals();
	//選択中のステージへ遷移
	void ConfirmSelection();
	//マウスカーソルが重なっているカード番号を取得
	int GetHoveredCardIndex(int mouseX, int mouseY) const;
	float GetCardCenterX(int index) const;

public:
	void Init() override;
	void Update() override;

	bool CanPause() const override { return false; }
	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};
