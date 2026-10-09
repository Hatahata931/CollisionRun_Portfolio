//-----------------------------------------------------------
//  [ input.cpp ]
//  キーボード・マウス入力とカーソル表示を管理
//-----------------------------------------------------------
#include "main.h"
#include "input.h"

BYTE Input::m_OldKeyState[256];
BYTE Input::m_KeyState[256];
LONG Input::m_MouseDeltaX = 0;
LONG Input::m_MouseDeltaY = 0;
LONG Input::m_PendingMouseDeltaX = 0;
LONG Input::m_PendingMouseDeltaY = 0;
int Input::m_MouseWheelDelta = 0;
int Input::m_PendingMouseWheelDelta = 0;
int Input::m_MouseX = 0;
int Input::m_MouseY = 0;
bool Input::m_IsCursorVisible = true;

void Input::Init()
{
	memset(m_OldKeyState, 0, sizeof(m_OldKeyState));
	memset(m_KeyState, 0, sizeof(m_KeyState));
	m_MouseDeltaX = 0;
	m_MouseDeltaY = 0;
	m_PendingMouseDeltaX = 0;
	m_PendingMouseDeltaY = 0;
	m_MouseWheelDelta = 0;
	m_PendingMouseWheelDelta = 0;
	m_MouseX = 0;
	m_MouseY = 0;
	m_IsCursorVisible = true;

	RAWINPUTDEVICE mouseDevice{};
	mouseDevice.usUsagePage = 0x01;
	mouseDevice.usUsage = 0x02;
	mouseDevice.dwFlags = 0;
	mouseDevice.hwndTarget = GetWindow();
	RegisterRawInputDevices(&mouseDevice, 1, sizeof(mouseDevice));
}

void Input::Uninit()
{
	SetCursorVisible(true);

	RAWINPUTDEVICE mouseDevice{};
	mouseDevice.usUsagePage = 0x01;
	mouseDevice.usUsage = 0x02;
	mouseDevice.dwFlags = RIDEV_REMOVE;
	mouseDevice.hwndTarget = nullptr;
	RegisterRawInputDevices(&mouseDevice, 1, sizeof(mouseDevice));
}

void Input::Update()
{
	memcpy(m_OldKeyState, m_KeyState, sizeof(m_OldKeyState));

	GetKeyboardState(m_KeyState);

	//WndProcが前フレームから蓄積した値を今回の入力として確定する
	m_MouseDeltaX = m_PendingMouseDeltaX;
	m_MouseDeltaY = m_PendingMouseDeltaY;
	m_MouseWheelDelta = m_PendingMouseWheelDelta;
	m_PendingMouseDeltaX = 0;
	m_PendingMouseDeltaY = 0;
	m_PendingMouseWheelDelta = 0;

	//マウス座標取得(ウィンドウ左上基準)
	POINT mousePosition{};
	if (GetCursorPos(&mousePosition) && ScreenToClient(GetWindow(), &mousePosition))
	{
		m_MouseX = mousePosition.x;
		m_MouseY = mousePosition.y;
	}
}

void Input::InputMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
	//マウス移動
	if (message == WM_INPUT)
	{
		RAWINPUT rawInput{};
		UINT dataSize = sizeof(rawInput);
		const UINT readSize = GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam),
			RID_INPUT, &rawInput, &dataSize, sizeof(RAWINPUTHEADER));

		if (readSize == dataSize && rawInput.header.dwType == RIM_TYPEMOUSE &&
			(rawInput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE) == 0)
		{
			m_PendingMouseDeltaX += rawInput.data.mouse.lLastX;
			m_PendingMouseDeltaY += rawInput.data.mouse.lLastY;
		}
	}
	//マウスホイール
	else if (message == WM_MOUSEWHEEL)
		m_PendingMouseWheelDelta += GET_WHEEL_DELTA_WPARAM(wParam);
}

//ゲーム画面内のマウスカーソル表示切り替え
void Input::SetCursorVisible(bool visible)
{
	//非表示中はImGuiがカーソルを戻しても再び隠せるよう毎フレーム反映する
	if (m_IsCursorVisible == visible && visible)
		return;

	m_IsCursorVisible = visible;

	//カーソルがゲーム画面内にある場合は切り替えをすぐに反映する
	POINT cursorPosition{};
	RECT clientRect{};
	if (GetCursorPos(&cursorPosition) && ScreenToClient(GetWindow(), &cursorPosition) &&
		GetClientRect(GetWindow(), &clientRect) && PtInRect(&clientRect, cursorPosition))
		SetCursor(visible ? LoadCursor(nullptr, IDC_ARROW) : nullptr);
}

bool Input::GetKeyPress(BYTE keyCode)
{
	return (m_KeyState[keyCode] & 0x80) != 0;
}

bool Input::GetKeyTrigger(BYTE keyCode)
{
	return (m_KeyState[keyCode] & 0x80) != 0 && (m_OldKeyState[keyCode] & 0x80) == 0;
}
