//-----------------------------------------------------------
//  [ terrainBlock.cpp ]
//  ステージ地形用ブロック
//-----------------------------------------------------------
#include "main.h"
#include "terrainBlock.h"

#include <algorithm>
#include <cmath>
#include <cstring>

#include "obbColliderComponent.h"
#include "renderer.h"
#include "texture.h"

//Scaleからテクスチャの繰り返し回数を計算
float TerrainBlock::TileValue(float value)
{
	return std::max(std::fabs(value), MinimumTileValue);
}

void TerrainBlock::SetVertex(VERTEX_3D& vertex, const Vector3& position, const Vector3& normal, float u, float v)
{
	vertex.Position = XMFLOAT3(position.x, position.y, position.z);
	vertex.Normal = XMFLOAT3(normal.x, normal.y, normal.z);
	vertex.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vertex.TexCoord = XMFLOAT2(u, v);
}

//4頂点の情報を設定して1つの面を作成
void TerrainBlock::SetFace(VERTEX_3D* vertices, FaceIndex faceIndex,
	const Vector3 positions[VerticesPerFace], const Vector3& normal,
	const XMFLOAT2 texcoords[VerticesPerFace])
{
	VERTEX_3D* face = vertices + faceIndex * VerticesPerFace;

	for (int i = 0; i < VerticesPerFace; ++i)
		SetVertex(face[i], positions[i], normal, texcoords[i].x, texcoords[i].y);
}

void TerrainBlock::Init()
{
	GameObject::Init();

	auto* collider = AddComponent<OBBColliderComponent>();
	collider->SetHalfSize({ 1.0f, 1.0f, 1.0f });

	Renderer::CreateVertexShader(&m_VertexShader, &m_InputLayout, "shader\\vertexLightingVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\vertexLightingPS.cso");

	CreateBuffers();
}

void TerrainBlock::Uninit()
{
	if (m_VertexBuffer)
	{
		m_VertexBuffer->Release();
		m_VertexBuffer = nullptr;
	}

	if (m_IndexBuffer)
	{
		m_IndexBuffer->Release();
		m_IndexBuffer = nullptr;
	}

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

	GameObject::Uninit();
}

void TerrainBlock::CreateBuffers()
{
	//頂点バッファ作成
	D3D11_BUFFER_DESC vertexBufferDesc{};
	vertexBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	vertexBufferDesc.ByteWidth = sizeof(VERTEX_3D) * VertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	Renderer::GetDevice()->CreateBuffer(&vertexBufferDesc, nullptr, &m_VertexBuffer);

	//インデックス配列作成
	unsigned int indices[IndexCount]{};
	for (int faceIndex = 0; faceIndex < FaceCount; ++faceIndex)
	{
		const unsigned int vertexStart = faceIndex * VerticesPerFace;
		const int indexStart = faceIndex * IndicesPerFace;

		//三角形1
		indices[indexStart + 0] = vertexStart + 0;
		indices[indexStart + 1] = vertexStart + 1;
		indices[indexStart + 2] = vertexStart + 2;
		//三角形2
		indices[indexStart + 3] = vertexStart + 0;
		indices[indexStart + 4] = vertexStart + 3;
		indices[indexStart + 5] = vertexStart + 1;
	}

	//インデックスバッファ作成
	D3D11_BUFFER_DESC indexBufferDesc{};
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned int) * IndexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData{};
	indexData.pSysMem = indices;

	Renderer::GetDevice()->CreateBuffer(&indexBufferDesc, &indexData, &m_IndexBuffer);
}

//スケールが変化した場合だけ頂点情報を更新
void TerrainBlock::UpdateVerticesIfNeeded()
{
	const Vector3 currentScale = GetScale();

	if (!m_VertexBufferDirty && currentScale == m_LastVertexScale)
		return;

	UpdateVertices();
	m_LastVertexScale = currentScale;
	m_VertexBufferDirty = false;
}

