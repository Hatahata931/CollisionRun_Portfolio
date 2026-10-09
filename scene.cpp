//-----------------------------------------------------------
//  [ scene.cpp ]
//  ゲームオブジェクトをレイヤーごとに管理するシーン基底クラス
//-----------------------------------------------------------
#include "main.h"
#include "scene.h"

#include "camera.h"

void Scene::Init()
{
}

void Scene::Uninit()
{
	for (int i = 0; i < LayerCount; i++)
	{
		for (auto* gameObject : m_GameObjects[i])
		{
			//シーン終了時の一括破棄でも派生シーンの管理ポインタを先に切り離す
			OnGameObjectDestroying(gameObject);
			gameObject->Uninit();
			delete gameObject;
		}

		m_GameObjects[i].clear();
	}
}

void Scene::Update()
{
	for (int i = 0; i < LayerCount; i++)
	{
		for (auto* gameObject : m_GameObjects[i])
		{
			gameObject->Update();
		}
	}

	//全GameObjectの通常更新後に追従位置などを確定
	for (int i = 0; i < LayerCount; i++)
	{
		for (auto* gameObject : m_GameObjects[i])
		{
			gameObject->LateUpdate();
		}
	}

	for (int i = 0; i < LayerCount; i++)
	{
		m_GameObjects[i].remove_if([this](GameObject* object)
		{
			if (!object->IsDestroyRequested())
				return false;

			//削除前に派生シーンへ通知し、管理中のポインタを安全に切り離す
			OnGameObjectDestroying(object);
			object->Destroy();
			return true;
		});
	}
}

//シーン遷移中に追従位置と表示に必要な状態のみ更新
void Scene::UpdateTransitionObjects()
{
	//CameraやShadowなど、全オブジェクト更新後に確定する表示位置を更新
	for (int i = 0; i < LayerCount; i++)
	{
		for (auto* gameObject : m_GameObjects[i])
		{
			gameObject->LateUpdate();
		}
	}

	//入力・当たり判定・タイマーを進めず、必要な見た目だけを更新
	for (int i = 0; i < LayerCount; i++)
	{
		for (auto* gameObject : m_GameObjects[i])
		{
			gameObject->TransitionUpdate();
		}
	}
}

void Scene::Draw()
{
	//半透明エフェクトをカメラから遠い順に描画(Zソート)
	auto* camera = GetGameObject<Camera>();
	if (camera != nullptr)
	{
		const Vector3 cameraPosition = camera->GetPosition();
		const Vector3 cameraForward = camera->GetForward();

		m_GameObjects[EFFECT].sort([&](GameObject* object1, GameObject* object2)
		{
			return object1->GetZ(cameraPosition, cameraForward) >
				object2->GetZ(cameraPosition, cameraForward);
		});
	}

	for (int i = 0; i < LayerCount; i++)
	{
		for (auto* gameObject : m_GameObjects[i])
		{
			gameObject->Draw();
		}
	}
}
