//-----------------------------------------------------------
//  [ audio.cpp ]
//  WAV音声の読み込み・再生を管理
//-----------------------------------------------------------
#include "main.h"
#include "audio.h"

IXAudio2* Audio::m_Xaudio = nullptr;
IXAudio2MasteringVoice* Audio::m_MasteringVoice = nullptr;

//XAudio2全体を初期化
void Audio::InitMaster()
{
	//COM初期化
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	//XAudio生成
	XAudio2Create(&m_Xaudio, 0);

	//マスタリングボイス生成
	m_Xaudio->CreateMasteringVoice(&m_MasteringVoice);
}

//XAudio2全体を終了
void Audio::UninitMaster()
{
	if (m_MasteringVoice)
	{
		m_MasteringVoice->DestroyVoice();
		m_MasteringVoice = nullptr;
	}

	if (m_Xaudio)
	{
		m_Xaudio->Release();
		m_Xaudio = nullptr;
	}

	CoUninitialize();
}

void Audio::Load(const char* fileName)
{
	//サウンドデータ読込
	WAVEFORMATEX wfx{};

	{
		HMMIO hmmio = nullptr;
		MMIOINFO mmioinfo{};
		MMCKINFO riffchunkinfo{};
		MMCKINFO datachunkinfo{};
		MMCKINFO mmckinfo{};
		UINT32 buflen;
		LONG readlen;

		hmmio = mmioOpen((LPSTR)fileName, &mmioinfo, MMIO_READ);
		assert(hmmio);

		riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		mmioDescend(hmmio, &riffchunkinfo, nullptr, MMIO_FINDRIFF);

		mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
		mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		if (mmckinfo.cksize >= sizeof(WAVEFORMATEX))
		{
			mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx));
		}
		else
		{
			PCMWAVEFORMAT pcmwf = { 0 };
			mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf));
			memset(&wfx, 0x00, sizeof(wfx));
			memcpy(&wfx, &pcmwf, sizeof(pcmwf));
			wfx.cbSize = 0;
		}
		mmioAscend(hmmio, &mmckinfo, 0);

		datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
		mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK);

		buflen = datachunkinfo.cksize;
		m_SoundData = new unsigned char[buflen];
		readlen = mmioRead(hmmio, (HPSTR)m_SoundData, buflen);

		m_Length = readlen;
		m_PlayLength = readlen / wfx.nBlockAlign;

		mmioClose(hmmio, 0);
	}

	//サウンドソース生成
	m_Xaudio->CreateSourceVoice(&m_SourceVoice, &wfx);
	assert(m_SourceVoice);
}

void Audio::Uninit()
{
	Stop();

	if (m_SourceVoice)
	{
		m_SourceVoice->DestroyVoice();
		m_SourceVoice = nullptr;
	}

	delete[] m_SoundData;
	m_SoundData = nullptr;
	m_Length = 0;
	m_PlayLength = 0;
}

void Audio::Play(bool loop)
{
	if (!m_SourceVoice || !m_SoundData)
		return;

	Stop();

	//バッファ設定
	XAUDIO2_BUFFER buffer{};
	buffer.AudioBytes = m_Length;
	buffer.pAudioData = m_SoundData;
	buffer.PlayBegin = 0;
	buffer.PlayLength = m_PlayLength;

	//ループ設定
	if (loop)
	{
		buffer.LoopBegin = 0;
		buffer.LoopLength = m_PlayLength;
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	m_SourceVoice->SubmitSourceBuffer(&buffer, nullptr);

	//再生
	m_SourceVoice->Start();
}

void Audio::Stop()
{
	if (!m_SourceVoice)
		return;

	m_SourceVoice->Stop();
	m_SourceVoice->FlushSourceBuffers(); //再生待ちになっている音データをクリア
}

void Audio::SetVolume(float volume)
{
	if (!m_SourceVoice)
		return;

	m_SourceVoice->SetVolume(volume);
}
