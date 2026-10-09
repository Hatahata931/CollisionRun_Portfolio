//-----------------------------------------------------------
//  [ modelRendererComponent.cpp ]
//  obj形式3Dモデル描画コンポーネント
//-----------------------------------------------------------
#include "main.h"
#include "modelRendererComponent.h"
#include "gameObject.h"
#include "modelRenderer.h"
#include "renderer.h"
#include "texture.h"
#include "transformComponent.h"

void ModelRendererComponent::LoadModel(const char* modelPath)
{
	//すでにモデルがあれば削除（再読み込みにも対応）
	if (m_ModelRenderer)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}

	m_ModelRenderer = new ModelRenderer();
	m_ModelRenderer->Load(modelPath);
}

void ModelRendererComponent::SetModel(const char* modelPath)
{
	LoadModel(modelPath);
}

//描画モデルを囲むローカル座標の境界を取得
bool ModelRendererComponent::GetLocalBounds(Vector3& center, Vector3& halfSize) const
{
	return m_ModelRenderer && m_ModelRenderer->GetLocalBounds(center, halfSize);
}

void ModelRendererComponent::SetShader(const char* vertexShaderPath, const char* pixelShaderPath)
{
	//すでにシェーダーがあれば解放（上書きに対応）
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

	if (m_InputLayout)
	{
		m_InputLayout->Release();
		m_InputLayout = nullptr;
	}

	Renderer::CreateVertexShader(&m_VertexShader, &m_InputLayout, vertexShaderPath);
	Renderer::CreatePixelShader(&m_PixelShader, pixelShaderPath);
}

//モデル本来のテクスチャを指定画像で上書き
void ModelRendererComponent::SetTexture(const char* texturePath)
{
	if (!texturePath || texturePath[0] == '\0')
	{
		m_TextureOverride = nullptr;
		return;
	}

	m_TextureOverride = Texture::Load(texturePath);
}

void ModelRendererComponent::Draw()
{
	if (!m_ModelRenderer || !m_VertexShader || !m_PixelShader || !m_InputLayout)
		return;

	Renderer::GetDeviceContext()->IASetInputLayout(m_InputLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	auto* transform = m_Owner->GetTransform();
	//OwnerのTransformを変えず、描画モデルだけをワールド空間で移動
	const XMMATRIX drawTranslation = XMMatrixTranslation(
		m_DrawPositionOffset.x, m_DrawPositionOffset.y, m_DrawPositionOffset.z);
	const XMMATRIX world = transform->GetWorldMatrix() * drawTranslation;
	Renderer::SetWorldMatrix(world);
	Renderer::SetUVOffset(m_UOffset, m_VOffset);

	m_ModelRenderer->Draw(m_TextureOverride);
}

void ModelRendererComponent::Uninit()
{
	if (m_ModelRenderer)
	{
		delete m_ModelRenderer;
		m_ModelRenderer = nullptr;
	}

	m_TextureOverride = nullptr;

	if (m_InputLayout)
	{
		m_InputLayout->Release();
		m_InputLayout = nullptr;
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
