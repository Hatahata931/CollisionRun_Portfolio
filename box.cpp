//-----------------------------------------------------------
//  [ box.cpp ]
//  汎用箱クラス
//-----------------------------------------------------------
#include "main.h"
#include "box.h"
#include "modelRendererComponent.h"
#include "obbColliderComponent.h"

namespace
{
	//オフセット(Boxの中心からの距離・方向)へ回転行列を適用し、変換後の位置を返す
	Vector3 TransformOffset(const Vector3& offset, const XMMATRIX& matrix)
	{
		const XMVECTOR vector = XMVectorSet(offset.x, offset.y, offset.z, 0.0f);
		XMFLOAT3 transformed;
		XMStoreFloat3(&transformed, XMVector3Transform(vector, matrix));

		return { transformed.x, transformed.y, transformed.z };
	}
}

void Box::Init()
{
	GameObject::Init();

	m_PreviousPosition = m_Transform->m_Position;
	m_PreviousRotation = m_Transform->m_Rotation;

	auto renderer = AddComponent<ModelRendererComponent>();
	renderer->LoadModel("asset\\model\\cube.obj");
	renderer->SetShader("shader\\vertexLightingVS.cso", "shader\\vertexLightingPS.cso");

	auto collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 1.0f, 1.0f, 1.0f });
}

void Box::Update()
{
	GameObject::Update();

	//回転前のTransformを保存し、乗っているプレイヤーの追従量計算に使う
	m_PreviousPosition = m_Transform->m_Position;
	m_PreviousRotation = m_Transform->m_Rotation;

	UpdateRotation();
}

void Box::UpdateRotation()
{
	m_Transform->m_Rotation += m_RotationVelocity;
}

void Box::SetModel(const char* filename)
{
	auto renderer = GetComponent<ModelRendererComponent>();
	if (!renderer)
		return;

	renderer->SetModel(filename);
}

void Box::SetTexture(const char* filename)
{
	auto renderer = GetComponent<ModelRendererComponent>();
	if (!renderer)
		return;

	renderer->SetShader("shader\\tiledBoxVS.cso", "shader\\vertexLightingPS.cso");
	renderer->SetTexture(filename);
}

void Box::SetColliderHalfSize(const Vector3& halfSize)
{
	auto collider = GetComponent<OBBColliderComponent>();
	if (!collider)
		return;

	collider->SetHalfSize(halfSize);
}

//モデル原点から見たOBB中心のローカルオフセットを設定
void Box::SetColliderCenterOffset(const Vector3& centerOffset)
{
	auto collider = GetComponent<OBBColliderComponent>();
	if (!collider)
		return;

	collider->SetCenterOffset(centerOffset);
}

//指定位置における前フレームからの足場移動量を取得
Vector3 Box::GetPlatformDeltaAtPosition(const Vector3& position) const
{
	const XMMATRIX previousRotation = XMMatrixRotationRollPitchYaw(
		m_PreviousRotation.x, m_PreviousRotation.y, m_PreviousRotation.z);
	const XMMATRIX currentRotation = XMMatrixRotationRollPitchYaw(
		m_Transform->m_Rotation.x, m_Transform->m_Rotation.y, m_Transform->m_Rotation.z);
	const XMMATRIX inversePreviousRotation = XMMatrixInverse(nullptr, previousRotation);

	//前フレームの回転を打ち消し、指定位置をBoxのローカル座標へ戻す
	const Vector3 previousOffset = position - m_PreviousPosition;
	const Vector3 localOffset = TransformOffset(previousOffset, inversePreviousRotation);

	//現在の回転を適用し、指定位置が移動する先を求める
	const Vector3 currentOffset = TransformOffset(localOffset, currentRotation);

	const Vector3 previousWorldPosition = m_PreviousPosition + previousOffset;
	const Vector3 currentWorldPosition = m_Transform->m_Position + currentOffset;

	return currentWorldPosition - previousWorldPosition;
}
