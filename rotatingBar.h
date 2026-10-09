//-----------------------------------------------------------
//  [ rotatingBar.h ]
//  接触するとダメージを受ける回転バー
//-----------------------------------------------------------
#pragma once

#include "box.h"

class RotatingBar : public Box
{
private:
	Vector3 m_RotationAxis{ 0.0f, 1.0f, 0.0f };
	float m_RotationSpeed = 0.03f;
	Vector3 m_BaseRotation{ 0.0f, 0.0f, 0.0f };
	float m_RotationAngle = 0.0f;
	bool m_IsRotationInitialized = false;

protected:
	void UpdateRotation() override;

public:
	void Init() override;

	void SetRotationAxis(const Vector3& axis);
	void SetRotationSpeed(float speed);
	bool HasPlatformMotion() const override;
};
