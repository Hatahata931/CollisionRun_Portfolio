//-----------------------------------------------------------
//  [ objectComponent.h ]
//  GameObjectが持つ各種コンポーネントの基底クラス
//-----------------------------------------------------------
#pragma once

class GameObject;

class ObjectComponent
{
protected:
	GameObject* m_Owner = nullptr; //このコンポーネントを持つGameObject

public:
	explicit ObjectComponent(GameObject* owner) : m_Owner(owner) {}
	virtual ~ObjectComponent() = default;

	virtual void Init() {}
	virtual void Update() {}
	virtual void Draw() {}
	virtual void Uninit() {}
};
