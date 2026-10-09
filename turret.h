//-----------------------------------------------------------
//  [ turret.h ]
//  弾を撃つ砲台
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Turret : public GameObject
{
private:
	//効果音の距離
	static constexpr float SoundFullVolumeDistance = 6.0f; //最大音量で再生する距離
	static constexpr float SoundMaxDistance = 25.0f; //効果音が聞こえる最大距離

	int m_ShotIntervalFrames = 90;
	int m_ShotTimer = 0;
	float m_BulletSpeed = 0.2f;
	int m_BulletLifeFrames = 180;
	Vector3 m_MuzzleOffset{ 0.0f, 0.38f, 0.2f }; //砲台のローカル座標を基準にした発射位置
	float m_BulletScaleRate = 0.6f; //砲台の平均スケールに対する弾の大きさ

	void Shoot();

public:
	void Init() override;
	void Update() override;

	void SetShotIntervalFrames(int shotIntervalFrames) { m_ShotIntervalFrames = shotIntervalFrames; }
	void SetBulletSpeed(float bulletSpeed) { m_BulletSpeed = bulletSpeed; }
	void SetBulletLifeFrames(int bulletLifeFrames) { m_BulletLifeFrames = bulletLifeFrames; }
	void SetMuzzleOffset(Vector3 muzzleOffset) { m_MuzzleOffset = muzzleOffset; }
	void SetBulletScaleRate(float bulletScaleRate) { m_BulletScaleRate = bulletScaleRate; }
};
