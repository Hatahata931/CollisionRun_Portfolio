//-----------------------------------------------------------
//  [ gameOver.cpp ]
//  ゲームオーバー画面
//-----------------------------------------------------------
#include "main.h"
#include "gameOver.h"
#include "audioManager.h"
#include "camera.h"
#include "displayCharacter.h"
#include "gameObject.h"
#include "imGuiManager.h"
#include "input.h"
#include "manager.h"
#include "polygonRendererComponent.h"
#include "scene.h"
#include "sky.h"
#include "stageSelect.h"
#include "title.h"

void GameOver::Init()
{
	AudioManager::StopBGM();

	CreateSceneObjects();
	CreateMenu();

	m_PreviousMouseX = Input::GetMouseX();
	m_PreviousMouseY = Input::GetMouseY();
}

//フェードイン完了後に効果音を再生
void GameOver::OnFadeInFinished()
{
	AudioManager::PlaySE(SEId::GameOver);
}

void GameOver::Update()
{
	Scene::Update();

	//キーボード操作(ImGui操作中は除外)
	if (!ImGuiManager::WantsKeyboardCapture())
	{
		if (Input::GetKeyTrigger(VK_UP))
			SetSelectedItem(static_cast<MenuItem>((static_cast<int>(m_SelectedItem) + MenuItemCount - 1) % MenuItemCount));
		else if (Input::GetKeyTrigger(VK_DOWN))
			SetSelectedItem(static_cast<MenuItem>((static_cast<int>(m_SelectedItem) + 1) % MenuItemCount));

		if (Input::GetKeyTrigger(VK_RETURN))
		{
			ConfirmSelection();
			return;
		}
	}

	const int mouseX = Input::GetMouseX();
	const int mouseY = Input::GetMouseY();
	const bool mouseMoved = mouseX != m_PreviousMouseX || mouseY != m_PreviousMouseY;
	m_PreviousMouseX = mouseX;
	m_PreviousMouseY = mouseY;

	if (ImGuiManager::WantsMouseCapture())
		return;

	//マウス操作(カーソルが動いた場合のみ位置に応じて選択項目を変更)
	if (mouseMoved)
	{
		if (IsInsideButton(mouseX, mouseY, RetryCenterY))
			SetSelectedItem(MenuItem::Retry);
		else if (IsInsideButton(mouseX, mouseY, ReturnStageSelectCenterY))
			SetSelectedItem(MenuItem::ReturnStageSelect);
		else if (IsInsideButton(mouseX, mouseY, ReturnTitleCenterY))
			SetSelectedItem(MenuItem::ReturnTitle);
	}

	if (!Input::GetKeyTrigger(VK_LBUTTON))
		return;

	//クリックされた項目を選択して決定
	if (IsInsideButton(mouseX, mouseY, RetryCenterY))
	{
		SetSelectedItem(MenuItem::Retry);
		ConfirmSelection();
	}
	else if (IsInsideButton(mouseX, mouseY, ReturnStageSelectCenterY))
	{
		SetSelectedItem(MenuItem::ReturnStageSelect);
		ConfirmSelection();
	}
	else if (IsInsideButton(mouseX, mouseY, ReturnTitleCenterY))
	{
		SetSelectedItem(MenuItem::ReturnTitle);
		ConfirmSelection();
	}
}

//画面座標を中心として2D画像を生成
GameObject* GameOver::CreateFixedImage(const char* texturePath, Vector3 position, Vector3 scale)
{
	auto object = AddGameObject<GameObject>(Scene::OBJECT2D);
	object->AddComponent<PolygonRendererComponent>()->InitScreenSpaceCentered(texturePath);
	object->SetPosition(position);
	object->SetScale(scale);
	return object;
}

void GameOver::CreateSceneObjects()
{
	auto camera = AddGameObject<Camera>(Scene::CAMERA);
	camera->SetPosition({ 1.0f, 1.5f, -3.8f });
	camera->SetTarget({ 1.5f, 0.6f, 0.0f });
	AddGameObject<Sky>(Scene::CAMERA)->SetScale({ 100.0f, 100.0f, 100.0f });
	LoadStageObjects(GetStageObjectDataPath());

	auto displayCharacter = AddGameObject<DisplayCharacter>(Scene::OBJECT3D);
	displayCharacter->SetPosition({ 2.8f, 0.001f, -1.0f });
	displayCharacter->SetRotation({ 0.0f, 3.6f, 0.0f });
	displayCharacter->SetAnimationPreset(DisplayCharacter::AnimationPreset::KneelDown);
}

