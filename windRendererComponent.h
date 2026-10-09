//-----------------------------------------------------------
//  [ windRendererComponent.h ]
//  風エリア内を流れる線の描画コンポーネント
//-----------------------------------------------------------
#pragma once

#include <array>
#include <d3d11.h>

#include "objectComponent.h"
#include "vector3.h"

struct VERTEX_3D;

class WindRendererComponent : public ObjectComponent
{
private:
	//風線の本数
	static constexpr int MinimumStreakCount = 12;
	static constexpr int MaximumStreakCount = 64;

	//風線の長さ
	static constexpr float StreakLengthRatio = 0.16f; //風エリアの広さに対する風線の長さ
	static constexpr float MinimumStreakLength = 0.45f;
	static constexpr float MaximumStreakLength = 2.2f;

	static constexpr int VerticesPerStreak = 6; //風線1本の頂点数
	static constexpr float StreakDensity = 1.75f; //風エリアの大きさに対する風線の密度
	static constexpr float VisualSpeed = 0.08f; //風力に依存しない見た目上の移動速度
	static constexpr float BoundaryFadeWidth = 0.18f; //風エリアの端で透明にする範囲
	
	//計算を安定させるための小さな値
	static constexpr float MinimumHalfExtent = 0.0001f; //ゼロ除算を防ぐOBB半サイズの下限
	static constexpr float ParallelEpsilon = 0.000001f; //方向ベクトルを判定する許容誤差

	//風線1本の見た目と移動に使う情報
	struct Streak
	{
		Vector3 LocalPosition; //OBB内を-1～1で表した位置
		float LengthScale = 1.0f;
		float Width = 0.05f;
		float SpeedScale = 1.0f;
		float Opacity = 0.2f;
	};

	std::array<Streak, MaximumStreakCount> m_Streaks{};
	Vector3 m_WindDirection{ 1.0f, 0.0f, 0.0f };

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	//風エリアを越えた座標を反対側へ循環させる
	static float WrapNormalizedPosition(float value);
	//風エリアの大きさに応じた表示本数を取得
	int GetActiveStreakCount(const Vector3& halfSize) const;
	//OBBとカメラの向きに合わせて風線1本分の頂点を作る
	void WriteStreakVertices(VERTEX_3D* vertices, int streakIndex,
		const Vector3& center, const Vector3 axis[3], const Vector3& halfSize,
		const Vector3& direction, const Vector3& cameraPosition) const;

public:
	using ObjectComponent::ObjectComponent;

	void Init() override;
	void Update() override;
	void Draw() override;
	void Uninit() override;

	void SetWindDirection(const Vector3& direction) { m_WindDirection = direction; }
};
