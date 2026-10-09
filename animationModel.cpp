//-----------------------------------------------------------
//  [ animationModel.cpp ]
//  ボーンアニメーション付き3Dモデルの読み込み・描画を管理
//-----------------------------------------------------------
#include "main.h"
#include "renderer.h"
#include "animationModel.h"

#include <cmath>

namespace
{
	//AnimationModelごとの初期ローカル姿勢、更新後のワールド姿勢、メッシュ所有ノード名
	std::unordered_map<const AnimationModel*, std::unordered_map<std::string, aiMatrix4x4>> g_BoneLocalTransform;
	std::unordered_map<const AnimationModel*, std::unordered_map<std::string, aiMatrix4x4>> g_BoneWorldTransform;
	std::unordered_map<const AnimationModel*, std::vector<std::string>> g_MeshNodeName;

	//各メッシュを所有するノード名を登録
	void RegisterMeshNodeNames(const AnimationModel* model, aiNode* node)
	{
		auto& meshNodeNames = g_MeshNodeName[model];

		for (unsigned int i = 0; i < node->mNumMeshes; ++i)
		{
			const unsigned int meshIndex = node->mMeshes[i];
			if (meshIndex < meshNodeNames.size())
			{
				meshNodeNames[meshIndex] = node->mName.C_Str();
			}
		}

		for (unsigned int i = 0; i < node->mNumChildren; ++i)
		{
			RegisterMeshNodeNames(model, node->mChildren[i]);
		}
	}

	//透明度以外に使われているアルファ値を完全不透明にする
	void ForceOpaqueAlpha(ScratchImage& image)
	{
		const DirectX::Image* images = image.GetImages();
		const size_t imageCount = image.GetImageCount();

		for (size_t i = 0; i < imageCount; ++i)
		{
			const DXGI_FORMAT format = images[i].format;
			if (format != DXGI_FORMAT_R8G8B8A8_UNORM &&
				format != DXGI_FORMAT_B8G8R8A8_UNORM &&
				format != DXGI_FORMAT_R8G8B8A8_UNORM_SRGB &&
				format != DXGI_FORMAT_B8G8R8A8_UNORM_SRGB)
			{
				continue;
			}

			for (size_t y = 0; y < images[i].height; ++y)
			{
				unsigned char* row = images[i].pixels + y * images[i].rowPitch;
				for (size_t x = 0; x < images[i].width; ++x)
				{
					row[x * 4 + 3] = 255;
				}
			}
		}
	}

	//モデル内の埋め込みテクスチャを読み込み、描画用のSRVを生成
	bool LoadEmbeddedTextureSRV(const aiTexture* aiTextureData, ID3D11ShaderResourceView** outTexture)
	{
		if (!aiTextureData || !outTexture)
		{
			return false;
		}

		*outTexture = nullptr;

		TexMetadata metadata{};
		ScratchImage image;
		HRESULT hr = E_FAIL;

		//mHeightが0の場合、テクスチャは圧縮されたファイルデータとして格納されている
		if (aiTextureData->mHeight == 0)
		{
			if (_stricmp(aiTextureData->achFormatHint, "tga") == 0)
			{
				hr = LoadFromTGAMemory(aiTextureData->pcData, aiTextureData->mWidth, &metadata, image);
			}
			else
			{
				hr = LoadFromWICMemory(aiTextureData->pcData, aiTextureData->mWidth, WIC_FLAGS_NONE, &metadata, image);
			}
		}

		if (FAILED(hr))
		{
			return false;
		}

		ForceOpaqueAlpha(image);

		hr = CreateShaderResourceView(Renderer::GetDevice(), image.GetImages(), image.GetImageCount(), metadata, outTexture);
		return SUCCEEDED(hr) && *outTexture != nullptr;
	}

