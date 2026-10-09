//-----------------------------------------------------------
//  [ scene.h ]
//  ゲームオブジェクトをレイヤーごとに管理するシーン基底クラス
//-----------------------------------------------------------
#pragma once

#include <list>
#include <vector>

#include "gameObject.h"

class Scene
{
private:
	static constexpr int LayerCount = 4;

	std::list<GameObject*> m_GameObjects[LayerCount];

	//シーン遷移中に追従位置と表示に必要な状態のみ更新
	void UpdateTransitionObjects();

	//遷移中の内部更新はManagerだけに許可
	friend class Manager;

protected:
	//GameObjectを削除する直前に派生シーンへ通知
	virtual void OnGameObjectDestroying(GameObject* object) {}
	//フェードインが完了してシーン全体が表示されたときに派生シーンへ通知
	virtual void OnFadeInFinished() {}

public:
	virtual ~Scene() = default;

	enum Layer
	{
		NONE = -1,
		CAMERA,
		OBJECT3D,
		EFFECT,
		OBJECT2D,
	};

	virtual void Init();
	virtual void Uninit();
	virtual void Update();
	virtual void Draw();
	virtual bool CanPause() const { return false; } //現在のシーンでポーズ機能を使ってよいか

	//指定したレイヤーへGameObjectを生成して追加
	template <typename T>
	T* AddGameObject(int layer)
	{
		auto* gameObject = new T();
		gameObject->Init();
		m_GameObjects[layer].push_back(gameObject);

		return gameObject;
	}

	//指定した型に一致する最初のGameObjectを取得
	template <typename T>
	T* GetGameObject()
	{
		for (int i = 0; i < LayerCount; i++)
		{
			for (auto* gameObject : m_GameObjects[i])
			{
				auto* foundObject = dynamic_cast<T*>(gameObject);
				if (foundObject != nullptr)
					return foundObject;
			}
		}

		return nullptr;
	}

	//指定した型に一致するすべてのGameObjectを取得
	template <typename T>
	std::vector<T*> GetGameObjects()
	{
		std::vector<T*> foundObjects;

		for (int i = 0; i < LayerCount; i++)
		{
			for (auto* gameObject : m_GameObjects[i])
			{
				auto* foundObject = dynamic_cast<T*>(gameObject);
				if (foundObject != nullptr)
					foundObjects.push_back(foundObject);
			}
		}

		return foundObjects;
	}
};