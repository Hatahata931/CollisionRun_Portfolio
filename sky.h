//-----------------------------------------------------------
//  [ sky.h ]
//  スカイボックスを使った背景
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Sky : public GameObject
{
public:
	void Init() override;
	void LateUpdate() override;
};
