//-----------------------------------------------------------
//  [ polygon2D.h ]
//  2D画像を簡単に表示するためのラッパー
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Polygon2D : public GameObject
{
public:
	void Init() override {}
	void Init(float x, float y, float width, float height, const char* texturePath);
};