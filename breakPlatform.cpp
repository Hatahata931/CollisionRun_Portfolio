//-----------------------------------------------------------
//  [ breakPlatform.cpp ]
//  乗ると崩れる足場
//-----------------------------------------------------------
#include "main.h"
#include "breakPlatform.h"

#include <algorithm>
#include <cmath>

#include "audioManager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"

void BreakPlatform::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\break_platform.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.5f, 0.1f, 0.5f });
}

void BreakPlatform::Update()
{
	GameObject::Update();

	switch (m_State)
	{
	case BreakPlatformState::Idle:
		break;

	case BreakPlatformState::Breaking:
		m_Timer++;
		UpdateShake(); //崩れる前に揺らす
		if (m_Timer >= BreakWaitFrames)
		{
			ResetShake();
			m_State = BreakPlatformState::Hidden;
			m_Timer = 0;
		}
		break;

	case BreakPlatformState::Hidden:
		m_Timer++;
		if (m_Timer >= HiddenFrames)
		{
			ResetShake();
			m_State = BreakPlatformState::Idle;
			m_Timer = 0;
		}
		break;
	}
}

void BreakPlatform::Draw()
{
	if (!IsActive())
		return;

	GameObject::Draw();
}

void BreakPlatform::UpdateShake()
{
	auto renderer = GetComponent<ModelRendererComponent>();
	if (!renderer)
		return;

	const int durationFrames = std::max(BreakWaitFrames, 1);
	const float progress = std::min(static_cast<float>(m_Timer) / static_cast<float>(durationFrames), 1.0f);
	const float shakeOffset = MaxShakeOffset * progress;
	const float timer = static_cast<float>(m_Timer);

	//モデルの描画位置のみずらしOBBは動かさない
	renderer->SetDrawPositionOffset({std::sin(timer * ShakeSpeedX) * shakeOffset,
		0.0f, std::sin(timer * ShakeSpeedZ) * shakeOffset});
}

void BreakPlatform::ResetShake()
{
	auto renderer = GetComponent<ModelRendererComponent>();
	if (renderer)
		renderer->SetDrawPositionOffset({ 0.0f, 0.0f, 0.0f });
}

//プレイヤー着地時に崩壊待機を開始
void BreakPlatform::OnPlayerLanded(Player* player)
{
	if (m_State != BreakPlatformState::Idle)
		return;

	ResetShake();
	m_State = BreakPlatformState::Breaking;
	m_Timer = 0;
	AudioManager::PlaySEForFrames(SEId::BreakPlatform, BreakWaitFrames);
}
