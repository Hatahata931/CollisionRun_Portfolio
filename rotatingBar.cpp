//-----------------------------------------------------------
//  [ rotatingBar.cpp ]
//  接触するとダメージを受ける回転バー
//-----------------------------------------------------------
#include "main.h"
#include "rotatingBar.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float GimbalLockThreshold = 0.0001f;

	//回転行列をTransformで使用するオイラー角へ変換
	Vector3 ToEulerAngles(const XMMATRIX& rotationMatrix)
	{
		XMFLOAT4X4 matrix;
		XMStoreFloat4x4(&matrix, rotationMatrix);

		const float sinPitch = std::max(-1.0f, std::min(-matrix._32, 1.0f));
		const float pitch = std::asin(sinPitch);
		const float cosPitch = std::cos(pitch);

		if (std::fabs(cosPitch) > GimbalLockThreshold)
		{
			return {
				pitch,
				std::atan2(matrix._31, matrix._33),
				std::atan2(matrix._12, matrix._22)
			};
		}

		return {
			pitch,
			std::atan2(-matrix._13, matrix._11),
			0.0f
		};
	}
}

void RotatingBar::Init()
{
	Box::Init();

	SetModel("asset\\model\\rotating_bar.obj");
	SetColliderHalfSize({ 1.0f, 0.25f, 0.25f });
	SetColliderCenterOffset({ 0.0f, 0.5f, 0.0f });
	SetDamageEnabled(true);
}

void RotatingBar::UpdateRotation()
{
	if (!m_IsRotationInitialized)
	{
		m_BaseRotation = m_Transform->m_Rotation;
		m_IsRotationInitialized = true;
	}

	if (m_RotationSpeed == 0.0f || m_RotationAxis == Vector3(0.0f, 0.0f, 0.0f))
		return;

	//回転角度を更新し一定範囲に収める
	m_RotationAngle = std::fmod(m_RotationAngle + m_RotationSpeed, XM_2PI);
	if (m_RotationAngle < 0.0f)
		m_RotationAngle += XM_2PI;

	const XMVECTOR rotationAxis = XMVectorSet(m_RotationAxis.x, m_RotationAxis.y, m_RotationAxis.z, 0.0f);
	const XMMATRIX localRotation = XMMatrixRotationAxis(rotationAxis, m_RotationAngle);
	const XMMATRIX baseRotation = XMMatrixRotationRollPitchYaw(m_BaseRotation.x, m_BaseRotation.y, m_BaseRotation.z);

	m_Transform->m_Rotation = ToEulerAngles(localRotation * baseRotation);
}

void RotatingBar::SetRotationAxis(const Vector3& axis)
{
	m_RotationAxis = axis;
	m_RotationAxis.normalize();
}

void RotatingBar::SetRotationSpeed(float speed)
{
	m_RotationSpeed = speed;
}

bool RotatingBar::HasPlatformMotion() const
{
	return m_RotationSpeed != 0.0f &&
		m_RotationAxis != Vector3(0.0f, 0.0f, 0.0f);
}
