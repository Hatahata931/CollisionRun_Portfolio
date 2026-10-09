//-----------------------------------------------------------
//  [ polygonRendererComponent.h ]
//  板ポリゴン描画コンポーネント
//-----------------------------------------------------------
#pragma once

#include "objectComponent.h"

struct VERTEX_3D;

enum class PolygonRenderMode
{
	WorldSpace, //3D空間へ配置
	ScreenSpaceCentered, //中心を基準に2D配置
	ScreenSpaceRect, //左上を基準に2D配置
};

class PolygonRendererComponent : public ObjectComponent
{
private:
	static constexpr UINT VertexCount = 4;

	ID3D11Buffer* m_VertexBuffer = nullptr;

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;

	ID3D11ShaderResourceView* m_Texture = nullptr;
	XMFLOAT4 m_Color{ 1.0f, 1.0f, 1.0f, 1.0f };

	PolygonRenderMode m_RenderMode = PolygonRenderMode::WorldSpace;
	bool m_Visible = true;

	//各モードで共通する頂点情報と描画リソースを初期化
	void InitializeRenderer(VERTEX_3D* vertices, const char* filename);

public:
	using ObjectComponent::ObjectComponent;

	void Init() override {}
	//中心を原点とする3D空間用ポリゴンを初期化
	void InitWorldSpace(const char* filename);
	//中心を原点とする2D用ポリゴンを初期化
	void InitScreenSpaceCentered(const char* filename);
	//左上を原点とする2D用ポリゴンを初期化
	void InitScreenSpaceRect(float x, float y, float width, float height, const char* filename);
	void Uninit() override;
	void Draw() override;
	void SetTexture(const char* filename);
	void SetColor(float red, float green, float blue, float alpha);
	void SetVisible(bool visible) { m_Visible = visible; }
};
