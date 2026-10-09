//-----------------------------------------------------------
//  [ audioManager.h ]
//  ゲーム全体で使うBGMとSEをまとめて管理
//-----------------------------------------------------------
#pragma once

#include <array>
#include "audio.h"

class Vector3;

enum class SEId
{
	Jump,
	Footstep1,
	Footstep2,
	Footstep3,
	Footstep4,
	Footstep5,
	Landing,
	Star,
	StarComplete,
	Heart,
	Damage,
	JumpPad,
	BreakPlatform,
	Checkpoint,
	GateRequirementNotice,
	GateOpen,
	Goal,
	TurretShot,
	GameOver,
	MenuMove,
	Click,
	Max //効果音の種類数を取得するための終端値
};

//長時間再生し、状況に応じて音量を変えるループ音
enum class LoopSEId
{
	Wind,
	Max
};

enum class BGMId
{
	Title,
	Stage,
	Clear,
	Max //BGMの種類数を取得するための終端値
};

class AudioManager
{
private:
	static constexpr int SEChannelCount = 4; //同じSEを重ねて鳴らすためのチャンネル数
	static constexpr int SECount = static_cast<int>(SEId::Max); //効果音の種類数
	static constexpr int LoopSECount = static_cast<int>(LoopSEId::Max); //ループ音の種類数
	static constexpr int BGMCount = static_cast<int>(BGMId::Max); //BGMの種類数

	//効果音1種類分のデータ
	struct SEData
	{
		std::array<Audio, SEChannelCount> Channels; //同じSEを複数再生するための配列
		bool Loaded = false; //読み込めたか
		int NextChannel = 0; //次にどのチャンネルで再生するか
		float BaseVolume = 1.0f; //距離減衰を適用する前の基準音量
		std::array<int, SEChannelCount> RemainingFrames{}; //指定時間だけ再生するSEの残りフレーム（0は制限なし）
	};

	//ループ音1種類分のデータ
	struct LoopSEData
	{
		Audio Channel; //ループ音を再生する専用チャンネル
		bool Loaded = false; //読み込めたか
		bool Playing = false; //現在ループ再生中か
		float BaseVolume = 1.0f; //距離減衰を適用する前の基準音量
		float CurrentVolumeScale = 0.0f; //フェードを反映した現在の音量倍率
		float RequestedVolumeScale = 0.0f; //各オブジェクトから要求された最大音量倍率
	};

	static std::array<SEData, SECount> m_SE;
	static std::array<LoopSEData, LoopSECount> m_LoopSE;
	static std::array<Audio, BGMCount> m_BGM;
	static std::array<bool, BGMCount> m_BGMLoaded;
	static BGMId m_CurrentBGM;

	//指定された音声ファイルが存在するか確認
	static bool FileExists(const char* fileName);
	static void LoadSE(SEId id, const char* fileName, float volume);
	static void LoadLoopSE(LoopSEId id, const char* fileName, float volume);
	static void LoadBGM(BGMId id, const char* fileName, float volume);
	//効果音の基準音量に指定倍率を掛けて再生
	static void PlaySEWithVolumeScale(SEId id, float volumeScale, int playFrames = 0);

public:
	static void Init();
	static void Uninit();

	static void PlaySE(SEId id);
	//指定フレームが経過したら停止する効果音を再生
	static void PlaySEForFrames(SEId id, int playFrames);
	//音源と聞き手の距離に応じて音量を減衰させて効果音を再生
	static void PlaySEAt(SEId id, const Vector3& sourcePosition,
		const Vector3& listenerPosition, float fullVolumeDistance, float maxDistance);
	//各オブジェクトから受け取ったループ音量のうち最大値を次フレームへ反映
	static void RequestLoopSE(LoopSEId id, float volumeScale);
	static void UpdateSE();
	//要求された音量へ近づけながらループ音の再生状態を更新
	static void UpdateLoopSE();
	static void PlayBGM(BGMId id, bool restart = false);
	static void StopBGM();
};
