//-----------------------------------------------------------
//  [ audioManager.cpp ]
//  ゲーム全体で使うBGMとSEをまとめて管理
//-----------------------------------------------------------
#include "main.h"
#include "audioManager.h"

#include <algorithm>
#include <fstream>

#include "vector3.h"

std::array<AudioManager::SEData, AudioManager::SECount> AudioManager::m_SE{};
std::array<AudioManager::LoopSEData, AudioManager::LoopSECount> AudioManager::m_LoopSE{};
std::array<Audio, AudioManager::BGMCount> AudioManager::m_BGM{};
std::array<bool, AudioManager::BGMCount> AudioManager::m_BGMLoaded{};
BGMId AudioManager::m_CurrentBGM = BGMId::Max;

//補助関数
namespace
{
	constexpr float LoopSEVolumeStep = 0.05f; //ループ音量が1フレームで変化する最大量

	//列挙値を配列の添字へ変換
	int ToIndex(SEId id)
	{
		return static_cast<int>(id);
	}

	int ToIndex(LoopSEId id)
	{
		return static_cast<int>(id);
	}

	int ToIndex(BGMId id)
	{
		return static_cast<int>(id);
	}

	//現在値を指定量以内で目標値へ近づける
	float MoveTowards(float current, float target, float maximumDelta)
	{
		if (current < target)
			return std::min(current + maximumDelta, target);
		return std::max(current - maximumDelta, target);
	}
}

//指定された音声ファイルが存在するか確認
bool AudioManager::FileExists(const char* fileName)
{
	std::ifstream file(fileName, std::ios::binary);
	return file.good();
}

void AudioManager::LoadSE(SEId id, const char* fileName, float volume)
{
	if (!FileExists(fileName))
		return;

	SEData& se = m_SE[ToIndex(id)];
	for (Audio& audio : se.Channels)
	{
		audio.Load(fileName);
		audio.SetVolume(volume);
	}

	se.Loaded = true;
	se.NextChannel = 0;
	se.BaseVolume = volume;
}

void AudioManager::LoadLoopSE(LoopSEId id, const char* fileName, float volume)
{
	if (!FileExists(fileName))
		return;

	LoopSEData& loopSE = m_LoopSE[ToIndex(id)];
	loopSE.Channel.Load(fileName);
	loopSE.Channel.SetVolume(0.0f);
	loopSE.Loaded = true;
	loopSE.Playing = false;
	loopSE.BaseVolume = volume;
	loopSE.CurrentVolumeScale = 0.0f;
	loopSE.RequestedVolumeScale = 0.0f;
}

void AudioManager::LoadBGM(BGMId id, const char* fileName, float volume)
{
	if (!FileExists(fileName))
		return;

	const int index = ToIndex(id);
	m_BGM[index].Load(fileName);
	m_BGM[index].SetVolume(volume);
	m_BGMLoaded[index] = true;
}

void AudioManager::Init()
{
	//音はここに登録する
	LoadBGM(BGMId::Title, "asset\\audio\\bgm_title.wav", 0.35f);
	LoadBGM(BGMId::Stage, "asset\\audio\\bgm_stage.wav", 0.35f);
	LoadBGM(BGMId::Clear, "asset\\audio\\bgm_clear.wav", 0.35f);

	LoadSE(SEId::Jump, "asset\\audio\\se_jump.wav", 0.6f);
	LoadSE(SEId::Footstep1, "asset\\audio\\se_footstep_01.wav", 0.35f);
	LoadSE(SEId::Footstep2, "asset\\audio\\se_footstep_02.wav", 0.35f);
	LoadSE(SEId::Footstep3, "asset\\audio\\se_footstep_03.wav", 0.35f);
	LoadSE(SEId::Footstep4, "asset\\audio\\se_footstep_04.wav", 0.35f);
	LoadSE(SEId::Footstep5, "asset\\audio\\se_footstep_05.wav", 0.35f);
	LoadSE(SEId::Landing, "asset\\audio\\se_landing.wav", 1.0f);
	LoadSE(SEId::Star, "asset\\audio\\se_star.wav", 0.6f);
	LoadSE(SEId::StarComplete, "asset\\audio\\se_star_complete.wav", 0.7f);
	LoadSE(SEId::Heart, "asset\\audio\\se_heart.wav", 0.7f);
	LoadSE(SEId::Damage, "asset\\audio\\se_damage.wav", 0.24f);
	LoadSE(SEId::JumpPad, "asset\\audio\\se_jump_pad.wav", 0.6f);
	LoadSE(SEId::BreakPlatform, "asset\\audio\\se_break_platform.wav", 0.5f);
	LoadSE(SEId::Checkpoint, "asset\\audio\\se_checkpoint.wav", 0.7f);
	LoadSE(SEId::GateRequirementNotice, "asset\\audio\\se_gate_requirement_notice.wav", 0.8f);
	LoadSE(SEId::GateOpen, "asset\\audio\\se_gate_open.wav", 0.6f);
	LoadSE(SEId::Goal, "asset\\audio\\se_goal.wav", 0.8f);
	LoadSE(SEId::TurretShot, "asset\\audio\\se_turret_shot.wav", 0.4f);
	LoadSE(SEId::GameOver, "asset\\audio\\se_game_over.wav", 0.7f);
	LoadSE(SEId::MenuMove, "asset\\audio\\se_menu_move.wav", 0.6f);
	LoadSE(SEId::Click, "asset\\audio\\se_click.wav", 0.65f);

	LoadLoopSE(LoopSEId::Wind, "asset\\audio\\se_wind.wav", 0.6f);
}

