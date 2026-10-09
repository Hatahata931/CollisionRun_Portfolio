//-----------------------------------------------------------
//  [ gate.cpp ]
//  集めたStar数で開くゲート
//-----------------------------------------------------------
#include "main.h"
#include "gate.h"
#include "audioManager.h"
#include "effectManager.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "scene.h"
#include "starCounter.h"

void Gate::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\barrier.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 3.5f, 1.4f, 0.069f });
	collider->SetCenterOffset({ 0.0f, 0.7f, 0.0f });

	SetScale({ 1.5f, 1.5f, 1.5f });
}

void Gate::Update()
{
	GameObject::Update();

	StarCounter* starCounter = Manager::GetScene()->GetGameObject<StarCounter>();
	Player* player = Manager::GetScene()->GetGameObject<Player>();
	if (!starCounter || !player)
		return;

	//Star数が必要数未満なら終了
	if (starCounter->GetValue() < m_RequiredItemCount)
		return;

	const float openDistance = GetOpenDistance();
	const Vector3 directionToPlayer = player->GetPosition() - m_Transform->m_Position;

	if (directionToPlayer.length() <= openDistance)
	{
		AudioManager::PlaySE(SEId::GateOpen);
		EffectManager::Play(EffectId::GateOpen, GetPosition() + Vector3{ 0.0f, 1.0f, 0.0f });
		SetDestroy();
	}
}

//Gateの大きさを考慮した開放距離を取得
float Gate::GetOpenDistance() const
{
	//横幅と奥行きの大きい方に余白を足し、Gateの向きに左右されない範囲にする
	return ((m_Transform->m_Scale.x > m_Transform->m_Scale.z)
		? m_Transform->m_Scale.x : m_Transform->m_Scale.z) + OpenDistanceMargin;
}
