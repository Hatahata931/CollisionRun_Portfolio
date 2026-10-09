//-----------------------------------------------------------
//  [ jumpPad.h ]
//  ジャンプ台
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Player;

class JumpPad : public GameObject
{
private:
	static constexpr float JumpPower = 0.38f;
	static constexpr float EffectOffsetY = 0.5f; //エフェクト発生位置のY方向ずらし量

public:
	void Init() override;

	//プレイヤー着地時に上方向へ打ち上げる
	void OnPlayerLanded(Player* player);
};
