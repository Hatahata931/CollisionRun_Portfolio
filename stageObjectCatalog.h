//-----------------------------------------------------------
//  [ stageObjectCatalog.h ]
//  ステージエディターで配置できるアセットの登録情報
//-----------------------------------------------------------
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "vector3.h"

//カタログに登録されたモデルをどのクラスで生成するか
enum class StageObjectKind
{
	Box, //当たり判定を持つ汎用箱
	Prop, //当たり判定を持たない装飾用オブジェクト
	Terrain, //面ごとのテクスチャを持つ地形ブロック
	BreakPlatform,
	Checkpoint,
	JumpPad,
	Spike,
	Star,
	Heart,
};

//TerrainBlockの各面へ設定するテクスチャ一式
struct TerrainTextureInfo
{
	const char* TopTexturePath;
	const char* UpperSideTexturePath;
	const char* RepeatingSideTexturePath;
	const char* BottomTexturePath;
};

//配置アセット1種類分の生成設定
struct StageObjectDefinition
{
	const char* Id; //保存用の識別名
	const char* DisplayName; //エディター上の表示名
	StageObjectKind Kind;
	const char* ModelPath; //Box・Prop用モデルパス
	Vector3 ColliderHalfSize; //Box用OBB判定の半サイズ
	Vector3 ColliderCenterOffset; //Box用OBB判定オフセット
	TerrainTextureInfo TerrainTextures; //Terrain用の面別テクスチャ
	const char* BoxTexturePath; //Box用テクスチャのパス
	Vector3 DefaultRotationVelocity; //新規配置時の回転速度
};

//配置可能なアセット情報を一元管理するカタログ
class StageObjectCatalog
{
public:
	//登録されている全配置アセット情報を取得
	static const std::vector<StageObjectDefinition>& GetAll();
	//保存用IDに一致する配置アセット情報を取得
	static const StageObjectDefinition* Find(const std::string& id);
	//保存用IDに一致する配置アセットの登録番号を取得
	static std::size_t FindIndex(const std::string& id);
};
