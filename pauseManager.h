//-----------------------------------------------------------
//  [ pauseManager.h ]
//  ポーズ状態を管理
//-----------------------------------------------------------
#pragma once

struct ID3D11ShaderResourceView;

//Managerに渡す処理結果
enum class PauseAction
{
	None,
	ReturnStageSelect,
	ReturnTitle,
};

class PauseManager
{
private:
	//ポーズメニューの選択肢
	enum class MenuItem
	{
		Resume,
		ReturnStageSelect,
		ReturnTitle,
	};
	static constexpr int MenuItemCount = 3;

	static constexpr float OverlayOpacity = 0.5f; //ポーズ中に重ねる暗幕の不透明度

	//各UIの中心Y座標
	static constexpr float PauseTextCenterY = 115.0f;
	static constexpr float ResumeCenterY = 290.0f;
	static constexpr float ReturnStageSelectCenterY = 395.0f;
	static constexpr float ReturnTitleCenterY = 500.0f;

	//未選択時のボタンサイズ
	static constexpr float NormalButtonWidth = 440.0f;
	static constexpr float NormalButtonHeight = 82.0f;
	static constexpr float SelectedScale = 1.1f; //選択時の拡大倍率

	//各文字画像の基本サイズ
	static constexpr float PauseTextWidth = 550.0f;
	static constexpr float PauseTextHeight = 134.474f;
	static constexpr float ResumeTextWidth = 230.0f;
	static constexpr float ReturnStageSelectTextWidth = 400.0f;
	static constexpr float ReturnTitleTextWidth = 400.0f;
	static constexpr float MenuTextHeight = 84.0f;

	static bool m_IsPaused;
	static bool m_IsPauseLocked; //演出中などにポーズ操作を禁止するか
	static MenuItem m_SelectedItem;

	//マウス移動検出用の前フレーム座標
	static int m_PreviousMouseX;
	static int m_PreviousMouseY;

	static ID3D11ShaderResourceView* m_ButtonTexture;
	static ID3D11ShaderResourceView* m_PauseTextTexture;
	static ID3D11ShaderResourceView* m_ResumeTextTexture;
	static ID3D11ShaderResourceView* m_ReturnStageSelectTextTexture;
	static ID3D11ShaderResourceView* m_ReturnTitleTextTexture;

	static void Open();
	static void SetSelectedItem(MenuItem item);
	static PauseAction ConfirmSelection();
	static bool IsInsideButton(int mouseX, int mouseY, float centerY);

public:
	static void Init();
	static PauseAction Update(bool canPause);
	static void Draw();

	static void SetPauseLocked(bool locked);
	static void Reset();
	static bool IsPaused() { return m_IsPaused; }
};
