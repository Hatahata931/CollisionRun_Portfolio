//-----------------------------------------------------------
//  [ modelRendererComponent.h ]
//  obj形式3Dモデル描画コンポーネント
//-----------------------------------------------------------
#pragma once

#include <d3d11.h>

#include "objectComponent.h"
#include "vector3.h"

class ModelRenderer;

class ModelRendererComponent : public ObjectComponent
{
private:
	ModelRenderer* m_ModelRenderer = nullptr;
	ID3D11ShaderResourceView* m_TextureOverride = nullptr;
	float m_UOffset = 0.0f;
	float m_VOffset = 0.0f;
	Vector3 m_DrawPositionOffset{}; //当たり判定に影響しない描画専用の移動量

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;

public:
	//ObjectComponentのコンストラクタを引き継ぐ
	using ObjectComponent::ObjectComponent;
	~ModelRendererComponent() override = default;

	void Uninit() override;
	void Draw() override;

	void LoadModel(const char* modelPath);
	void SetModel(const char* modelPath);
	//描画モデルを囲むローカル座標の境界を取得
	bool GetLocalBounds(Vector3& center, Vector3& halfSize) const;
	void SetShader(const char* vertexShaderPath, const char* pixelShaderPath);
	//モデル本来のテクスチャを指定画像で上書き
	void SetTexture(const char* texturePath);
	void SetUVOffset(float uOffset, float vOffset)
	{
		m_UOffset = uOffset;
		m_VOffset = vOffset;
	}
	void SetDrawPositionOffset(const Vector3& offset) { m_DrawPositionOffset = offset; }
};
