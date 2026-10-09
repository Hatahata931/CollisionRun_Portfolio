//-----------------------------------------------------------
//  [ animationModel.h ]
//  ボーンアニメーション付き3Dモデルの読み込み・描画を管理
//-----------------------------------------------------------
#pragma once

#include <unordered_map>

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/matrix4x4.h"
#pragma comment (lib, "assimp-vc143-mt.lib")

//変形後頂点構造体
struct DEFORM_VERTEX
{
	aiVector3D Position;
	aiVector3D Normal;
	int BoneNum;
	std::string BoneName[4];
	float BoneWeight[4];
};

//ボーン構造体
struct BONE
{
	aiMatrix4x4 Matrix;
	aiMatrix4x4 AnimationMatrix;
	aiMatrix4x4 OffsetMatrix;
};

//アニメーションが最後のフレームまで進んだ後の再生方法
enum class AnimationPlaybackMode
{
	Loop,
	Clamp, //最後のフレームで停止
};

//指定ノードの位置アニメーションを適用するか
enum class AnimationPositionMode
{
	Animated,
	Locked,
};

class AnimationModel
{
private:
	const aiScene* m_AiScene = nullptr;
	std::unordered_map<std::string, const aiScene*> m_Animation;

	ID3D11Buffer** m_VertexBuffer = nullptr;
	ID3D11Buffer** m_IndexBuffer = nullptr;

	std::unordered_map<std::string, ID3D11ShaderResourceView*> m_Texture;
	std::vector<DEFORM_VERTEX>* m_DeformVertex = nullptr; //変形後頂点データ
	std::unordered_map<std::string, BONE> m_Bone; //ボーンデータ(名前で参照)

	//専用ライト
	bool m_UseLightOverride = false; //描画中だけ専用ライトへ切り替えるか
	float m_LightAmbient = 0.0f; //環境光
	float m_LightDiffuse = 0.0f; //拡散光

	std::string m_RootMotionNodeName; //ルート移動を持つノード名

	//ノード階層からボーンの初期姿勢を作成
	void CreateBone(aiNode* node);
	//親子関係を反映してボーン行列を再帰的に更新
	void UpdateBoneMatrix(aiNode* node, aiMatrix4x4 matrix);

public:

	void Load(const char* fileName);
	void LoadAnimation(const char* fileName, const char* name);
	void Uninit();
	//2つのアニメーションの間の姿勢を補間
	void Update(const char* animationName1, double frame1,
		const char* animationName2, double frame2, float blendRate,
		AnimationPlaybackMode playbackMode1 = AnimationPlaybackMode::Loop,
		AnimationPlaybackMode playbackMode2 = AnimationPlaybackMode::Loop,
		AnimationPositionMode positionMode1 = AnimationPositionMode::Animated,
		AnimationPositionMode positionMode2 = AnimationPositionMode::Animated);

	void SetLightOverride(float ambient, float diffuse);
	//位置アニメーションの適用を切り替える対象ノード名を設定する
	void SetRootMotionNode(const char* nodeName);
	void Draw();
};
