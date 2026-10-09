//-----------------------------------------------------------
//  [ stage2.cpp ]
//  ステージ2
//-----------------------------------------------------------
#include "main.h"
#include "stage2.h"
#include "stageId.h"

#include "audioManager.h"
#include "camera.h"
#include "gameOver.h"
#include "gate.h"
#include "goal.h"
#include "box.h"
#include "wind.h"
#include "gateRequirementDisplay.h"
#include "gateRequirementNotice.h"
#include "life.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "player.h"
#include "scene.h"
#include "sky.h"
#include "star.h"
#include "rotatingBar.h"
#include "starCounter.h"

namespace
{
	constexpr const char* ShortRotatingBarModelPath = "asset\\model\\rotating_bar_short.obj";
	constexpr const char* MediumRotatingBarModelPath = "asset\\model\\rotating_bar_medium.obj";
	constexpr const char* LongRotatingBarModelPath = "asset\\model\\rotating_bar_long.obj";
}

void Stage2::Init()
{
	//Star数表示
	auto starCounter = AddGameObject<StarCounter>(Scene::OBJECT2D);
	starCounter->SetPosition({ 930.0f, 25.0f, 0.0f });
	AddGameObject<Life>(Scene::OBJECT2D)->SetPosition({ 30.0f, 25.0f, 0.0f });
	AddGameObject<GateRequirementDisplay>(Scene::OBJECT2D)->SetPosition({ 930.0f, 80.0f, 0.0f });
	AddGameObject<GateRequirementNotice>(Scene::OBJECT2D)->SetPosition({ SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT * 0.25f, 0.0f });

	AddGameObject<Camera>(Scene::CAMERA);
	AddGameObject<Sky>(Scene::CAMERA)->SetScale({ 100.0f, 100.0f, 100.0f });

	LoadStageObjects(GetStageObjectDataPath());
	CreateGimmicks();
	CreatePlayer();

	starCounter->SetMaxValue(static_cast<int>(GetGameObjects<Star>().size()));

	AudioManager::PlayBGM(BGMId::Stage);
}

void Stage2::Update()
{
	Scene::Update();

	Player* player = GetGameObject<Player>();
	if (!player)
		Manager::SetScene<GameOver>(StageId::Stage2);
}

void Stage2::CreateGimmicks()
{
	auto wind = AddGameObject<Wind>(Scene::OBJECT3D);
	wind->SetPosition({ 29.1f, 12.5f, 55.7f });
	wind->SetScale({ 21.5f, 3.5f, 6.0f });
	wind->SetRotation({ 0.0f, 0.0f, XMConvertToRadians(15.0f)});
	wind->SetWindDirection({ 0.0f, 0.0f, 1.0f });
	wind->SetWindPower(0.005f);

	auto rotatingBar = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar->SetPosition({ 71.5f, 14.15f, 55.7f });
	rotatingBar->SetModel(MediumRotatingBarModelPath);
	rotatingBar->SetColliderHalfSize({ 3.8f, 0.25f, 0.25f });
	rotatingBar->SetRotationAxis({ 0.0f, 1.0f, 0.0f });
	rotatingBar->SetRotationSpeed(0.01f);

	auto gate = AddGameObject<Gate>(Scene::OBJECT3D);
	gate->SetPosition({ 113.0f, 10.509f, 96.5f });
	gate->SetRequiredItemCount(30);

	auto rotatingBar2 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar2->SetPosition({ 116.0f, 9.459f, 106.5f });
	rotatingBar2->SetModel(ShortRotatingBarModelPath);
	rotatingBar2->SetColliderHalfSize({ 2.8f, 0.25f, 0.25f });
	rotatingBar2->SetRotationAxis({ 0.0f, 1.0f, 0.0f });
	rotatingBar2->SetRotationSpeed(0.01f);

	auto rotatingBar3 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar3->SetPosition({ 110.0f, 9.459f, 113.5f });
	rotatingBar3->SetModel(ShortRotatingBarModelPath);
	rotatingBar3->SetColliderHalfSize({ 2.8f, 0.25f, 0.25f });
	rotatingBar3->SetRotationAxis({ 0.0f, -1.0f, 0.0f });
	rotatingBar3->SetRotationSpeed(0.010f);

	auto rotatingBar4 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar4->SetPosition({ 116.0f, 9.459f, 122.5f });
	rotatingBar4->SetModel(ShortRotatingBarModelPath);
	rotatingBar4->SetColliderHalfSize({ 2.8f, 0.25f, 0.25f });
	rotatingBar4->SetRotationAxis({ 0.0f, 1.0f, 0.0f });
	rotatingBar4->SetRotationSpeed(0.015f);

	auto rotatingBar5 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar5->SetPosition({ 113.0f, 9.459f, 145.0f });
	rotatingBar5->SetModel(LongRotatingBarModelPath);
	rotatingBar5->SetColliderHalfSize({ 4.8f, 0.25f, 0.25f });
	rotatingBar5->SetRotationAxis({ 0.0f, 1.0f, 0.0f });
	rotatingBar5->SetRotationSpeed(0.01f);

	auto rotatingBar6 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar6->SetPosition({ 113.0f, 10.459f, 145.0f });
	rotatingBar6->SetModel(LongRotatingBarModelPath);
	rotatingBar6->SetColliderHalfSize({ 4.8f, 0.25f, 0.25f });
	rotatingBar6->SetRotationAxis({ 0.0f, 1.0f, 0.0f });
	rotatingBar6->SetRotationSpeed(0.01f);

	auto rotatingBar7 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar7->SetPosition({ 116.0f, 9.5f, 178.0f });
	rotatingBar7->SetRotationAxis({ 0.0f, 0.0f, 0.0f });
	rotatingBar7->SetRotationSpeed(0.00f);

	auto rotatingBar8 = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar8->SetPosition({ 107.5f, 9.5f, 176.0f });
	rotatingBar8->SetRotationAxis({ 0.0f, 0.0f, 0.0f });
	rotatingBar8->SetRotation({ 0.0f, XMConvertToRadians(-20.0f), 0.0f});
	rotatingBar8->SetRotationSpeed(0.00f);

	auto gate2 = AddGameObject<Gate>(Scene::OBJECT3D);
	gate2->SetPosition({ 113.0f, 10.509f, 193.0f });
	gate2->SetRequiredItemCount(60);

	auto goal = AddGameObject<Goal>(Scene::OBJECT3D);
	goal->SetPosition({ 113.0f, 9.659f, 201.6f });
	goal->SetRotation({ 0.0f, XM_PI, 0.0f });

	auto goalStar = CreateBoxModel({ 113.0f, 9.959f, 201.6f }, { 0.65f, 0.1f, 0.65f }, "asset\\model\\star_goal.obj");
	goalStar->SetCollisionEnabled(false);
	goalStar->SetRotationVelocity({ 0.0f, 0.01f, 0.0f });

	auto renderer = goalStar->GetComponent<ModelRendererComponent>();
	if (renderer)
		renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");
}

void Stage2::CreatePlayer()
{
	auto player = AddGameObject<Player>(Scene::OBJECT3D);
	player->SetPosition({ 0.0f, 3.0f, 4.0f });
	player->SetFallLimitY(PlayerFallLimitY);
}
