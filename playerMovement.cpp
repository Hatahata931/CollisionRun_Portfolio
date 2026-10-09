//-----------------------------------------------------------
//  [ playerMovement.cpp ]
//  プレイヤーの入力・移動
//-----------------------------------------------------------
#include "main.h"
#include "player.h"
#include "audioManager.h"
#include "camera.h"
#include "effectManager.h"
#include "input.h"

//補助定数
namespace
{
	//プレイヤーの位置に対するエフェクトの発生位置のずらし量
	constexpr float LandingEffectHeightOffset = 0.1f;
	constexpr float DashEffectHeightOffset = 0.04f;
	constexpr float DashEffectBackOffset = 0.18f;
}

//移動入力から移動方向とプレイヤーの向きを求める
void Player::HandleInput(Camera* camera, Vector3& move, float& desiredYaw, bool& isMoving)
{
	Vector3 forward = camera->GetForward();
	forward.y = 0.0f;
	forward.normalize();

	Vector3 right = camera->GetRight();
	right.y = 0.0f;
	right.normalize();

	if (Input::GetKeyPress('W'))
		move += forward;

	if (Input::GetKeyPress('S'))
		move -= forward;

	if (Input::GetKeyPress('A'))
		move -= right;

	if (Input::GetKeyPress('D'))
		move += right;

	//すべての入力を合成した移動方向へプレイヤーモデルを向ける
	if (move.length() > 0.0f)
	{
		desiredYaw = atan2f(move.x, move.z);
		isMoving = true;
	}
}

//接地猶予と入力保存を使ってジャンプを開始
void Player::UpdateJump()
{
	//接地中は足場から離れた直後に使えるジャンプ猶予を補充
	if (m_IsGrounded)
		m_CoyoteTimer = CoyoteTimeFrames;

	//押した瞬間に接地していなくても短時間入力を保存
	if (Input::GetKeyTrigger(VK_SPACE))
		m_JumpBufferTimer = JumpBufferFrames;

	const bool canJump = m_IsGrounded || m_CoyoteTimer > 0;
	if (canJump && m_JumpBufferTimer > 0)
	{
		AudioManager::PlaySE(SEId::Jump);
		m_Velocity.y = JumpPower;
		m_IsGrounded = false;
		m_JumpAnimationRequested = true;
		m_CoyoteTimer = 0;
		m_JumpBufferTimer = 0;
		return;
	}

	if (!m_IsGrounded && m_CoyoteTimer > 0)
		m_CoyoteTimer--;

	if (m_JumpBufferTimer > 0)
		m_JumpBufferTimer--;
}

//重力と最大落下速度を反映
void Player::ApplyGravity()
{
	if (!m_IsGrounded)
	{
		m_Velocity.y -= Gravity;

		//最大落下速度
		if (m_Velocity.y < -MaxFallSpeed)
			m_Velocity.y = -MaxFallSpeed;
	}
}

//足音と着地音を更新し、本当の着地が発生したフレームだけtrueを返す
bool Player::UpdateMovementSounds(bool isMoving, bool wasGrounded, bool groundedThisFrame)
{
	//復帰待機中
	if (m_IsRespawning)
	{
		m_FootstepTimer = 0;
		m_AirborneFrames = 0;
		return false;
	}

	//接地していない
	if (!groundedThisFrame)
	{
		//着地判定用の滞空フレーム数を上限まで加算する
		if (m_AirborneFrames < MinLandingAirFrames)
			m_AirborneFrames++;

		//実際の滞空と確定した場合だけ足音タイマーをリセットする
		if (m_AirborneFrames >= MinLandingAirFrames)
			m_FootstepTimer = 0;

		return false;
	}

	//前のフレームで接地しておらず、かつMinLandingAirFrames以上空中にいる
	const bool landed = !wasGrounded && m_AirborneFrames >= MinLandingAirFrames;
	m_AirborneFrames = 0;

	//十分な滞空後に着地した場合
	if (landed)
	{
		AudioManager::PlaySE(SEId::Landing);

		Vector3 effectPosition = m_Transform->m_Position;
		effectPosition.y += LandingEffectHeightOffset;
		EffectManager::Play(EffectId::PlayerLanding, effectPosition);

		//着地音と足音が同じフレームで重ならないよう、次の足音まで間隔を空ける
		m_FootstepTimer = m_IsDashing ? DashFootstepIntervalFrames : WalkFootstepIntervalFrames;

		return true;
	}

	//入力中でも実際に移動できているか水平速度で確認する
	const float horizontalSpeedSq = m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z;
	if (!isMoving)
	{
		m_FootstepTimer = 0;
		return false;
	}

	//衝突で一時的に低速になった場合は、足音間隔を最初からやり直さず維持する
	const float minFootstepSpeedSq = FootstepMinSpeed * FootstepMinSpeed;
	if (horizontalSpeedSq < minFootstepSpeedSq)
		return false;

	if (m_FootstepTimer > 0)
		m_FootstepTimer--;

	if (m_FootstepTimer > 0)
		return false;

	PlayNextFootstep();

	if (m_IsDashing)
	{
		Vector3 movementDirection{ m_Velocity.x, 0.0f, m_Velocity.z };
		if (movementDirection.length() > 0.0f)
		{
			movementDirection.normalize();

			Vector3 effectPosition = m_Transform->m_Position - movementDirection * DashEffectBackOffset;
			effectPosition.y += DashEffectHeightOffset;
			EffectManager::Play(EffectId::PlayerDash, effectPosition);
		}
	}

	m_FootstepTimer = m_IsDashing ? DashFootstepIntervalFrames : WalkFootstepIntervalFrames;

	return false;
}

//同じ音が連続しないよう、足音を配列の順番で再生
void Player::PlayNextFootstep()
{
	static constexpr SEId FootstepSounds[]
	{
		SEId::Footstep1,
		SEId::Footstep2,
		SEId::Footstep3,
		SEId::Footstep4,
		SEId::Footstep5,
	};
	static constexpr int FootstepSoundCount =
		static_cast<int>(sizeof(FootstepSounds) / sizeof(FootstepSounds[0]));

	AudioManager::PlaySE(FootstepSounds[m_FootstepSoundIndex]);
	m_FootstepSoundIndex =
		(m_FootstepSoundIndex + 1) % FootstepSoundCount;
}

//JumpPadなど外部のギミックから上向き速度を設定する
void Player::Launch(float power)
{
	if (!m_IsGameplayEnabled)
		return;

	m_Velocity.y = power;
	m_IsGrounded = false;
	m_CoyoteTimer = 0;
	m_JumpBufferTimer = 0;
	m_JumpAnimationRequested = true;
}

//Windなど外部のギミックから速度を加算する
void Player::AddForce(const Vector3& force)
{
	if (!m_IsGameplayEnabled)
		return;

	m_Velocity += force;
	//上向きの力が加わった場合はジャンプアニメーションを開始する
	if (force.y > 0.0f)
		m_JumpAnimationRequested = true;
}
