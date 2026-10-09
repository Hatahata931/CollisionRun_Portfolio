//-----------------------------------------------------------
//  [ gate.h ]
//  集めたStar数で開くゲート
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Gate : public GameObject
{
private:
	static constexpr float OpenDistanceMargin = 1.0f; //開放距離に加える余白

	int m_RequiredItemCount = 0; //必要Star数

public:
	void Init() override;
	void Update() override;

	//Gateの大きさを考慮した開放距離を取得
	float GetOpenDistance() const;

	void SetRequiredItemCount(int requiredItemCount) { m_RequiredItemCount = requiredItemCount; }
	int GetRequiredItemCount() const { return m_RequiredItemCount; }
};
