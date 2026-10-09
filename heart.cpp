//-----------------------------------------------------------
//  [ heart.cpp ]
//  回復アイテム
//-----------------------------------------------------------
#include "main.h"
#include "heart.h"
#include "audioManager.h"
#include "effectManager.h"
#include "life.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "scene.h"

void Heart::Init()
{
    GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\heart.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");
}

void Heart::Update()
{
    GameObject::Update();

	m_Transform->m_Rotation.y += RotationSpeed;

    Player* player = Manager::GetScene()->GetGameObject<Player>();
    if (!player)
        return;

	if (CheckHit(player))
		OnCollected();
}

bool Heart::CheckHit(Player* player)
{
    auto collider = player->GetComponent<OBBColliderComponent>();
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
void Heart::OnCollected()
{
    Life* life = Manager::GetScene()->GetGameObject<Life>();
    if (!life)
        return;

    //ライフが最大なら取得しない
    if (life->GetValue() >= life->GetMaxValue())
        return;

	life->Add(HealAmount);
    AudioManager::PlaySE(SEId::Heart);
    EffectManager::Play(EffectId::HeartGet, GetPosition());
    SetDestroy();
}
