//-----------------------------------------------------------
//  [ audio.h ]
//  WAV音声の読み込み・再生を管理
//-----------------------------------------------------------
#pragma once

#include <xaudio2.h>

class Audio
{
private:
	static IXAudio2* m_Xaudio;
	static IXAudio2MasteringVoice* m_MasteringVoice;

	IXAudio2SourceVoice* m_SourceVoice{};
	BYTE* m_SoundData{};
	int m_Length{};
	int m_PlayLength{};

public:
	//XAudio2全体を初期化
	static void InitMaster();
	//XAudio2全体を終了
	static void UninitMaster();

	void Uninit();
	void Load(const char* fileName);
	void Play(bool loop = false);
	void Stop();
	void SetVolume(float volume);
};

