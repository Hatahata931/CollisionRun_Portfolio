//-----------------------------------------------------------
//  [ playerCollision.cpp ]
//  プレイヤーの移動とステージオブジェクトとの当たり判定
//-----------------------------------------------------------
#include "main.h"
#include "player.h"

#include <cmath>

#include "box.h"
#include "breakPlatform.h"
#include "gate.h"
#include "goal.h"
#include "jumpPad.h"
#include "manager.h"
#include "movingPlatform.h"
#include "obbColliderComponent.h"
#include "scene.h"
#include "terrainBlock.h"

namespace
{
	//このファイルだけで使う衝突解決の精度と接地判定の調整値
	constexpr float MaxSubstepLength = 0.2f; //1フレームの移動を分割した1回分の最大距離
	constexpr int MaxSubsteps = 60; //1フレームの移動を分割する最大回数
	constexpr int MaxSolverIterations = 6; //分割した移動1回につき重なりを解消する最大回数
	constexpr float CollisionSlop = 0.001f; //押し戻し後に重なりが残るのを防ぐ余白
	constexpr float GroundNormalThreshold = 0.5f; //床や天井として扱う法線のY成分の基準値
	constexpr float MaxSnapDistance = 0.15f; //接地中に追従できる下り段差と斜面の最大距離
	constexpr float MaxSnapUpwardVelocity = 0.000001f; //停止中の計算誤差として許容する上向き速度
	constexpr float MaxSnapPenetration = 0.01f; //接地面へ戻してよいわずかなめり込み量
	constexpr float GroundSupportTolerance = 0.002f; //面の境界と候補の距離比較に使う許容誤差

	//1回の衝突解決で見つかった最も深い衝突の情報
	struct CollisionHit
	{
		bool Hit = false;
		float Depth = 0.0f;
		Vector3 Correction{ 0.0f, 0.0f, 0.0f }; //プレイヤーを押し戻す方向と距離
		Vector3 Normal{ 0.0f, 0.0f, 0.0f }; //プレイヤーを押し戻す面の法線方向
		Vector3 PlatformDelta{ 0.0f, 0.0f, 0.0f }; //動く足場の前フレームからの移動量

		bool HasPlatformMotion = false; //動く足場か
		bool UseSurfaceTangentFollow = false; //法線方向を除き、接触面に沿う移動量だけ追従に使うか
		bool AppliesDamage = false; //接触したBoxからダメージを受けるか
		BreakPlatform* BreakPlatformObject = nullptr;
		JumpPad* JumpPadObject = nullptr;
	};

	//見つかった最も近い歩行可能面の情報
	struct GroundHit
	{
		bool Hit = false;
		float VerticalDistance = 0.0f; //接地面までに必要なワールドY方向の下降距離
		Vector3 Normal{ 0.0f, 1.0f, 0.0f };
		Vector3 PlatformDelta{ 0.0f, 0.0f, 0.0f }; 

		bool HasPlatformMotion = false; 
		bool UseSurfaceTangentFollow = false;
		bool AppliesDamage = false; //接触したBoxからダメージを受けるか
		BreakPlatform* BreakPlatformObject = nullptr; 
	};

	//指定方向から見たOBBの投影半径を求める
	float GetProjectedRadius(const Vector3 axis[3], const Vector3& half, const Vector3& direction)
	{
		return
			half.x * std::fabs(Vector3::dot(axis[0], direction)) +
			half.y * std::fabs(Vector3::dot(axis[1], direction)) +
			half.z * std::fabs(Vector3::dot(axis[2], direction));
	}

	//指定方向から見たOBB底面の投影半径を求める
	float GetFootProjectedRadius(const Vector3 axis[3], const Vector3& half, const Vector3& direction)
	{
		return
			half.x * std::fabs(Vector3::dot(axis[0], direction)) +
			half.z * std::fabs(Vector3::dot(axis[2], direction));
	}

