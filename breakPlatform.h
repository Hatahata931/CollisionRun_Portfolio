//-----------------------------------------------------------
//  [ breakPlatform.h ]
//  乗ると崩れる足場
//-----------------------------------------------------------
#pragma once
#include "gameObject.h"

enum class BreakPlatformState
{
	Idle,
	Breaking,
	Hidden
};

class BreakPlatform : public GameObject
{
private:
	//消滅予告の揺れの移動量・速度
	static constexpr float MaxShakeOffset = 0.06f; //描画位置の最大移動量
	static constexpr float ShakeSpeedX = 0.9f;
	static constexpr float ShakeSpeedZ = 1.3f;

	static constexpr int BreakWaitFrames = 80; //乗ってから崩れるまでの時間
	static constexpr int HiddenFrames = 120; //崩れてから復活するまでの時間

	BreakPlatformState m_State = BreakPlatformState::Idle;
	int m_Timer = 0;

	void UpdateShake();
	void ResetShake();

public:
	void Init() override;
	void Update() override;
	void Draw() override;

	//プレイヤー着地時に崩壊待機を開始
	void OnPlayerLanded(class Player* player);

	bool IsActive() const { return m_State != BreakPlatformState::Hidden; }
};