//スケールに合わせて各面のUVを設定し、頂点バッファを更新
void TerrainBlock::UpdateVertices()
{
	constexpr float LocalSideHeight = 2.0f; //ローカル座標上の側面の高さ

	//スケールとテクスチャの繰り返し回数を取得
	const Vector3 scale = GetScale();
	const float tileX = TileValue(scale.x);
	const float tileY = TileValue(scale.y);
	const float tileZ = TileValue(scale.z);
	const float upperSideTileY = std::min(tileY, 1.0f);
	const float repeatingSideTileY = std::max(tileY - upperSideTileY, 0.0f);
	const float upperSideFraction = upperSideTileY / tileY;
	const float sideSplitY = 1.0f - LocalSideHeight * upperSideFraction;
	m_HasRepeatingSide = repeatingSideTileY > 0.0f;

	VERTEX_3D vertices[VertexCount]{};

	//各面の頂点座標とUVの作成
	//底面
	const Vector3 bottom[4] =
	{
		{ 1.0f, -1.0f,  1.0f },
		{-1.0f, -1.0f, -1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{-1.0f, -1.0f,  1.0f }
	};
	const XMFLOAT2 bottomUv[4] =
	{
		{ tileX, 0.0f },
		{ 0.0f,  tileZ },
		{ tileX, tileZ },
		{ 0.0f,  0.0f }
	};
	SetFace(vertices, FaceBottom, bottom, { 0.0f, -1.0f, 0.0f }, bottomUv);

	//上面
	const Vector3 top[4] =
	{
		{-1.0f,  1.0f, -1.0f },
		{ 1.0f,  1.0f,  1.0f },
		{ 1.0f,  1.0f, -1.0f },
		{-1.0f,  1.0f,  1.0f }
	};
	const XMFLOAT2 topUv[4] =
	{
		{ 0.0f,  tileZ },
		{ tileX, 0.0f },
		{ tileX, tileZ },
		{ 0.0f,  0.0f }
	};
	SetFace(vertices, FaceTop, top, { 0.0f, 1.0f, 0.0f }, topUv);

	//右面上部
	const Vector3 rightUpper[4] =
	{
		{ 1.0f,  1.0f, -1.0f },
		{ 1.0f, sideSplitY,  1.0f },
		{ 1.0f, sideSplitY, -1.0f },
		{ 1.0f,  1.0f,  1.0f }
	};
	const XMFLOAT2 rightUpperUv[4] =
	{
		{ 0.0f,  0.0f },
		{ tileZ, upperSideTileY },
		{ 0.0f,  upperSideTileY },
		{ tileZ, 0.0f }
	};
	SetFace(vertices, FaceRightUpper, rightUpper, { 1.0f, 0.0f, 0.0f }, rightUpperUv);

	//前面上部
	const Vector3 frontUpper[4] =
	{
		{ 1.0f,  1.0f,  1.0f },
		{-1.0f, sideSplitY,  1.0f },
		{ 1.0f, sideSplitY,  1.0f },
		{-1.0f,  1.0f,  1.0f }
	};
	const XMFLOAT2 frontUpperUv[4] =
	{
		{ tileX, 0.0f },
		{ 0.0f,  upperSideTileY },
		{ tileX, upperSideTileY },
		{ 0.0f,  0.0f }
	};
	SetFace(vertices, FaceFrontUpper, frontUpper, { 0.0f, 0.0f, 1.0f }, frontUpperUv);

	//左面上部
	const Vector3 leftUpper[4] =
	{
		{-1.0f, sideSplitY,  1.0f },
		{-1.0f,  1.0f, -1.0f },
		{-1.0f, sideSplitY, -1.0f },
		{-1.0f,  1.0f,  1.0f }
	};
	const XMFLOAT2 leftUpperUv[4] =
	{
		{ tileZ, upperSideTileY },
		{ 0.0f,  0.0f },
		{ 0.0f,  upperSideTileY },
		{ tileZ, 0.0f }
	};
	SetFace(vertices, FaceLeftUpper, leftUpper, { -1.0f, 0.0f, 0.0f }, leftUpperUv);

	//背面上部
	const Vector3 backUpper[4] =
	{
		{ 1.0f, sideSplitY, -1.0f },
		{-1.0f,  1.0f, -1.0f },
		{ 1.0f,  1.0f, -1.0f },
		{-1.0f, sideSplitY, -1.0f }
	};
	const XMFLOAT2 backUpperUv[4] =
	{
		{ tileX, upperSideTileY },
		{ 0.0f,  0.0f },
		{ tileX, 0.0f },
		{ 0.0f,  upperSideTileY }
	};
	SetFace(vertices, FaceBackUpper, backUpper, { 0.0f, 0.0f, -1.0f }, backUpperUv);

	//右面下部
	const Vector3 rightLower[4] =
	{
		{ 1.0f, sideSplitY, -1.0f },
		{ 1.0f, -1.0f,  1.0f },
		{ 1.0f, -1.0f, -1.0f },
		{ 1.0f, sideSplitY,  1.0f }
	};
	const XMFLOAT2 rightLowerUv[4] =
	{
		{ 0.0f,  0.0f },
		{ tileZ, repeatingSideTileY },
		{ 0.0f,  repeatingSideTileY },
		{ tileZ, 0.0f }
	};
	SetFace(vertices, FaceRightLower, rightLower, { 1.0f, 0.0f, 0.0f }, rightLowerUv);

	//前面下部
	const Vector3 frontLower[4] =
	{
		{ 1.0f, sideSplitY,  1.0f },
		{-1.0f, -1.0f,  1.0f },
		{ 1.0f, -1.0f,  1.0f },
		{-1.0f, sideSplitY,  1.0f }
	};
	const XMFLOAT2 frontLowerUv[4] =
	{
		{ tileX, 0.0f },
		{ 0.0f,  repeatingSideTileY },
		{ tileX, repeatingSideTileY },
		{ 0.0f,  0.0f }
	};
	SetFace(vertices, FaceFrontLower, frontLower, { 0.0f, 0.0f, 1.0f }, frontLowerUv);

	//左面下部
	const Vector3 leftLower[4] =
	{
		{-1.0f, -1.0f,  1.0f },
		{-1.0f, sideSplitY, -1.0f },
		{-1.0f, -1.0f, -1.0f },
		{-1.0f, sideSplitY,  1.0f }
	};
	const XMFLOAT2 leftLowerUv[4] =
	{
		{ tileZ, repeatingSideTileY },
		{ 0.0f,  0.0f },
		{ 0.0f,  repeatingSideTileY },
		{ tileZ, 0.0f }
	};
	SetFace(vertices, FaceLeftLower, leftLower, { -1.0f, 0.0f, 0.0f }, leftLowerUv);

	//背面下部
	const Vector3 backLower[4] =
	{
		{ 1.0f, -1.0f, -1.0f },
		{-1.0f, sideSplitY, -1.0f },
		{ 1.0f, sideSplitY, -1.0f },
		{-1.0f, -1.0f, -1.0f }
	};
	const XMFLOAT2 backLowerUv[4] =
	{
		{ tileX, repeatingSideTileY },
		{ 0.0f,  0.0f },
		{ tileX, 0.0f },
		{ 0.0f,  repeatingSideTileY }
	};
	SetFace(vertices, FaceBackLower, backLower, { 0.0f, 0.0f, -1.0f }, backLowerUv);

	//頂点バッファにコピー
	D3D11_MAPPED_SUBRESOURCE mappedResource{};
	Renderer::GetDeviceContext()->Map(m_VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	std::memcpy(mappedResource.pData, vertices, sizeof(vertices));
	Renderer::GetDeviceContext()->Unmap(m_VertexBuffer, 0);
}

void TerrainBlock::Draw()
{
	if (!m_VertexBuffer || !m_IndexBuffer || !m_VertexShader || !m_PixelShader || !m_InputLayout)
		return;

	UpdateVerticesIfNeeded();

	Renderer::GetDeviceContext()->IASetInputLayout(m_InputLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, nullptr, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, nullptr, 0);

	Renderer::SetWorldMatrix(m_Transform->GetWorldMatrix());

	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DrawFaces(FaceBottom, 1, m_Textures[TextureBottom]);
	DrawFaces(FaceTop, 1, m_Textures[TextureTop]);
	//土に草の色が混ざるのを防ぐために一時的にサンプラーステートをClampにする
	Renderer::SetTextureVClamp(true);
	DrawFaces(FaceRightUpper, SideFaceCount, m_Textures[TextureUpperSide]);
	Renderer::SetTextureVClamp(false);
	if (m_HasRepeatingSide)
		DrawFaces(FaceRightLower, SideFaceCount, m_Textures[TextureRepeatingSide]);

	GameObject::Draw();
}

void TerrainBlock::DrawFaces(FaceIndex firstFaceIndex, int faceCount, ID3D11ShaderResourceView* texture)
{
	MATERIAL material{};
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.TextureEnable = (texture != nullptr);
	Renderer::SetMaterial(material);

	if (texture)
		Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

	Renderer::GetDeviceContext()->DrawIndexed(
		IndicesPerFace * faceCount,
		firstFaceIndex * IndicesPerFace,
		0);
}

void TerrainBlock::SetTextures(const char* topTexturePath, const char* upperSideTexturePath,
	const char* repeatingSideTexturePath, const char* bottomTexturePath)
{
	m_Textures[TextureTop] = Texture::Load(topTexturePath);
	m_Textures[TextureUpperSide] = Texture::Load(upperSideTexturePath);
	m_Textures[TextureRepeatingSide] = Texture::Load(repeatingSideTexturePath);
	m_Textures[TextureBottom] = Texture::Load(bottomTexturePath);
}
