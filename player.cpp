//-----------------------------------------------------------
//  [ player.cpp ]
//  プレイヤーキャラクターの制御
//-----------------------------------------------------------
#include "main.h"
#include "player.h"
#include "animationModel.h"
#include "camera.h"
#include "input.h"
#include "manager.h"
#include "obbColliderComponent.h"
#include "renderer.h"
#include "scene.h"
#include "shadow.h"
#include "transformComponent.h"

void Player::Init()
{
	GameObject::Init();

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.16f, 0.5f, 0.16f });
	collider->SetCenterOffset({ 0.0f, 0.5f, 0.0f });

	collider->SetUseScale(false);
	collider->SetUseRotation(false);

	m_AnimationModel = new AnimationModel();
	m_AnimationModel->Load("asset\\model\\unitychan.fbx");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_Run.fbx", "Run");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_Walk.fbx", "Walk");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_Idle.fbx", "Idle");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_JumpUp.fbx", "JumpUp");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_JumpDown.fbx", "JumpDown");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_Landing.fbx", "Landing");
	m_AnimationModel->SetRootMotionNode("center");
	m_AnimationModel->SetLightOverride(CharacterLightAmbient, CharacterLightDiffuse);

	m_Transform->m_Scale = { 0.01f, 0.01f, 0.01f };

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\vertexLightingVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\vertexLightingPS.cso");

	m_Shadow = Manager::GetScene()->AddGameObject<Shadow>(Scene::OBJECT3D);
	m_Shadow->SetScale({ 2.0f, 2.0f, 2.0f });
	m_Shadow->SetTarget(this);
}

void Player::Uninit()
{
	m_AnimationModel->Uninit();
	delete m_AnimationModel;

	m_VertexLayout->Release();
	m_VertexShader->Release();
	m_PixelShader->Release();

	GameObject::Uninit();
}

//影との追従関係を切り離してPlayerの削除を予約
void Player::RequestDestroy()
{
	if (m_Shadow)
	{
		//影が破棄済みのプレイヤーを参照しないよう追従を解除する
		m_Shadow->SetTarget(nullptr);
		m_Shadow->SetDestroy();
		m_Shadow = nullptr;
	}

	SetDestroy();
}

void Player::Update()
{
	GameObject::Update();

	//プレイヤー生成時の位置を最初の復帰地点にする
	if (!m_SpawnPositionInitialized)
	{
		m_SpawnPosition = GetPosition();
		m_SpawnPositionInitialized = true;
	}

	//クリア演出中はゲームプレイを止めアニメーションのみ更新
	if (!m_IsGameplayEnabled)
	{
		UpdateAnimationState(false, false);
		UpdateAnimationPlayback();
		return;
	}

	//復帰待機中は復帰処理のみ行い、通常の更新は行わない
	if (m_IsRespawning)
	{
		UpdateRespawn();
		return;
	}

	auto collider = GetComponent<OBBColliderComponent>();

	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();

	Vector3 move{ 0.0f, 0.0f, 0.0f };
	//着地判定で比較できるよう、当たり判定前の接地状態を保存する
	const bool wasGrounded = m_IsGrounded;
	bool isMoving = false;
	float desiredYaw = m_Transform->m_Rotation.y;

	HandleInput(camera, move, desiredYaw, isMoving); //入力処理

	m_IsDashing = Input::GetKeyPress(VK_SHIFT) || Input::GetKeyPress(VK_LSHIFT) || Input::GetKeyPress(VK_RSHIFT);

	if (move.length() > 0.0f)
		move.normalize();

	//速度計算
	const float currentMoveSpeed = m_IsDashing ? DashSpeed : WalkSpeed;
	const Vector3 targetVelocity = move * currentMoveSpeed;
	const float accelRate = isMoving ? GroundAccel : GroundDecel;
	m_Velocity.x += (targetVelocity.x - m_Velocity.x) * accelRate;
	m_Velocity.z += (targetVelocity.z - m_Velocity.z) * accelRate;

	UpdateJump(); //ジャンプ入力
	ApplyGravity(); //重力

	//今回の移動と当たり判定から現在フレームの接地状態を求める
	bool groundedThisFrame = false;
	MoveAndResolveCollisions(collider, groundedThisFrame);

	m_Transform->m_Rotation.y = desiredYaw;

	CheckFallAndStartRespawn(groundedThisFrame);
	//接地状態を上書きする前に着地音と着地アニメーションの開始を判定する
	const bool landedThisFrame =
		UpdateMovementSounds(isMoving, wasGrounded, groundedThisFrame);

	//今回の判定結果を次のフレームで使う接地状態として保存する
	m_IsGrounded = groundedThisFrame;

	//アニメーション状態決定
	UpdateAnimationState(isMoving, landedThisFrame);

	if (m_DamageCooldownTimer > 0)
		m_DamageCooldownTimer--;

	//モデルの姿勢、再生フレーム、切り替え補間を更新
	UpdateAnimationPlayback();
}

//入力・物理処理を有効または無効にする
void Player::SetGameplayEnabled(bool enabled)
{
	if (m_IsGameplayEnabled == enabled)
		return;

	m_IsGameplayEnabled = enabled;
	if (enabled)
		return;

	//演出中に直前の移動や無敵点滅が残らないよう、ゲームプレイ用の状態を止める
	m_Velocity = { 0.0f, 0.0f, 0.0f };
	m_IsDashing = false;
	m_FootstepTimer = 0;
	m_AirborneFrames = 0;
	m_CoyoteTimer = 0;
	m_JumpBufferTimer = 0;
	m_JumpAnimationRequested = false;
	m_DamageCooldownTimer = 0;
}

void Player::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	const XMMATRIX scale = XMMatrixScaling(m_Transform->m_Scale.x,m_Transform->m_Scale.y,m_Transform->m_Scale.z);
	const XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Transform->m_Rotation.x,m_Transform->m_Rotation.y,m_Transform->m_Rotation.z);
	const XMMATRIX translation = XMMatrixTranslation(m_Transform->m_Position.x,m_Transform->m_Position.y,m_Transform->m_Position.z);
	const XMMATRIX world = scale * rotation * translation;

	Renderer::SetWorldMatrix(world);

	//無敵時間中はモデルを点滅
	if (ShouldDrawModel())
		m_AnimationModel->Draw();

	GameObject::Draw();
}

//モデルを描画するか確認
bool Player::ShouldDrawModel() const
{
	//復帰待機中はダメージ点滅より優先して、落下する姿を表示する
	if (m_IsRespawning)
		return true;

	if (m_DamageCooldownTimer <= 0)
		return true;

	const int elapsedFrames = DamageCooldownFrames - m_DamageCooldownTimer;
	return (elapsedFrames / DamageBlinkIntervalFrames) % 2 != 0;
}
