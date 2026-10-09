//-----------------------------------------------------------
//  [ debugScene.cpp ]
//  デバッグ用シーン
//-----------------------------------------------------------
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "input.h"
#include "camera.h"
#include "player.h"
#include "polygon2D.h"
#include "title.h"
#include "starCounter.h"
#include "clear.h"
#include "sky.h"
#include "life.h"
#include "box.h"
#include "breakPlatform.h"
#include "movingPlatform.h"
#include "gate.h"
#include "goal.h"
#include "jumpPad.h"
#include "rotatingBar.h"
#include "spike.h"
#include "star.h"
#include "wind.h"
#include "checkpoint.h"
#include "turret.h"
#include "heart.h"
#include "debugScene.h"

void DebugScene::Init()
{
	auto score = AddGameObject<StarCounter>(Scene::OBJECT2D);
	score->SetPosition({ 30.0f, 20.0f, 0.0f });

	AddGameObject<Life>(Scene::OBJECT2D)->SetPosition({ 30.0f, 70.0f, 0.0f });

	AddGameObject<Camera>(Scene::CAMERA);
	AddGameObject<Sky>(Scene::CAMERA)->SetScale({ 100.0f, 100.0f, 100.0f });

	LoadStageObjects(GetStageObjectDataPath());

	AddGameObject<Goal>(Scene::OBJECT3D)->SetPosition({ 0.0f, 7.0f, 22.0f });

	auto box5 = AddGameObject<Box>(Scene::OBJECT3D);
	box5->SetPosition({ 0.0f, -1.0f, 0.0f });
	box5->SetScale({ 20.0f, 1.0f, 50.0f });

	auto box = AddGameObject<Box>(Scene::OBJECT3D);
	box->SetPosition({ -8.0f, 1.0f, -8.0f });
	box->SetScale({ 5.0f, 1.0f, 1.0f });
	box->SetRotationVelocity({ 0.0f, 0.01f, 0.0f });

	auto box2 = AddGameObject<Box>(Scene::OBJECT3D);
	box2->SetPosition({ 0.0f, 0.75f, 0.0f });
	box2->SetScale({ 3.0f, 1.5f, 3.0f });
	box2->SetRotation({ 1.0f, 2.0f, 0.0f });
	
	auto box3 = AddGameObject<Box>(Scene::OBJECT3D);
	box3->SetPosition({ 0.0f, 1.5f, 9.0f });
	box3->SetScale({ 3.0f, 3.0f, 3.0f });

	auto checkpoint = AddGameObject<Checkpoint>(Scene::OBJECT3D);
	checkpoint->SetPosition({ -12.0f, 1.0f, 20.0f });
	checkpoint->SetScale({ 1.5f, 0.5f, 1.5f });

	auto gate = AddGameObject<Gate>(Scene::OBJECT3D);
	gate->SetPosition({ 0.0f, 2.0f, 14.5f });
	gate->SetScale({ 1.0f, 1.0f, 1.0f });
	gate->SetRequiredItemCount(1);

	auto movingPlatform = AddGameObject<MovingPlatform>(Scene::OBJECT3D);
	movingPlatform->SetPosition({ 5.0f, 2.0f, 9.0f });
	movingPlatform->SetScale({ 2.0f, 0.4f, 2.0f });
	movingPlatform->SetEndPosition({ 15.0f, 7.0f, 9.0f });
	movingPlatform->SetSpeed(0.01f);
	movingPlatform->SetWaitFrames(60);

	auto star = AddGameObject<Star>(Scene::OBJECT3D);
	star->SetPosition({ 0.0f, 0.5f, -5.0f });
	
	auto star2 = AddGameObject<Star>(Scene::OBJECT3D);
	star2->SetPosition({ 1.0f, 0.5f, -5.0f });

	auto box4 = AddGameObject<Box>(Scene::OBJECT3D);
	box4->SetPosition({ 0.0f, 5.0f, 20.0f });
	box4->SetScale({ 7.0f, 0.8f, 4.0f });

	auto box6 = AddGameObject<Box>(Scene::OBJECT3D);
	box6->SetPosition({ -6.0f, 2.0f, 9.0f });
	box6->SetScale({ 2.0f, 1.0f, 2.0f });
	box6->SetRotation({ 0.7f, 2.5f, 0.0f });

	auto breakPlatform = AddGameObject<BreakPlatform>(Scene::OBJECT3D);
	breakPlatform->SetPosition({ 7.5f, 2.0f, 3.5f });
	breakPlatform->SetScale({ 3.0f, 3.0f, 3.0f });

	auto jumpPad = AddGameObject<JumpPad>(Scene::OBJECT3D);
	jumpPad->SetPosition({ 8.0f, 0.2f, -5.0f });
	jumpPad->SetScale({ 1.0f, 1.0f, 1.0f });

	auto spike = AddGameObject<Spike>(Scene::OBJECT3D);
	spike->SetPosition({ -3.0f, 1.0f, -5.0f });
	spike->SetScale({ 1.0f, 1.0f, 1.0f });

	auto wind = AddGameObject<Wind>(Scene::OBJECT3D);
	wind->SetPosition({ -10.0f, 1.0f, -15.0f });
	wind->SetScale({ 4.0f, 2.0f, 4.0f });
	wind->SetWindDirection({ -1.0f, 0.0f, 0.0f });
	wind->SetWindPower(0.025f);

	auto turret = AddGameObject<Turret>(Scene::OBJECT3D);
	turret->SetPosition({ -8.0f, 1.5f, 15.0f });
	turret->SetScale({ 1.0f, 1.0f, 1.0f });
	turret->SetRotation({ 0.0f, 0.0f, 0.0f });
	turret->SetShotIntervalFrames(30);
	turret->SetBulletSpeed(0.2f);

	AddGameObject<Heart>(Scene::OBJECT3D)->SetPosition({ 0.0f, 0.5f, -7.0f });

	AddGameObject<Player>(Scene::OBJECT3D)->SetPosition({ 0.0f, 3.0f, -15.0f });

	score->SetMaxValue(static_cast<int>(GetGameObjects<Star>().size()));
}

void DebugScene::Uninit()
{
	Scene::Uninit();
}

void DebugScene::Update()
{
	Scene::Update();

	Player* player = GetGameObject<Player>();
	if (player == nullptr)
	{
		Manager::SetScene<Clear>();
	}

	if (Input::GetKeyTrigger(VK_RETURN))
	{
		Manager::SetScene<Clear>();
	}
}
