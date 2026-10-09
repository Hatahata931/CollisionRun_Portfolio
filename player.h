//-----------------------------------------------------------
//  [ player.h ]
//  プレイヤーキャラクターの制御
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class AnimationModel;
class Camera;
class OBBColliderComponent;
class Shadow;

class Player : public GameObject
{
private:
	enum class AnimationState
	{
		Idle,
		Walk,
		Run,
		JumpUp,
		JumpAir,
		JumpDown,
		Landing,
	};

	//各アニメーションの最終フレーム番号
	static constexpr float JumpUpLastFrame = 14.0f;
	static constexpr float JumpDownLastFrame = 8.0f;
	static constexpr float LandingLastFrame = 34.0f;

	//各アニメーションの1フレームあたりの進行量
	static constexpr float GroundAnimationFrameAdvance = 0.5f;
	static constexpr float JumpUpFrameAdvance = 0.5f;
	static constexpr float JumpDownFrameAdvance = 0.5f;
	static constexpr float LandingFrameAdvance = 0.5f;

	static constexpr float LandingCancelableFrame = 4.0f; //移動アニメーションへ切り替えてよい着地フレーム
	static constexpr float AnimationBlendSpeed = 0.15f; //アニメーション切り替え時の補間速度

	//移動・復帰
	static constexpr float Gravity = 0.011f;
	static constexpr float MaxFallSpeed = 0.35f;
	static constexpr float GroundSnapVelocity = 0.12f; //接地・斜面追従を許可する上向き速度の上限
	static constexpr float JumpPower = 0.24f;
	static constexpr int CoyoteTimeFrames = 3; //足場から離れた直後もジャンプできる猶予時間
	static constexpr int JumpBufferFrames = 6; //着地前のジャンプ入力を保存する時間
	static constexpr float WalkSpeed = 0.06f;
	static constexpr float DashSpeed = 0.15f;
	static constexpr float GroundAccel = 0.13f; //加速割合
	static constexpr float GroundDecel = 0.28f; //減速割合
	static constexpr int RespawnDelayFrames = 60; //落下してから復帰までにかかる時間

	//ダメージ
	static constexpr int DamageCooldownFrames = 120; //ダメージ後の無敵時間
	static constexpr int DamageBlinkIntervalFrames = 6; //ダメージ演出のプレイヤー点滅間隔

	//足音
	static constexpr int WalkFootstepIntervalFrames = 38; //歩行時の足音間隔
	static constexpr int DashFootstepIntervalFrames = 20; //ダッシュ時の足音間隔
	static constexpr int MinLandingAirFrames = 6; //着地扱いに必要な最低滞空フレーム数
	static constexpr float FootstepMinSpeed = 0.01f; //足音を鳴らす最低水平移動速度

	//専用ライト
	static constexpr float CharacterLightAmbient = 0.5f; //環境光
	static constexpr float CharacterLightDiffuse = 0.9f; //拡散光

	//入力・移動
	//移動入力から移動方向とプレイヤーの向きを求める
	void HandleInput(Camera* camera, Vector3& move, float& desiredYaw, bool& isMoving);
	//接地猶予と入力保存を使ってジャンプを開始
	void UpdateJump();
	//重力と最大落下速度を反映
	void ApplyGravity();
	//足音と着地音を更新し、本当の着地が発生したフレームだけtrueを返す
	bool UpdateMovementSounds(bool isMoving, bool wasGrounded, bool groundedThisFrame);
	//同じ音が連続しないよう、足音を配列の順番で再生
	void PlayNextFootstep();

	//当たり判定
	//速度を分割して移動し、衝突の押し戻しと接地状態を解決
	void MoveAndResolveCollisions(OBBColliderComponent* collider, bool& groundedThisFrame);

	//ダメージ・落下・復帰
	//落下判定ラインを越えたか確認し、復帰待機を開始
	void CheckFallAndStartRespawn(bool& groundedThisFrame);
	void UpdateRespawn();

	//表示・アニメーション
	//次に再生するアニメーションを決定
	void UpdateAnimationState(bool isMoving, bool landedThisFrame);
	//モデルの姿勢、再生フレーム、切り替え補間を更新
	void UpdateAnimationPlayback();
	void ChangeAnimationState(AnimationState nextState);
	//状態に応じて現在と次のアニメーション再生位置を進める
	void AdvanceAnimationFrames();
	static const char* GetAnimationName(AnimationState state);
	static float GetAnimationStartFrame(AnimationState state);
	//指定したアニメーションがループ再生か確認
	static bool IsLoopingAnimation(AnimationState state);
	//指定したアニメーションが空中用か確認
	static bool IsAirborneAnimation(AnimationState state);
	//モデルを描画するか確認
	bool ShouldDrawModel() const;
	//影との追従関係を切り離してPlayerの削除を予約
	void RequestDestroy();

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	AnimationModel* m_AnimationModel = nullptr;
	AnimationState m_CurrentAnimationState = AnimationState::Idle;
	AnimationState m_NextAnimationState = AnimationState::Idle;
	float m_CurrentAnimationFrame = 0.0f;
	float m_NextAnimationFrame = 0.0f;
	float m_AnimationBlend = 1.0f; //現在と次のアニメーションのブレンド率

	Shadow* m_Shadow = nullptr;

	Vector3 m_Velocity{ 0.0f, 0.0f, 0.0f };
	bool m_IsGrounded = false; //直近の衝突判定で確定した接地状態
	int m_CoyoteTimer = 0;
	int m_JumpBufferTimer = 0;
	bool m_IsDashing = false;
	bool m_JumpAnimationRequested = false; //ジャンプアニメーションの開始要求
	bool m_IsGameplayEnabled = true; //入力や移動などのゲームプレイ処理を行うか

	Vector3 m_SpawnPosition{ 0.0f, 0.0f, 0.0f };
	bool m_SpawnPositionInitialized = false; //初期復帰位置を更新したか
	float m_FallLimitY = -20.0f; //ステージ側で未設定の場合の落下判定位置
	int m_RespawnTimer = 0;
	bool m_IsRespawning = false; //復帰待機中か

	int m_DamageCooldownTimer = 0;
	int m_FootstepTimer = 0;
	int m_FootstepSoundIndex = 0; //次に再生する足音の番号
	int m_AirborneFrames = 0; //着地判定に使う連続滞空フレーム数

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void TransitionUpdate() override;
	void Draw() override;

	//JumpPadなど外部のギミックから上向き速度を設定する
	void Launch(float power);
	//Windなど外部のギミックから速度を加算する
	void AddForce(const Vector3& force);

	//入力・物理処理を有効または無効にする
	void SetGameplayEnabled(bool enabled);

	//ダメージを受け、ignoreCooldownがtrueなら無敵時間を無視する
	void TakeDamage(int amount, bool ignoreCooldown = false);

	bool IsGameplayEnabled() const { return m_IsGameplayEnabled; }
	void SetFallLimitY(float fallLimitY) { m_FallLimitY = fallLimitY; }
	bool IsRespawning() const { return m_IsRespawning; }
	bool IsGround() const { return m_IsGrounded; }
	void SetRespawnPosition(const Vector3& position);
};
