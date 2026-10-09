//-----------------------------------------------------------
//  [ stageProp.cpp ]
//  当たり判定を持たないステージ装飾
//-----------------------------------------------------------
#include "main.h"
#include "stageProp.h"
#include "modelRendererComponent.h"

void StageProp::Init()
{
	GameObject::Init();

	auto* renderer = AddComponent<ModelRendererComponent>();
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");
}

void StageProp::SetModel(const char* modelPath)
{
	auto* renderer = GetComponent<ModelRendererComponent>();
	if (!renderer)
		return;

	renderer->SetModel(modelPath);
}