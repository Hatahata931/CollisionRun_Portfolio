//-----------------------------------------------------------
//  [ goal.cpp ]
//  乗るとクリア画面に移動するゴール
//-----------------------------------------------------------
#include "main.h"
#include "goal.h"
#include "audioManager.h"
#include "clear.h"
#include "effectManager.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "pauseManager.h"
#include "player.h"
#include "polygonRendererComponent.h"
#include "scene.h"
#include "starCounter.h"

void Goal::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\goal.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 1.5f, 0.2f, 1.5f });
}

void Goal::Update()
{
	GameObject::Update();

	if (m_IsClearSequenceActive)
	{
		UpdateClearSequence();
		return;
	}

	Player* player = Manager::GetScene()->GetGameObject<Player>();
	if (IsPlayerOnGoal(player))
		StartClearSequence(player);
}

//プレイヤーが乗っているか判定
bool Goal::IsPlayerOnGoal(Player* player)
{
	if (!player || player->IsDestroyRequested())
		return false;

	if (!player->IsGround())
		return false;

	const OBBColliderComponent* collider = GetComponent<OBBColliderComponent>();
	if (!collider)
		return false;

	const Vector3& halfSize = collider->GetHalfSize();
	const Vector3 goalPosition = GetPosition();
	const Vector3 playerPosition = player->GetPosition();
	//プレイヤーへの方向をローカル軸へ投影
	const Vector3 direction = playerPosition - goalPosition;

	const Vector3 axisX = GetRight();
	const float dotX = Vector3::dot(direction, axisX);

	const Vector3 axisZ = GetForward();
	const float dotZ = Vector3::dot(direction, axisZ);

	//スケール適用後の上面の範囲内にいるか判定
	const Vector3 boxScale = GetScale();
	const float goalHalfWidth = halfSize.x * boxScale.x;
	const float goalHalfDepth = halfSize.z * boxScale.z;
	const bool isInsideGoalTop = -goalHalfWidth < dotX && dotX < goalHalfWidth &&
		-goalHalfDepth < dotZ && dotZ < goalHalfDepth;

	//上面とプレイヤーの足元の高さが許容範囲内か判定
	const float goalTopY = goalPosition.y + halfSize.y * boxScale.y;
	const float playerFootY = playerPosition.y;
	const bool isOnGoalTop = goalTopY - LandingHeightTolerance < playerFootY &&
		playerFootY < goalTopY + LandingHeightTolerance;

	return isInsideGoalTop && isOnGoalTop;
}

//クリア演出を開始
void Goal::StartClearSequence(Player* player)
{
	StarCounter* starCounter = Manager::GetScene()->GetGameObject<StarCounter>();
	if (starCounter)
	{
		//クリア画面へ渡すStar数を保存
		m_CollectedStars = starCounter->GetValue();
		m_TotalStars = starCounter->GetMaxValue();
	}

	AudioManager::PlaySE(SEId::Goal);
	EffectManager::Play(EffectId::Goal, GetPosition() + Vector3{ 0.0f, 0.8f, 0.0f });

	//ステージクリアの文字画像
	auto clearText = Manager::GetScene()->AddGameObject<GameObject>(Scene::OBJECT2D);
	clearText->AddComponent<PolygonRendererComponent>()->InitScreenSpaceCentered("asset\\texture\\text_stage_clear.png");
	clearText->SetPosition({ ClearTextPositionX, ClearTextPositionY, 0.0f });
	clearText->SetScale({ ClearTextWidth, ClearTextHeight, 1.0f });

	PauseManager::SetPauseLocked(true); //演出中はポーズ不可
	player->SetGameplayEnabled(false);
	m_ClearSequenceTimer = 0;
	m_IsClearSequenceActive = true;
}

//クリア演出を進めて遷移
void Goal::UpdateClearSequence()
{
	m_ClearSequenceTimer++;
	if (m_ClearSequenceTimer < ClearDelayFrames)
		return;

	ClearResult result{};
	result.CollectedStars = m_CollectedStars;
	result.TotalStars = m_TotalStars;
	Manager::SetScene<Clear>(result);
}
