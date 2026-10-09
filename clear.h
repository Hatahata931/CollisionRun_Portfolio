//-----------------------------------------------------------
//  [ clear.h ]
//  クリア画面
//-----------------------------------------------------------
#pragma once

#include "stageBase.h"

class GameObject;

//ステージで集めたStarの結果
struct ClearResult
{
	int CollectedStars = 0; //取得したStar数
	int TotalStars = 0; //ステージ内の総Star数
};

class Clear : public StageBase
{
private:
	enum class MenuItem
	{
		ReturnStageSelect,
		ReturnTitle,
	};

	static constexpr int MenuItemCount = 2;

	//3Dオブジェクト配置データ
	static constexpr const char* StageObjectDataPath = "asset\\stage\\clear_editor.json";

	static constexpr float MenuCenterX = 360.0f; //ボタン、文字の中心X座標

	//各ボタンの中心Y座標
	static constexpr float ReturnStageSelectCenterY = 445.0f;
	static constexpr float ReturnTitleCenterY = 550.0f;

	//未選択時のボタンサイズ
	static constexpr float NormalButtonWidth = 440.0f;
	static constexpr float NormalButtonHeight = 82.0f;

	static constexpr float SelectedScale = 1.1f; //選択時の拡大倍率

	//選択時のボタンサイズ
	static constexpr float SelectedButtonWidth = NormalButtonWidth * SelectedScale;
	static constexpr float SelectedButtonHeight = NormalButtonHeight * SelectedScale;

	//文字のサイズ
	static constexpr float MenuTextWidth = 400.0f;
	static constexpr float MenuTextHeight = 84.0f;

	ClearResult m_Result{};
	MenuItem m_SelectedItem = MenuItem::ReturnStageSelect;
	GameObject* m_ReturnStageSelectButton = nullptr;
	GameObject* m_ReturnTitleButton = nullptr;
	GameObject* m_ReturnStageSelectText = nullptr;
	GameObject* m_ReturnTitleText = nullptr;
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
	//フェードイン完了後にBGMを再生
	void OnFadeInFinished() override;

public:
	Clear() = default;
	//ステージで集めたStar数を受け取る
	explicit Clear(const ClearResult& result) : m_Result(result) {}

	void Init() override;
	void Update() override;
	bool CanPause() const override { return false; }
	const char* GetStageObjectDataPath() const override { return StageObjectDataPath; }
};
