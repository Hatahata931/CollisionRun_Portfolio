//-----------------------------------------------------------
//  [ shadow.cpp ]
//  プレイヤーの下に表示する影
//-----------------------------------------------------------
#include "main.h"
#include "shadow.h"
#include "box.h"
#include "breakPlatform.h"
#include "jumpPad.h"
#include "manager.h"
#include "movingPlatform.h"
#include "obbColliderComponent.h"
#include "polygonRendererComponent.h"
#include "renderer.h"
#include "scene.h"
#include "terrainBlock.h"

//指定したオブジェクトから影を投影する面を調べる
bool Shadow::TryGetShadowSurface(GameObject* object, const Vector3& rayOrigin, SurfaceHit& outHit)
{
	auto* collider = object->GetComponent<OBBColliderComponent>();
	if (!collider)
		return false;

	Vector3 center, half;
	Vector3 axis[3];
	collider->GetWorldOBB(center, axis, half);

	const Vector3 rayDirection = { 0.0f, -1.0f, 0.0f };
	const float halfSizeValues[3] = { half.x, half.y, half.z };
	bool foundSurface = false;
	SurfaceHit bestHit{};

	//OBBの6面を調べ、下向きレイが最初に当たる最も高い面を採用する
	for (int faceAxisIndex = 0; faceAxisIndex < 3; ++faceAxisIndex)
	{
		for (int faceDirection = -1; faceDirection <= 1; faceDirection += 2)
		{
			const Vector3 faceNormal =
				axis[faceAxisIndex] * static_cast<float>(faceDirection);
			const float rayNormalDot = Vector3::dot(rayDirection, faceNormal);
			if (rayNormalDot >= -DirectionEpsilon)
				continue;

			const Vector3 faceCenter =
				center + faceNormal * halfSizeValues[faceAxisIndex];
			const float rayDistance =
				Vector3::dot(faceCenter - rayOrigin, faceNormal) / rayNormalDot;
			if (rayDistance < 0.0f)
				continue;

			const Vector3 hitPoint = rayOrigin + rayDirection * rayDistance;
			const Vector3 relativeHitPosition = hitPoint - center;
			bool insideFace = true;

			//面を構成する残り2軸の範囲内に交点があるか確認する
			for (int boundsAxisIndex = 0; boundsAxisIndex < 3; ++boundsAxisIndex)
			{
				if (boundsAxisIndex == faceAxisIndex)
					continue;

				const float localPosition =
					Vector3::dot(relativeHitPosition, axis[boundsAxisIndex]);
				if (localPosition < -halfSizeValues[boundsAxisIndex] - SurfaceBoundsMargin ||
					localPosition > halfSizeValues[boundsAxisIndex] + SurfaceBoundsMargin)
				{
					insideFace = false;
					break;
				}
			}

			if (!insideFace)
				continue;

			if (!foundSurface || hitPoint.y > bestHit.Position.y)
			{
				bestHit.Position = hitPoint;
				bestHit.Normal = faceNormal;
				foundSurface = true;
			}
		}
	}

	if (!foundSurface)
		return false;

	outHit = bestHit;
	return true;
}

//投影面の法線から影を面と平行にする回転角度を求める
void Shadow::SetRotationFromSurfaceNormal(const Vector3& surfaceNormal)
{
	const float horizontalLengthSq =
		surfaceNormal.x * surfaceNormal.x + surfaceNormal.z * surfaceNormal.z;
	if (horizontalLengthSq <= DirectionEpsilon * DirectionEpsilon)
	{
		SetRotation({ XM_PIDIV2, 0.0f, 0.0f });
		return;
	}

	float normalY = surfaceNormal.y;
	if (normalY < -1.0f)
		normalY = -1.0f;
	if (normalY > 1.0f)
		normalY = 1.0f;

	//板ポリゴンの-Z面を上面法線へ合わせ、投影面と平行にする
	const float pitch = asinf(normalY);
	const float yaw = atan2f(-surfaceNormal.x, -surfaceNormal.z);
	SetRotation({ pitch, yaw, 0.0f });
}

