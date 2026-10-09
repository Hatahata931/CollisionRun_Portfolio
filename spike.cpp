//-----------------------------------------------------------
//  [ spike.cpp ]
//  当たるとダメージを受けるトゲ
//-----------------------------------------------------------
#include "main.h"
#include "spike.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "scene.h"

void Spike::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\trap-spikes-large.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.25f, 0.11f, 0.25f });
	//原点を当たり判定の底面に合わせる
	collider->SetCenterOffset({ 0.0f, collider->GetHalfSize().y, 0.0f });
}

void Spike::Update()
{
	GameObject::Update();

	CheckHit();
}

void Spike::CheckHit()
{
	Player* player = Manager::GetScene()->GetGameObject<Player>();
	if (!player)
		return;

	auto collider = GetComponent<OBBColliderComponent>();
	if (!collider)
		return;

	auto playerCollider = player->GetComponent<OBBColliderComponent>();
	if (!playerCollider)
		return;

	if (playerCollider->Intersects(collider))
		player->TakeDamage(1);
}
