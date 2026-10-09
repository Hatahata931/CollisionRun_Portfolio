//-----------------------------------------------------------
//  [ stageSelect.cpp ]
//  ステージ選択画面
//-----------------------------------------------------------
#include "main.h"
#include "stageSelect.h"

#include "audioManager.h"
#include "camera.h"
#include "gameObject.h"
#include "imGuiManager.h"
#include "input.h"
#include "manager.h"
#include "polygonRendererComponent.h"
#include "scene.h"
#include "sky.h"
#include "title.h"

namespace
{
	constexpr StageId StageIds[] =
	{
		StageId::Stage1,
		StageId::Stage2,
		StageId::Stage3,
	};

	constexpr const char* PreviewTexturePaths[] =
	{
		"asset\\texture\\preview_stage1.png",
		"asset\\texture\\preview_stage2.png",
		"asset\\texture\\preview_stage3.png",
	};

	constexpr const char* TitleTexturePaths[] =
	{
		"asset\\texture\\text_stage_1.png",
		"asset\\texture\\text_stage_2.png",
		"asset\\texture\\text_stage_3.png",
	};

	constexpr const char* DifficultyTexturePaths[] =
	{
		"asset\\texture\\text_stage_difficulty_1.png",
		"asset\\texture\\text_stage_difficulty_2.png",
		"asset\\texture\\text_stage_difficulty_3.png",
	};
}

void StageSelect::Init()
{
	CreateSceneObjects();
	CreateMenu();

	m_PreviousMouseX = Input::GetMouseX();
	m_PreviousMouseY = Input::GetMouseY();

	AudioManager::PlayBGM(BGMId::Title);
}

void StageSelect::Update()
{
	Scene::Update();

	//キーボード操作
	if (!ImGuiManager::WantsKeyboardCapture())
	{
		if (Input::GetKeyTrigger(VK_ESCAPE))
		{
			AudioManager::PlaySE(SEId::Click);
			Manager::SetScene<Title>();
			return;
		}

		if (Input::GetKeyTrigger(VK_LEFT))
			SetSelectedIndex((m_SelectedIndex + CardCount - 1) % CardCount);
		else if (Input::GetKeyTrigger(VK_RIGHT))
			SetSelectedIndex((m_SelectedIndex + 1) % CardCount);

		if (Input::GetKeyTrigger(VK_RETURN))
		{
			ConfirmSelection();
			return;
		}
	}

	const int mouseX = Input::GetMouseX();
	const int mouseY = Input::GetMouseY();
	//キーボードで選んだカードを静止中のカーソル位置で上書きしない
	const bool mouseMoved = mouseX != m_PreviousMouseX || mouseY != m_PreviousMouseY;
	m_PreviousMouseX = mouseX;
	m_PreviousMouseY = mouseY;

	if (ImGuiManager::WantsMouseCapture())
		return;

	//マウス操作
	const int hoveredIndex = GetHoveredCardIndex(mouseX, mouseY);
	if (mouseMoved && hoveredIndex >= 0)
		SetSelectedIndex(hoveredIndex);

	if (Input::GetKeyTrigger(VK_LBUTTON) && hoveredIndex >= 0)
	{
		SetSelectedIndex(hoveredIndex);
		ConfirmSelection();
	}
}

//画面座標を中心とする2D画像を生成
GameObject* StageSelect::CreateFixedImage(const char* texturePath, Vector3 position, Vector3 scale)
{
	auto object = AddGameObject<GameObject>(Scene::OBJECT2D);
	object->AddComponent<PolygonRendererComponent>()->InitScreenSpaceCentered(texturePath);
	object->SetPosition(position);
	object->SetScale(scale);
	return object;
}

void StageSelect::CreateSceneObjects()
{
	auto camera = AddGameObject<Camera>(Scene::CAMERA);
	camera->SetPosition({ 0.0f, 1.0f, -4.98f });
	camera->SetTarget({ -0.7f, 0.0f, 0.0f });

	AddGameObject<Sky>(Scene::CAMERA)->SetScale({ 100.0f, 100.0f, 100.0f });
	LoadStageObjects(GetStageObjectDataPath());
}