	//再生方法に応じて指定フレームをアニメーション内の時刻へ変換
	double ResolveAnimationTime(const aiAnimation* animation,
		double frame, AnimationPlaybackMode playbackMode)
	{
		if (!animation || animation->mDuration <= 0.0)
			return 0.0;

		//Clampの場合最終フレームを超えないようにする
		const double sourceFrame = frame > 0.0f ? frame : 0.0;
		if (playbackMode == AnimationPlaybackMode::Clamp)
			return sourceFrame < animation->mDuration
				? sourceFrame : animation->mDuration;

		//Loopの場合最終フレームまで表示してから先頭に戻す
		const double loopFrameCount = std::floor(animation->mDuration + 0.0001) + 1.0;
		double animationTime = std::fmod(sourceFrame, loopFrameCount);
		if (animationTime > animation->mDuration)
			animationTime = animation->mDuration;

		return animationTime;
	}

	//指定したノードのアニメーションを探す
	const aiNodeAnim* FindNodeAnimation(const aiAnimation* animation, const std::string& nodeName)
	{
		if (!animation)
			return nullptr;

		//名前が一致したらそのノードの情報を返す
		for (unsigned int channelIndex = 0;
			channelIndex < animation->mNumChannels; ++channelIndex)
		{
			const aiNodeAnim* nodeAnimation = animation->mChannels[channelIndex];
			if (nodeAnimation->mNodeName == aiString(nodeName))
				return nodeAnimation;
		}

		return nullptr;
	}

	//キーの時刻を基準に位置またはスケールを補間
	aiVector3D SampleVectorKeys(const aiVectorKey* keys, unsigned int keyCount,
		double animationTime, const aiVector3D& fallback)
	{
		if (!keys || keyCount == 0)
			return fallback;
		if (keyCount == 1 || animationTime <= keys[0].mTime)
			return keys[0].mValue;
		if (animationTime >= keys[keyCount - 1].mTime)
			return keys[keyCount - 1].mValue;

		for (unsigned int keyIndex = 0; keyIndex + 1 < keyCount; ++keyIndex)
		{
			const aiVectorKey& currentKey = keys[keyIndex];
			const aiVectorKey& nextKey = keys[keyIndex + 1];
			if (animationTime > nextKey.mTime)
				continue;

			const double keyDuration = nextKey.mTime - currentKey.mTime;
			const float rate = keyDuration > 0.0
				? static_cast<float>((animationTime - currentKey.mTime) / keyDuration)
				: 0.0f;
			return currentKey.mValue * (1.0f - rate) + nextKey.mValue * rate;
		}

		return keys[keyCount - 1].mValue;
	}

	//キーの時刻を基準に回転を補間
	aiQuaternion SampleQuaternionKeys(const aiQuatKey* keys, unsigned int keyCount,
		double animationTime, const aiQuaternion& fallback)
	{
		if (!keys || keyCount == 0)
			return fallback;
		if (keyCount == 1 || animationTime <= keys[0].mTime)
			return keys[0].mValue;
		if (animationTime >= keys[keyCount - 1].mTime)
			return keys[keyCount - 1].mValue;

		for (unsigned int keyIndex = 0; keyIndex + 1 < keyCount; ++keyIndex)
		{
			const aiQuatKey& currentKey = keys[keyIndex];
			const aiQuatKey& nextKey = keys[keyIndex + 1];
			if (animationTime > nextKey.mTime)
				continue;

			const double keyDuration = nextKey.mTime - currentKey.mTime;
			const float rate = keyDuration > 0.0
				? static_cast<float>((animationTime - currentKey.mTime) / keyDuration)
				: 0.0f;

			aiQuaternion result;
			aiQuaternion::Interpolate(result, currentKey.mValue, nextKey.mValue, rate);
			result.Normalize();
			return result;
		}

		return keys[keyCount - 1].mValue;
	}

	//1つのアニメーションチャンネルから指定時刻の変換情報を取得
	void SampleNodeTransform(const aiNodeAnim* nodeAnimation, double animationTime,
		bool lockPosition, aiVector3D& scale, aiQuaternion& rotation, aiVector3D& position)
	{
		if (!nodeAnimation)
			return;

		scale = SampleVectorKeys(nodeAnimation->mScalingKeys,
			nodeAnimation->mNumScalingKeys, animationTime, scale);
		rotation = SampleQuaternionKeys(nodeAnimation->mRotationKeys,
			nodeAnimation->mNumRotationKeys, animationTime, rotation);

		if (!lockPosition)
		{
			position = SampleVectorKeys(nodeAnimation->mPositionKeys,
				nodeAnimation->mNumPositionKeys, animationTime, position);
		}
	}
}

