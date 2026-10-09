//-----------------------------------------------------------
//  [ effectSprite.h ]
//  透明テクスチャを使った汎用ビルボードエフェクト
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

//見た目と動きの設定
struct EffectSpriteSetting
{
	const char* TexturePath = "asset\\texture\\effect_spark.png";

	//スプライトシートの分割数と再生設定
	int Columns = 1;
	int Rows = 1;
	int TotalFrames = 1;
	int FrameInterval = 1;
	int LifeFrames = 30;

	//再生中にスケールと色を補間して広がる・消える動きを作る
	Vector3 StartScale{ 1.0f, 1.0f, 1.0f };
	Vector3 EndScale{ 1.0f, 1.0f, 1.0f };

	XMFLOAT4 StartColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	XMFLOAT4 EndColor{ 1.0f, 1.0f, 1.0f, 0.0f };

	float RotationSpeed = 0.0f;
	bool Additive = false; //加算合成
	bool DisableDepthWrite = true; //描画時に深度書き込みを止めるか
	bool CullNone = true; //両面描画するか
};

class EffectSprite : public GameObject
{
private:
	static constexpr int VertexCount = 4; //板ポリゴンの頂点数

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11ShaderResourceView* m_Texture = nullptr;

	EffectSpriteSetting m_Setting;
	XMFLOAT4 m_CurrentColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	int m_Frame = 0;

	//現在のスプライトシート範囲を頂点バッファへ反映
	void UpdateVertices();
	//エフェクトの進行度を0～1で取得
	float GetLifeRate() const;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	//エフェクトの見た目と動きの設定を反映
	void Configure(const EffectSpriteSetting& setting);
};
