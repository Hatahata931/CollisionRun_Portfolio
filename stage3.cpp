//-----------------------------------------------------------
//  [ stage3.cpp ]
//  ステージ3
//-----------------------------------------------------------
#include "main.h"
#include "stage3.h"
#include "stageId.h"

#include "audioManager.h"
#include "box.h"
#include "camera.h"
#include "gameOver.h"
#include "gate.h"
#include "gateRequirementDisplay.h"
#include "gateRequirementNotice.h"
#include "goal.h"
#include "life.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "player.h"
#include "rotatingBar.h"
#include "scene.h"
#include "sky.h"
#include "star.h"
#include "movingPlatform.h"
#include "turret.h"
#include "wind.h"
#include "starCounter.h"

void Stage3::Init()
{
	//Star数を表示
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

void Stage3::Update()
{
	Scene::Update();

	Player* player = GetGameObject<Player>();
	if (!player)
		Manager::SetScene<GameOver>(StageId::Stage3);
}

void Stage3::CreateGimmicks()
{
	auto lowerMovingPlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
	lowerMovingPlatform->SetPosition({ 4.0f, 3.7f, 37.0f });
	lowerMovingPlatform->SetScale({ 2.0f, 2.0f, 2.0f });
	lowerMovingPlatform->SetEndPosition({ 9.0f, 9.2f, 36.0f });
	lowerMovingPlatform->SetSpeed(0.007f);
	lowerMovingPlatform->SetWaitFrames(100);

	auto upperMovingPlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
	upperMovingPlatform->SetPosition({ 18.0f, 13.7f, 41.0f });
	upperMovingPlatform->SetScale({ 2.0f, 2.0f, 2.0f });
	upperMovingPlatform->SetEndPosition({ 11.0f, 9.2f, 36.0f });
	upperMovingPlatform->SetSpeed(0.007f);
	upperMovingPlatform->SetWaitFrames(100);

	auto detourMovingPlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
	detourMovingPlatform->SetPosition({ 14.0f, 9.32f, 29.1f });
	detourMovingPlatform->SetScale({ 1.2f, 1.2f, 1.2f });
	detourMovingPlatform->SetEndPosition({ 11.0f, 9.32f, 34.4f });
	detourMovingPlatform->SetSpeed(0.007f);
	detourMovingPlatform->SetWaitFrames(100);

	for (int i = 0; i < 5; ++i)
	{
		Vector3 position = { 21.0f + 0.7f * i, 13.7f, 51.0f + 2.0f * i };
		Vector3 endPosition = { position.x - 6.0f, position.y, position.z };

		auto movingBridgePlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
		movingBridgePlatform->SetPosition(position);
		movingBridgePlatform->SetScale({ 2.0f, 2.0f, 2.0f });
		movingBridgePlatform->SetEndPosition(endPosition);
		movingBridgePlatform->SetSpeed(0.013f);
		movingBridgePlatform->SetWaitFrames(0);
	}

	auto gate = AddGameObject<Gate>(Scene::OBJECT3D);
	gate->SetPosition({ -2.894f, 24.506f, 20.8f });
	gate->SetRotation({ 0.249639f, -XM_PIDIV2, 0.0f });
	gate->SetRequiredItemCount(30);

	auto gate2 = AddGameObject<Gate>(Scene::OBJECT3D);
	gate2->SetPosition({ -3.234f, 44.204f, 18.721f });
	gate2->SetRotation({ -0.000177f, -1.765254f, -0.073947f });
	gate2->SetRequiredItemCount(60);

	auto rotatingBar = AddGameObject<RotatingBar>(Scene::OBJECT3D);
	rotatingBar->SetPosition({ -12.787f, 42.058f, 16.287f });
	rotatingBar->SetRotation({ 0.073565f, -0.293933f, 0.282423f });
	rotatingBar->SetModel("asset\\model\\rotating_bar_medium.obj");
	rotatingBar->SetColliderHalfSize({ 3.8f, 0.25f, 0.25f });
	rotatingBar->SetScale({ 1.0f, 1.0f, 1.0f });
	rotatingBar->SetRotationAxis({ 0.0f, 1.0f, 0.0f });
	rotatingBar->SetRotationSpeed(0.01f);

	auto wind = AddGameObject<Wind>(Scene::OBJECT3D);
	wind->SetPosition({ 0.064f, 48.5f, 34.0f });
	wind->SetScale({ 5.0f, 3.0f, 11.944f });
	wind->SetRotation({ -0.284586f, -0.006069f, 0.0f });
	wind->SetWindDirection({ -1.0f, 0.0f, 0.0f });
	wind->SetWindPower(0.005f);

	auto turret = AddGameObject<Turret>(Scene::OBJECT3D);
	turret->SetPosition({ 14.895f, 45.5f, 32.410f });
	turret->SetRotation({ 0.0f, -XM_PIDIV2, 0.0f });
	turret->SetScale({ 2.0f, 2.0f, 2.0f });
	turret->SetShotIntervalFrames(90);

	auto turret2 = AddGameObject<Turret>(Scene::OBJECT3D);
	turret2->SetPosition({ 14.895f, 47.663f, 39.977f });
	turret2->SetRotation({ 0.0f, -XM_PIDIV2, 0.0f });
	turret2->SetScale({ 2.0f, 2.0f, 2.0f });
	turret2->SetShotIntervalFrames(80);

	auto turret3 = AddGameObject<Turret>(Scene::OBJECT3D);
	turret3->SetPosition({ 14.895f, 48.536f, 42.490f });
	turret3->SetRotation({ 0.0f, -XM_PIDIV2, 0.0f });
	turret3->SetScale({ 2.0f, 2.0f, 2.0f });
	turret3->SetShotIntervalFrames(70);

	auto goal = AddGameObject<Goal>(Scene::OBJECT3D);
	goal->SetPosition({ 0.0f, 50.2f, 53.0f });
	goal->SetRotation({ 0.0f, XM_PI, 0.0f });

	auto goalStar = CreateBoxModel(
		{ 0.0f, 50.5f, 53.0f }, { 0.65f, 0.1f, 0.65f }, "asset\\model\\star_goal.obj");
	goalStar->SetCollisionEnabled(false);
	goalStar->SetRotationVelocity({ 0.0f, 0.01f, 0.0f });

	auto renderer = goalStar->GetComponent<ModelRendererComponent>();
	if (renderer)
		renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");
}

void Stage3::CreatePlayer()
{
	auto player = AddGameObject<Player>(Scene::OBJECT3D);
	player->SetPosition({ 0.0f, 3.0f, -3.0f });
	player->SetFallLimitY(PlayerFallLimitY);
}
