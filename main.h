//-----------------------------------------------------------
//  [ main.h ]
//  アプリケーション全体で使用する共通定義
//-----------------------------------------------------------
#pragma once


#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <functional>
#include "vector3.h"

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <DirectXMath.h>
using namespace DirectX;

#include "DirectXTex.h" //関数のプロトタイプ宣言が入ってる

#if _DEBUG
#pragma comment(lib, "DirectXTex_Debug.lib") //関数をコンパイルしたものが入ってる(デバッグ用)
#else
#pragma comment(lib, "DirectXTex_Release.lib") //リリース用
#endif

#pragma comment(lib, "winmm.lib")

#define SCREEN_WIDTH	(1280)
#define SCREEN_HEIGHT	(720)

HWND GetWindow();
