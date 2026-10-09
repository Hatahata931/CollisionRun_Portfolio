//-----------------------------------------------------------
//  [ polygon2D.cpp ]
//  2D画像を簡単に表示するためのラッパー
//-----------------------------------------------------------
#include "main.h"
#include "polygon2D.h"
#include "polygonRendererComponent.h"

void Polygon2D::Init(float x, float y, float width, float height, const char* texturePath)
{
	GameObject::Init();

	auto renderer = AddComponent<PolygonRendererComponent>();
	renderer->InitScreenSpaceRect(x, y, width, height, texturePath); //左上原点
}