//-----------------------------------------------------------
//  [ obbColliderComponent.cpp ]
//  SATを用いたOBB同士の衝突判定と押し戻し処理 処理理解のためコメントを多めに記載
//-----------------------------------------------------------

#include "main.h"

#include <cmath>
#include <limits>

#include "obbColliderComponent.h"
#include "gameObject.h"
#include "input.h"
#include "renderer.h"
#include "transformComponent.h"

namespace
{
	//OBB全体を包む球の半径を取得
	//詳細なOBB判定の前に、明らかに遠い相手を弾くために使う
	float GetBroadPhaseRadius(const Vector3& half)
	{
		return std::sqrt(half.x * half.x + half.y * half.y + half.z * half.z);
	}

#ifdef _DEBUG
	constexpr int DebugVertexCount = 24;

	bool g_DrawObbDebug = false;
	bool g_DebugToggleLatch = false;

	ID3D11Buffer* g_DebugVertexBuffer = nullptr;
	ID3D11VertexShader* g_DebugVertexShader = nullptr;
	ID3D11PixelShader* g_DebugPixelShader = nullptr;
	ID3D11InputLayout* g_DebugVertexLayout = nullptr;

	void EnsureDebugResources()
	{
		if (!g_DebugVertexBuffer)
		{
			//頂点バッファ作成
			D3D11_BUFFER_DESC bufferDesc{};
			bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			bufferDesc.ByteWidth = sizeof(VERTEX_3D) * DebugVertexCount;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			Renderer::GetDevice()->CreateBuffer(&bufferDesc, nullptr, &g_DebugVertexBuffer);
		}

		if (!g_DebugVertexShader || !g_DebugPixelShader || !g_DebugVertexLayout)
		{
			//シェーダー作成
			Renderer::CreateVertexShader(&g_DebugVertexShader, &g_DebugVertexLayout, "shader\\unlitTextureVS.cso");
			Renderer::CreatePixelShader(&g_DebugPixelShader, "shader\\unlitTexturePS.cso");
		}
	}

	Vector3 RotateExtent(const Vector3 axis[3], const Vector3& local)
	{
		return axis[0] * local.x + axis[1] * local.y + axis[2] * local.z;
	}
#endif
}

//判定に使用するOBBの中心・向き・半サイズをワールド座標で取得
void OBBColliderComponent::GetWorldOBB(Vector3& center, Vector3 axis[3], Vector3& half) const
{
	//向きの取得
	if (m_UseRotation)
	{
		//回転を使う場合
		axis[0] = m_Owner->GetRight();
		axis[1] = m_Owner->GetUp();
		axis[2] = m_Owner->GetForward();

		axis[0].normalize();
		axis[1].normalize();
		axis[2].normalize();
	}
	else
	{
		//回転を使わない場合は固定
		axis[0] = { 1.0f, 0.0f, 0.0f };
		axis[1] = { 0.0f, 1.0f, 0.0f };
		axis[2] = { 0.0f, 0.0f, 1.0f };
	}

	Vector3 scale = { 1.0f, 1.0f, 1.0f };
	if (m_UseScale)
		scale = m_Owner->GetTransform()->m_Scale;

	Vector3 centerOffset = m_CenterOffset;
	if (m_UseScale)
	{
		centerOffset.x *= scale.x;
		centerOffset.y *= scale.y;
		centerOffset.z *= scale.z;
	}

	//ローカルオフセットをOBBの各軸でワールド座標へ変換
	const Vector3 worldCenterOffset =
		axis[0] * centerOffset.x +
		axis[1] * centerOffset.y +
		axis[2] * centerOffset.z;
	center = m_Owner->GetPosition() + worldCenterOffset;

	//スケールを反映した半サイズを取得
	half =
	{
		m_HalfSize.x * scale.x,
		m_HalfSize.y * scale.y,
		m_HalfSize.z * scale.z
	};
}

void OBBColliderComponent::Update()
{
#ifndef _DEBUG
	return;
#else
	//F1キーでOBBのサイズのワイヤーフレームを表示(Debugのみ)
	const bool togglePressed = Input::GetKeyTrigger(VK_F1);
	if (togglePressed && !g_DebugToggleLatch)
	{
		//複数のOBBが同じフレームで更新されても、表示切り替えは1回だけにする
		g_DrawObbDebug = !g_DrawObbDebug;
		g_DebugToggleLatch = true;
	}
	else if (!Input::GetKeyPress(VK_F1))
		g_DebugToggleLatch = false;
#endif
}

