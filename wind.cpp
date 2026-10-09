//-----------------------------------------------------------
//  [ wind.cpp ]
//  範囲内にいると押し流される風エリア
//-----------------------------------------------------------
#include "main.h"
#include "wind.h"

#include <algorithm>
#include <cmath>

#include "audioManager.h"
#include "manager.h"
#include "obbColliderComponent.h"
#include "player.h"
#include "scene.h"
#include "windRendererComponent.h"

namespace
{
	constexpr float WindSoundDistance = 6.0f; //風域の外側で風音が届く距離
	constexpr float DirectionEpsilon = 0.000001f;

	//OBBまでの最短距離を取得
	float GetDistanceToOBB(const Vector3& point, OBBColliderComponent* collider)
	{
		Vector3 center;
		Vector3 axis[3];
		Vector3 halfSize;
		collider->GetWorldOBB(center, axis, halfSize);

		const float halfValues[3] =
		{
			std::fabs(halfSize.x),
			std::fabs(halfSize.y),
			std::fabs(halfSize.z),
		};

		const Vector3 centerDelta = point - center;
		float squaredDistance = 0.0f;
		for (int i = 0; i < 3; ++i)
		{
			const float distanceOnAxis = std::fabs(Vector3::dot(centerDelta, axis[i]));
			const float outsideDistance = std::max(distanceOnAxis - halfValues[i], 0.0f);
			squaredDistance += outsideDistance * outsideDistance;
		}

		return std::sqrt(squaredDistance);
	}

	//風域内を最大音量とし、外側では滑らかに減衰させる
	float GetWindVolumeScale(float distance)
	{
		const float normalizedDistance = std::max(0.0f,
			std::min(distance / WindSoundDistance, 1.0f));
		const float smoothDistance = normalizedDistance * normalizedDistance *
			(3.0f - 2.0f * normalizedDistance);
		return 1.0f - smoothDistance;
	}
}

void Wind::Init()
{
	GameObject::Init();

	m_WindRenderer = AddComponent<WindRendererComponent>();
	m_WindRenderer->SetWindDirection(m_WindDirection);

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 1.0f, 1.0f, 1.0f });
}

void Wind::Update()
{
	GameObject::Update();
	UpdatePlayerInteraction();
}

void Wind::SetWindDirection(const Vector3& direction)
{
	m_WindDirection = direction;
	if (m_WindRenderer)
		m_WindRenderer->SetWindDirection(direction);
}

//プレイヤーとの距離と接触状態に応じて風音と風力を更新
void Wind::UpdatePlayerInteraction()
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

	Vector3 direction = m_WindDirection;
	if (direction.length() <= DirectionEpsilon || std::fabs(m_WindPower) <= DirectionEpsilon)
		return;

	Vector3 playerCenter;
	Vector3 playerAxis[3];
	Vector3 playerHalfSize;
	playerCollider->GetWorldOBB(playerCenter, playerAxis, playerHalfSize);

	const float distanceToWind = GetDistanceToOBB(playerCenter, collider);
	AudioManager::RequestLoopSE(LoopSEId::Wind, GetWindVolumeScale(distanceToWind));

	if (playerCollider->Intersects(collider))
	{
		direction.normalize();
		player->AddForce(direction * m_WindPower);
	}
}
