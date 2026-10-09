//-----------------------------------------------------------
//  [ wind.h ]
//  範囲内にいると押し流される風エリア
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class WindRendererComponent;

class Wind : public GameObject
{
private:
	Vector3 m_WindDirection{ 1.0f, 0.0f, 0.0f };
	float m_WindPower = 0.01f;
	WindRendererComponent* m_WindRenderer = nullptr;

	//プレイヤーとの距離と接触状態に応じて風音と風力を更新
	void UpdatePlayerInteraction();

public:
	void Init() override;
	void Update() override;

	void SetWindDirection(const Vector3& direction);
	void SetWindPower(float power) { m_WindPower = power; }
};
