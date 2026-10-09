//-----------------------------------------------------------
//  [ playerRespawn.cpp ]
//  プレイヤーのダメージ・落下・復帰処理
//-----------------------------------------------------------
#include "main.h"
#include "player.h"
#include "audioManager.h"
#include "life.h"
#include "manager.h"
#include "scene.h"

void Player::SetRespawnPosition(const Vector3& position)
{
	m_SpawnPosition = position;
	m_SpawnPositionInitialized = true;
}

//ダメージを受け、ignoreCooldownがtrueなら無敵時間を無視する
void Player::TakeDamage(int amount, bool ignoreCooldown)
{
	if (!m_IsGameplayEnabled)
		return;

	if (!ignoreCooldown && m_DamageCooldownTimer > 0)
		return;

	auto* life = Manager::GetScene()->GetGameObject<Life>();
	if (!life)
		return;

	life->Add(-amount);
	AudioManager::PlaySE(SEId::Damage);
	m_DamageCooldownTimer = DamageCooldownFrames;

	if (life->GetValue() <= 0)
		RequestDestroy();
}

//落下判定ラインを越えたか確認し、復帰待機を開始
void Player::CheckFallAndStartRespawn(bool& groundedThisFrame)
{
	if (m_Transform->m_Position.y > m_FallLimitY)
		return;

	//復帰まで横移動を止め、落下だけを継続させる
	m_IsRespawning = true;
	m_RespawnTimer = RespawnDelayFrames;
	m_Velocity.x = 0.0f;
	m_Velocity.z = 0.0f;
	groundedThisFrame = false;
	m_IsGrounded = false;
	m_CoyoteTimer = 0;
	m_JumpBufferTimer = 0;
}

void Player::UpdateRespawn()
{
	ApplyGravity();

	//当たり判定を行わず、復帰演出中のプレイヤーを落下させる
	m_Transform->m_Position.y += m_Velocity.y;

	//復帰待機中も通常ダメージの無敵時間を進める
	if (m_DamageCooldownTimer > 0)
		m_DamageCooldownTimer--;

	if (m_RespawnTimer > 0)
		m_RespawnTimer--;
	if (m_RespawnTimer > 0)
		return;

	//落下は通常ダメージの無敵時間に関係なく必ずダメージを与える
	TakeDamage(1, true);
	//ライフがなくなり削除予定になった場合は復帰させない
	if (m_Destroy)
		return;

	//復帰処理
	m_Transform->m_Position = m_SpawnPosition;
	m_Velocity = { 0.0f, 0.0f, 0.0f };
	m_IsGrounded = false;
	m_IsDashing = false;
	m_FootstepTimer = 0;
	m_AirborneFrames = 0;
	m_IsRespawning = false;
}
