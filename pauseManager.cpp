//-----------------------------------------------------------
//  [ pauseManager.cpp ]
//  ポーズ状態を管理
//-----------------------------------------------------------
#include "main.h"
#include "pauseManager.h"

#include <cstdint>
#include <fstream>

#include "imgui.h"

#include "audioManager.h"
#include "imguiManager.h"
#include "input.h"
#include "texture.h"

bool PauseManager::m_IsPaused = false;
bool PauseManager::m_IsPauseLocked = false;
PauseManager::MenuItem PauseManager::m_SelectedItem = PauseManager::MenuItem::Resume;
int PauseManager::m_PreviousMouseX = 0;
int PauseManager::m_PreviousMouseY = 0;

ID3D11ShaderResourceView* PauseManager::m_ButtonTexture = nullptr;
ID3D11ShaderResourceView* PauseManager::m_PauseTextTexture = nullptr;
ID3D11ShaderResourceView* PauseManager::m_ResumeTextTexture = nullptr;
ID3D11ShaderResourceView* PauseManager::m_ReturnStageSelectTextTexture = nullptr;
ID3D11ShaderResourceView* PauseManager::m_ReturnTitleTextTexture = nullptr;

//補助関数
namespace
{
	//UI画像を安全に読み込む
	ID3D11ShaderResourceView* LoadOptionalUITexture(const char* fileName)
	{
		std::ifstream file(fileName, std::ios::binary);
		if (!file.good())
			return nullptr;

		return Texture::Load(fileName, TextureLoadMode::IgnoreSRGB);
	}

	//テクスチャをImGuiが受け取れる型へ変換
	ImTextureRef ToImTextureRef(ID3D11ShaderResourceView* texture)
	{
		return ImTextureRef(static_cast<ImTextureID>(reinterpret_cast<std::uintptr_t>(texture)));
	}

	//指定した位置を中心とした画像を描画
	void DrawCenteredImage(ImDrawList* drawList, ID3D11ShaderResourceView* texture,
		float centerX, float centerY, float width, float height)
	{
		if (!texture)
			return;

		const ImVec2 minimum{ centerX - width * 0.5f, centerY - height * 0.5f };
		const ImVec2 maximum{ centerX + width * 0.5f, centerY + height * 0.5f };
		drawList->AddImage(ToImTextureRef(texture), minimum, maximum);
	}
}

void PauseManager::Init()
{
	m_ButtonTexture = LoadOptionalUITexture("asset\\texture\\menu_button.png");
	m_PauseTextTexture = LoadOptionalUITexture("asset\\texture\\text_pause.png");
	m_ResumeTextTexture = LoadOptionalUITexture("asset\\texture\\text_resume.png");
	m_ReturnStageSelectTextTexture = LoadOptionalUITexture("asset\\texture\\text_return_stage_select.png");
	m_ReturnTitleTextTexture = LoadOptionalUITexture("asset\\texture\\text_return_title.png");
	Reset();
}

PauseAction PauseManager::Update(bool canPause)
{
	//ゲームシーン以外と演出などによるロック中はポーズ機能オフ
	if (!canPause || m_IsPauseLocked)
	{
		m_IsPaused = false;
		return PauseAction::None;
	}

	const bool keyboardCaptured = ImGuiManager::WantsKeyboardCapture();
	if (!m_IsPaused)
	{
		if (!keyboardCaptured && Input::GetKeyTrigger(VK_ESCAPE))
		{
			AudioManager::PlaySE(SEId::Click);
			Open();
		}

		return PauseAction::None;
	}

	//キーボード操作
	if (!keyboardCaptured)
	{
		if (Input::GetKeyTrigger(VK_ESCAPE))
		{
			AudioManager::PlaySE(SEId::Click);
			m_IsPaused = false;
			return PauseAction::None;
		}

		if (Input::GetKeyTrigger(VK_UP))
			SetSelectedItem(static_cast<MenuItem>((static_cast<int>(m_SelectedItem) + MenuItemCount - 1) % MenuItemCount));
		else if (Input::GetKeyTrigger(VK_DOWN))
			SetSelectedItem(static_cast<MenuItem>((static_cast<int>(m_SelectedItem) + 1) % MenuItemCount));

		if (Input::GetKeyTrigger(VK_RETURN))
			return ConfirmSelection();
	}

	const int mouseX = Input::GetMouseX();
	const int mouseY = Input::GetMouseY();
	const bool mouseMoved = mouseX != m_PreviousMouseX || mouseY != m_PreviousMouseY;
	m_PreviousMouseX = mouseX;
	m_PreviousMouseY = mouseY;

	if (ImGuiManager::WantsMouseCapture())
		return PauseAction::None;

	//マウス操作(カーソルが動いた場合のみ位置に応じて選択項目を変更)
	if (mouseMoved)
	{
		if (IsInsideButton(mouseX, mouseY, ResumeCenterY))
			SetSelectedItem(MenuItem::Resume);
		else if (IsInsideButton(mouseX, mouseY, ReturnStageSelectCenterY))
			SetSelectedItem(MenuItem::ReturnStageSelect);
		else if (IsInsideButton(mouseX, mouseY, ReturnTitleCenterY))
			SetSelectedItem(MenuItem::ReturnTitle);
	}

	if (!Input::GetKeyTrigger(VK_LBUTTON))
		return PauseAction::None;

	//クリックした項目を決定
	if (IsInsideButton(mouseX, mouseY, ResumeCenterY))
	{
		m_SelectedItem = MenuItem::Resume;
		return ConfirmSelection();
	}
	if (IsInsideButton(mouseX, mouseY, ReturnStageSelectCenterY))
	{
		m_SelectedItem = MenuItem::ReturnStageSelect;
		return ConfirmSelection();
	}
	if (IsInsideButton(mouseX, mouseY, ReturnTitleCenterY))
	{
		m_SelectedItem = MenuItem::ReturnTitle;
		return ConfirmSelection();
	}

	return PauseAction::None;
}

