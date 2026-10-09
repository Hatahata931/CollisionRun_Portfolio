//-----------------------------------------------------------
//  [ star.cpp ]
//  収集アイテム
//-----------------------------------------------------------
#include "main.h"
#include "star.h"
#include "audioManager.h"
#include "effectManager.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "scene.h"
#include "starCounter.h"

void Star::Init()
{
	GameObject::Init();

	auto* renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\star.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	m_Transform->m_Scale = { 1.2f, 1.2f, 0.9f };
}

void Star::Update()
{
	GameObject::Update();

	m_Transform->m_Rotation.y += RotationSpeed;

	auto* player = Manager::GetScene()->GetGameObject<Player>();
	if (!player)
		return;

	if (CheckHit(player))
		OnCollected();
}

bool Star::CheckHit(Player* player)
{
	auto* collider = player->GetComponent<OBBColliderComponent>();
	if (!collider)
		return false;

	//プレイヤーのOBBを球として近似し、取得範囲との距離で判定
	const Vector3 center = player->GetPosition() + collider->GetCenterOffset();
	const float radius = collider->GetHalfSize().x;

	const Vector3 direction = center - GetPosition();
	const float distanceSq = Vector3::dot(direction, direction);
	const float hitRadius = radius + CollisionRadius;

	return distanceSq <= hitRadius * hitRadius;
}

//取得処理
void Star::OnCollected()
{
	AudioManager::PlaySE(SEId::Star);

	auto* starCounter = Manager::GetScene()->GetGameObject<StarCounter>();
	if (starCounter)
		starCounter->Add(1);

	const Vector3 effectPosition = GetPosition() + Vector3{ 0.0f, EffectOffsetY, 0.0f };
	EffectManager::Play(EffectId::StarGet, effectPosition);
	SetDestroy();
}
