//-----------------------------------------------------------
//  [ manager.cpp ]
//  ゲーム全体の初期化・更新・描画とシーン遷移を管理
//-----------------------------------------------------------
#include "main.h"
#include "manager.h"

#include "imgui.h"

#include "audio.h"
#include "audioManager.h"
#include "debugScene.h"
#include "gameOver.h"
#include "imguiManager.h"
#include "input.h"
#include "pauseManager.h"
#include "player.h"
#include "renderer.h"
#include "scene.h"
#include "stage1.h"
#include "stage2.h"
#include "stage3.h"
#include "stageEditor.h"
#include "stageId.h"
#include "stageSelect.h"
#include "texture.h"
#include "title.h"

Scene* Manager::m_Scene = nullptr;
Scene* Manager::m_SceneNext = nullptr;
Manager::SceneTransitionState Manager::m_TransitionState = Manager::SceneTransitionState::None;
float Manager::m_FadeAlpha = 0.0f;

void Manager::Init()
{
	Renderer::Init();
	ImGuiManager::Init();
	Input::Init();
	Audio::InitMaster();
	AudioManager::Init();
	PauseManager::Init();

	m_Scene = new Title();

	m_Scene->Init();

	//フェードインからスタート
	m_TransitionState = SceneTransitionState::FadeIn;
	m_FadeAlpha = 1.0f;
}

void Manager::Uninit()
{
	if (m_Scene)
	{
		m_Scene->Uninit();
		delete m_Scene;
		m_Scene = nullptr;
	}

	//フェードアウト中に終了した場合、未初期化の次シーンも破棄する
	if (m_SceneNext)
	{
		//Initが呼ばれていないためUninitは呼ばない
		delete m_SceneNext;
		m_SceneNext = nullptr;
	}

	AudioManager::Uninit();
	Audio::UninitMaster();
	Input::Uninit();
	ImGuiManager::Uninit();
	Texture::UnloadAll();
	Renderer::Uninit();
}

void Manager::Update()
{
	Input::Update();
	ImGuiManager::BeginFrame();
	AudioManager::UpdateSE();
	AudioManager::UpdateLoopSE();
	UpdateCursorVisibility();

	//フェード中は入力状態だけ更新し、シーン・ポーズ・エディターを停止
	if (IsTransitioning())
	{
		//ポーズ画面からの遷移では、停止中のシーンの見た目もそのまま維持
		const bool freezePausedScene =
			m_TransitionState == SceneTransitionState::FadeOut && PauseManager::IsPaused();
		//完全な黒を表示した次のフレームは、旧シーンを更新せず読み込みへ進む
		const bool switchScene = m_TransitionState == SceneTransitionState::SwitchScene;
		if (!freezePausedScene && !switchScene && m_Scene)
			m_Scene->UpdateTransitionObjects();

		UpdateTransition(); //フェードの更新
		return;
	}

	const PauseAction pauseAction = PauseManager::Update(m_Scene && m_Scene->CanPause());
	//ポーズ画面からステージセレクトまたはタイトルに戻る
	if (pauseAction == PauseAction::ReturnStageSelect)
	{
		SetScene<StageSelect>();
		UpdateTransition();
		return;
	}
	if (pauseAction == PauseAction::ReturnTitle)
	{
		SetScene<Title>();
		UpdateTransition();
		return;
	}

#ifdef _DEBUG
	//エディターUIと配置データの操作をシーン更新前に処理
	StageEditor::Update(m_Scene);
#endif

	//ポーズとエディターの切り替えをカーソル表示へ同じフレームで反映
	UpdateCursorVisibility();

	//入力とデバッグUIは動かしたままゲーム内オブジェクトの更新だけを停止
	if (!PauseManager::IsPaused())
		m_Scene->Update();

	//シーン更新中に遷移が始まった場合、以降の操作を行わずフェードを開始
	if (IsTransitioning())
	{
		UpdateTransition();
		return;
	}

#ifdef _DEBUG
	//カメラ更新後の行列を使ってギズモ操作とクリック選択を処理
	StageEditor::UpdateSceneInteraction(m_Scene);
#endif
}

void Manager::Draw()
{
	Renderer::Begin();
	
	m_Scene->Draw();
	PauseManager::Draw();
	DrawTransition();
	ImGuiManager::Draw();

	Renderer::End();
}

//識別子で指定したゲームステージへ遷移
void Manager::SetStage(StageId stageId)
{
	switch (stageId)
	{
	case StageId::Stage1:
		SetScene<Stage1>();
		break;
	case StageId::Stage2:
		SetScene<Stage2>();
		break;
	case StageId::Stage3:
		SetScene<Stage3>();
		break;
	}
}

//現在の画面と操作状態に合わせてカーソル表示を切り替える
void Manager::UpdateCursorVisibility()
{
	//フェードアウト中は遷移元のカーソル表示をシーン切り替えまで維持
	if (m_TransitionState == SceneTransitionState::FadeOut ||
		m_TransitionState == SceneTransitionState::SwitchScene)
		return;

	bool shouldShowCursor = true;
	const bool hasPlayer = m_Scene && m_Scene->GetGameObject<Player>() != nullptr;

	if (hasPlayer && !PauseManager::IsPaused())
	{
#ifdef _DEBUG
		shouldShowCursor = StageEditor::IsOpen();
#else
		shouldShowCursor = false;
#endif
	}

	Input::SetCursorVisible(shouldShowCursor);
}

//シーン遷移中か確認
bool Manager::IsTransitioning()
{
	return m_TransitionState != SceneTransitionState::None;
}

//フェード演出の状態と透明度を更新
void Manager::UpdateTransition()
{
	const float fadeStep = 1.0f / static_cast<float>(FadeDurationFrames);

	if (m_TransitionState == SceneTransitionState::FadeOut)
	{
		m_FadeAlpha += fadeStep;
		if (m_FadeAlpha >= 1.0f)
		{
			m_FadeAlpha = 1.0f;
			//完全な黒を一度描画してから、次の更新でシーンを読み込む
			m_TransitionState = SceneTransitionState::SwitchScene;
		}
	}
	else if (m_TransitionState == SceneTransitionState::SwitchScene)
		ChangeScene();
	else if (m_TransitionState == SceneTransitionState::FadeIn)
	{
		m_FadeAlpha -= fadeStep;
		if (m_FadeAlpha <= 0.0f)
		{
			m_FadeAlpha = 0.0f;
			m_TransitionState = SceneTransitionState::None;
			if (m_Scene)
				m_Scene->OnFadeInFinished();
		}
	}
}

void Manager::ChangeScene()
{
	if (!m_SceneNext)
		return;

	m_Scene->Uninit();
	delete m_Scene;

	m_Scene = m_SceneNext;
	m_SceneNext = nullptr;
	PauseManager::Reset();

	//新しいシーンを黒画面の裏で初期化し、次フレームから表示
	m_TransitionState = SceneTransitionState::FadeIn;
	m_FadeAlpha = 1.0f;
	m_Scene->Init();
}

//フェード用の黒い幕を描画
void Manager::DrawTransition()
{
	if (!IsTransitioning() || m_FadeAlpha <= 0.0f)
		return;

	//ImGuiを使って画面全体を覆う黒を表示
	const ImGuiIO& io = ImGui::GetIO();
	ImDrawList* drawList = ImGui::GetForegroundDrawList();
	drawList->AddRectFilled(ImVec2(0.0f, 0.0f), io.DisplaySize,
		ImGui::GetColorU32(ImVec4(0.0f, 0.0f, 0.0f, m_FadeAlpha)));
}