void PauseManager::Draw()
{
	if (!m_IsPaused)
		return;

	//ウィンドウ表示領域に合わせて暗幕を重ねる(ImGuiを使用)
	const ImGuiIO& io = ImGui::GetIO();
	ImDrawList* backgroundDrawList = ImGui::GetBackgroundDrawList();
	backgroundDrawList->AddRectFilled(ImVec2(0.0f, 0.0f), io.DisplaySize,
		ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, OverlayOpacity)));

	const float centerX = io.DisplaySize.x * 0.5f;
	ImDrawList* foregroundDrawList = ImGui::GetForegroundDrawList();
	const float resumeScale = m_SelectedItem == MenuItem::Resume ? SelectedScale : 1.0f;
	const float returnStageSelectScale = m_SelectedItem == MenuItem::ReturnStageSelect ? SelectedScale : 1.0f;
	const float returnTitleScale = m_SelectedItem == MenuItem::ReturnTitle ? SelectedScale : 1.0f;

	DrawCenteredImage(foregroundDrawList, m_ButtonTexture, centerX, ResumeCenterY,
		NormalButtonWidth * resumeScale, NormalButtonHeight * resumeScale);
	DrawCenteredImage(foregroundDrawList, m_ButtonTexture, centerX, ReturnStageSelectCenterY,
		NormalButtonWidth * returnStageSelectScale, NormalButtonHeight * returnStageSelectScale);
	DrawCenteredImage(foregroundDrawList, m_ButtonTexture, centerX, ReturnTitleCenterY,
		NormalButtonWidth * returnTitleScale, NormalButtonHeight * returnTitleScale);

	DrawCenteredImage(foregroundDrawList, m_PauseTextTexture, centerX, PauseTextCenterY,
		PauseTextWidth, PauseTextHeight);

	DrawCenteredImage(foregroundDrawList, m_ResumeTextTexture, centerX, ResumeCenterY,
		ResumeTextWidth * resumeScale, MenuTextHeight * resumeScale);
	DrawCenteredImage(foregroundDrawList, m_ReturnStageSelectTextTexture, centerX, ReturnStageSelectCenterY,
		ReturnStageSelectTextWidth * returnStageSelectScale, MenuTextHeight * returnStageSelectScale);

	DrawCenteredImage(foregroundDrawList, m_ReturnTitleTextTexture, centerX, ReturnTitleCenterY,
		ReturnTitleTextWidth * returnTitleScale, MenuTextHeight * returnTitleScale);
}

void PauseManager::Open()
{
	m_IsPaused = true;
	m_SelectedItem = MenuItem::Resume;
	m_PreviousMouseX = Input::GetMouseX();
	m_PreviousMouseY = Input::GetMouseY();
}

void PauseManager::SetSelectedItem(MenuItem item)
{
	//選択が変わった場合のみ処理を行う
	if (m_SelectedItem == item)
		return;

	m_SelectedItem = item;
	AudioManager::PlaySE(SEId::MenuMove);
}

PauseAction PauseManager::ConfirmSelection()
{
	AudioManager::PlaySE(SEId::Click);

	if (m_SelectedItem == MenuItem::Resume)
	{
		m_IsPaused = false;
		return PauseAction::None;
	}
	if (m_SelectedItem == MenuItem::ReturnStageSelect)
		return PauseAction::ReturnStageSelect;

	return PauseAction::ReturnTitle;
}

bool PauseManager::IsInsideButton(int mouseX, int mouseY, float centerY)
{
	const float centerX = ImGui::GetIO().DisplaySize.x * 0.5f;
	const float halfWidth = NormalButtonWidth * SelectedScale * 0.5f;
	const float halfHeight = NormalButtonHeight * SelectedScale * 0.5f;
	return mouseX >= centerX - halfWidth && mouseX <= centerX + halfWidth &&
		mouseY >= centerY - halfHeight && mouseY <= centerY + halfHeight;
}

void PauseManager::SetPauseLocked(bool locked)
{
	m_IsPauseLocked = locked;

	if (m_IsPauseLocked)
		m_IsPaused = false;
}

void PauseManager::Reset()
{
	m_IsPaused = false;
	m_IsPauseLocked = false;
	m_SelectedItem = MenuItem::Resume;
}
