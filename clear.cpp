//-----------------------------------------------------------
//  [ clear.cpp ]
//  クリア画面
//-----------------------------------------------------------
#include "main.h"
#include "clear.h"
#include "audioManager.h"
#include "box.h"
#include "camera.h"
#include "displayCharacter.h"
#include "goal.h"
#include "imGuiManager.h"
#include "input.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "polygonRendererComponent.h"
#include "scene.h"
#include "sky.h"
#include "stageSelect.h"
#include "starCounter.h"
#include "title.h"

void Clear::Init()
{
	AudioManager::StopBGM();

	CreateSceneObjects();

	//ステージクリアの文字
	auto clearText = AddGameObject<GameObject>(Scene::OBJECT2D);
	clearText->AddComponent<PolygonRendererComponent>()->InitScreenSpaceCentered(
		"asset\\texture\\text_stage_clear.png");
	clearText->SetPosition({ 360.0f, 140.0f, 0.0f });
	clearText->SetScale({ 600.0f, 121.21212f, 1.0f });

	//Star数の結果の後ろのパネル
	auto resultPanel = AddGameObject<GameObject>(Scene::OBJECT2D);
	resultPanel->AddComponent<PolygonRendererComponent>()->InitScreenSpaceCentered(
		"asset\\texture\\star_result_panel.png");
	resultPanel->SetPosition({ 360.0f, 290.0f, 0.0f });
	resultPanel->SetScale({ 500.0f, 125.0f, 1.0f });

	//StarCounterを使ってステージでのStar数の結果を表示する
	auto starCounter = AddGameObject<StarCounter>(Scene::OBJECT2D);
	starCounter->SetPosition({ 170.4f, 260.0f, 0.0f });
	starCounter->SetScale({ 1.2f, 1.2f, 1.0f });
	starCounter->SetMaxValue(m_Result.TotalStars);
	starCounter->SetValue(m_Result.CollectedStars);

	CreateMenu();
	m_PreviousMouseX = Input::GetMouseX();
	m_PreviousMouseY = Input::GetMouseY();
}

//クリア画面の3Dシーンを生成
void Clear::CreateSceneObjects()
{
	auto camera = AddGameObject<Camera>(Scene::CAMERA);
	camera->SetPosition({ 1.0f, 1.7f, -3.8f });
	camera->SetTarget({ 1.5f, 0.6f, 0.0f });
	AddGameObject<Sky>(Scene::CAMERA)->SetScale({ 100.0f, 100.0f, 100.0f });
	LoadStageObjects(GetStageObjectDataPath());

	auto goal = AddGameObject<Goal>(Scene::OBJECT3D);
	goal->SetPosition({ 3.6f, 0.2f, -0.2f });
	goal->SetRotation({ 0.0f, XMConvertToRadians(210.0f), 0.0f });

	//ゴールの上の星
	auto goalStar = CreateBoxModel({ 3.6f, 0.5f, -0.2f }, { 0.65f, 0.1f, 0.65f }, "asset\\model\\star_goal.obj");
	goalStar->SetCollisionEnabled(false);
	goalStar->SetRotationVelocity({ 0.0f, 0.01f, 0.0f });
	auto renderer = goalStar->GetComponent<ModelRendererComponent>();
	if (renderer)
		renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");

	//ユニティちゃん
	auto displayCharacter = AddGameObject<DisplayCharacter>(Scene::OBJECT3D);
	displayCharacter->SetPosition({ 3.0f, 0.6f, -1.0f });
	displayCharacter->SetRotation({ 0.0f, 3.7f, 0.0f });
	displayCharacter->SetAnimationPreset(DisplayCharacter::AnimationPreset::Salute);
}

//フェードイン完了後にBGMを再生
void Clear::OnFadeInFinished()
{
	AudioManager::PlayBGM(BGMId::Clear);
}

