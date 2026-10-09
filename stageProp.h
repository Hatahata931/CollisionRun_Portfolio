//-----------------------------------------------------------
//  [ stageProp.h ]
//  当たり判定を持たないステージ装飾
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class StageProp : public GameObject
{
public:
	void Init() override;
	void SetModel(const char* modelPath);
};