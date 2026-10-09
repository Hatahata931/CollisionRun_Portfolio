//-----------------------------------------------------------
//  [ imguiManager.cpp ]
//  ImGuiの初期化・フレーム処理・描画・終了を管理
//-----------------------------------------------------------
#include "main.h"
#include "renderer.h"
#include "imguiManager.h"

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "external/imguizmo/ImGuizmo.h"

void ImGuiManager::Init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	ImGui::StyleColorsDark();

	//バックエンド(ImGuiとOS・描画APIをつなぐ接続部分)の初期化
	ImGui_ImplWin32_Init(GetWindow());
	ImGui_ImplDX11_Init(Renderer::GetDevice(), Renderer::GetDeviceContext());
}

void ImGuiManager::BeginFrame()
{
	//新しいフレームの準備
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#ifdef _DEBUG
	//Debug用ステージエディターで使うギズモのフレームを開始
	ImGuizmo::BeginFrame();
#endif
}

void ImGuiManager::Draw()
{
	//構築したUIを描画データへ変換し、DirectX11バックエンドへ渡す
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiManager::Uninit()
{
	//バックエンドを終了し、コンテキストを破棄
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

//ImGuiがマウス入力を使用しているか確認
bool ImGuiManager::WantsMouseCapture()
{
	return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureMouse;
}

//ImGuiがキーボード入力を使用しているか確認
bool ImGuiManager::WantsKeyboardCapture()
{
	return ImGui::GetCurrentContext() != nullptr && ImGui::GetIO().WantCaptureKeyboard;
}