	//現在の候補より深い衝突だった場合に、採用する衝突情報を更新する
	bool TryUpdateBestHit(OBBColliderComponent* playerCollider, OBBColliderComponent* targetCollider,
		bool preferHorizontalOnDownward, CollisionHit& bestHit)
	{
		if (!targetCollider)
			return false;

		//OBB判定を行う
		Vector3 correction;
		if (!playerCollider->ResolveCollision(targetCollider, correction, preferHorizontalOnDownward))
			return false;

		//重なりの深さを求める
		const float depth = correction.length();
		if (depth <= bestHit.Depth)
			return false;

		//現在より深い場合更新
		bestHit.Hit = true;
		bestHit.Depth = depth;
		bestHit.Correction = correction;
		bestHit.Normal = correction / depth;

		//採用対象が変わるため、前の候補に紐づくギミック固有情報を初期化
		bestHit.PlatformDelta = { 0.0f, 0.0f, 0.0f };
		bestHit.HasPlatformMotion = false;
		bestHit.UseSurfaceTangentFollow = false;
		bestHit.AppliesDamage = false;
		bestHit.BreakPlatformObject = nullptr;
		bestHit.JumpPadObject = nullptr;
		return true;
	}

	//現在より近い歩行可能面が足元にあれば、接地候補を更新
	bool TryUpdateBestGroundHit(OBBColliderComponent* playerCollider,
		OBBColliderComponent* targetCollider, GroundHit& bestHit)
	{
		if (!playerCollider || !targetCollider)
			return false;

		Vector3 playerCenter, playerHalf;
		Vector3 playerAxis[3];
		playerCollider->GetWorldOBB(playerCenter, playerAxis, playerHalf);

		Vector3 targetCenter, targetHalf;
		Vector3 targetAxis[3];
		targetCollider->GetWorldOBB(targetCenter, targetAxis, targetHalf);

		const float targetHalfValues[3] =
		{
			targetHalf.x,
			targetHalf.y,
			targetHalf.z
		};
		bool updatedBestHit = false;

		//対象OBBの6面からプレイヤーの足元にある歩行可能面を探す
		for (int faceAxisIndex = 0; faceAxisIndex < 3; ++faceAxisIndex)
		{
			for (int faceDirection = -1; faceDirection <= 1; faceDirection += 2)
			{
				//面の外側を向く法線を作る
				const Vector3 faceNormal =
					targetAxis[faceAxisIndex] * static_cast<float>(faceDirection);

				if (faceNormal.y <= GroundNormalThreshold)
					continue;

				const Vector3 faceCenter =
					targetCenter + faceNormal * targetHalfValues[faceAxisIndex];
				//面法線方向のプレイヤー半径
				const float playerRadiusOnNormal =
					GetProjectedRadius(playerAxis, playerHalf, faceNormal);
				//プレイヤーと面の距離
				const float distanceOnNormal =
					Vector3::dot(playerCenter - faceCenter, faceNormal) - playerRadiusOnNormal;
				const float verticalDistance = distanceOnNormal / faceNormal.y;

				//大きな上方向補正は許可せず、接地中に追従できる下方向だけを調べる
				if (verticalDistance < -MaxSnapPenetration ||
					verticalDistance > MaxSnapDistance + GroundSupportTolerance)
					continue;

				const Vector3 snappedPlayerCenter =
					playerCenter + Vector3{ 0.0f, -verticalDistance, 0.0f };
				const Vector3 snappedFootCenter =
					snappedPlayerCenter - playerAxis[1] * playerHalf.y;
				bool overlapsFaceBounds = true; //プレイヤーの足元が対象OBBの面の範囲内に重なっているか

				//プレイヤーの足元範囲が接地面に一部でも残っているか確認する
				for (int boundsAxisIndex = 0; boundsAxisIndex < 3; ++boundsAxisIndex)
				{
					if (boundsAxisIndex == faceAxisIndex)
						continue;

					const Vector3& boundsAxis = targetAxis[boundsAxisIndex];
					const float footRadius =
						GetFootProjectedRadius(playerAxis, playerHalf, boundsAxis);
					const float footCenterPosition =
						Vector3::dot(snappedFootCenter - targetCenter, boundsAxis);
					const float boundsLimit =
						targetHalfValues[boundsAxisIndex] + footRadius + GroundSupportTolerance;

					if (std::fabs(footCenterPosition) > boundsLimit)
					{
						overlapsFaceBounds = false;
						break;
					}
				}

				if (!overlapsFaceBounds)
					continue;

				//今回調べた面の方が少ない下方向移動で接地できるか
				const bool isCloser = !bestHit.Hit ||
					verticalDistance + GroundSupportTolerance < bestHit.VerticalDistance;
				//2つの面までの距離がほぼ同じか
				const bool isSameDistance = bestHit.Hit &&
					std::fabs(verticalDistance - bestHit.VerticalDistance) <= GroundSupportTolerance;
				//今回の面が現在の候補より上向きか
				const bool isFlatter = faceNormal.y > bestHit.Normal.y;
				if (!isCloser && !(isSameDistance && isFlatter))
					continue;

				bestHit.Hit = true;
				bestHit.VerticalDistance = verticalDistance;
				bestHit.Normal = faceNormal;
				bestHit.PlatformDelta = { 0.0f, 0.0f, 0.0f };
				bestHit.HasPlatformMotion = false;
				bestHit.UseSurfaceTangentFollow = false;
				bestHit.AppliesDamage = false;
				bestHit.BreakPlatformObject = nullptr;
				updatedBestHit = true;
			}
		}

		return updatedBestHit;
	}
}

