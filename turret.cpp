//-----------------------------------------------------------
//  [ turret.cpp ]
//  弾を撃つ砲台
//-----------------------------------------------------------
#include "main.h"
#include "turret.h"
#include "audioManager.h"
#include "bullet.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "player.h"
#include "scene.h"

void Turret::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\weapon-cannon.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");
}

void Turret::Update()
{
	GameObject::Update();

	m_ShotTimer++;
	if (m_ShotTimer >= m_ShotIntervalFrames)
	{
		m_ShotTimer = 0;
		Shoot();
	}
}

void Turret::Shoot()
{
	Vector3 direction = GetForward();
	direction.y = 0.0f;
	if (direction.length() <= 0.0f)
		return;

	direction.normalize();

	const Vector3 scale = GetScale();
	const Vector3 scaledOffset =
	{
		m_MuzzleOffset.x * scale.x,
		m_MuzzleOffset.y * scale.y,
		m_MuzzleOffset.z * scale.z
	};

	//発射位置(スケール対応)
	const Vector3 muzzlePosition = m_Transform->m_Position + GetRight() * scaledOffset.x 
		+ GetUp() * scaledOffset.y + GetForward() * scaledOffset.z;

	//弾の設定
	auto bullet = Manager::GetScene()->AddGameObject<Bullet>(Scene::EFFECT);
	bullet->SetPosition(muzzlePosition);
	//砲台の各軸の平均スケールを基準に弾の大きさを決める
	const float bulletScale = (scale.x + scale.y + scale.z) / 3.0f * m_BulletScaleRate;
	bullet->SetScale({ bulletScale, bulletScale, bulletScale });
	bullet->SetVelocity(direction * m_BulletSpeed);
	bullet->SetLifeFrames(m_BulletLifeFrames);

	Player* player = Manager::GetScene()->GetGameObject<Player>();
	if (player && !player->IsDestroyRequested() && player->IsGameplayEnabled())
	{
		//プレイヤーの位置に合わせた音量で再生
		AudioManager::PlaySEAt(SEId::TurretShot, muzzlePosition,
			player->GetPosition(), SoundFullVolumeDistance, SoundMaxDistance);
	}
}
