//-----------------------------------------------------------
//  [ bullet.cpp ]
//  Turretから発射される弾
//-----------------------------------------------------------
#include "main.h"
#include "bullet.h"
#include "box.h"
#include "breakPlatform.h"
#include "effectManager.h"
#include "gate.h"
#include "jumpPad.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "movingPlatform.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "scene.h"
#include "terrainBlock.h"

void Bullet::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\bullet.obj");
	renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.17f, 0.17f, 0.17f });
	collider->SetUseRotation(false);
}

void Bullet::Update()
{
	GameObject::Update();

	m_Transform->m_Position += m_Velocity;

	CheckHitPlayer();
	if (IsDestroyRequested())
		return;

	CheckHitStageObjects();

	m_LifeFrames--;
	if (m_LifeFrames <= 0)
		SetDestroy();
}

void Bullet::CheckHitPlayer()
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

	Vector3 correction;
	if (playerCollider->ResolveCollision(collider, correction, false))
	{
		player->TakeDamage(1);
		EffectManager::Play(EffectId::BulletHit, GetPosition());
		SetDestroy();
	}
}

void Bullet::CheckHitStageObjects()
{
	auto collider = GetComponent<OBBColliderComponent>();
	if (!collider)
		return;

	auto checkCollision = [&](auto* otherCollider)
	{
		if (!otherCollider)
			return false;

		Vector3 correction;
		if (collider->ResolveCollision(otherCollider, correction, false))
		{
			EffectManager::Play(EffectId::BulletHit, GetPosition());
			SetDestroy();
			return true;
		}

		return false;
	};

	for (auto terrainBlock : Manager::GetScene()->GetGameObjects<TerrainBlock>())
	{
		if (checkCollision(terrainBlock->GetComponent<OBBColliderComponent>()))
			return;
	}

	for (auto box : Manager::GetScene()->GetGameObjects<Box>())
	{
		if (!box->IsCollisionEnabled())
			continue;

		if (checkCollision(box->GetComponent<OBBColliderComponent>()))
			return;
	}

	for (auto gate : Manager::GetScene()->GetGameObjects<Gate>())
	{
		//開いたGateはSceneが破棄するまでの間も弾を遮らない
		if (gate->IsDestroyRequested())
			continue;

		if (checkCollision(gate->GetComponent<OBBColliderComponent>()))
			return;
	}

	for (auto movingPlatform : Manager::GetScene()->GetGameObjects<MovingPlatform>())
	{
		if (checkCollision(movingPlatform->GetComponent<OBBColliderComponent>()))
			return;
	}

	for (auto breakPlatform : Manager::GetScene()->GetGameObjects<BreakPlatform>())
	{
		if (!breakPlatform->IsActive())
			continue;

		if (checkCollision(breakPlatform->GetComponent<OBBColliderComponent>()))
			return;
	}

	for (auto jumpPad : Manager::GetScene()->GetGameObjects<JumpPad>())
	{
		if (checkCollision(jumpPad->GetComponent<OBBColliderComponent>()))
			return;
	}
}