//速度を分割して移動し、衝突の押し戻しと接地状態を解決
void Player::MoveAndResolveCollisions(OBBColliderComponent* collider, bool& groundedThisFrame)
{
	//プレイヤーを押し戻す対象を種類ごとに取得
	const auto terrainBlocks = Manager::GetScene()->GetGameObjects<TerrainBlock>();
	const auto boxes = Manager::GetScene()->GetGameObjects<Box>();
	const auto gates = Manager::GetScene()->GetGameObjects<Gate>();
	const auto goals = Manager::GetScene()->GetGameObjects<Goal>();
	const auto breakPlatforms = Manager::GetScene()->GetGameObjects<BreakPlatform>();
	const auto jumpPads = Manager::GetScene()->GetGameObjects<JumpPad>();
	const auto movingPlatforms = Manager::GetScene()->GetGameObjects<MovingPlatform>();

	//最後に接地した動く足場の追従情報を保持
	bool shouldFollowPlatform = false;
	Vector3 platformDelta{ 0.0f, 0.0f, 0.0f };
	Vector3 platformNormal{ 0.0f, 1.0f, 0.0f };
	bool useSurfaceTangentFollow = false;
	bool appliesDamage = false;

	//最後に接地した床へ合わせて足場追従に使う情報を更新
	const auto updatePlatformFollow = [&](bool hasPlatformMotion,
		const Vector3& delta, const Vector3& normal, bool useTangentFollow)
	{
		shouldFollowPlatform = hasPlatformMotion;
		if (!shouldFollowPlatform)
		{
			platformDelta = { 0.0f, 0.0f, 0.0f };
			useSurfaceTangentFollow = false;
			return;
		}

		platformDelta = delta;
		platformNormal = normal;
		useSurfaceTangentFollow = useTangentFollow;
	};

	//高速移動時のすり抜けを減らすため1フレームの移動を分割
	int steps = static_cast<int>(ceilf(m_Velocity.length() / MaxSubstepLength));
	if (steps < 1)
		steps = 1;
	if (steps > MaxSubsteps)
		steps = MaxSubsteps;

	const float substepRate = 1.0f / static_cast<float>(steps);
	for (int i = 0; i < steps; ++i)
	{
		//分割1回分移動
		const Vector3 substepDelta = m_Velocity * substepRate;
		m_Transform->m_Position += substepDelta;

		//複数オブジェクトとの重なりを順番に解消
		for (int iteration = 0; iteration < MaxSolverIterations; ++iteration)
		{
			//この反復で最も深い衝突と、その足場固有情報を保持
			CollisionHit bestHit;
			//強く上昇していない間は、側面寄りの下向き補正を水平へ置き換える
			const bool preferHorizontalOnDownward =
				m_Velocity.y <= GroundSnapVelocity;

			for (auto* terrainBlock : terrainBlocks)
			{
				TryUpdateBestHit(
					collider,
					terrainBlock->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit);
			}

			for (auto* box : boxes)
			{
				//当たり判定なしの場合判定しない
				if (!box->IsCollisionEnabled())
					continue;

				if (TryUpdateBestHit(
					collider,
					box->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit))
				{
					bestHit.AppliesDamage = box->IsDamageEnabled();

					//回転するBoxの場合移動量保存
					if (box->HasPlatformMotion())
					{
						bestHit.PlatformDelta = box->GetPlatformDeltaAtPosition(GetPosition());
						bestHit.HasPlatformMotion = true;
					}
				}
			}

			for (auto* gate : gates)
			{
				TryUpdateBestHit(
					collider,
					gate->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit);
			}

			for (auto* goal : goals)
			{
				TryUpdateBestHit(
					collider,
					goal->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit);
			}

			for (auto* breakPlatform : breakPlatforms)
			{
				if (!breakPlatform->IsActive())
					continue;

				if (TryUpdateBestHit(
					collider,
					breakPlatform->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit))
				{
					bestHit.BreakPlatformObject = breakPlatform;
				}
			}

			for (auto* movingPlatform : movingPlatforms)
			{
				if (TryUpdateBestHit(
					collider,
					movingPlatform->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit))
				{
					//移動量保存
					bestHit.PlatformDelta = movingPlatform->GetMoveDelta();
					bestHit.HasPlatformMotion = true;
					bestHit.UseSurfaceTangentFollow = true;
				}
			}

			for (auto* jumpPad : jumpPads)
			{
				if (TryUpdateBestHit(
					collider,
					jumpPad->GetComponent<OBBColliderComponent>(),
					preferHorizontalOnDownward,
					bestHit))
				{
					bestHit.JumpPadObject = jumpPad;
				}
			}

			if (!bestHit.Hit)
				break;

			const bool isGroundCollision = bestHit.Normal.y > GroundNormalThreshold &&
				m_Velocity.y <= GroundSnapVelocity;

			Vector3 collisionCorrection = bestHit.Correction + bestHit.Normal * CollisionSlop;
			if (isGroundCollision)
			{
				//歩行可能面は垂直に押し戻し、斜面法線の水平成分によるずり落ちを防ぐ
				const float correctionAlongNormal = Vector3::dot(collisionCorrection, bestHit.Normal);
				collisionCorrection = { 0.0f, correctionAlongNormal / bestHit.Normal.y, 0.0f };
			}

			//最も深い重なりを解消し、わずかな余白を作る
			m_Transform->m_Position += collisionCorrection;
			appliesDamage |= bestHit.AppliesDamage;

			//衝突面へ向かう速度成分を除き、壁や斜面に沿って移動できるようにする
			const float velocityAlongNormal = Vector3::dot(m_Velocity, bestHit.Normal);
			if (velocityAlongNormal < 0.0f)
				m_Velocity -= bestHit.Normal * velocityAlongNormal;

			//床との衝突
			if (isGroundCollision)
			{
				groundedThisFrame = true;
				m_Velocity.y = 0.0f;

				//JumpPadの場合着地処理
				if (bestHit.JumpPadObject)
				{
					bestHit.JumpPadObject->OnPlayerLanded(this);
					groundedThisFrame = false;
					//JumpPadから離れるため、直前の足場追従情報を解除する
					updatePlatformFollow(false, {}, {}, false);
				}
				else
				{
					//BreakPlatformの着地処理
					if (bestHit.BreakPlatformObject)
						bestHit.BreakPlatformObject->OnPlayerLanded(this);

					//静止床へ切り替わった場合も含め、最後の接地対象を反映する
					updatePlatformFollow(
						bestHit.HasPlatformMotion,
						bestHit.PlatformDelta,
						bestHit.Normal,
						bestHit.UseSurfaceTangentFollow);
				}
			}
			//天井との衝突
			else if (bestHit.Normal.y < -GroundNormalThreshold && m_Velocity.y > 0.0f)
				m_Velocity.y = 0.0f;
		}
	}

	//前フレームまで接地していて上昇していない場合だけ、下り斜面や小さな段差へ追従する
	//ジャンプ中や空中からの落下中には吸着させず、通常の衝突処理に任せる
	if (!groundedThisFrame && m_IsGrounded &&
		m_Velocity.y <= MaxSnapUpwardVelocity)
	{
		GroundHit groundHit;

		//全対象を調べ、配置順に依存せず足元から最も近い歩行可能面を採用する
		//JumpPadは斜面スナップで再着地させないため、ここでは接地候補から除外する
		for (auto* terrainBlock : terrainBlocks)
		{
			TryUpdateBestGroundHit(
				collider,
				terrainBlock->GetComponent<OBBColliderComponent>(),
				groundHit);
		}

		for (auto* box : boxes)
		{
			if (!box->IsCollisionEnabled())
				continue;

			if (TryUpdateBestGroundHit(
				collider,
				box->GetComponent<OBBColliderComponent>(),
				groundHit))
			{
				groundHit.AppliesDamage = box->IsDamageEnabled();

				//回転するBoxの場合移動量保存
				if (box->HasPlatformMotion())
				{
					groundHit.HasPlatformMotion = true;
					groundHit.PlatformDelta = box->GetPlatformDeltaAtPosition(GetPosition());
				}
			}
		}

		for (auto* gate : gates)
		{
			TryUpdateBestGroundHit(
				collider,
				gate->GetComponent<OBBColliderComponent>(),
				groundHit);
		}
		
		for (auto* goal : goals)
		{
			TryUpdateBestGroundHit(
				collider,
				goal->GetComponent<OBBColliderComponent>(),
				groundHit);
		}

		for (auto* breakPlatform : breakPlatforms)
		{
			if (!breakPlatform->IsActive())
				continue;

			if (TryUpdateBestGroundHit(
				collider,
				breakPlatform->GetComponent<OBBColliderComponent>(),
				groundHit))
			{
				groundHit.BreakPlatformObject = breakPlatform;
			}
		}

		for (auto* movingPlatform : movingPlatforms)
		{
			if (TryUpdateBestGroundHit(
				collider,
				movingPlatform->GetComponent<OBBColliderComponent>(),
				groundHit))
			{
				//移動量保存
				groundHit.HasPlatformMotion = true;
				groundHit.PlatformDelta = movingPlatform->GetMoveDelta();
				groundHit.UseSurfaceTangentFollow = true;
			}
		}

		//選ばれた面に接地させる
		if (groundHit.Hit)
		{
			//水平位置を変えず、歩行可能面までワールドY方向に追従する
			m_Transform->m_Position.y -= groundHit.VerticalDistance;
			groundedThisFrame = true;
			m_Velocity.y = 0.0f;
			appliesDamage |= groundHit.AppliesDamage;

			if (groundHit.BreakPlatformObject)
				groundHit.BreakPlatformObject->OnPlayerLanded(this);

			updatePlatformFollow(
				groundHit.HasPlatformMotion,
				groundHit.PlatformDelta,
				groundHit.Normal,
				groundHit.UseSurfaceTangentFollow);
		}
	}

	//接地が確定した場合だけ、移動する床の追従処理を行う
	if (groundedThisFrame && shouldFollowPlatform)
	{
		Vector3 followDelta = platformDelta;
		if (useSurfaceTangentFollow)
		{
			//法線方向は衝突補正と斜面スナップで反映済みなので、接線方向だけを加える
			followDelta -= platformNormal *
				Vector3::dot(platformDelta, platformNormal);
		}

		m_Transform->m_Position += followDelta;
	}

	//複数回の押し戻しが発生しても、接触ダメージは1フレームに1回だけ処理する
	if (appliesDamage)
		TakeDamage(1);
}
