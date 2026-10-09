//-----------------------------------------------------------
//  [ sky.cpp ]
//  スカイボックスを使った背景
//-----------------------------------------------------------
#include "main.h"
#include "sky.h"
#include "camera.h"
#include "manager.h"
#include "modelRendererComponent.h"
#include "scene.h"

void Sky::Init()
{
	GameObject::Init();

	auto* renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\sky.obj");
	renderer->SetShader("shader\\unlitTextureVS.cso", "shader\\unlitTexturePS.cso");

	SetRotation({ 0.0f, -1.7f, 0.0f });
}

void Sky::LateUpdate()
{
	//カメラに追従させる
	auto* camera = Manager::GetScene()->GetGameObject<Camera>();
	SetPosition(camera->GetPosition());
}
