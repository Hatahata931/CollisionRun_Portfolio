//-----------------------------------------------------------
//  [ camera.cpp ]
//  プレイヤーを追従するカメラ
//-----------------------------------------------------------
#include "main.h"
#include "camera.h"
#include "imguiManager.h"
#include "input.h"
#include "manager.h"
#include "player.h"
#include "renderer.h"
#include "scene.h"
#ifdef _DEBUG
#include "stageEditor.h"
#endif

void Camera::Init()
{
	GameObject::Init();

	m_Yaw = 0.0f;
	m_Pitch = DefaultPitch;
	m_CurrentDistance = DefaultDistance;
	m_TargetDistance = DefaultDistance;
	m_Target = { 0.0f, 0.0f, 0.0f };
	m_Transform->m_Rotation = { m_Pitch, m_Yaw, 0.0f };
	m_FollowInitialized = false;
	m_WasRespawning = false;
	UpdateOrbitPosition();
	//描画前でも取得できるよう初期行列を作成
	UpdateMatrices();
}

void Camera::Update()
{
	Player* player = Manager::GetScene()->GetGameObject<Player>();

	if (!player)
		return;

	//プレイヤーの落下演出中は操作と追従をしない
	if (player->IsRespawning())
	{
		m_WasRespawning = true;
		return;
	}

	//最初のフレームと復帰後は補間せずにすぐプレイヤーに注視点を合わせる
	if (!m_FollowInitialized || m_WasRespawning)
		SnapTargetToPlayer(player);

	if (player->IsGameplayEnabled())
		HandleInput(player);

	//プレイヤーの移動処理が同じフレームのカメラ方向を参照できるように角度を先に反映する
	UpdateOrbitPosition();
}

void Camera::LateUpdate()
{
	Player* player = Manager::GetScene()->GetGameObject<Player>();
	if (!player)
	{
		UpdateMatrices();
		return;
	}

	//落下演出中は操作と追従をしない
	if (player->IsRespawning())
	{
		m_WasRespawning = true;
		UpdateMatrices();
		return;
	}

	//最初のフレームと復帰後は補間せずにすぐプレイヤーに注視点を合わせる
	if (!m_FollowInitialized || m_WasRespawning)
		SnapTargetToPlayer(player);
	else
		UpdateFollowTarget(player); //滑らかに追従させる

	//ホイール操作時の急な距離変化を抑える
	m_CurrentDistance += (m_TargetDistance - m_CurrentDistance) * ZoomFollowRate;
	UpdateOrbitPosition();
	UpdateMatrices();
}

//カメラ回転・ズーム入力処理
void Camera::HandleInput(Player* player)
{
	const bool mouseAvailable = !ImGuiManager::WantsMouseCapture();
	const bool keyboardAvailable = !ImGuiManager::WantsKeyboardCapture();

#ifdef _DEBUG
	const bool requiresRightDrag = StageEditor::IsOpen();
#else
	const bool requiresRightDrag = false;
#endif

	//マウスで視点移動
	//エディター表示中はギズモ操作と競合しないよう右クリックしながら
	if (mouseAvailable &&
		(!requiresRightDrag || Input::GetKeyPress(VK_RBUTTON)))
	{
		m_Yaw += static_cast<float>(Input::GetMouseDeltaX()) * MouseRotationSensitivity;
		m_Pitch += static_cast<float>(Input::GetMouseDeltaY()) * MouseRotationSensitivity;
	}

	//キーボードでのカメラ操作
	if (keyboardAvailable)
	{
		//矢印キーで回転
		if (Input::GetKeyPress(VK_LEFT))
			m_Yaw -= KeyboardRotationSpeed;
		if (Input::GetKeyPress(VK_RIGHT))
			m_Yaw += KeyboardRotationSpeed;
		if (Input::GetKeyPress(VK_UP))
			m_Pitch -= KeyboardRotationSpeed;
		if (Input::GetKeyPress(VK_DOWN))
			m_Pitch += KeyboardRotationSpeed;

		//Cキーでカメラリセット
		if (Input::GetKeyTrigger('C'))
		{
			m_Yaw = player->GetRotation().y;
			m_Pitch = DefaultPitch;
		}
	}

	//マウスホイールでズーム
	if (mouseAvailable)
	{
		const float wheelSteps =
			static_cast<float>(Input::GetMouseWheelDelta()) / WHEEL_DELTA;
		m_TargetDistance -= wheelSteps * ZoomStep;
	}

	if (m_Pitch < MinPitch)
		m_Pitch = MinPitch;
	if (m_Pitch > MaxPitch)
		m_Pitch = MaxPitch;
	if (m_TargetDistance < MinDistance)
		m_TargetDistance = MinDistance;
	if (m_TargetDistance > MaxDistance)
		m_TargetDistance = MaxDistance;

	//回転し続けても角度が増え続けないよう、-PI～PIに収める
	while (m_Yaw > XM_PI)
		m_Yaw -= XM_2PI;
	while (m_Yaw < -XM_PI)
		m_Yaw += XM_2PI;
}