void GameOver::CreateMenu()
{
	//ゲームオーバー文字
	CreateFixedImage("asset\\texture\\text_game_over.png",
		{ MenuCenterX, 140.0f, 0.0f }, { 510.0f, 100.0f, 1.0f });

	//ボタン
	m_RetryButton = CreateFixedImage("asset\\texture\\menu_button.png",
		{ MenuCenterX, RetryCenterY, 0.0f }, { NormalButtonWidth, NormalButtonHeight, 1.0f });
	m_ReturnStageSelectButton = CreateFixedImage("asset\\texture\\menu_button.png",
		{ MenuCenterX, ReturnStageSelectCenterY, 0.0f }, { NormalButtonWidth, NormalButtonHeight, 1.0f });
	m_ReturnTitleButton = CreateFixedImage("asset\\texture\\menu_button.png",
		{ MenuCenterX, ReturnTitleCenterY, 0.0f }, { NormalButtonWidth, NormalButtonHeight, 1.0f });

	//ボタンテキスト
	m_RetryText = CreateFixedImage("asset\\texture\\text_retry.png",
		{ MenuCenterX, RetryCenterY, 0.0f }, { RetryTextWidth, TextHeight, 1.0f });
	m_ReturnStageSelectText = CreateFixedImage("asset\\texture\\text_return_stage_select.png",
		{ MenuCenterX, ReturnStageSelectCenterY, 0.0f }, { ReturnStageSelectTextWidth, TextHeight, 1.0f });
	m_ReturnTitleText = CreateFixedImage("asset\\texture\\text_return_title.png",
		{ MenuCenterX, ReturnTitleCenterY, 0.0f }, { ReturnTitleTextWidth, TextHeight, 1.0f });

	UpdateMenuVisuals();
}

void GameOver::SetSelectedItem(MenuItem item)
{
	//選択が変わった場合のみ処理を行う
	if (m_SelectedItem == item)
		return;

	m_SelectedItem = item;
	AudioManager::PlaySE(SEId::MenuMove);
	UpdateMenuVisuals();
}

//選択中のボタン背景と文字を拡大
void GameOver::UpdateMenuVisuals()
{
	if (!m_RetryButton || !m_ReturnStageSelectButton || !m_ReturnTitleButton ||
		!m_RetryText || !m_ReturnStageSelectText || !m_ReturnTitleText)
		return;

	const Vector3 selectedButtonScale{ SelectedButtonWidth, SelectedButtonHeight, 1.0f };
	const Vector3 normalButtonScale{ NormalButtonWidth, NormalButtonHeight, 1.0f };
	m_RetryButton->SetScale(m_SelectedItem == MenuItem::Retry ? selectedButtonScale : normalButtonScale);
	m_ReturnStageSelectButton->SetScale(m_SelectedItem == MenuItem::ReturnStageSelect ? selectedButtonScale : normalButtonScale);
	m_ReturnTitleButton->SetScale(m_SelectedItem == MenuItem::ReturnTitle ? selectedButtonScale : normalButtonScale);

	const float retryTextScale = m_SelectedItem == MenuItem::Retry ? SelectedScale : 1.0f;
	const float returnStageSelectTextScale = m_SelectedItem == MenuItem::ReturnStageSelect ? SelectedScale : 1.0f;
	const float returnTitleTextScale = m_SelectedItem == MenuItem::ReturnTitle ? SelectedScale : 1.0f;
	m_RetryText->SetScale({ RetryTextWidth * retryTextScale, TextHeight * retryTextScale, 1.0f });
	m_ReturnStageSelectText->SetScale({ ReturnStageSelectTextWidth * returnStageSelectTextScale,
		TextHeight * returnStageSelectTextScale, 1.0f });
	m_ReturnTitleText->SetScale({ ReturnTitleTextWidth * returnTitleTextScale,
		TextHeight * returnTitleTextScale, 1.0f });
}

void GameOver::ConfirmSelection()
{
	AudioManager::PlaySE(SEId::Click);

	if (m_SelectedItem == MenuItem::Retry)
		Manager::SetStage(m_RetryStage);
	else if (m_SelectedItem == MenuItem::ReturnStageSelect)
		Manager::SetScene<StageSelect>();
	else
		Manager::SetScene<Title>();
}

bool GameOver::IsInsideButton(int mouseX, int mouseY, float centerY) const
{
	const float halfWidth = SelectedButtonWidth * 0.5f;
	const float halfHeight = SelectedButtonHeight * 0.5f;
	return mouseX >= MenuCenterX - halfWidth && mouseX <= MenuCenterX + halfWidth &&
		mouseY >= centerY - halfHeight && mouseY <= centerY + halfHeight;
}
