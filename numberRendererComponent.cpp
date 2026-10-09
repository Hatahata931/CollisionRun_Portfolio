//-----------------------------------------------------------
//  [ numberRendererComponent.cpp ]
//  等間隔の数字スプライトを使った2D数値描画
//-----------------------------------------------------------
#include "main.h"
#include "numberRendererComponent.h"

#include <vector>

#include "gameObject.h"
#include "renderer.h"
#include "texture.h"
#include "transformComponent.h"

void NumberRendererComponent::Init(const char* texturePath, int columns, int rows)
{
	VERTEX_3D vertices[VertexCount]{};

	//文字ごとに頂点位置とUVを書き換えるため動的頂点バッファを使用する
	D3D11_BUFFER_DESC bufferDesc{};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = sizeof(vertices);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA subresourceData{};
	subresourceData.pSysMem = vertices;
	Renderer::GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, &m_VertexBuffer);

	SetTexture(texturePath);
	SetSpriteSheet(columns, rows);

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void NumberRendererComponent::Uninit()
{
	if (m_VertexBuffer)
		m_VertexBuffer->Release();
	if (m_VertexLayout)
		m_VertexLayout->Release();
	if (m_VertexShader)
		m_VertexShader->Release();
	if (m_PixelShader)
		m_PixelShader->Release();

	m_VertexBuffer = nullptr;
	m_VertexLayout = nullptr;
	m_VertexShader = nullptr;
	m_PixelShader = nullptr;
	m_Texture = nullptr;
}

void NumberRendererComponent::Draw()
{
	if (!m_Visible || !m_VertexBuffer || !m_VertexShader ||
		!m_PixelShader || !m_VertexLayout || !m_Texture)
		return;

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);
	Renderer::SetWorldViewProjection2D();

	auto* transform = m_Owner->GetTransform();
	Renderer::SetWorldMatrix(transform->GetWorldMatrix());

	MATERIAL material{};
	material.Diffuse = m_Color;
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &m_Texture);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//全体の文字数を先に求め、左・中央・右揃えの開始位置を決める
	int glyphCount = GetDigitCount(m_PrimaryValue);
	if (m_ShowSecondaryValue)
	{
		//区切り記号1文字分も含めて全体の横幅を計算
		glyphCount += 1 + GetDigitCount(m_SecondaryValue);
	}

	float x = GetStartX(glyphCount);
	x = DrawNumber(m_PrimaryValue, x, m_OffsetY);

	if (m_ShowSecondaryValue)
	{
		//現在値 / 最大値のように、2つの数値の間へ区切り記号を描く
		DrawSprite(m_SeparatorSpriteIndex, x, m_OffsetY);
		DrawNumber(m_SecondaryValue, x + m_Spacing, m_OffsetY);
	}
}

void NumberRendererComponent::SetTexture(const char* texturePath)
{
	if (!texturePath || texturePath[0] == '\0')
		return;

	m_Texture = Texture::Load(texturePath, TextureLoadMode::IgnoreSRGB);
}

void NumberRendererComponent::SetSpriteSheet(int columns, int rows)
{
	m_Columns = columns > 0 ? columns : 1;
	m_Rows = rows > 0 ? rows : 1;
}

//指定した数字に対応するスプライト番号を設定
void NumberRendererComponent::SetDigitSpriteIndex(int digit, int spriteIndex)
{
	if (digit < 0 || digit >= static_cast<int>(m_DigitSpriteIndices.size()))
		return;

	m_DigitSpriteIndices[digit] = spriteIndex;
}

//0～9に対応するスプライト番号をまとめて設定
void NumberRendererComponent::SetDigitSpriteIndices(const std::array<int, 10>& spriteIndices)
{
	m_DigitSpriteIndices = spriteIndices;
}

void NumberRendererComponent::SetSpriteSize(float width, float height)
{
	if (width > 0.0f)
		m_SpriteWidth = width;
	if (height > 0.0f)
		m_SpriteHeight = height;
}

void NumberRendererComponent::SetSpacing(float spacing)
{
	if (spacing > 0.0f)
		m_Spacing = spacing;
}

void NumberRendererComponent::SetOffset(float xOffset, float yOffset)
{
	m_OffsetX = xOffset;
	m_OffsetY = yOffset;
}

void NumberRendererComponent::SetAlignment(NumberTextAlign alignment)
{
	m_Alignment = alignment;
}

