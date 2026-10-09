//-----------------------------------------------------------
//  [ gameObject.h ]
//  全てのゲームオブジェクトの基底クラス
//-----------------------------------------------------------
#pragma once

#include <utility>
#include <vector>

#include "vector3.h"
#include "objectComponent.h"
#include "transformComponent.h"

class GameObject
{
protected:
	bool m_Destroy = false;

	//追加されたコンポーネントを所有する
	std::vector<ObjectComponent*> m_Components;
	TransformComponent* m_Transform = nullptr;

public:
	virtual ~GameObject() = default;

	virtual void Init()
	{
		//全てのゲームオブジェクトが必ずトランスフォームを持つ
		m_Transform = AddComponent<TransformComponent>();
	}

	virtual void Uninit()
	{
		for (ObjectComponent* component : m_Components)
		{
			component->Uninit();
			delete component;
		}
		m_Components.clear();
	}

	virtual void Update()
	{
		for (ObjectComponent* component : m_Components)
			component->Update();
	}

	//フェード中にゲーム進行を伴わず、表示に必要な状態だけを更新する
	virtual void TransitionUpdate() {}
	//全GameObjectのUpdate後に実行される
	virtual void LateUpdate() {}

	virtual void Draw()
	{
		for (ObjectComponent* component : m_Components)
			component->Draw();
	}

	//削除予約を設定しSceneの更新終了時に安全に破棄させる
	void SetDestroy() { m_Destroy = true; }
	//Sceneが実際に削除する前に削除予約の有無を確認
	bool IsDestroyRequested() const { return m_Destroy; }

	//削除予約済みなら終了処理を行って自身を破棄
	bool Destroy()
	{
		if (!m_Destroy)
			return false;

		Uninit();
		delete this;
		return true;
	}

	//コンポーネントを生成して初期化
	template <typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		T* component = new T(this, std::forward<Args>(args)...);
		m_Components.push_back(component);
		component->Init();
		return component;
	}

	//指定した型のコンポーネントを取得
	template <typename T>
	T* GetComponent()
	{
		for (ObjectComponent* component : m_Components)
		{
			if (T* casted = dynamic_cast<T*>(component))
				return casted;
		}
		return nullptr;
	}

	TransformComponent* GetTransform() { return m_Transform; }

	Vector3 GetPosition() const { return m_Transform->m_Position; }
	void SetPosition(Vector3 position) { m_Transform->m_Position = position; }

	Vector3 GetRotation() const { return m_Transform->m_Rotation; }
	void SetRotation(Vector3 rotation) { m_Transform->m_Rotation = rotation; }

	Vector3 GetScale() const { return m_Transform->m_Scale; }
	void SetScale(Vector3 scale) { m_Transform->m_Scale = scale; }

	Vector3 GetRight() const { return m_Transform->GetRight(); }
	Vector3 GetUp() const { return m_Transform->GetUp(); }
	virtual Vector3 GetForward() { return m_Transform->GetForward(); }

	float GetDistance(const Vector3& position) const
	{
		const Vector3 direction = GetPosition() - position;
		return direction.length();
	}

	float GetZ(const Vector3& position, const Vector3& forward) const
	{
		const Vector3 direction = GetPosition() - position;
		return Vector3::dot(direction, forward);
	}
};

