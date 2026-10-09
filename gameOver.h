//-----------------------------------------------------------
//  [ gameOver.h ]
//  ゲームオーバー画面
//-----------------------------------------------------------
#pragma once

#include "stageBase.h"
#include "stageId.h"

class GameObject;

class GameOver : public StageBase
{
private:
	//ゲームオーバーメニューの選択項目
	enum class MenuItem
	{
		Retry,
		ReturnStageSelect,
		ReturnTitle,
	};
	static constexpr int MenuItemCount = 3;

	//3Dオブジェクト配置データ
	static constexpr const char* StageObjectDataPath = "asset\\stage\\game_over_editor.json";

	static constexpr float MenuCenterX = 360.0f; //ボタン、文字の中心X座標

	//各ボタンの中心Y座標
	static constexpr float RetryCenterY = 340.0f;
	static constexpr float ReturnStageSelectCenterY = 445.0f;
	static constexpr float ReturnTitleCenterY = 550.0f;

	//未選択時のボタンサイズ
	static constexpr float NormalButtonWidth = 440.0f;
	static constexpr float NormalButtonHeight = 82.0f;

	static constexpr float SelectedScale = 1.1f; //選択時の拡大倍率

	//選択時のボタンサイズ
	static constexpr float SelectedButtonWidth = NormalButtonWidth * SelectedScale;
	static constexpr float SelectedButtonHeight = NormalButtonHeight * SelectedScale;

	//未選択時の文字の横幅
	static constexpr float RetryTextWidth = 260.0f;
	static constexpr float ReturnStageSelectTextWidth = 400.0f;
	static constexpr float ReturnTitleTextWidth = 400.0f;

	static constexpr float TextHeight = 84.0f; //未選択時の文字の高さ

	MenuItem m_SelectedItem = MenuItem::Retry;
	StageId m_RetryStage;
	GameObject* m_RetryButton = nullptr;
	GameObject* m_ReturnStageSelectButton = nullptr;
	GameObject* m_ReturnTitleButton = nullptr;
	GameObject* m_RetryText = nullptr;
	GameObject* m_ReturnStageSelectText = nullptr;
	GameObject* m_ReturnTitleText = nullptr;

	//マウスが動いたか判定するための前回座標
	int m_PreviousMouseX = 0;
	int m_PreviousMouseY = 0;

	//画面座標を中心として2D画像を生成
	GameObject* CreateFixedImage(const char* texturePath, Vector3 position, Vector3 scale);
	void CreateSceneObjects();
	void CreateMenu();
	void SetSelectedItem(MenuItem item);
	//選択中のボタン背景と文字を拡大
	void UpdateMenuVisuals();
	void ConfirmSelection();
	bool IsInsideButton(int mouseX, int mouseY, float centerY) const;

protected:
	//フェードイン完了後に効果音を再生
	void OnFadeInFinished() override;

public:
	//ゲームオーバー前のステージをリトライ先として保持
	explicit GameOver(StageId retryStage) : m_RetryStage(retryStage) {}

	void Init() override;
	void Update() override;

	bool CanPause() const override { return false; }
	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};
