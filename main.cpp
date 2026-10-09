//-----------------------------------------------------------
//  [ main.cpp ]
//  ウィンドウ生成とゲームのメインループを管理
//-----------------------------------------------------------
#include "main.h"
#include "input.h"
#include "manager.h"
#include "resource.h"
#include "imgui_impl_win32.h"

#include <chrono>


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	constexpr char ClassName[] = "AppClass";
	constexpr char WindowName[] = "こりじょんらん！";

	//ウィンドウ最大化、ドラッグでのサイズ変更不可
	constexpr DWORD WindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	using FrameClock = std::chrono::steady_clock; //長い型名をFrameClockという名前で使えるようにする
	constexpr double TargetFramesPerSecond = 60.0; //目標(最大)FPS
	const FrameClock::duration FrameDuration = std::chrono::duration_cast<FrameClock::duration>(
			std::chrono::duration<double>(1.0 / TargetFramesPerSecond));
	constexpr DWORD SleepMarginMs = 1; //スリープの行き過ぎを防ぐ余白

	HWND windowHandle = nullptr;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}

HWND GetWindow()
{
	return windowHandle;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX windowClass{};
	{
		windowClass.cbSize = sizeof(WNDCLASSEX);
		windowClass.style = 0;
		windowClass.lpfnWndProc = WndProc;
		windowClass.cbClsExtra = 0;
		windowClass.cbWndExtra = 0;
		windowClass.hInstance = hInstance;
		windowClass.hIcon = static_cast<HICON>(LoadImage(hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
			GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR | LR_SHARED));
		windowClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
		windowClass.hbrBackground = nullptr;
		windowClass.lpszMenuName = nullptr;
		windowClass.lpszClassName = ClassName;
		windowClass.hIconSm = static_cast<HICON>(LoadImage(
			hInstance, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR | LR_SHARED));

		RegisterClassEx(&windowClass);

		RECT windowRect{ 0, 0, static_cast<LONG>(SCREEN_WIDTH), static_cast<LONG>(SCREEN_HEIGHT) };
		AdjustWindowRect(&windowRect, WindowStyle, FALSE);

		windowHandle = CreateWindowEx(0, ClassName, WindowName, WindowStyle, CW_USEDEFAULT, CW_USEDEFAULT,
			windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
			nullptr, nullptr, hInstance, nullptr);
	}

	CoInitializeEx(nullptr, COINITBASE_MULTITHREADED);

	Manager::Init();

	ShowWindow(windowHandle, nCmdShow);
	UpdateWindow(windowHandle);

	timeBeginPeriod(1);
	auto nextFrameTime = FrameClock::now() + FrameDuration;

#ifdef _DEBUG
	int frameCount = 0;
	auto fpsLastTime = FrameClock::now();
#endif

	MSG msg{};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			const auto currentTime = FrameClock::now();

			if (currentTime >= nextFrameTime)
			{
				//大きく遅れた場合は連続更新せず、現在時刻から次のフレームを待つ
				if (currentTime - nextFrameTime >= FrameDuration)
					nextFrameTime = currentTime;
				nextFrameTime += FrameDuration;

				Manager::Update();
				Manager::Draw();

#ifdef _DEBUG
				frameCount++;
				const auto fpsCurrentTime = FrameClock::now();
				const double fpsElapsedSeconds = std::chrono::duration<double>(fpsCurrentTime - fpsLastTime).count();
				if (fpsElapsedSeconds >= 1.0)
				{
					const int framesPerSecond = static_cast<int>(static_cast<double>(frameCount) / fpsElapsedSeconds + 0.5);
					char windowTitle[128];
					sprintf_s(windowTitle, "%s - FPS: %d", WindowName, framesPerSecond);
					SetWindowText(windowHandle, windowTitle);

					frameCount = 0;
					fpsLastTime = fpsCurrentTime;
				}
#endif
			}
			else
			{
				const auto remainingTime = std::chrono::duration_cast<std::chrono::milliseconds>(
					nextFrameTime - currentTime);

				//最後の1msは時刻を確認し続け、SleepによるFPS低下を防ぐ
				if (remainingTime.count() > SleepMarginMs)
					Sleep(static_cast<DWORD>(remainingTime.count() - SleepMarginMs));
			}
		}
	}

	timeEndPeriod(1);

	UnregisterClass(ClassName, windowClass.hInstance);

	Manager::Uninit();

	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

namespace
{
	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		Input::InputMouse(uMsg, wParam, lParam);

		//ゲームプレイ中はImGuiより先にクライアント領域のカーソルを非表示にする
		if (uMsg == WM_SETCURSOR && LOWORD(lParam) == HTCLIENT &&
			!Input::IsCursorVisible())
		{
			SetCursor(nullptr);
			return true;
		}

		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) && uMsg != WM_INPUT)
			return true;

		switch (uMsg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			break;
		}

		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}