void OBBColliderComponent::Draw()
{
#ifndef _DEBUG
	return;
#else
	if (!g_DrawObbDebug)
		return;

	EnsureDebugResources();
	if (!g_DebugVertexBuffer || !g_DebugVertexShader || !g_DebugPixelShader || !g_DebugVertexLayout)
		return;

	Vector3 center;
	Vector3 half;
	Vector3 axis[3];
	GetWorldOBB(center, axis, half);

	//箱の各頂点
	const Vector3 localCorners[8] =
	{
		{ -half.x, -half.y, -half.z },
		{  half.x, -half.y, -half.z },
		{  half.x,  half.y, -half.z },
		{ -half.x,  half.y, -half.z },
		{ -half.x, -half.y,  half.z },
		{  half.x, -half.y,  half.z },
		{  half.x,  half.y,  half.z },
		{ -half.x,  half.y,  half.z },
	};

	//各頂点をワールド座標に変換
	Vector3 worldCorners[8];
	for (int i = 0; i < 8; ++i)
		worldCorners[i] = center + RotateExtent(axis, localCorners[i]);

	//どの頂点同士を線で結ぶか
	const int edgeIndices[DebugVertexCount] =
	{
		0, 1, 1, 2, 2, 3, 3, 0,
		4, 5, 5, 6, 6, 7, 7, 4,
		0, 4, 1, 5, 2, 6, 3, 7
	};

	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	Renderer::GetDeviceContext()->Map(g_DebugVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto* vertex = static_cast<VERTEX_3D*>(mappedResource.pData);

	//頂点バッファに書き込む
	for (int i = 0; i < DebugVertexCount; ++i)
	{
		const Vector3& position = worldCorners[edgeIndices[i]];
		vertex[i].Position = XMFLOAT3(position.x, position.y, position.z);
		vertex[i].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertex[i].Diffuse = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f); //マゼンタ
		vertex[i].TexCoord = XMFLOAT2(0.0f, 0.0f);
	}

	Renderer::GetDeviceContext()->Unmap(g_DebugVertexBuffer, 0);

	Renderer::GetDeviceContext()->IASetInputLayout(g_DebugVertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(g_DebugVertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(g_DebugPixelShader, nullptr, 0);

	const XMMATRIX world = XMMatrixIdentity();
	Renderer::SetWorldMatrix(world);

	//マテリアル設定
	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.TextureEnable = false;
	Renderer::SetMaterial(material);

	//テクスチャをリセット
	ID3D11ShaderResourceView* nullTexture = nullptr;
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &nullTexture);

	const UINT stride = sizeof(VERTEX_3D);
	const UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &g_DebugVertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	Renderer::GetDeviceContext()->Draw(DebugVertexCount, 0);
#endif
}

//SATでOBB同士が交差しているかのみ判定
bool OBBColliderComponent::Intersects(const OBBColliderComponent* other) const
{
	if (!other)
		return false;

	//押し戻しベクトルは必要ないので、ダミーを用意して渡す
	Vector3 dummy;
	return ResolveCollision(other, dummy);
}

//SATでOBB同士の交差を判定し押し戻し量を計算
//preferHorizontalOnDownwardがtrueの場合、側面寄りの下向き補正を水平補正へ置き換える
bool OBBColliderComponent::ResolveCollision(const OBBColliderComponent* other, Vector3& outCorrection, bool preferHorizontalOnDownward) const
{
	outCorrection = { 0.0f, 0.0f, 0.0f };

	Vector3 aCenter;
	Vector3 bCenter;
	Vector3 aHalf;
	Vector3 bHalf;
	Vector3 aAxis[3];
	Vector3 bAxis[3];
	GetWorldOBB(aCenter, aAxis, aHalf);
	other->GetWorldOBB(bCenter, bAxis, bHalf);

	const Vector3 centerDelta = bCenter - aCenter; //Aから見てBがどこにいるか

	//詳細なSAT判定に入る前にOBBを包む球で大まかに距離チェックを行う
	//球同士が離れている場合、OBB同士も当たらないため早めに終了
	constexpr float BroadPhaseMargin = 0.001f; //浮動小数点演算の誤差防止のための小さな余裕
	const float broadPhaseRadius = GetBroadPhaseRadius(aHalf) + GetBroadPhaseRadius(bHalf) + BroadPhaseMargin;
	if (Vector3::dot(centerDelta, centerDelta) > broadPhaseRadius * broadPhaseRadius)
		return false;

	//内積を保存する配列
	float axisDot[3][3];
	float absAxisDot[3][3];

	//浮動小数点の計算を安定させるための保険
	constexpr float Epsilon = 0.000001f; //内積や水平軸の長さに使う
	constexpr float TieTolerance = 0.001f; //2つの重なり量がほぼ同じと判断する範囲
	constexpr float ParallelAxisEpsilon = 0.00001f; //外積軸の長さがこれより小さければ2本の軸はほぼ平行と判断する
	constexpr float SeparationSlop = 0.001f; //押し戻し量へ加える余裕

	//AとBの各軸の向きの関係をすべての組み合わせで求める
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			axisDot[i][j] = Vector3::dot(aAxis[i], bAxis[j]);
			absAxisDot[i][j] = std::fabs(axisDot[i][j]) + Epsilon;
		}
	}

	//Aから見た(Aの軸基準の)Bの位置
	const float centerDeltaOnA[3] =
	{
		Vector3::dot(centerDelta, aAxis[0]),
		Vector3::dot(centerDelta, aAxis[1]),
		Vector3::dot(centerDelta, aAxis[2])
	};

	const float aHalfValues[3] = { aHalf.x, aHalf.y, aHalf.z };
	const float bHalfValues[3] = { bHalf.x, bHalf.y, bHalf.z };

	//押し戻し候補を保存する変数
	float minOverlap = (std::numeric_limits<float>::max)(); //最小重なり量
	Vector3 bestAxis = { 0.0f, 0.0f, 0.0f }; //最小重なり量の軸(押し戻し方向)
	float bestSignedDistance = 0.0f; //軸上で見たAからBの中心間距離(正負で押し戻し方向を決める)
	bool hasFaceCandidate = false; //押し戻し候補があるか

	//横方向用
	float minHorizontalDepth = (std::numeric_limits<float>::max)();
	Vector3 bestHorizontalAxis = { 0.0f, 0.0f, 0.0f };
	float bestHorizontalSignedDistance = 0.0f;
	bool hasHorizontalCandidate = false;

	//ある1本の軸について影が重なっているかチェックして、押し戻し候補として覚えるか決める
	//引数(調べる軸のワールド方向、重なり量、軸上で見たAとBの中心間距離、この軸を押し戻し方向の候補に使うか)
	auto testAxis = [&](const Vector3& axisWorld, float overlap, float signedDistance, bool useResolutionAxis)
	{
		if (overlap < 0.0f)
			return false; //AとBの影が離れていたら当たっていないので終了

		if (!useResolutionAxis)
			return true; //押し戻し方向の候補として使わないなら終了

		//面軸を水平面へ投影し、同じ重なりを解消するために必要な水平距離を求める
		Vector3 horizontalAxis = { axisWorld.x, 0.0f, axisWorld.z };
		const float horizontalAxisLength = horizontalAxis.length();
		if (horizontalAxisLength > Epsilon)
		{
			horizontalAxis /= horizontalAxisLength;
			const float horizontalDepth = overlap / horizontalAxisLength; //必要な水平移動距離

			if (horizontalDepth + TieTolerance < minHorizontalDepth)
			{
				minHorizontalDepth = horizontalDepth;
				bestHorizontalAxis = horizontalAxis;
				bestHorizontalSignedDistance = signedDistance;
				hasHorizontalCandidate = true;
			}
		}

		//なるべく少ない移動で離したいので、最小の重なりを覚える
		if (overlap + TieTolerance < minOverlap)
		{
			minOverlap = overlap;
			bestAxis = axisWorld;
			bestSignedDistance = signedDistance;
			hasFaceCandidate = true;
		}
		//重なり量がほぼ同じ場合、より縦寄りの軸を優先する(縦の押し戻しを自然にするため)
		else if (std::fabs(overlap - minOverlap) <= TieTolerance)
		{
			if (std::fabs(axisWorld.y) > std::fabs(bestAxis.y))
			{
				minOverlap = overlap;
				bestAxis = axisWorld;
				bestSignedDistance = signedDistance;
				hasFaceCandidate = true;
			}
		}

		return true;
	};

	//Aの3本の軸それぞれについて、その向きから見た2つの箱の影が重なっているか
	for (int i = 0; i < 3; ++i)
	{
		const float aProjectionRadius = aHalfValues[i];
		//BがAのそれぞれの軸に対してどれだけ広がって見えるか
		const float bProjectionRadius =
			bHalf.x * absAxisDot[i][0] +
			bHalf.y * absAxisDot[i][1] +
			bHalf.z * absAxisDot[i][2];

		const float centerDistanceOnAxis = centerDeltaOnA[i]; //Aの各軸上に投影したAとBの中心間距離
		//Aの影の半径 + Bの影の半径 - 中心距離で重なり量を計算
		const float overlap = (aProjectionRadius + bProjectionRadius) - std::fabs(centerDistanceOnAxis);
		if (!testAxis(aAxis[i], overlap, centerDistanceOnAxis, true))
			return false;
	}

	//Bの軸
	for (int j = 0; j < 3; ++j)
	{
		const float aProjectionRadius =
			aHalf.x * absAxisDot[0][j] +
			aHalf.y * absAxisDot[1][j] +
			aHalf.z * absAxisDot[2][j];
		const float bProjectionRadius = bHalfValues[j];

		//Bの判定軸へ変換
		const float centerDistanceOnAxis =
			centerDeltaOnA[0] * axisDot[0][j] +
			centerDeltaOnA[1] * axisDot[1][j] +
			centerDeltaOnA[2] * axisDot[2][j];
		const float overlap = (aProjectionRadius + bProjectionRadius) - std::fabs(centerDistanceOnAxis);
		if (!testAxis(bAxis[j], overlap, centerDistanceOnAxis, true))
			return false;
	}

	//Aの軸とBの軸の外積
	for (int i = 0; i < 3; ++i)
	{
		//Aのi以外の2本の軸
		const int aOther1 = (i + 1) % 3;
		const int aOther2 = (i + 2) % 3;

		for (int j = 0; j < 3; ++j)
		{
			//Bのj以外の2本の軸
			const int bOther1 = (j + 1) % 3;
			const int bOther2 = (j + 2) % 3;

			Vector3 axis = Vector3::cross(aAxis[i], bAxis[j]);
			const float axisLength = axis.length();
			if (axisLength < ParallelAxisEpsilon)
				continue; //ほぼゼロベクトルなら飛ばす

			axis /= axisLength;

			//A、Bが外積軸に対してどれだけ広がって見えるか
			const float aProjectionRadius =
				aHalfValues[aOther1] * absAxisDot[aOther2][j] +
				aHalfValues[aOther2] * absAxisDot[aOther1][j];

			const float bProjectionRadius =
				bHalfValues[bOther1] * absAxisDot[i][bOther2] +
				bHalfValues[bOther2] * absAxisDot[i][bOther1];

			const float centerDistanceOnAxis =
				centerDeltaOnA[aOther2] * axisDot[aOther1][j] -
				centerDeltaOnA[aOther1] * axisDot[aOther2][j];
			const float overlap = (aProjectionRadius + bProjectionRadius) - std::fabs(centerDistanceOnAxis);
			//外積軸は押し戻し方向に使うと不安定なので、候補から外す
			if (!testAxis(axis, overlap, centerDistanceOnAxis, false))
				return false;
		}
	}

	//押し戻しに使える候補の軸がなかったら終了
	if (!hasFaceCandidate)
		return false;

	Vector3 pushDirection = bestAxis;
	pushDirection.normalize();

	//+,-どちらの方向に押し出すか決定
	if (bestSignedDistance > 0.0f)
		pushDirection = -pushDirection;

	const float pushHorizontalLength =
		std::sqrt(pushDirection.x * pushDirection.x + pushDirection.z * pushDirection.z);
	const bool isSideDominant = pushHorizontalLength > std::fabs(pushDirection.y);

	//下向きでも側面寄りの補正なら、必要な移動距離が最小の水平候補を優先する
	if (preferHorizontalOnDownward && pushDirection.y < -0.5f &&
		isSideDominant && hasHorizontalCandidate)
	{
		pushDirection = bestHorizontalAxis;
		if (bestHorizontalSignedDistance > 0.0f)
			pushDirection = -pushDirection;
		minOverlap = minHorizontalDepth;
	}

	//押し戻し量に少し余裕を足して、接触判定のチラつきを減らす
	outCorrection = pushDirection * (minOverlap + SeparationSlop);
	return true;
}
