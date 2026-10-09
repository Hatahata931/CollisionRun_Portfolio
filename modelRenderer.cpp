//-----------------------------------------------------------
//  [ modelRenderer.cpp ]
//  OBJモデルの読み込み・共有・描画を管理
//-----------------------------------------------------------
#include "main.h"
#include "modelRenderer.h"

#include <algorithm>
#include <stdio.h>
#include <shlwapi.h>

#include "renderer.h"

#pragma comment(lib, "shlwapi.lib")

std::unordered_map<std::string, MODEL*> ModelRenderer::m_ModelPool{};

void ModelRenderer::Draw()
{
	Draw(nullptr);
}

//指定したテクスチャへ差し替えてモデルを描画
void ModelRenderer::Draw(ID3D11ShaderResourceView* overrideTexture)
{
	//頂点バッファ設定
	UINT stride = sizeof(VERTEX_3D);
	UINT offset = 0;
	Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_Model->VertexBuffer, &stride, &offset);

	//インデックスバッファ設定
	Renderer::GetDeviceContext()->IASetIndexBuffer(m_Model->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (unsigned int i = 0; i < m_Model->SubsetNum; i++)
	{
		ID3D11ShaderResourceView* const texture =
			overrideTexture ? overrideTexture : m_Model->SubsetArray[i].Material.Texture;
		MATERIAL material = m_Model->SubsetArray[i].Material.Material;
		material.TextureEnable = texture != nullptr;

		//マテリアル設定
		Renderer::SetMaterial(material);

		//テクスチャ設定
		if (texture)
			Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &texture);

		//ポリゴン描画
		Renderer::GetDeviceContext()->DrawIndexed(
			m_Model->SubsetArray[i].IndexNum, m_Model->SubsetArray[i].StartIndex, 0);
	}
}

//指定したモデルをモデルプールへ事前に読み込む
void ModelRenderer::Preload(const char* fileName)
{
	if (m_ModelPool.count(fileName) > 0)
		return;

	MODEL* model = new MODEL{};
	LoadModel(fileName, model);

	m_ModelPool[fileName] = model;
}

void ModelRenderer::UnloadAll()
{
	for (const auto& pair : m_ModelPool)
	{
		pair.second->VertexBuffer->Release();
		pair.second->IndexBuffer->Release();

		for (unsigned int i = 0; i < pair.second->SubsetNum; i++)
		{
			if (pair.second->SubsetArray[i].Material.Texture)
				pair.second->SubsetArray[i].Material.Texture->Release();
		}

		delete[] pair.second->SubsetArray;
		delete pair.second;
	}

	m_ModelPool.clear();
}

void ModelRenderer::Load(const char* fileName)
{
	if (m_ModelPool.count(fileName) > 0)
	{
		m_Model = m_ModelPool[fileName];
		return;
	}

	m_Model = new MODEL{};
	LoadModel(fileName, m_Model);

	m_ModelPool[fileName] = m_Model;
}

//読み込み済みモデルを囲むローカル座標の境界を取得
bool ModelRenderer::GetLocalBounds(Vector3& center, Vector3& halfSize) const
{
	if (!m_Model || !m_Model->HasLocalBounds)
		return false;

	center = m_Model->LocalBoundsCenter;
	halfSize = m_Model->LocalBoundsHalfSize;
	return true;
}

//読み込んだモデルデータから描画用バッファを生成
void ModelRenderer::LoadModel(const char* fileName, MODEL* model)
{
	MODEL_OBJ modelObj{};
	LoadObj(fileName, &modelObj);

	//CPU上の頂点を破棄する前に、エディターの選択判定で使うモデル境界を保存
	if (modelObj.VertexNum > 0)
	{
		const XMFLOAT3& firstPosition = modelObj.VertexArray[0].Position;
		Vector3 minimum(firstPosition.x, firstPosition.y, firstPosition.z);
		Vector3 maximum = minimum;

		for (unsigned int i = 1; i < modelObj.VertexNum; ++i)
		{
			const XMFLOAT3& position = modelObj.VertexArray[i].Position;
			minimum.x = std::min(minimum.x, position.x);
			minimum.y = std::min(minimum.y, position.y);
			minimum.z = std::min(minimum.z, position.z);
			maximum.x = std::max(maximum.x, position.x);
			maximum.y = std::max(maximum.y, position.y);
			maximum.z = std::max(maximum.z, position.z);
		}

		model->LocalBoundsCenter = (minimum + maximum) * 0.5f;
		model->LocalBoundsHalfSize = (maximum - minimum) * 0.5f;
		model->HasLocalBounds = true;
	}

	//頂点バッファ生成
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(VERTEX_3D) * modelObj.VertexNum;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subresourceData{};
		subresourceData.pSysMem = modelObj.VertexArray;

		Renderer::GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, &model->VertexBuffer);
	}

	//インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bufferDesc{};
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = sizeof(unsigned int) * modelObj.IndexNum;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA subresourceData{};
		subresourceData.pSysMem = modelObj.IndexArray;

		Renderer::GetDevice()->CreateBuffer(&bufferDesc, &subresourceData, &model->IndexBuffer);
	}

	//サブセット設定
	{
		model->SubsetArray = new SUBSET[modelObj.SubsetNum];
		model->SubsetNum = modelObj.SubsetNum;

		for (unsigned int i = 0; i < modelObj.SubsetNum; i++)
		{
			model->SubsetArray[i].StartIndex = modelObj.SubsetArray[i].StartIndex;
			model->SubsetArray[i].IndexNum = modelObj.SubsetArray[i].IndexNum;

			model->SubsetArray[i].Material.Material = modelObj.SubsetArray[i].Material.Material;

			model->SubsetArray[i].Material.Texture = nullptr;

			//テクスチャ読み込み
			TexMetadata metadata;
			ScratchImage image;
			wchar_t wc[256];
			mbstowcs(wc, modelObj.SubsetArray[i].Material.TextureName, sizeof(wc));
			LoadFromWICFile(wc, WIC_FLAGS_NONE, &metadata, image);
			CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(),
				metadata, &model->SubsetArray[i].Material.Texture);

			model->SubsetArray[i].Material.Material.TextureEnable =
				model->SubsetArray[i].Material.Texture != nullptr;
		}
	}

	delete[] modelObj.VertexArray;
	delete[] modelObj.IndexArray;
	delete[] modelObj.SubsetArray;
}

