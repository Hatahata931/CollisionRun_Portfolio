//-----------------------------------------------------------
//  [ transformComponent.h ]
//  オブジェクトの位置、回転、拡大率を管理するコンポーネント
//-----------------------------------------------------------
#pragma once

#include <cstddef>
#include <DirectXMath.h>

#include "objectComponent.h"
#include "vector3.h"

class TransformComponent : public ObjectComponent
{
private:
	//回転後の指定したローカル軸を取得
	Vector3 GetRotatedAxis(std::size_t axisIndex) const
	{
		const DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
			m_Rotation.x, m_Rotation.y, m_Rotation.z);

		DirectX::XMFLOAT3 axis;
		DirectX::XMStoreFloat3(&axis, rotationMatrix.r[axisIndex]);

		return Vector3(axis.x, axis.y, axis.z);
	}

public:
	Vector3 m_Position{ 0.0f, 0.0f, 0.0f };
	Vector3 m_Rotation{ 0.0f, 0.0f, 0.0f };
	Vector3 m_Scale{ 1.0f, 1.0f, 1.0f };

	//ObjectComponentのコンストラクタを引き継ぐ
	using ObjectComponent::ObjectComponent;

	//スケール、回転、位置からワールド行列を作成
	DirectX::XMMATRIX GetWorldMatrix() const
	{
		const DirectX::XMMATRIX scaleMatrix = DirectX::XMMatrixScaling(
			m_Scale.x, m_Scale.y, m_Scale.z);
		const DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
			m_Rotation.x, m_Rotation.y, m_Rotation.z);
		const DirectX::XMMATRIX translationMatrix = DirectX::XMMatrixTranslation(
			m_Position.x, m_Position.y, m_Position.z);

		return scaleMatrix * rotationMatrix * translationMatrix;
	}

	//回転を反映した各方向を取得
	Vector3 GetRight() const { return GetRotatedAxis(0); }
	Vector3 GetUp() const { return GetRotatedAxis(1); }
	Vector3 GetForward() const { return GetRotatedAxis(2); }
};
