//-----------------------------------------------------------
//  [ heart.h ]
//  ‰ñ•œƒAƒCƒeƒ€
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Player;

class Heart : public GameObject
{
private:
	static constexpr float CollisionRadius = 0.5f;
	static constexpr float RotationSpeed = 0.03f;
	static constexpr int HealAmount = 1; //æ“¾‚Ì‰ñ•œ—Ê

	bool CheckHit(Player* player);
	//æ“¾ˆ—
	void OnCollected();

public:
	void Init() override;
	void Update() override;
};
