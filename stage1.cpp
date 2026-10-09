//-----------------------------------------------------------
//  [ stage1.cpp ]
//  ステージ1
//-----------------------------------------------------------
#include "main.h"
#include "stage1.h"
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
#include "movingPlatform.h"
#include "player.h"
#include "scene.h"
#include "sky.h"
#include "star.h"
#include "starCounter.h"
#include "turret.h"

void Stage1::Init()
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

void Stage1::Update()
{
	Scene::Update();

	Player* player = GetGameObject<Player>();
	if (!player)
		Manager::SetScene<GameOver>(StageId::Stage1);
}

void Stage1::CreateGimmicks()
{
	auto movingPlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
	movingPlatform->SetPosition({ 41.0f, 0.7f, 34.0f });
	movingPlatform->SetScale({ 2.0f, 2.0f, 2.0f });
	movingPlatform->SetEndPosition({ 41.0f, -1.3f, 42.5f });
	movingPlatform->SetSpeed(0.015f);

	auto gate = AddGameObject<Gate>(Scene::OBJECT3D);
	gate->SetPosition({ 59.0f, 2.05f, 40.0f });
	gate->SetRequiredItemCount(24);

	auto turret = AddGameObject<Turret>(Scene::OBJECT3D);
	turret->SetPosition({ 78.0f, 1.0f, 110.0f });
	turret->SetScale({ 2.0f, 2.0f, 2.0f });
	turret->SetRotation({ 0.0f, XM_PI, 0.0f });
	turret->SetShotIntervalFrames(110);

	auto turret2 = AddGameObject<Turret>(Scene::OBJECT3D);
	turret2->SetPosition({ 84.0f, 1.0f, 110.0f });
	turret2->SetScale({ 2.0f, 2.0f, 2.0f });
	turret2->SetRotation({ 0.0f, XM_PI, 0.0f });
	turret2->SetShotIntervalFrames(110);

	auto gate2 = AddGameObject<Gate>(Scene::OBJECT3D);
	gate2->SetPosition({ 59.0f, 2.05f, 109.0f });
	gate2->SetRequiredItemCount(44);

	auto mechanicalStone2 = CreateBoxTexture({ 59.4f, 4.283f, 162.5f }, { 1.0f, 1.0f, 1.0f }, MechanicalStoneTexturePath, { 0.154f, -0.593f, 0.233f });

	auto goal = AddGameObject<Goal>(Scene::OBJECT3D);
	goal->SetPosition({ 55.7f, 4.2f, 168.1f });
	goal->SetRotation({ 0.0f, XM_PI, 0.0f });

	auto goalStar = CreateBoxModel({ 55.7f, 4.5f, 168.1f }, { 0.65f, 0.1f, 0.65f }, "asset\\model\\star_goal.obj");
	goalStar->SetCollisionEnabled(false);
	goalStar->SetRotationVelocity({ 0.0f, 0.01f, 0.0f });

	auto renderer = goalStar->GetComponent<ModelRendererComponent>();
	if (renderer)
		renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");
}

void Stage1::CreatePlayer()
{
	auto player = AddGameObject<Player>(Scene::OBJECT3D);
	player->SetPosition({ 0.0f, 3.0f, 4.0f });
	player->SetFallLimitY(PlayerFallLimitY);
}