void StageSelect::CreateMenu()
{
	//3D背景を淡くしてカードを見やすくする
	auto overlay = AddGameObject<GameObject>(Scene::OBJECT2D);
	auto overlayRenderer = overlay->AddComponent<PolygonRendererComponent>();
	overlayRenderer->InitScreenSpaceCentered(nullptr);
	overlayRenderer->SetColor(0.65f, 0.75f, 0.8f, 0.25f);
	overlay->SetPosition({ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.5f, 0.0f });
	overlay->SetScale({ static_cast<float>(SCREEN_WIDTH), static_cast<float>(SCREEN_HEIGHT), 1.0f });

	//「ステージセレクト」の文字
	CreateFixedImage("asset\\texture\\text_stage_select.png",
		{ SCREEN_WIDTH * 0.5f, 82.0f, 0.0f }, { 760.0f, 124.0f, 1.0f });

	//カード3枚
	for (int i = 0; i < CardCount; ++i)
	{
		StageCard& card = m_Cards[i];
		card.Id = StageIds[i];
		card.Panel = CreateFixedImage("asset\\texture\\stage_select_panel.png",
			{}, { CardWidth, CardHeight, 1.0f });
		card.Preview = CreateFixedImage(PreviewTexturePaths[i],
			{}, { PreviewWidth, PreviewHeight, 1.0f });
		card.Title = CreateFixedImage(TitleTexturePaths[i],
			{}, { TitleWidth, TitleHeight, 1.0f });
		card.Difficulty = CreateFixedImage(DifficultyTexturePaths[i],
			{}, { DifficultyWidth, DifficultyHeight, 1.0f });
	}

	//選択枠
	m_SelectionFrame = CreateFixedImage("asset\\texture\\stage_select_frame.png",
		{}, { CardWidth, CardHeight, 1.0f });

	//操作説明
	CreateFixedImage("asset\\texture\\text_stage_select_controls.png",
		{ SCREEN_WIDTH * 0.5f, 665.0f, 0.0f }, { 1050.0f, 58.0f, 1.0f });

	UpdateCardVisuals();
}

void StageSelect::SetSelectedIndex(int index)
{
	if (m_SelectedIndex == index)
		return;

	m_SelectedIndex = index;
	AudioManager::PlaySE(SEId::MenuMove);
	UpdateCardVisuals();
}

void StageSelect::UpdateCardVisuals()
{
	//カード3枚の見た目を更新
	for (int i = 0; i < CardCount; ++i)
	{
		const float scale = i == m_SelectedIndex ? SelectedScale : 1.0f;
		const float centerX = GetCardCenterX(i);
		StageCard& card = m_Cards[i];

		card.Panel->SetPosition({ centerX, CardCenterY, 0.0f });
		card.Panel->SetScale({ CardWidth * scale, CardHeight * scale, 1.0f });
		card.Preview->SetPosition({ centerX, PreviewCenterY, 0.0f });
		card.Preview->SetScale({ PreviewWidth * scale, PreviewHeight * scale, 1.0f });
		card.Title->SetPosition({ centerX, TitleCenterY, 0.0f });
		card.Title->SetScale({ TitleWidth * scale, TitleHeight * scale, 1.0f });
		card.Difficulty->SetPosition({ centerX, DifficultyCenterY, 0.0f });
		card.Difficulty->SetScale({ DifficultyWidth * scale, DifficultyHeight * scale, 1.0f });
	}

	//選択枠の位置を更新
	const float frameScale = SelectedScale;
	m_SelectionFrame->SetPosition({ GetCardCenterX(m_SelectedIndex), CardCenterY, 0.0f });
	m_SelectionFrame->SetScale(
		{ (CardWidth + SelectionFrameMargin) * frameScale,
		(CardHeight + SelectionFrameMargin) * frameScale, 1.0f });
}

void StageSelect::ConfirmSelection()
{
	AudioManager::PlaySE(SEId::Click);
	Manager::SetStage(m_Cards[m_SelectedIndex].Id);
}

//マウスカーソルが重なっているカード番号を取得
int StageSelect::GetHoveredCardIndex(int mouseX, int mouseY) const
{
	const float halfWidth = CardWidth * SelectedScale * 0.5f;
	const float halfHeight = CardHeight * SelectedScale * 0.5f;

	for (int i = 0; i < CardCount; ++i)
	{
		const float centerX = GetCardCenterX(i);
		if (mouseX >= centerX - halfWidth && mouseX <= centerX + halfWidth &&
			mouseY >= CardCenterY - halfHeight && mouseY <= CardCenterY + halfHeight)
			return i;
	}

	return -1;
}

float StageSelect::GetCardCenterX(int index) const
{
	return FirstCardCenterX + CardSpacing * index;
}
