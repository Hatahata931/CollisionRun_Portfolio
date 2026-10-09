//-----------------------------------------------------------
//  [ manager.h ]
//  ゲーム全体の初期化・更新・描画とシーン遷移を管理
//-----------------------------------------------------------
#pragma once

#include <utility>

class Scene;
enum class StageId;

class Manager
{
private:
	//シーン遷移の状態(フェード演出用)
	enum class SceneTransitionState
	{
		None,
		FadeOut,
		SwitchScene, //完全な黒を描画してからシーンを切り替える状態
		FadeIn,
	};

	static constexpr int FadeDurationFrames = 20; //フェードイン・アウトそれぞれにかけるフレーム数

	static Scene* m_Scene;
	static Scene* m_SceneNext;
	static SceneTransitionState m_TransitionState;
	static float m_FadeAlpha;

	//現在の画面と操作状態に合わせてカーソル表示を切り替える
	static void UpdateCursorVisibility();
	//シーン遷移中か確認
	static bool IsTransitioning();
	//フェード演出の状態と透明度を更新
	static void UpdateTransition();
	static void ChangeScene();
	//フェード用の黒い幕を描画
	static void DrawTransition();

public:
	static void Init();
	static void Uninit();
	static void Update();
	static void Draw();
	//識別子で指定したゲームステージへ遷移
	static void SetStage(StageId stageId);

	static Scene* GetScene() { return m_Scene; }

	//次のシーンのコンストラクタへ必要なデータを渡して生成
	template <typename T, typename... Args>
	static void SetScene(Args&&... args)
	{
		//遷移中の連続入力による次シーンの上書きを防ぐ
		if (IsTransitioning() || m_SceneNext != nullptr)
			return;

		m_SceneNext = new T(std::forward<Args>(args)...);
		m_TransitionState = SceneTransitionState::FadeOut;
		m_FadeAlpha = 0.0f;
	}
};
