//-----------------------------------------------------------
//  [ modelRenderer.h ]
//  OBJモデルの読み込み・共有・描画を管理
//-----------------------------------------------------------
#pragma once

#include <string>
#include <unordered_map>

#include "renderer.h"
#include "vector3.h"

//マテリアル構造体
struct MODEL_MATERIAL
{
	char Name[256];
	MATERIAL Material;
	char TextureName[256];
	ID3D11ShaderResourceView* Texture;
};

//描画サブセット構造体
struct SUBSET
{
	unsigned int StartIndex;
	unsigned int IndexNum;
	MODEL_MATERIAL Material;
};

//OBJファイルから読み込んだ一時データ
struct MODEL_OBJ
{
	VERTEX_3D* VertexArray;
	unsigned int VertexNum;

	unsigned int* IndexArray;
	unsigned int IndexNum;

	SUBSET* SubsetArray;
	unsigned int SubsetNum;
};

//GPUへ転送した描画用モデル
struct MODEL
{
	ID3D11Buffer* VertexBuffer;
	ID3D11Buffer* IndexBuffer;

	SUBSET* SubsetArray;
	unsigned int SubsetNum;

	//モデルを囲む箱(エディター用)
	Vector3 LocalBoundsCenter;
	Vector3 LocalBoundsHalfSize;
	bool HasLocalBounds;
};

class ModelRenderer
{
private:
	static std::unordered_map<std::string, MODEL*> m_ModelPool;

	//読み込んだモデルデータから描画用バッファを生成
	static void LoadModel(const char* fileName, MODEL* model);
	//OBJファイルを解析して一時モデルデータを生成
	static void LoadObj(const char* fileName, MODEL_OBJ* modelObj);
	static void LoadMaterial(const char* fileName, MODEL_MATERIAL** materialArrayOutput,
		unsigned int* materialNumOutput);

	MODEL* m_Model{};

public:
	//指定したモデルをモデルプールへ事前に読み込む
	static void Preload(const char* fileName);
	static void UnloadAll();

	void Load(const char* fileName);
	//モデルを囲むローカル座標の境界を取得
	bool GetLocalBounds(Vector3& center, Vector3& halfSize) const;
	void Draw();
	//指定したテクスチャへ差し替えてモデルを描画
	void Draw(ID3D11ShaderResourceView* overrideTexture);
};
