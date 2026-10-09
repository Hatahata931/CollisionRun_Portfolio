//-----------------------------------------------------------
//  [ title.cpp ]
//  タイトル画面
//-----------------------------------------------------------
#include "main.h"
#include "title.h"
#include "audioManager.h"
#include "box.h"
#include "camera.h"
#include "displayCharacter.h"
#include "imGuiManager.h"
#include "input.h"
#include "manager.h"
#include "movingPlatform.h"
#include "polygon2D.h"
#include "scene.h"
#include "sky.h"
#include "stageSelect.h"

void Title::Init()
{
	CreateSceneObjects();

	//タイトルロゴ
	AddGameObject<Polygon2D>(Scene::OBJECT2D)->Init(70.0f, 90.0f, 600.0f, 167.0378f,
		"asset\\texture\\title_logo.png");

	//操作案内
	AddGameObject<Polygon2D>(Scene::OBJECT2D)->Init(180.0f, 400.0f, 350.0f, 115.97f,
		"asset\\texture\\text_enter_click_start.png");

	AudioManager::PlayBGM(BGMId::Title);
}

//タイトル画面の3Dシーンを生成
void Title::CreateSceneObjects()
{
	auto camera = AddGameObject<Camera>(Scene::CAMERA);
	camera->SetPosition({ 0.0f, 1.0f, -4.98f });
	camera->SetTarget({ -0.7f, 0.0f, 0.0f });

	AddGameObject<Sky>(Scene::CAMERA)->SetScale({ 100.0f, 100.0f, 100.0f });
	LoadStageObjects(GetStageObjectDataPath());

	//ユニティちゃん
	auto displayCharacter = AddGameObject<DisplayCharacter>(Scene::OBJECT3D);
	displayCharacter->SetPosition({ 1.1f, -0.2999f, -2.0f });
	displayCharacter->SetRotation({ 0.0f, 3.5f, 0.0f });
	displayCharacter->SetAnimationPreset(DisplayCharacter::AnimationPreset::Idle);

	CreateBoxTexture({ 3.8f, 0.05f, 0.25f }, { 0.5f, 0.5f, 0.5f },
		"asset\\texture\\mechanical_stone_block.png", { -0.2f, 0.11f, 0.0f });

	auto movingPlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
	movingPlatform->SetPosition({ 20.7f, 5.2f, 10.0f });
	movingPlatform->SetScale({ 1.5f, 1.5f, 1.5f });
	movingPlatform->SetRotation({ 0.0f, -1.62316f, 0.0f });
	movingPlatform->SetEndPosition({ 20.7f, 5.2f, 26.8f });
	movingPlatform->SetSpeed(0.003f);
	movingPlatform->SetWaitFrames(180);
}

void Title::Update()
{
	Scene::Update();

	//エディター操作中の入力では開始しないよう、入力デバイスごとに判定する
	const bool keyboardStart = !ImGuiManager::WantsKeyboardCapture() &&
		Input::GetKeyTrigger(VK_RETURN);
	const bool mouseStart = !ImGuiManager::WantsMouseCapture() &&
		Input::GetKeyTrigger(VK_LBUTTON);

	if (keyboardStart || mouseStart)
	{
		AudioManager::PlaySE(SEId::Click);
		Manager::SetScene<StageSelect>();
	}
}