//OBJファイルを解析して一時モデルデータを生成
void ModelRenderer::LoadObj(const char* fileName, MODEL_OBJ* modelObj)
{
	char dir[MAX_PATH];
	strcpy(dir, fileName);
	PathRemoveFileSpec(dir);

	XMFLOAT3	*positionArray;
	XMFLOAT3	*normalArray;
	XMFLOAT2	*texcoordArray;

	unsigned int	positionNum = 0;
	unsigned int	normalNum = 0;
	unsigned int	texcoordNum = 0;
	unsigned int	vertexNum = 0;
	unsigned int	indexNum = 0;
	unsigned int	in = 0;
	unsigned int	subsetNum = 0;

	MODEL_MATERIAL	*materialArray = nullptr;
	unsigned int	materialNum = 0;

	char str[256];
	char *s;
	char c;

	FILE *file;
	file = fopen(fileName, "rt");
	assert(file);

	//要素数カウント
	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "v" ) == 0 )
		{
			positionNum++;
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			normalNum++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			texcoordNum++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			subsetNum++;
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			in = 0;

			do
			{
				fscanf( file, "%s", str );
				vertexNum++;
				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c!= '\r' );

			//四角は三角に分割
			if( in == 4 )
				in = 6;

			indexNum += in;
		}
	}

	//メモリ確保
	positionArray = new XMFLOAT3[ positionNum ];
	normalArray = new XMFLOAT3[ normalNum ];
	texcoordArray = new XMFLOAT2[ texcoordNum ];

	modelObj->VertexArray = new VERTEX_3D[vertexNum];
	modelObj->VertexNum = vertexNum;

	modelObj->IndexArray = new unsigned int[indexNum];
	modelObj->IndexNum = indexNum;

	modelObj->SubsetArray = new SUBSET[subsetNum];
	modelObj->SubsetNum = subsetNum;

	//要素読込
	XMFLOAT3 *position = positionArray;
	XMFLOAT3 *normal = normalArray;
	XMFLOAT2 *texcoord = texcoordArray;

	unsigned int vc = 0;
	unsigned int ic = 0;
	unsigned int sc = 0;

	fseek( file, 0, SEEK_SET );

	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "mtllib" ) == 0 )
		{
			//マテリアルファイル
			fscanf( file, "%s", str );

			char path[256];
			strcpy( path, dir );
			strcat( path, "\\" );
			strcat( path, str );

			LoadMaterial( path, &materialArray, &materialNum );
		}
		else if( strcmp( str, "o" ) == 0 )
		{
			//オブジェクト名
			fscanf( file, "%s", str );
		}
		else if( strcmp( str, "v" ) == 0 )
		{
			//頂点座標
			fscanf( file, "%f", &position->x );
			fscanf( file, "%f", &position->y );
			fscanf( file, "%f", &position->z );
			position++;

			//頂点カラーなど余分な要素があっても次の行まで読み飛ばす
			do
			{
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' && c != EOF );
		}
		else if( strcmp( str, "vn" ) == 0 )
		{
			//法線
			fscanf( file, "%f", &normal->x );
			fscanf( file, "%f", &normal->y );
			fscanf( file, "%f", &normal->z );
			normal++;
		}
		else if( strcmp( str, "vt" ) == 0 )
		{
			//テクスチャ座標
			fscanf( file, "%f", &texcoord->x );
			fscanf( file, "%f", &texcoord->y );

			texcoord->y = 1.0f - texcoord->y;
			texcoord++;
		}
		else if( strcmp( str, "usemtl" ) == 0 )
		{
			//マテリアル
			fscanf( file, "%s", str );

			if( sc != 0 )
				modelObj->SubsetArray[sc - 1].IndexNum = ic - modelObj->SubsetArray[sc - 1].StartIndex;

			modelObj->SubsetArray[sc].StartIndex = ic;


			for( unsigned int i = 0; i < materialNum; i++ )
			{
				if( strcmp( str, materialArray[i].Name ) == 0 )
				{
					modelObj->SubsetArray[sc].Material.Material = materialArray[i].Material;
					strcpy(modelObj->SubsetArray[sc].Material.TextureName, materialArray[i].TextureName);
					strcpy(modelObj->SubsetArray[sc].Material.Name, materialArray[i].Name);

					break;
				}
			}

			sc++;
			
		}
		else if( strcmp( str, "f" ) == 0 )
		{
			//面
			in = 0;

			do
			{
				fscanf( file, "%s", str );

				s = strtok( str, "/");	
				modelObj->VertexArray[vc].Position = positionArray[atoi(s) - 1];
				if( s[ strlen( s ) + 1 ] != '/' )
				{
					//テクスチャ座標が存在しない場合もある
					s = strtok( nullptr, "/" );
					modelObj->VertexArray[vc].TexCoord = texcoordArray[atoi(s) - 1];
				}
				s = strtok( nullptr, "/" );	
				modelObj->VertexArray[vc].Normal = normalArray[atoi(s) - 1];

				modelObj->VertexArray[vc].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

				modelObj->IndexArray[ic] = vc;
				ic++;
				vc++;

				in++;
				c = fgetc( file );
			}
			while( c != '\n' && c != '\r' );

			//四角は三角に分割
			if( in == 4 )
			{
				modelObj->IndexArray[ic] = vc - 4;
				ic++;
				modelObj->IndexArray[ic] = vc - 2;
				ic++;
			}
		}
	}

	if( sc != 0 )
		modelObj->SubsetArray[sc - 1].IndexNum = ic - modelObj->SubsetArray[sc - 1].StartIndex;

	fclose( file );

	delete[] positionArray;
	delete[] normalArray;
	delete[] texcoordArray;
	delete[] materialArray;
}

