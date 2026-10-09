//-----------------------------------------------------------
//  [ input.h ]
//  キーボード・マウス入力とカーソル表示を管理
//-----------------------------------------------------------
#pragma once

class Input
{
private:
	static BYTE m_OldKeyState[256];
	static BYTE m_KeyState[256];

	static LONG m_MouseDeltaX;
	static LONG m_MouseDeltaY;
	static int m_MouseWheelDelta;

	//WndProcから非同期に届く入力を次のUpdateまで蓄積
	static LONG m_PendingMouseDeltaX;
	static LONG m_PendingMouseDeltaY;
	static int m_PendingMouseWheelDelta; //次のUpdateまで蓄積するホイール回転量

	static int m_MouseX;
	static int m_MouseY;
	static bool m_IsCursorVisible;

public:
	static void Init();
	static void Uninit();
	static void Update();

	static void InputMouse(UINT message, WPARAM wParam, LPARAM lParam);
	static void SetCursorVisible(bool visible); //ゲーム画面内のマウスカーソル表示切り替え
	static bool IsCursorVisible() { return m_IsCursorVisible; }

	static bool GetKeyPress(BYTE keyCode);
	static bool GetKeyTrigger(BYTE keyCode);
	static LONG GetMouseDeltaX() { return m_MouseDeltaX; }
	static LONG GetMouseDeltaY() { return m_MouseDeltaY; }
	static int GetMouseWheelDelta() { return m_MouseWheelDelta; }
	static int GetMouseX() { return m_MouseX; }
	static int GetMouseY() { return m_MouseY; }
};
