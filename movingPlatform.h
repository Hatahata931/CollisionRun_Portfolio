//-----------------------------------------------------------
//  [ movingPlatform.h ]
//  往復移動する足場
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class MovingPlatform : public GameObject
{
private:
	//往復移動の開始位置と折り返し位置
	Vector3 m_StartPosition{ 0.0f, 0.0f, 0.0f };
	Vector3 m_EndPosition{ 0.0f, 0.0f, 0.0f };

	float m_Speed = 0.05f;
	int m_WaitFrames = 90; //折り返し地点で停止するフレーム数
	int m_WaitTimer = 0;
	float m_MoveRate = 0.0f; //進み具合(0.0～1.0)
	Vector3 m_MoveDelta{ 0.0f, 0.0f, 0.0f }; //前フレームからの移動量

	bool m_IsInitialized = false; //初期化したか
	bool m_IsMovingToEnd = true; //折り返し地点(m_EndPosition)に向かっているか

public:
	void Init() override;
	void Update() override;

	void SetEndPosition(const Vector3& endPosition) { m_EndPosition = endPosition; }
	void SetSpeed(float speed) { m_Speed = speed; }
	void SetWaitFrames(int waitFrames) { m_WaitFrames = waitFrames; }
	Vector3 GetMoveDelta() const { return m_MoveDelta; }
};