void ModelRenderer::LoadMaterial(const char* fileName, MODEL_MATERIAL** materialArrayOutput,
	unsigned int* materialNumOutput)
{
	char dir[MAX_PATH];
	strcpy(dir, fileName);
	PathRemoveFileSpec(dir);

	char str[256];

	FILE *file;
	file = fopen(fileName, "rt");
	assert(file);

	MODEL_MATERIAL *materialArray;
	unsigned int materialNum = 0;

	//要素数カウント
	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "newmtl" ) == 0 )
		{
			materialNum++;
		}
	}

	//メモリ確保
	materialArray = new MODEL_MATERIAL[ materialNum ];

	//要素読込
	int mc = -1;

	fseek( file, 0, SEEK_SET );

	while( true )
	{
		fscanf( file, "%s", str );

		if( feof( file ) != 0 )
			break;

		if( strcmp( str, "newmtl" ) == 0 )
		{
			//マテリアル名
			mc++;
			fscanf( file, "%s", materialArray[ mc ].Name );
			strcpy( materialArray[ mc ].TextureName, "" );

			materialArray[mc].Material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			materialArray[mc].Material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			materialArray[mc].Material.Specular = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			materialArray[mc].Material.Emission = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			materialArray[mc].Material.Shininess = 0.0f;
			materialArray[mc].Material.TextureEnable = FALSE;
		}
		else if( strcmp( str, "Ka" ) == 0 )
		{
			//アンビエント
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Ambient.z );
			materialArray[ mc ].Material.Ambient.w = 1.0f;
		}
		else if( strcmp( str, "Kd" ) == 0 )
		{
			//ディフューズ
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.z );
			materialArray[ mc ].Material.Diffuse.w = 1.0f;
		}
		else if( strcmp( str, "Ks" ) == 0 )
		{
			//スペキュラ
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.x );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.y );
			fscanf( file, "%f", &materialArray[ mc ].Material.Specular.z );
			materialArray[ mc ].Material.Specular.w = 1.0f;
		}
		else if( strcmp( str, "Ns" ) == 0 )
		{
			//スペキュラ強度
			fscanf( file, "%f", &materialArray[ mc ].Material.Shininess );
		}
		else if( strcmp( str, "d" ) == 0 )
		{
			//アルファ
			fscanf( file, "%f", &materialArray[ mc ].Material.Diffuse.w );
		}
		else if( strcmp( str, "map_Kd" ) == 0 )
		{
			//テクスチャ
			fscanf( file, "%s", str );

			char path[256];
			strcpy( path, dir );
			strcat( path, "\\" );
			strcat( path, str );

			strcat( materialArray[ mc ].TextureName, path );
		}
	}

	fclose(file);

	*materialArrayOutput = materialArray;
	*materialNumOutput = materialNum;
}
