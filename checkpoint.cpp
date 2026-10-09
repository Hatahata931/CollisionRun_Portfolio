?//-----------------------------------------------------------
//  [ checkpoint.cpp ]
//  復帰地点を更新するチェックポイント
//-----------------------------------------------------------
#include "main.h"
#include "checkpoint.h"
#include "audioManager.h"
#include "effectManager.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "renderer.h"
#include "scene.h"

void Checkpoint::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\checkpoint_inactive.obj");
	renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.07f, 0.45f, 0.07f });
	collider->SetCenterOffset({ 0.0f, 0.45f, 0.0f });
}

void Checkpoint::Update()
{
	GameObject::Update();

	CheckHit();
}

void Checkpoint::Draw()
{
	//両面描画
	Renderer::SetCullNone();
	GameObject::Draw();
	Renderer::SetCullBack();
}

void Checkpoint::CheckHit()
{
	Player* player = Manager::GetScene()->GetGameObject<Player>();
	if (!player)
		return;

	auto collider = GetComponent<OBBColliderComponent>();
	auto playerCollider = player->GetComponent<OBBColliderComponent>();
	if (!collider || !playerCollider)
		return;

	if (!playerCollider->Intersects(collider))
		return;

	//少し上に復帰
	const Vector3 respawnPosition = GetPosition() + Vector3{ 0.0f, 1.0f, 0.0f };
	player->SetRespawnPosition(respawnPosition);

	if (m_IsActivated)
		return;

	m_IsActivated = true;
	AudioManager::PlaySE(SEId::Checkpoint);
	EffectManager::Play(EffectId::Checkpoint, respawnPosition);

	//アクティブ状態のモデルに変更
	auto renderer = GetComponent<ModelRendererComponent>();
	if (renderer)
		renderer->SetModel("asset\\model\\checkpoint_active.obj");
}