void AnimationModel::Draw()
{
	//モデル固有の明るさを設定
	LIGHT previousLight{};
	if (m_UseLightOverride)
	{
		previousLight = Renderer::GetLight();
		LIGHT overrideLight = previousLight;
		overrideLight.Ambient = XMFLOAT4(m_LightAmbient, m_LightAmbient, m_LightAmbient, 1.0f);
		overrideLight.Diffuse = XMFLOAT4(m_LightDiffuse, m_LightDiffuse, m_LightDiffuse, 1.0f);
		Renderer::SetLight(overrideLight);
	}

	//プリミティブトポロジ設定
	Renderer::GetDeviceContext()->IASetPrimitiveTopology(
		D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	//マテリアル設定
	MATERIAL material;
	ZeroMemory(&material, sizeof(material));
	material.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.Ambient = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	material.TextureEnable = true;
	Renderer::SetMaterial(material);

	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		//マテリアル設定
		aiString texture;
		aiColor3D diffuse(1.0f, 1.0f, 1.0f);
		float opacity = 1.0f;

		aiMaterial* aimaterial = m_AiScene->mMaterials[mesh->mMaterialIndex];
		const bool hasTexture = aimaterial->GetTexture(aiTextureType_DIFFUSE, 0, &texture) == aiReturn_SUCCESS
			&& texture.length > 0;
		aimaterial->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
		aimaterial->Get(AI_MATKEY_OPACITY, opacity);

		ID3D11ShaderResourceView* textureResource = nullptr;
		if (hasTexture)
		{
			auto it = m_Texture.find(texture.C_Str());
			if (it != m_Texture.end())
			{
				textureResource = it->second;
			}
		}

		Renderer::GetDeviceContext()->PSSetShaderResources(0, 1, &textureResource);
		material.TextureEnable = textureResource != nullptr;

		material.Diffuse = XMFLOAT4(diffuse.r, diffuse.g, diffuse.b, opacity);
		material.Ambient = material.Diffuse;
		Renderer::SetMaterial(material);

		UINT stride = sizeof(VERTEX_3D);
		UINT offset = 0;
		Renderer::GetDeviceContext()->IASetVertexBuffers(0, 1, &m_VertexBuffer[m], &stride, &offset);

		Renderer::GetDeviceContext()->IASetIndexBuffer(m_IndexBuffer[m], DXGI_FORMAT_R32_UINT, 0);

		Renderer::GetDeviceContext()->DrawIndexed(mesh->mNumFaces * 3, 0, 0);
	}

	//他のオブジェクトに専用ライトが影響しないように描画前の設定へ戻す
	if (m_UseLightOverride)
		Renderer::SetLight(previousLight);
}


void AnimationModel::SetLightOverride(float ambient, float diffuse)
{
	m_UseLightOverride = true;
	m_LightAmbient = ambient;
	m_LightDiffuse = diffuse;
}

//位置アニメーションの適用を切り替える対象ノード名を設定
void AnimationModel::SetRootMotionNode(const char* nodeName)
{
	m_RootMotionNodeName = nodeName ? nodeName : "";
}

