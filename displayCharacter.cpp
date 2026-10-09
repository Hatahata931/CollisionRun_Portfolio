//-----------------------------------------------------------
//  [ displayCharacter.cpp ]
//  各画面の演出として表示するユニティちゃん
//-----------------------------------------------------------
#include "main.h"
#include "displayCharacter.h"
#include "animationModel.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include "shadow.h"

void DisplayCharacter::Init()
{
	GameObject::Init();

	m_AnimationModel = new AnimationModel();
	m_AnimationModel->Load("asset\\model\\unitychan.fbx");
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_Idle.fbx", IdleAnimationName);
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_Salute.fbx", SaluteAnimationName);
	m_AnimationModel->LoadAnimation("asset\\model\\unitychan_KneelDown.fbx", KneelDownAnimationName);
	m_AnimationModel->SetLightOverride(CharacterLightAmbient, CharacterLightDiffuse);

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\vertexLightingVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\vertexLightingPS.cso");

	//影はSceneが所有し、追従対象の削除予約に合わせて影側で破棄する
	auto shadow = Manager::GetScene()->AddGameObject<Shadow>(Scene::OBJECT3D);
	shadow->SetTarget(this);
}

void DisplayCharacter::Uninit()
{
	if (m_AnimationModel)
	{
		m_AnimationModel->Uninit();
		delete m_AnimationModel;
		m_AnimationModel = nullptr;
	}

	if (m_VertexLayout)
	{
		m_VertexLayout->Release();
		m_VertexLayout = nullptr;
	}
	if (m_VertexShader)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}
	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}

	GameObject::Uninit();
}

void DisplayCharacter::Update()
{
	GameObject::Update();
	UpdateAnimation();
}

//フェード中もアニメーション更新
void DisplayCharacter::TransitionUpdate()
{
	UpdateAnimation();
}

void DisplayCharacter::UpdateAnimation()
{
	if (!m_AnimationModel)
		return;

	//Idleはループさせ、それ以外は最終フレームで止める
	const AnimationPlaybackMode playbackMode = IsLoopingAnimation(m_AnimationPreset)
		? AnimationPlaybackMode::Loop : AnimationPlaybackMode::Clamp;
	const char* animationName = GetAnimationName(m_AnimationPreset);

	m_AnimationModel->Update(animationName, m_AnimationFrame, animationName,
		m_AnimationFrame, 1.0f, playbackMode, playbackMode);

	AdvanceAnimationFrame(); //アニメーションの再生フレームを進める
}

void DisplayCharacter::Draw()
{
	if (!m_AnimationModel || !m_VertexLayout || !m_VertexShader || !m_PixelShader)
		return;

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	const Vector3 scale = GetScale();
	const Vector3 rotation = GetRotation();
	const Vector3 position = GetPosition();
	const XMMATRIX world =
		XMMatrixScaling(scale.x * ModelScale, scale.y * ModelScale, scale.z * ModelScale) *
		XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) *
		XMMatrixTranslation(position.x, position.y, position.z);

	Renderer::SetWorldMatrix(world);
	m_AnimationModel->Draw();
	GameObject::Draw();
}

void DisplayCharacter::SetAnimationPreset(AnimationPreset preset)
{
	m_AnimationPreset = preset;
	m_AnimationFrame = 0.0f;
}

const char* DisplayCharacter::GetAnimationName(AnimationPreset preset)
{
	switch (preset)
	{
	case AnimationPreset::Salute:
		return SaluteAnimationName;
	case AnimationPreset::KneelDown:
		return KneelDownAnimationName;
	case AnimationPreset::Idle:
	default:
		return IdleAnimationName;
	}
}

float DisplayCharacter::GetAnimationLastFrame(AnimationPreset preset)
{
	switch (preset)
	{
	case AnimationPreset::Salute:
		return SaluteLastFrame;
	case AnimationPreset::KneelDown:
		return KneelDownLastFrame;
	case AnimationPreset::Idle: //Idleはループアニメーションなのでなし
	default:
		return 0.0f;
	}
}

bool DisplayCharacter::IsLoopingAnimation(AnimationPreset preset)
{
	return preset == AnimationPreset::Idle;
}

//アニメーションの再生フレームを進める
void DisplayCharacter::AdvanceAnimationFrame()
{
	m_AnimationFrame += AnimationFrameAdvance;

	if (IsLoopingAnimation(m_AnimationPreset))
		return;

	//非ループアニメーションは最後のフレームで止める
	const float lastFrame = GetAnimationLastFrame(m_AnimationPreset);
	if (m_AnimationFrame > lastFrame)
		m_AnimationFrame = lastFrame;
}
