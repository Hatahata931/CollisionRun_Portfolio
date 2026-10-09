//-----------------------------------------------------------
//  [ checkpoint.h ]
//  復帰地点を更新するチェックポイント
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Checkpoint : public GameObject
{
private:
	bool m_IsActivated = false;

	void CheckHit();

public:
	void Init() override;
	void Update() override;
	void Draw() override;
};
