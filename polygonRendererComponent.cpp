//-----------------------------------------------------------
//  [ polygonRendererComponent.cpp ]
//  板ポリゴン描画コンポーネント
//-----------------------------------------------------------
#include "main.h"
#include "polygonRendererComponent.h"
#include "gameObject.h"
#include "renderer.h"
#include "texture.h"
#include "transformComponent.h"

//各モードで共通する頂点情報と描画リソースを初期化
void PolygonRendererComponent::InitializeRenderer(VERTEX_3D* vertices, const char* filename)
{
	for (UINT i = 0; i < VertexCount; i++)
	{
		vertices[i].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertices[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	vertices[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertices[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertices[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertices[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(VERTEX_3D) * VertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;

	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, &m_VertexBuffer);

	//2D画像はsRGB情報によって色が変化しないようsRGBを無視して読み込む
	const TextureLoadMode loadMode = m_RenderMode == PolygonRenderMode::WorldSpace
		? TextureLoadMode::Default : TextureLoadMode::IgnoreSRGB;
	if (filename)
		m_Texture = Texture::Load(filename, loadMode);

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

//中心を原点とする3D空間用ポリゴンを初期化
void PolygonRendererComponent::InitWorldSpace(const char* filename)
{
	m_RenderMode = PolygonRenderMode::WorldSpace;

	VERTEX_3D vertices[VertexCount]{};

	vertices[0].Position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertices[1].Position = XMFLOAT3(0.5f, 0.5f, 0.0f);
	vertices[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertices[3].Position = XMFLOAT3(0.5f, -0.5f, 0.0f);

	InitializeRenderer(vertices, filename);
}

//中心を原点とする2D用ポリゴンを初期化
void PolygonRendererComponent::InitScreenSpaceCentered(const char* filename)
{
	m_RenderMode = PolygonRenderMode::ScreenSpaceCentered;

	VERTEX_3D vertices[VertexCount]{};

	vertices[0].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertices[1].Position = XMFLOAT3(0.5f, -0.5f, 0.0f);
	vertices[2].Position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertices[3].Position = XMFLOAT3(0.5f, 0.5f, 0.0f);

	InitializeRenderer(vertices, filename);
}

//左上を原点とする2D用ポリゴンを初期化
void PolygonRendererComponent::InitScreenSpaceRect(
	float x, float y, float width, float height, const char* filename)
{
	m_RenderMode = PolygonRenderMode::ScreenSpaceRect;

	VERTEX_3D vertices[VertexCount]{};

	vertices[0].Position = XMFLOAT3(x, y, 0.0f);
	vertices[1].Position = XMFLOAT3(x + width, y, 0.0f);
	vertices[2].Position = XMFLOAT3(x, y + height, 0.0f);
	vertices[3].Position = XMFLOAT3(x + width, y + height, 0.0f);

	InitializeRenderer(vertices, filename);
}

void PolygonRendererComponent::Uninit()
{
	if (m_VertexBuffer)
		m_VertexBuffer->Release();
	if (m_VertexLayout)
		m_VertexLayout->Release();
	if (m_VertexShader)
		m_VertexShader->Release();
	if (m_PixelShader)
		m_PixelShader->Release();
}

void PolygonRendererComponent::Draw()
{
	if (!m_Visible)
		return;

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	if (m_RenderMode != PolygonRenderMode::WorldSpace)
		Renderer::SetWorldViewProjection2D();

	auto* transform = m_Owner->GetTransform();
	const XMMATRIX world = transform->GetWorldMatrix();
	Renderer::SetWorldMatrix(world);

	MATERIAL material{};
	material.Diffuse = m_Color;
	material.TextureEnable = m_Texture != nullptr;
	Renderer::SetMaterial(material);

	const UINT stride = sizeof(VERTEX_3D);
	const UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	Renderer::GetDeviceContext()->Draw(VertexCount, 0);
}

void PolygonRendererComponent::SetTexture(const char* filename)
{
	const TextureLoadMode loadMode = m_RenderMode == PolygonRenderMode::WorldSpace
		? TextureLoadMode::Default : TextureLoadMode::IgnoreSRGB;
	m_Texture = Texture::Load(filename, loadMode);
}

void PolygonRendererComponent::SetColor(float red, float green, float blue, float alpha)
{
	m_Color = XMFLOAT4(red, green, blue, alpha);
}