void AnimationModel::Load(const char* fileName)
{
	m_AiScene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_ConvertToLeftHanded);
	assert(m_AiScene);

	m_VertexBuffer = new ID3D11Buffer*[m_AiScene->mNumMeshes];
	m_IndexBuffer = new ID3D11Buffer*[m_AiScene->mNumMeshes];

	//変形後頂点配列生成
	m_DeformVertex = new std::vector<DEFORM_VERTEX>[m_AiScene->mNumMeshes];

	//再帰的にボーン生成
	CreateBone(m_AiScene->mRootNode);
	g_MeshNodeName[this] = std::vector<std::string>(m_AiScene->mNumMeshes);
	RegisterMeshNodeNames(this, m_AiScene->mRootNode);

	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		//頂点バッファ生成
		{
			VERTEX_3D* vertex = new VERTEX_3D[mesh->mNumVertices];

			for (unsigned int v = 0; v < mesh->mNumVertices; v++)
			{
				vertex[v].Position = XMFLOAT3(mesh->mVertices[v].x, mesh->mVertices[v].y, mesh->mVertices[v].z);
				vertex[v].Normal = XMFLOAT3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
				vertex[v].TexCoord = XMFLOAT2(mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y);
				vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.ByteWidth = sizeof(VERTEX_3D) * mesh->mNumVertices;
			bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = vertex;

			Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_VertexBuffer[m]);

			delete[] vertex;
		}

		//インデックスバッファ生成
		{
			unsigned int* index = new unsigned int[mesh->mNumFaces * 3];

			for (unsigned int f = 0; f < mesh->mNumFaces; f++)
			{
				const aiFace* face = &mesh->mFaces[f];

				assert(face->mNumIndices == 3);

				index[f * 3 + 0] = face->mIndices[0];
				index[f * 3 + 1] = face->mIndices[1];
				index[f * 3 + 2] = face->mIndices[2];
			}

			D3D11_BUFFER_DESC bd;
			ZeroMemory(&bd, sizeof(bd));
			bd.Usage = D3D11_USAGE_DEFAULT;
			bd.ByteWidth = sizeof(unsigned int) * mesh->mNumFaces * 3;
			bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bd.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA sd;
			ZeroMemory(&sd, sizeof(sd));
			sd.pSysMem = index;

			Renderer::GetDevice()->CreateBuffer(&bd, &sd, &m_IndexBuffer[m]);

			delete[] index;
		}

		//変形後頂点データ初期化
		for (unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX deformVertex;
			deformVertex.Position = mesh->mVertices[v];
			deformVertex.Normal = mesh->mNormals[v];
			deformVertex.BoneNum = 0;

			for (unsigned int b = 0; b < 4; b++)
			{
				deformVertex.BoneName[b] = "";
				deformVertex.BoneWeight[b] = 0.0f;
			}

			m_DeformVertex[m].push_back(deformVertex);
		}

		//ボーンデータ初期化
		for (unsigned int b = 0; b < mesh->mNumBones; b++)
		{
			aiBone* bone = mesh->mBones[b];

			m_Bone[bone->mName.C_Str()].OffsetMatrix = bone->mOffsetMatrix;

			//変形後頂点にボーンデータ格納
			for (unsigned int w = 0; w < bone->mNumWeights; w++)
			{
				aiVertexWeight weight = bone->mWeights[w];

				int num = m_DeformVertex[m][weight.mVertexId].BoneNum;

				m_DeformVertex[m][weight.mVertexId].BoneWeight[num] = weight.mWeight;
				m_DeformVertex[m][weight.mVertexId].BoneName[num] = bone->mName.C_Str();
				m_DeformVertex[m][weight.mVertexId].BoneNum++;

				assert(m_DeformVertex[m][weight.mVertexId].BoneNum <= 4);
			}
		}
	}

	//テクスチャ読み込み
	for (unsigned int i = 0; i < m_AiScene->mNumTextures; ++i)
	{
		aiTexture* aitexture = m_AiScene->mTextures[i];

		ID3D11ShaderResourceView* texture = nullptr;
		if (LoadEmbeddedTextureSRV(aitexture, &texture))
		{
			m_Texture[aitexture->mFilename.data] = texture;
		}
	}
}

void AnimationModel::LoadAnimation(const char* fileName, const char* name)
{
	m_Animation[name] = aiImportFile(fileName, aiProcess_ConvertToLeftHanded);
	assert(m_Animation[name]);
}

//ノード階層からボーンの初期姿勢を作成
void AnimationModel::CreateBone(aiNode* node)
{
	BONE bone{};
	bone.Matrix = aiMatrix4x4();
	bone.AnimationMatrix = node->mTransformation;
	bone.OffsetMatrix = aiMatrix4x4();

	g_BoneLocalTransform[this][node->mName.C_Str()] = node->mTransformation;
	m_Bone[node->mName.C_Str()] = bone;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		CreateBone(node->mChildren[n]);
	}
}

