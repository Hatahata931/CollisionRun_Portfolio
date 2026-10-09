//-----------------------------------------------------------
//  [ terrainBlock.h ]
//  ステージ地形用ブロック
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"
#include "renderer.h"

class TerrainBlock : public GameObject
{
private:
	//面ごとのテクスチャ
	enum TextureSlot
	{
		TextureTop,
		TextureUpperSide, //側面上部
		TextureRepeatingSide, //側面下部
		TextureBottom,
		TextureCount
	};

	//頂点配列と描画処理で共有する面番号
	enum FaceIndex
	{
		FaceBottom,
		FaceTop,
		FaceRightUpper,
		FaceFrontUpper,
		FaceLeftUpper,
		FaceBackUpper,
		FaceRightLower,
		FaceFrontLower,
		FaceLeftLower,
		FaceBackLower,
		FaceCount
	};

	//頂点数・インデックス数
	static constexpr int VerticesPerFace = 4; 
	static constexpr int IndicesPerFace = 6;
	static constexpr int SideFaceCount = 4;
	static constexpr int VertexCount = FaceCount * VerticesPerFace;
	static constexpr int IndexCount = FaceCount * IndicesPerFace;

	static constexpr float MinimumTileValue = 0.0001f; //UVの繰り返し回数の最小値

	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;

	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;

	ID3D11ShaderResourceView* m_Textures[TextureCount]{};
	Vector3 m_LastVertexScale{ 0.0f, 0.0f, 0.0f }; //前回頂点を更新したときのスケール
	bool m_VertexBufferDirty = true; //頂点バッファの更新が必要か
	bool m_HasRepeatingSide = false; //下部の土テクスチャを描画するか

	//スケールからテクスチャの繰り返し回数を計算
	static float TileValue(float value);
	static void SetVertex(VERTEX_3D& vertex, const Vector3& position, const Vector3& normal, float u, float v);
	//4頂点の情報を設定して1つの面を作成
	static void SetFace(VERTEX_3D* vertices, FaceIndex faceIndex,
		const Vector3 positions[VerticesPerFace], const Vector3& normal,
		const XMFLOAT2 texcoords[VerticesPerFace]);

	void CreateBuffers();
	//スケールが変化した場合のみ頂点情報を更新
	void UpdateVerticesIfNeeded();
	//スケールに合わせて各面のUVを設定し、頂点バッファを更新
	void UpdateVertices();
	void DrawFaces(FaceIndex firstFaceIndex, int faceCount, ID3D11ShaderResourceView* texture);

public:
	void Init() override;
	void Uninit() override;
	void Draw() override;

	void SetTextures(const char* topTexturePath, const char* upperSideTexturePath,
		const char* repeatingSideTexturePath, const char* bottomTexturePath);
};
