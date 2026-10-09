//-----------------------------------------------------------
//  [ windRendererComponent.cpp ]
//  風エリア内を流れる線の描画コンポーネント
//-----------------------------------------------------------
#include "main.h"
#include "windRendererComponent.h"

#include <algorithm>
#include <cmath>
#include <random>

#include "camera.h"
#include "gameObject.h"
#include "manager.h"
#include "obbColliderComponent.h"
#include "renderer.h"
#include "scene.h"

//補助関数
namespace
{
	float GetRandomValue(std::mt19937& generator, float minimum, float maximum)
	{
		std::uniform_real_distribution<float> distribution(minimum, maximum);
		return distribution(generator);
	}

	Vector3 GetPositiveHalfSize(const Vector3& halfSize)
	{
		return
		{
			std::fabs(halfSize.x),
			std::fabs(halfSize.y),
			std::fabs(halfSize.z)
		};
	}

	//風エリアの境界に近い風線ほど透明にする
	float GetBoundaryOpacity(const Vector3& localPosition, float fadeWidth)
	{
		const float distanceToBoundary = (std::min)(1.0f - std::fabs(localPosition.x),
			(std::min)(1.0f - std::fabs(localPosition.y), 1.0f - std::fabs(localPosition.z)));
		return (std::max)(0.0f, (std::min)(distanceToBoundary / fadeWidth, 1.0f));
	}

	void SetVertex(VERTEX_3D& vertex, const Vector3& position, float u, float v, float opacity)
	{
		vertex.Position = XMFLOAT3(position.x, position.y, position.z);
		vertex.Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		vertex.Diffuse = XMFLOAT4(0.65f, 0.88f, 1.0f, opacity);
		vertex.TexCoord = XMFLOAT2(u, v);
	}
}

void WindRendererComponent::Init()
{
	//風線ごとの見た目と移動速度に個体差を付ける範囲
	constexpr float MinimumLengthScale = 0.7f;
	constexpr float MaximumLengthScale = 1.3f;
	constexpr float MinimumWidth = 0.04f;
	constexpr float MaximumWidth = 0.09f;
	constexpr float MinimumSpeedScale = 0.75f;
	constexpr float MaximumSpeedScale = 1.25f;
	constexpr float MinimumOpacity = 0.3f;
	constexpr float MaximumOpacity = 0.44f;

	//起動するたびに風線の配置が変わらないよう固定シードを使う
	std::mt19937 generator(0x57494e44u);
	//OBB内のランダムな位置に個体差をつけて配置
	for (Streak& streak : m_Streaks)
	{
		streak.LocalPosition =
		{
			GetRandomValue(generator, -1.0f, 1.0f),
			GetRandomValue(generator, -1.0f, 1.0f),
			GetRandomValue(generator, -1.0f, 1.0f)
		};
		streak.LengthScale = GetRandomValue(generator, MinimumLengthScale, MaximumLengthScale);
		streak.Width = GetRandomValue(generator, MinimumWidth, MaximumWidth);
		streak.SpeedScale = GetRandomValue(generator, MinimumSpeedScale, MaximumSpeedScale);
		streak.Opacity = GetRandomValue(generator, MinimumOpacity, MaximumOpacity);
	}

	//毎フレーム作り直す全風線の頂点を保持
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT>(sizeof(VERTEX_3D) * MaximumStreakCount * VerticesPerStreak);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	const HRESULT result = Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &m_VertexBuffer);
	assert(SUCCEEDED(result));

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\windVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\windPS.cso");
}

void WindRendererComponent::Update()
{
	Vector3 direction = m_WindDirection;
	if (direction.length() <= ParallelEpsilon)
		return;
	direction.normalize();

	auto* collider = m_Owner->GetComponent<OBBColliderComponent>();
	if (!collider)
		return;

	Vector3 center;
	Vector3 axis[3];
	Vector3 halfSize;
	collider->GetWorldOBB(center, axis, halfSize);
	halfSize = GetPositiveHalfSize(halfSize);

	//風線の移動
	for (Streak& streak : m_Streaks)
	{
		const float travelDistance = VisualSpeed * streak.SpeedScale;
		if (halfSize.x > MinimumHalfExtent)
			streak.LocalPosition.x += Vector3::dot(direction, axis[0]) * travelDistance / halfSize.x;
		if (halfSize.y > MinimumHalfExtent)
			streak.LocalPosition.y += Vector3::dot(direction, axis[1]) * travelDistance / halfSize.y;
		if (halfSize.z > MinimumHalfExtent)
			streak.LocalPosition.z += Vector3::dot(direction, axis[2]) * travelDistance / halfSize.z;

		streak.LocalPosition.x = WrapNormalizedPosition(streak.LocalPosition.x);
		streak.LocalPosition.y = WrapNormalizedPosition(streak.LocalPosition.y);
		streak.LocalPosition.z = WrapNormalizedPosition(streak.LocalPosition.z);
	}
}