void Shadow::Init()
{
	GameObject::Init();

	m_Renderer = AddComponent<PolygonRendererComponent>();
	m_Renderer->InitWorldSpace("asset\\texture\\shadow.png");
	m_Transform->m_Rotation.x = XM_PIDIV2;
}

void Shadow::LateUpdate()
{
	//追従対象より影だけが後に残らないよう、削除予約を同じフレームで反映する
	if (m_Target && m_Target->IsDestroyRequested())
	{
		m_Target = nullptr;
		SetDestroy();
	}

	if (!m_Target || !m_Renderer)
	{
		if (m_Renderer)
			m_Renderer->SetVisible(false);
		return;
	}

	bool foundShadowSurface = false; //投影可能な面が見つかったか
	SurfaceHit bestHit{}; //最終的に採用する交点と法線

	Vector3 shadowOrigin = m_Target->GetPosition(); //レイの開始位置
	//レイの開始位置をOBBの底面少し上にする
	auto* targetCollider = m_Target->GetComponent<OBBColliderComponent>();
	if (targetCollider)
	{
		Vector3 targetCenter, targetHalf;
		Vector3 targetAxis[3];
		targetCollider->GetWorldOBB(targetCenter, targetAxis, targetHalf);
		shadowOrigin = targetCenter - targetAxis[1] * targetHalf.y;
		shadowOrigin.y += RayOriginOffset;
	}

	//指定した足場との交点を調べ、現在より高い交点なら更新する
	const auto updateBestSurface = [&](GameObject* object)
	{
		SurfaceHit candidate{};
		if (!TryGetShadowSurface(object, shadowOrigin, candidate))
			return;

		if (!foundShadowSurface || candidate.Position.y > bestHit.Position.y)
		{
			bestHit = candidate;
			foundShadowSurface = true;
		}
	};

	const auto terrainBlocks = Manager::GetScene()->GetGameObjects<TerrainBlock>();
	for (auto* terrainBlock : terrainBlocks)
		updateBestSurface(terrainBlock);

	const auto boxes = Manager::GetScene()->GetGameObjects<Box>();
	for (auto* box : boxes)
	{
		if (!box->IsCollisionEnabled())
			continue;

		updateBestSurface(box);
	}

	const auto breakPlatforms = Manager::GetScene()->GetGameObjects<BreakPlatform>();
	for (auto* breakPlatform : breakPlatforms)
	{
		if (!breakPlatform->IsActive())
			continue;

		updateBestSurface(breakPlatform);
	}

	const auto movingPlatforms = Manager::GetScene()->GetGameObjects<MovingPlatform>();
	for (auto* movingPlatform : movingPlatforms)
		updateBestSurface(movingPlatform);

	const auto jumpPads = Manager::GetScene()->GetGameObjects<JumpPad>();
	for (auto* jumpPad : jumpPads)
		updateBestSurface(jumpPad);

	//投影できる足場がなければ表示しない
	if (!foundShadowSurface)
	{
		m_Renderer->SetVisible(false);
		return;
	}

	m_Renderer->SetVisible(true);

	//Playerの足元と投影面の高さの差に応じて影の大きさを変える
	const float shadowHeight = shadowOrigin.y - bestHit.Position.y;
	float shadowScale = BaseShadowScale - shadowHeight * HeightScaleFactor;
	if (shadowScale < MinShadowScale)
		shadowScale = MinShadowScale;
	if (shadowScale > MaxShadowScale)
		shadowScale = MaxShadowScale;

	SetScale({ shadowScale, shadowScale, 1.0f });
	SetPosition(bestHit.Position + bestHit.Normal * SurfaceOffset);
	SetRotationFromSurfaceNormal(bestHit.Normal);
}

void Shadow::Draw()
{
	//影を投影面より確実に手前へ描くため、深度判定を一時的に無効にする
	Renderer::SetDepthEnable(false);
	GameObject::Draw();
	Renderer::SetDepthEnable(true);
}