void Clear::Update()
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
		if (IsInsideButton(mouseX, mouseY, ReturnStageSelectCenterY))
			SetSelectedItem(MenuItem::ReturnStageSelect);
		else if (IsInsideButton(mouseX, mouseY, ReturnTitleCenterY))
			SetSelectedItem(MenuItem::ReturnTitle);
	}

	if (!Input::GetKeyTrigger(VK_LBUTTON))
		return;

	//クリックされた項目を選択して決定
	if (IsInsideButton(mouseX, mouseY, ReturnStageSelectCenterY))
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

GameObject* Clear::CreateFixedImage(const char* texturePath, Vector3 position, Vector3 scale)
{
	auto object = AddGameObject<GameObject>(Scene::OBJECT2D);
	object->AddComponent<PolygonRendererComponent>()->InitScreenSpaceCentered(texturePath);
	object->SetPosition(position);
	object->SetScale(scale);
	return object;
}

void Clear::CreateMenu()
{
	m_ReturnStageSelectButton = CreateFixedImage("asset\\texture\\menu_button.png",
		{ MenuCenterX, ReturnStageSelectCenterY, 0.0f }, { NormalButtonWidth, NormalButtonHeight, 1.0f });
	m_ReturnTitleButton = CreateFixedImage("asset\\texture\\menu_button.png",
		{ MenuCenterX, ReturnTitleCenterY, 0.0f }, { NormalButtonWidth, NormalButtonHeight, 1.0f });
	m_ReturnStageSelectText = CreateFixedImage("asset\\texture\\text_return_stage_select.png",
		{ MenuCenterX, ReturnStageSelectCenterY, 0.0f }, { MenuTextWidth, MenuTextHeight, 1.0f });
	m_ReturnTitleText = CreateFixedImage("asset\\texture\\text_return_title.png",
		{ MenuCenterX, ReturnTitleCenterY, 0.0f }, { MenuTextWidth, MenuTextHeight, 1.0f });
	UpdateMenuVisuals();
}

void Clear::SetSelectedItem(MenuItem item)
{
	//選択が変わった場合のみ処理を行う
	if (m_SelectedItem == item)
		return;

	m_SelectedItem = item;
	AudioManager::PlaySE(SEId::MenuMove);
	UpdateMenuVisuals();
}

//選択中のボタン背景と文字を拡大
void Clear::UpdateMenuVisuals()
{
	if (!m_ReturnStageSelectButton || !m_ReturnTitleButton ||
		!m_ReturnStageSelectText || !m_ReturnTitleText)
		return;

	const bool stageSelectSelected = m_SelectedItem == MenuItem::ReturnStageSelect;
	const Vector3 selectedButtonScale{ SelectedButtonWidth, SelectedButtonHeight, 1.0f };
	const Vector3 normalButtonScale{ NormalButtonWidth, NormalButtonHeight, 1.0f };
	m_ReturnStageSelectButton->SetScale(stageSelectSelected ? selectedButtonScale : normalButtonScale);
	m_ReturnTitleButton->SetScale(stageSelectSelected ? normalButtonScale : selectedButtonScale);

	const float stageSelectTextScale = stageSelectSelected ? SelectedScale : 1.0f;
	const float titleTextScale = stageSelectSelected ? 1.0f : SelectedScale;
	m_ReturnStageSelectText->SetScale({ MenuTextWidth * stageSelectTextScale,
		MenuTextHeight * stageSelectTextScale, 1.0f });
	m_ReturnTitleText->SetScale({ MenuTextWidth * titleTextScale,
		MenuTextHeight * titleTextScale, 1.0f });
}

void Clear::ConfirmSelection()
{
	AudioManager::PlaySE(SEId::Click);
	if (m_SelectedItem == MenuItem::ReturnStageSelect)
		Manager::SetScene<StageSelect>();
	else
		Manager::SetScene<Title>();
}

bool Clear::IsInsideButton(int mouseX, int mouseY, float centerY) const
{
	const float halfWidth = SelectedButtonWidth * 0.5f;
	const float halfHeight = SelectedButtonHeight * 0.5f;
	return mouseX >= MenuCenterX - halfWidth && mouseX <= MenuCenterX + halfWidth &&
		mouseY >= centerY - halfHeight && mouseY <= centerY + halfHeight;
}