void AudioManager::Uninit()
{
	StopBGM();

	for (SEData& se : m_SE)
	{
		if (!se.Loaded)
			continue;

		for (Audio& audio : se.Channels)
		{
			audio.Uninit();
		}

		se.Loaded = false;
		se.NextChannel = 0;
		se.BaseVolume = 1.0f;
		se.RemainingFrames.fill(0);
	}

	for (LoopSEData& loopSE : m_LoopSE)
	{
		if (!loopSE.Loaded)
			continue;

		loopSE.Channel.Uninit();
		loopSE.Loaded = false;
		loopSE.Playing = false;
		loopSE.BaseVolume = 1.0f;
		loopSE.CurrentVolumeScale = 0.0f;
		loopSE.RequestedVolumeScale = 0.0f;
	}

	for (int i = 0; i < BGMCount; ++i)
	{
		if (!m_BGMLoaded[i])
			continue;

		m_BGM[i].Uninit();
		m_BGMLoaded[i] = false;
	}
}

void AudioManager::PlaySE(SEId id)
{
	PlaySEWithVolumeScale(id, 1.0f);
}

//指定フレームが経過したら停止する効果音を再生
void AudioManager::PlaySEForFrames(SEId id, int playFrames)
{
	if (playFrames <= 0)
		return;

	PlaySEWithVolumeScale(id, 1.0f, playFrames);
}

//音源と聞き手の距離に応じて音量を減衰させて効果音を再生
void AudioManager::PlaySEAt(SEId id, const Vector3& sourcePosition,
	const Vector3& listenerPosition, float fullVolumeDistance, float maxDistance)
{
	const Vector3 direction = sourcePosition - listenerPosition;
	const float distance = direction.length();
	if (maxDistance <= 0.0f || distance >= maxDistance)
		return;

	if (fullVolumeDistance < 0.0f)
		fullVolumeDistance = 0.0f;

	float volumeScale = 1.0f;
	if (distance > fullVolumeDistance && maxDistance > fullVolumeDistance)
	{
		const float fadeDistance = maxDistance - fullVolumeDistance;
		volumeScale = 1.0f - (distance - fullVolumeDistance) / fadeDistance;
	}

	PlaySEWithVolumeScale(id, volumeScale);
}

//効果音の基準音量に指定倍率を掛けて再生
void AudioManager::PlaySEWithVolumeScale(SEId id, float volumeScale, int playFrames)
{
	SEData& se = m_SE[ToIndex(id)];
	if (!se.Loaded)
		return;

	if (volumeScale <= 0.0f)
		return;
	if (volumeScale > 1.0f)
		volumeScale = 1.0f;

	//同じSEが短時間で続いても前の音を止めにくいようにチャンネルを順番に使う
	const int channelIndex = se.NextChannel;
	Audio& channel = se.Channels[channelIndex];
	channel.SetVolume(se.BaseVolume * volumeScale);
	channel.Play(false);
	se.RemainingFrames[channelIndex] = std::max(playFrames, 0);
	se.NextChannel = (se.NextChannel + 1) % SEChannelCount;
}

void AudioManager::UpdateSE()
{
	for (SEData& se : m_SE)
	{
		if (!se.Loaded)
			continue;

		for (int i = 0; i < SEChannelCount; ++i)
		{
			if (se.RemainingFrames[i] <= 0)
				continue;

			se.RemainingFrames[i]--;
			if (se.RemainingFrames[i] <= 0)
				se.Channels[i].Stop();
		}
	}
}

//各オブジェクトから受け取ったループ音量のうち最大値を次フレームへ反映
void AudioManager::RequestLoopSE(LoopSEId id, float volumeScale)
{
	LoopSEData& loopSE = m_LoopSE[ToIndex(id)];
	if (!loopSE.Loaded)
		return;

	volumeScale = std::max(0.0f, std::min(volumeScale, 1.0f));
	loopSE.RequestedVolumeScale = std::max(loopSE.RequestedVolumeScale, volumeScale);
}

//要求された音量へ近づけながらループ音の再生状態を更新
void AudioManager::UpdateLoopSE()
{
	for (LoopSEData& loopSE : m_LoopSE)
	{
		if (!loopSE.Loaded)
			continue;

		const float targetVolumeScale = loopSE.RequestedVolumeScale;
		loopSE.RequestedVolumeScale = 0.0f;

		if (!loopSE.Playing && targetVolumeScale > 0.0f)
		{
			loopSE.Channel.Play(true);
			loopSE.Playing = true;
		}

		loopSE.CurrentVolumeScale = MoveTowards(
			loopSE.CurrentVolumeScale, targetVolumeScale, LoopSEVolumeStep);
		loopSE.Channel.SetVolume(loopSE.BaseVolume * loopSE.CurrentVolumeScale);

		//音量が0になったら停止
		if (loopSE.Playing && targetVolumeScale <= 0.0f && loopSE.CurrentVolumeScale <= 0.0f)
		{
			loopSE.Channel.Stop();
			loopSE.Playing = false;
		}
	}
}

void AudioManager::PlayBGM(BGMId id, bool restart)
{
	const int index = ToIndex(id);
	if (!m_BGMLoaded[index])
		return;

	if (!restart && m_CurrentBGM == id)
		return;

	StopBGM();
	m_BGM[index].Play(true);
	m_CurrentBGM = id;
}

void AudioManager::StopBGM()
{
	if (m_CurrentBGM == BGMId::Max)
		return;

	const int index = ToIndex(m_CurrentBGM);
	if (m_BGMLoaded[index])
		m_BGM[index].Stop();

	m_CurrentBGM = BGMId::Max;
}
