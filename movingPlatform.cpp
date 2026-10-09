//-----------------------------------------------------------
//  [ movingPlatform.cpp ]
//  往復移動する足場
//-----------------------------------------------------------
#include "main.h"
#include "movingPlatform.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"

void MovingPlatform::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\moving_platform.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.5f, 0.15f, 0.5f });
}

void MovingPlatform::Update()
{
	GameObject::Update();

	m_MoveDelta = { 0.0f, 0.0f, 0.0f };

	//配置後の位置を移動開始地点として初回Updateで保存
	if (!m_IsInitialized)
	{
		m_StartPosition = GetPosition();
		m_IsInitialized = true;
	}

	//折り返し地点での停止中は移動しない
	if (m_WaitTimer > 0)
	{
		m_WaitTimer--;
		return;
	}

	const Vector3 previousPosition = GetPosition();
	const float direction = m_IsMovingToEnd ? 1.0f : -1.0f;
	m_MoveRate += m_Speed * direction;

	//端に到着したら向きを反転
	if (m_MoveRate >= 1.0f)
	{
		m_MoveRate = 1.0f;
		m_IsMovingToEnd = false;
		m_WaitTimer = m_WaitFrames;
	}
	else if (m_MoveRate <= 0.0f)
	{
		m_MoveRate = 0.0f;
		m_IsMovingToEnd = true;
		m_WaitTimer = m_WaitFrames;
	}

	//開始位置と終了位置の間を補間して位置を決める
	const Vector3 position = m_StartPosition + (m_EndPosition - m_StartPosition) * m_MoveRate;
	SetPosition(position);
	m_MoveDelta = position - previousPosition;
}
