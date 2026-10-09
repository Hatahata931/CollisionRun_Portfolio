//-----------------------------------------------------------
//  [ spike.h ]
//  当たるとダメージを受けるトゲ
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Spike : public GameObject
{
private:
	void CheckHit();

public:
	void Init() override;
	void Update() override;
};