//注視点を即座にプレイヤーに合わせる
void Camera::SnapTargetToPlayer(Player* player)
{
	m_Target = player->GetPosition() + Vector3(0.0f, TargetHeight, 0.0f);
	m_FollowInitialized = true;
	m_WasRespawning = false;
}

//注視点を少しずつ合わせる
void Camera::UpdateFollowTarget(Player* player)
{
	//最終的な目標注視点
	const Vector3 desiredTarget =
		player->GetPosition() + Vector3(0.0f, TargetHeight, 0.0f);

	//水平と上下で追従率を分けてジャンプ時の上下移動だけ少し穏やかにする
	m_Target.x += (desiredTarget.x - m_Target.x) * HorizontalFollowRate;
	m_Target.y += (desiredTarget.y - m_Target.y) * VerticalFollowRate;
	m_Target.z += (desiredTarget.z - m_Target.z) * HorizontalFollowRate;
}

//注視点を中心としたカメラ位置を計算
void Camera::UpdateOrbitPosition()
{
	const float horizontalDistance = cosf(m_Pitch) * m_CurrentDistance;
	const Vector3 offset
	{
		-sinf(m_Yaw) * horizontalDistance,
		sinf(m_Pitch) * m_CurrentDistance,
		-cosf(m_Yaw) * horizontalDistance
	};

	m_Transform->m_Position = m_Target + offset;
	m_Transform->m_Rotation = { m_Pitch, m_Yaw, 0.0f };
}

//ビュー行列、プロジェクション行列を更新
void Camera::UpdateMatrices()
{
	m_Projection = XMMatrixPerspectiveFovLH(FieldOfView,
		static_cast<float>(SCREEN_WIDTH) / SCREEN_HEIGHT,
		NearClip, FarClip);

	const XMFLOAT3 eye{ m_Transform->m_Position.x, m_Transform->m_Position.y, m_Transform->m_Position.z };
	const XMFLOAT3 target{ m_Target.x, m_Target.y, m_Target.z };
	const XMFLOAT3 up{ 0.0f, 1.0f, 0.0f };
	m_View = XMMatrixLookAtLH(XMLoadFloat3(&eye), XMLoadFloat3(&target), XMLoadFloat3(&up));
}

void Camera::Draw()
{
	//LateUpdateで確定した行列を描画へ渡す
	Renderer::SetProjectionMatrix(m_Projection);
	Renderer::SetViewMatrix(m_View);
	Renderer::SetCameraPosition(m_Transform->m_Position);
}

//カメラの向いている方向を取得
Vector3 Camera::GetForward()
{
	Vector3 forward = m_Target - m_Transform->m_Position;
	forward.normalize();

	return forward;
}
