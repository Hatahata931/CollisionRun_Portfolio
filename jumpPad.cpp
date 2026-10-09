//-----------------------------------------------------------
//  [ jumpPad.cpp ]
//  ジャンプ台
//-----------------------------------------------------------
#include "main.h"
#include "jumpPad.h"
#include "audioManager.h"
#include "effectManager.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"
#include "player.h"

void JumpPad::Init()
{
	GameObject::Init();

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\spring.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 0.34f, 0.234f, 0.34f });
	collider->SetCenterOffset({ 0.0f, 0.234f, 0.0f });
	collider->SetUseRotation(false);
}

//プレイヤー着地時に上方向へ打ち上げる
void JumpPad::OnPlayerLanded(Player* player)
{
	if (!player)
		return;

	player->Launch(JumpPower);
	AudioManager::PlaySE(SEId::JumpPad);
	EffectManager::Play(EffectId::JumpPad, GetPosition() + Vector3{0.0f, EffectOffsetY, 0.0f});
}
