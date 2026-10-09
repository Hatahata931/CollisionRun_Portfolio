//-----------------------------------------------------------
//  [ imguiManager.h ]
//  ImGuiの初期化・フレーム処理・描画・終了を管理
//-----------------------------------------------------------
#pragma once

class ImGuiManager
{
public:
	static void Init();
	static void BeginFrame();
	static void Draw();
	static void Uninit();
	
	static bool WantsMouseCapture(); //ImGuiがマウス入力を使用しているか確認
	static bool WantsKeyboardCapture(); //ImGuiがキーボード入力を使用しているか確認
};