void WindRendererComponent::Draw()
{
	if (!m_VertexBuffer || !m_VertexShader || !m_PixelShader || !m_VertexLayout)
		return;

	Vector3 direction = m_WindDirection;
	if (direction.length() <= ParallelEpsilon)
		return;
	direction.normalize();

	auto* collider = m_Owner->GetComponent<OBBColliderComponent>();
	Scene* scene = Manager::GetScene();
	Camera* camera = scene ? scene->GetGameObject<Camera>() : nullptr;
	if (!collider || !camera)
		return;

	Vector3 center;
	Vector3 axis[3];
	Vector3 halfSize;
	collider->GetWorldOBB(center, axis, halfSize);
	halfSize = GetPositiveHalfSize(halfSize);

	const int streakCount = GetActiveStreakCount(halfSize);
	//前フレームの内容を破棄して今回表示する風線の頂点を書き込む
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	const HRESULT result = Renderer::GetDeviceContext()->Map(
		m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
		return;

	auto* vertices = static_cast<VERTEX_3D*>(mappedResource.pData);
	for (int i = 0; i < streakCount; ++i)
	{
		WriteStreakVertices(vertices, i, center, axis, halfSize,
			direction, camera->GetPosition());
	}
	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
	//頂点はワールド座標で作成済みのため、ワールド行列は単位行列にする
	Renderer::SetWorldMatrix(XMMatrixIdentity());

	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	Renderer::SetMaterial(material);

	const UINT stride = sizeof(VERTEX_3D);
	const UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(
		0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//半透明の風線を一度の描画命令でまとめて描画
	Renderer::SetCullNone(); //両面描画
	Renderer::SetDepthEnable(false);
	Renderer::SetAddBlendEnable(true); //加算合成
	Renderer::GetDeviceContext()->Draw(streakCount * VerticesPerStreak, 0);
	//後続のオブジェクトへ描画状態を残さない
	Renderer::SetAddBlendEnable(false);
	Renderer::SetDepthEnable(true);
	Renderer::SetCullBack();
}

void WindRendererComponent::Uninit()
{
	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}
	if (m_VertexLayout)
	{
		m_VertexLayout->Release();
		m_VertexLayout = nullptr;
	}
	if (m_VertexShader)
	{
		m_VertexShader->Release();
		m_VertexShader = nullptr;
	}
	if (m_PixelShader)
	{
		m_PixelShader->Release();
		m_PixelShader = nullptr;
	}
}

//風エリアを越えた座標を反対側へ循環させる
float WindRendererComponent::WrapNormalizedPosition(float value)
{
	while (value > 1.0f)
		value -= 2.0f;
	while (value < -1.0f)
		value += 2.0f;
	return value;
}

//風エリアの大きさに応じた表示本数を取得
int WindRendererComponent::GetActiveStreakCount(const Vector3& halfSize) const
{
	const float size = halfSize.x + halfSize.y + halfSize.z;
	const int count = static_cast<int>(std::round(size * StreakDensity));
	return (std::max)(MinimumStreakCount, (std::min)(count, MaximumStreakCount));
}

//OBBとカメラの向きに合わせて風線1本分の頂点を作る
void WindRendererComponent::WriteStreakVertices(VERTEX_3D* vertices, int streakIndex,
	const Vector3& center, const Vector3 axis[3], const Vector3& halfSize,
	const Vector3& direction, const Vector3& cameraPosition) const
{
	const Streak& streak = m_Streaks[streakIndex];
	//OBB内のローカル座標をワールド座標へ変換
	const Vector3 streakCenter =
		center +
		axis[0] * (streak.LocalPosition.x * halfSize.x) +
		axis[1] * (streak.LocalPosition.y * halfSize.y) +
		axis[2] * (streak.LocalPosition.z * halfSize.z);

	Vector3 toCamera = cameraPosition - streakCenter;
	if (toCamera.length() > ParallelEpsilon)
		toCamera.normalize();

	//長さは風向きへ、幅はカメラ側を向くようにする
	Vector3 widthDirection = Vector3::cross(direction, toCamera);
	//視線と風向きが平行な場合は固定軸から幅方向を作る
	if (widthDirection.length() <= ParallelEpsilon)
		widthDirection = Vector3::cross(direction, Vector3{ 0.0f, 1.0f, 0.0f });
	if (widthDirection.length() <= ParallelEpsilon)
		widthDirection = Vector3::cross(direction, Vector3{ 1.0f, 0.0f, 0.0f });
	widthDirection.normalize();

	//風向きから見たOBBの広さを基準に風線の長さを決める
	const float directionHalfExtent =
		std::fabs(Vector3::dot(direction, axis[0])) * halfSize.x +
		std::fabs(Vector3::dot(direction, axis[1])) * halfSize.y +
		std::fabs(Vector3::dot(direction, axis[2])) * halfSize.z;
	const float baseLength = (std::max)(MinimumStreakLength,
		(std::min)(directionHalfExtent * StreakLengthRatio, MaximumStreakLength));
	const float halfLength = baseLength * streak.LengthScale * 0.5f;
	const float halfWidth = streak.Width * 0.5f;
	const float opacity = streak.Opacity * GetBoundaryOpacity(streak.LocalPosition, BoundaryFadeWidth);

	//四隅の位置を求める
	const Vector3 tailCenter = streakCenter - direction * halfLength;
	const Vector3 headCenter = streakCenter + direction * halfLength;
	const Vector3 tailLeft = tailCenter - widthDirection * halfWidth;
	const Vector3 tailRight = tailCenter + widthDirection * halfWidth;
	const Vector3 headLeft = headCenter - widthDirection * halfWidth;
	const Vector3 headRight = headCenter + widthDirection * halfWidth;

	//バッファへ書き込む
	VERTEX_3D* streakVertices = vertices + streakIndex * VerticesPerStreak;
	SetVertex(streakVertices[0], tailLeft, 0.0f, 0.0f, opacity);
	SetVertex(streakVertices[1], tailRight, 0.0f, 1.0f, opacity);
	SetVertex(streakVertices[2], headLeft, 1.0f, 0.0f, opacity);
	SetVertex(streakVertices[3], headLeft, 1.0f, 0.0f, opacity);
	SetVertex(streakVertices[4], tailRight, 0.0f, 1.0f, opacity);
	SetVertex(streakVertices[5], headRight, 1.0f, 1.0f, opacity);
}