void NumberRendererComponent::SetMinimumDigits(int digits)
{
	//0以下を指定しても最低1桁は表示できるようにする
	m_MinimumDigits = digits > 0 ? digits : 1;
}

//数値を1つだけ表示
void NumberRendererComponent::SetValue(int value)
{
	m_PrimaryValue = value >= 0 ? value : 0;
	m_ShowSecondaryValue = false;
}

//2つの数値を指定した記号で区切って表示
void NumberRendererComponent::SetPairValues(int primaryValue,
	int secondaryValue, int separatorSpriteIndex)
{
	m_PrimaryValue = primaryValue >= 0 ? primaryValue : 0;
	m_SecondaryValue = secondaryValue >= 0 ? secondaryValue : 0;
	m_SeparatorSpriteIndex = separatorSpriteIndex;
	m_ShowSecondaryValue = true;
}

void NumberRendererComponent::SetColor(const DirectX::XMFLOAT4& color)
{
	m_Color = color;
}

void NumberRendererComponent::SetVisible(bool visible)
{
	m_Visible = visible;
}

//0埋めを含む表示桁数を取得
int NumberRendererComponent::GetDigitCount(int value) const
{
	int count = 1;
	if (value > 0)
	{
		count = 0;
		while (value > 0)
		{
			value /= 10;
			++count;
		}
	}

	return count < m_MinimumDigits ? m_MinimumDigits : count;
}

//最初の文字を描画するX座標を計算
float NumberRendererComponent::GetStartX(int glyphCount) const
{
	const float width = m_SpriteWidth + m_Spacing * (glyphCount - 1);

	switch (m_Alignment)
	{
	case NumberTextAlign::Center:
		return m_OffsetX - width * 0.5f;
	case NumberTextAlign::Right:
		return m_OffsetX - width;
	case NumberTextAlign::Left:
	default:
		return m_OffsetX;
	}
}

//数値を描画し、次の描画開始X座標を返す
float NumberRendererComponent::DrawNumber(int value, float x, float y)
{
	if (value < 0)
		value = 0;

	const int digitCount = GetDigitCount(value);
	std::vector<int> digits(digitCount, 0);

	//下の桁から配列の右側へ入れることで、足りない上位桁を0のまま残す
	for (int i = digitCount - 1; i >= 0 && value > 0; --i)
	{
		digits[i] = value % 10;
		value /= 10;
	}

	for (int digit : digits)
	{
		DrawSprite(m_DigitSpriteIndices[digit], x, y);
		x += m_Spacing;
	}

	return x;
}

//指定されたスプライト番号の1文字を描画
void NumberRendererComponent::DrawSprite(int spriteIndex, float x, float y)
{
	const int spriteCount = m_Columns * m_Rows;
	if (spriteIndex < 0 || spriteIndex >= spriteCount)
		return;

	D3D11_MAPPED_SUBRESOURCE mappedSubresource{};
	if (FAILED(Renderer::GetDeviceContext()->Map(
		m_VertexBuffer,
		0,
		D3D11_MAP_WRITE_DISCARD,
		0,
		&mappedSubresource)))
	{
		return;
	}

	VERTEX_3D* vertices = static_cast<VERTEX_3D*>(mappedSubresource.pData);

	//セル番号を行と列へ分解し、正規化されたUV座標へ変換する
	const float textureWidth = 1.0f / m_Columns;
	const float textureHeight = 1.0f / m_Rows;
	const float textureX = (spriteIndex % m_Columns) * textureWidth;
	const float textureY = (spriteIndex / m_Columns) * textureHeight;

	vertices[0].Position = XMFLOAT3(x, y, 0.0f);
	vertices[0].TexCoord = XMFLOAT2(textureX, textureY);

	vertices[1].Position = XMFLOAT3(x + m_SpriteWidth, y, 0.0f);
	vertices[1].TexCoord = XMFLOAT2(textureX + textureWidth, textureY);

	vertices[2].Position = XMFLOAT3(x, y + m_SpriteHeight, 0.0f);
	vertices[2].TexCoord = XMFLOAT2(textureX, textureY + textureHeight);

	vertices[3].Position = XMFLOAT3(x + m_SpriteWidth, y + m_SpriteHeight, 0.0f);
	vertices[3].TexCoord = XMFLOAT2(
		textureX + textureWidth,
		textureY + textureHeight);

	for (int i = 0; i < VertexCount; ++i)
	{
		vertices[i].Normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		vertices[i].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
	Renderer::GetDeviceContext()->Draw(VertexCount, 0);
}
