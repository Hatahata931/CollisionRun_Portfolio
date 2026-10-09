//-----------------------------------------------------------
//  [ bullet.h ]
//  Turret‚©‚ç”­ŽË‚³‚ê‚é’e
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Bullet : public GameObject
{
private:
	Vector3 m_Velocity{};
	int m_LifeFrames = 180;

	void CheckHitPlayer();
	void CheckHitStageObjects();

public:
	void Init() override;
	void Update() override;

	void SetVelocity(Vector3 velocity) { m_Velocity = velocity; }
	void SetLifeFrames(int lifeFrames) { m_LifeFrames = lifeFrames; }
};
