//-----------------------------------------------------------
//  [ shadow.h ]
//  プレイヤーの下に表示する影
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class PolygonRendererComponent;

class Shadow : public GameObject
{
private:
	//投影できる面が見つかったときの情報
	struct SurfaceHit
	{
		Vector3 Position{ 0.0f, 0.0f, 0.0f };
		Vector3 Normal{ 0.0f, 1.0f, 0.0f };
	};

	//影を投影する判定の調整値
	static constexpr float DirectionEpsilon = 0.001f; //方向ベクトルの平行・ゼロ判定に使う許容誤差
	static constexpr float SurfaceBoundsMargin = 0.05f; //交点を面の内側とみなす判定余白
	static constexpr float RayOriginOffset = 0.1f; //プレイヤー底面からレイ開始位置を上げる距離

	//高さに応じた影の大きさと表示位置の調整値
	static constexpr float BaseShadowScale = 2.0f; //プレイヤーの足元にあるときの基準サイズ
	static constexpr float HeightScaleFactor = 0.3f; //投影面との高さ1.0あたりの縮小量
	static constexpr float MinShadowScale = 0.8f;
	static constexpr float MaxShadowScale = 2.0f;
	static constexpr float SurfaceOffset = 0.01f; //影を投影面から法線方向へ浮かせる距離

	GameObject* m_Target = nullptr;
	PolygonRendererComponent* m_Renderer = nullptr;

	//指定したオブジェクトから影を投影する面を調べる
	bool TryGetShadowSurface(GameObject* object, const Vector3& rayOrigin, SurfaceHit& outHit);
	//投影面の法線から影を面と平行にする回転角度を求める
	void SetRotationFromSurfaceNormal(const Vector3& surfaceNormal);

public:
	void Init() override;
	void LateUpdate() override;
	void Draw() override;
	void SetTarget(GameObject* target) { m_Target = target; }
};
