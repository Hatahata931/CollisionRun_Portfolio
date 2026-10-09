//-----------------------------------------------------------
//  [ star.h ]
//  収集アイテム
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Player;

class Star : public GameObject
{
private:
	static constexpr float CollisionRadius = 0.5f;
	static constexpr float RotationSpeed = 0.03f;
	static constexpr float EffectOffsetY = 0.2f; //取得エフェクトのY方向オフセット

	bool CheckHit(Player* player);
	//取得処理
	void OnCollected();

public:
	void Init() override;
	void Update() override;
};