void AnimationModel::Uninit()
{
	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		m_VertexBuffer[m]->Release();
		m_IndexBuffer[m]->Release();
	}

	delete[] m_VertexBuffer;
	delete[] m_IndexBuffer;
	delete[] m_DeformVertex;

	for (const auto& pair : m_Texture)
	{
		pair.second->Release();
	}

	aiReleaseImport(m_AiScene);

	for (const auto& pair : m_Animation)
	{
		aiReleaseImport(pair.second);
	}

	g_BoneLocalTransform.erase(this);
	g_BoneWorldTransform.erase(this);
	g_MeshNodeName.erase(this);
}

//2つのアニメーションの間の姿勢を補間
void AnimationModel::Update(const char* animationName1, double frame1,
	const char* animationName2, double frame2, float blendRate,
	AnimationPlaybackMode playbackMode1, AnimationPlaybackMode playbackMode2,
	AnimationPositionMode positionMode1, AnimationPositionMode positionMode2)
{
	if (m_Animation.count(animationName1) == 0)
		return;
	if (!m_Animation[animationName1]->HasAnimations())
		return;

	if (m_Animation.count(animationName2) == 0)
		return;
	if (!m_Animation[animationName2]->HasAnimations())
		return;

	//アニメーションデータからボーンマトリクス算出
	const aiAnimation* animation1 = m_Animation[animationName1]->mAnimations[0];
	const aiAnimation* animation2 = m_Animation[animationName2]->mAnimations[0];
	const double animationTime1 = ResolveAnimationTime(animation1, frame1, playbackMode1);
	const double animationTime2 = ResolveAnimationTime(animation2, frame2, playbackMode2);

	for (auto& pair : m_Bone)
	{
		BONE& bone = pair.second;
		aiVector3D scale1(1.0f, 1.0f, 1.0f);
		aiVector3D scale2(1.0f, 1.0f, 1.0f);
		aiQuaternion rot1 = aiQuaternion();
		aiQuaternion rot2 = aiQuaternion();
		aiVector3D pos1(0.0f, 0.0f, 0.0f);
		aiVector3D pos2(0.0f, 0.0f, 0.0f);

		//各ボーンの基本姿勢を用意
		auto localTransformSet = g_BoneLocalTransform.find(this);
		if (localTransformSet != g_BoneLocalTransform.end())
		{
			auto localTransform = localTransformSet->second.find(pair.first);
			if (localTransform != localTransformSet->second.end())
			{
				localTransform->second.Decompose(scale1, rot1, pos1);
				scale2 = scale1;
				rot2 = rot1;
				pos2 = pos1;
			}
		}

		//ルート位置を固定するか
		const aiNodeAnim* nodeAnim1 = FindNodeAnimation(animation1, pair.first);
		const aiNodeAnim* nodeAnim2 = FindNodeAnimation(animation2, pair.first);
		const bool rootMotionNode = !m_RootMotionNodeName.empty() && pair.first == m_RootMotionNodeName;
		const bool lockNodePosition1 = rootMotionNode && positionMode1 == AnimationPositionMode::Locked;
		const bool lockNodePosition2 = rootMotionNode && positionMode2 == AnimationPositionMode::Locked;

		//姿勢を取得
		SampleNodeTransform(nodeAnim1, animationTime1, lockNodePosition1, scale1, rot1, pos1);
		SampleNodeTransform(nodeAnim2, animationTime2, lockNodePosition2, scale2, rot2, pos2);

		//2つのアニメーションの姿勢を混ぜる
		if (nodeAnim1 || nodeAnim2)
		{
			aiVector3D scale;
			scale = scale1 * (1.0f - blendRate) + scale2 * blendRate;
			aiVector3D pos;
			pos = pos1 * (1.0f - blendRate) + pos2 * blendRate;

			aiQuaternion rot;
			aiQuaternion::Interpolate(rot, rot1, rot2, blendRate);

			//ボーン1個に対するアニメーション
			bone.AnimationMatrix = aiMatrix4x4(scale, rot, pos);
		}
	}

	//再帰的にボーンマトリクスを更新
	aiMatrix4x4 rootMatrix = aiMatrix4x4(aiVector3D(1.0f, 1.0f, 1.0f),
		aiQuaternion(static_cast<float>(AI_MATH_PI), 0.0f, 0.0f), aiVector3D(0.0f, 0.0f, 0.0f));

	UpdateBoneMatrix(m_AiScene->mRootNode, rootMatrix);

	//頂点変換（CPUスキニング）
	for (unsigned int m = 0; m < m_AiScene->mNumMeshes; m++)
	{
		aiMesh* mesh = m_AiScene->mMeshes[m];

		D3D11_MAPPED_SUBRESOURCE ms;
		Renderer::GetDeviceContext()->Map(m_VertexBuffer[m], 0,
			D3D11_MAP_WRITE_DISCARD, 0, &ms);

		VERTEX_3D* vertex = (VERTEX_3D*)ms.pData;

		for (unsigned int v = 0; v < mesh->mNumVertices; v++)
		{
			DEFORM_VERTEX* deformVertex = &m_DeformVertex[m][v];

			aiMatrix4x4 outMatrix = aiMatrix4x4();
			if (deformVertex->BoneNum > 0)
			{
				outMatrix = aiMatrix4x4(ai_real(0.0f), ai_real(0.0f), ai_real(0.0f), ai_real(0.0f),
					ai_real(0.0f), ai_real(0.0f), ai_real(0.0f), ai_real(0.0f),
					ai_real(0.0f), ai_real(0.0f), ai_real(0.0f), ai_real(0.0f),
					ai_real(0.0f), ai_real(0.0f), ai_real(0.0f), ai_real(0.0f));

				float weightSum = 0.0f;
				for (int b = 0; b < deformVertex->BoneNum; ++b)
				{
					auto it = m_Bone.find(deformVertex->BoneName[b]);
					if (it == m_Bone.end())
					{
						continue;
					}

					outMatrix = outMatrix + (it->second.Matrix * deformVertex->BoneWeight[b]);
					weightSum += deformVertex->BoneWeight[b];
				}

				if (weightSum > 0.0f)
				{
					outMatrix = outMatrix * (1.0f / weightSum);
				}
				else
				{
					outMatrix = aiMatrix4x4();
				}
			}
			else
			{
				auto meshNodeNameSet = g_MeshNodeName.find(this);
				if (meshNodeNameSet != g_MeshNodeName.end() &&
					m < meshNodeNameSet->second.size())
				{
					auto worldTransformSet = g_BoneWorldTransform.find(this);
					if (worldTransformSet != g_BoneWorldTransform.end())
					{
						auto worldTransform = worldTransformSet->second.find(meshNodeNameSet->second[m]);
						if (worldTransform != worldTransformSet->second.end())
						{
							outMatrix = worldTransform->second;
						}
					}
				}
			}

			deformVertex->Position = mesh->mVertices[v];
			deformVertex->Position *= outMatrix;

			//法線変換用に移動成分を削除
			outMatrix.a4 = 0.0f;
			outMatrix.b4 = 0.0f;
			outMatrix.c4 = 0.0f;

			deformVertex->Normal = mesh->mNormals[v];
			deformVertex->Normal *= outMatrix;

			//頂点バッファへ書き込み
			vertex[v].Position.x = deformVertex->Position.x;
			vertex[v].Position.y = deformVertex->Position.y;
			vertex[v].Position.z = deformVertex->Position.z;

			vertex[v].Normal.x = deformVertex->Normal.x;
			vertex[v].Normal.y = deformVertex->Normal.y;
			vertex[v].Normal.z = deformVertex->Normal.z;

			vertex[v].TexCoord.x = mesh->mTextureCoords[0][v].x;
			vertex[v].TexCoord.y = mesh->mTextureCoords[0][v].y;

			vertex[v].Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}

		Renderer::GetDeviceContext()->Unmap(m_VertexBuffer[m], 0);
	}
}

//親子関係を反映してボーン行列を再帰的に更新
void AnimationModel::UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix)
{
	BONE& bone = m_Bone[node->mName.C_Str()];

	aiMatrix4x4 worldMatrix;
	worldMatrix = matrix * bone.AnimationMatrix;

	g_BoneWorldTransform[this][node->mName.C_Str()] = worldMatrix;
	bone.Matrix = worldMatrix * bone.OffsetMatrix;

	for (unsigned int n = 0; n < node->mNumChildren; n++)
	{
		UpdateBoneMatrix(node->mChildren[n], worldMatrix);
	}
}

