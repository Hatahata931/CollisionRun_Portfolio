//-----------------------------------------------------------
//  [ effectSprite.cpp ]
//  透明テクスチャを使った汎用ビルボードエフェクト
//-----------------------------------------------------------
#include "main.h"
#include <algorithm>
#include "effectSprite.h"
#include "camera.h"
#include "manager.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"

//補助関数
namespace
{
	//値を0～1の範囲に収める
	float Clamp01(float value)
	{
		if (value < 0.0f)
			return 0.0f;

		if (value > 1.0f)
			return 1.0f;

		return value;
	}

	//開始値から終了値まで割合に応じて補間
	float Lerp(float start, float end, float rate)
	{
		return start + (end - start) * rate;
	}
}

void EffectSprite::Init()
{
	GameObject::Init();

	//板ポリゴン作成
	VERTEX_3D vertex[VertexCount]{};
	vertex[0].Position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertex[1].Position = XMFLOAT3(0.5f, 0.5f, 0.0f);
	vertex[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertex[3].Position = XMFLOAT3(0.5f, -0.5f, 0.0f);

	for (int i = 0; i < VertexCount; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertex[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	vertex[0].TexCoord = XMFLOAT2(0.0f, 0.0f);
	vertex[1].TexCoord = XMFLOAT2(1.0f, 0.0f);
	vertex[2].TexCoord = XMFLOAT2(0.0f, 1.0f);
	vertex[3].TexCoord = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(VERTEX_3D) * VertexCount;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertex;
	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, &m_VertexBuffer);

	m_Texture = Texture::Load(m_Setting.TexturePath);

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void EffectSprite::Uninit()
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

	GameObject::Uninit();
}

//エフェクトの見た目と動きの設定を反映
void EffectSprite::Configure(const EffectSpriteSetting& setting)
{
	m_Setting = setting;
	m_Frame = 0;
	m_CurrentColor = m_Setting.StartColor;
	m_Transform->m_Scale = m_Setting.StartScale;

	//Init後に設定を差し替えた場合でも、指定されたテクスチャを使えるようにする
	if (m_Setting.TexturePath)
		m_Texture = Texture::Load(m_Setting.TexturePath);
}

void EffectSprite::Update()
{
	GameObject::Update();

	//寿命の進行に合わせて大きさと色を補間
	const float rate = GetLifeRate();
	m_Transform->m_Scale =
	{
		Lerp(m_Setting.StartScale.x, m_Setting.EndScale.x, rate),
		Lerp(m_Setting.StartScale.y, m_Setting.EndScale.y, rate),
		Lerp(m_Setting.StartScale.z, m_Setting.EndScale.z, rate)
	};

	m_CurrentColor =
	{
		Lerp(m_Setting.StartColor.x, m_Setting.EndColor.x, rate),
		Lerp(m_Setting.StartColor.y, m_Setting.EndColor.y, rate),
		Lerp(m_Setting.StartColor.z, m_Setting.EndColor.z, rate),
		Lerp(m_Setting.StartColor.w, m_Setting.EndColor.w, rate)
	};

	m_Transform->m_Rotation.z += m_Setting.RotationSpeed;

	m_Frame++;

	if (m_Frame >= m_Setting.LifeFrames)
		SetDestroy();
}

void EffectSprite::Draw()
{
	if (!m_Texture)
		return;

	//スプライトシートの場合は、現在のフレームに合わせてUVを更新する
	UpdateVertices();

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	Camera* camera = Manager::GetScene()->GetGameObject<Camera>();
	if (!camera)
		return;

	XMMATRIX inverseView = XMMatrixInverse(nullptr, camera->GetViewMatrix());
	inverseView.r[3].m128_f32[0] = 0.0f;
	inverseView.r[3].m128_f32[1] = 0.0f;
	inverseView.r[3].m128_f32[2] = 0.0f;

	//カメラの向きだけを使って、板ポリゴンを常にカメラ側へ向ける(ビルボード)
	const XMMATRIX scale = XMMatrixScaling(
		m_Transform->m_Scale.x,
		m_Transform->m_Scale.y,
		m_Transform->m_Scale.z);
	const XMMATRIX rotation = XMMatrixRotationZ(m_Transform->m_Rotation.z);
	const XMMATRIX translation = XMMatrixTranslation(
		m_Transform->m_Position.x,
		m_Transform->m_Position.y,
		m_Transform->m_Position.z);
	Renderer::SetWorldMatrix(scale * rotation * inverseView * translation);

	MATERIAL material{};
	material.Diffuse = m_CurrentColor;
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	const UINT stride = sizeof(VERTEX_3D);
	const UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//描画中だけ必要なステートに切り替える
	if (m_Setting.CullNone)
		Renderer::SetCullNone();
	if (m_Setting.DisableDepthWrite)
		Renderer::SetDepthEnable(false);
	if (m_Setting.Additive)
		Renderer::SetAddBlendEnable(true);

	Renderer::GetDeviceContext()->Draw(VertexCount, 0);

	if (m_Setting.Additive)
		Renderer::SetAddBlendEnable(false);
	if (m_Setting.DisableDepthWrite)
		Renderer::SetDepthEnable(true);
	if (m_Setting.CullNone)
		Renderer::SetCullBack();
}

//現在のスプライトシート範囲を頂点バッファへ反映
void EffectSprite::UpdateVertices()
{
	//前回の内容を破棄し、現在のUVを含む頂点データを書き直す
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	VERTEX_3D* vertex = static_cast<VERTEX_3D*>(mappedResource.pData);

	const int columns = std::max(1, m_Setting.Columns);
	const int rows = std::max(1, m_Setting.Rows);
	const int sheetFrames = columns * rows;
	const int totalFrames = std::min(std::max(1, m_Setting.TotalFrames), sheetFrames);
	const int frameInterval = std::max(1, m_Setting.FrameInterval);
	const int frame = std::min(totalFrames - 1, m_Frame / frameInterval);

	//現在のアニメーション番号から、使うテクスチャ範囲を計算する
	const float textureWidth = 1.0f / static_cast<float>(columns);
	const float textureHeight = 1.0f / static_cast<float>(rows);
	const float textureX = static_cast<float>(frame % columns) * textureWidth;
	const float textureY = static_cast<float>(frame / columns) * textureHeight;

	vertex[0].Position = XMFLOAT3(-0.5f, 0.5f, 0.0f);
	vertex[1].Position = XMFLOAT3(0.5f, 0.5f, 0.0f);
	vertex[2].Position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
	vertex[3].Position = XMFLOAT3(0.5f, -0.5f, 0.0f);

	for (int i = 0; i < VertexCount; i++)
	{
		vertex[i].Normal = XMFLOAT3(0.0f, 0.0f, 1.0f);
		vertex[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	vertex[0].TexCoord = XMFLOAT2(textureX, textureY);
	vertex[1].TexCoord = XMFLOAT2(textureX + textureWidth, textureY);
	vertex[2].TexCoord = XMFLOAT2(textureX, textureY + textureHeight);
	vertex[3].TexCoord = XMFLOAT2(textureX + textureWidth, textureY + textureHeight);

	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

//エフェクトの進行度を0～1で取得
float EffectSprite::GetLifeRate() const
{
	const int lifeFrames = std::max(1, m_Setting.LifeFrames - 1);
	return Clamp01(static_cast<float>(m_Frame) / static_cast<float>(lifeFrames));
}
