//-----------------------------------------------------------
//  [ life.h ]
//  プレイヤーのライフ
//-----------------------------------------------------------
#pragma once

#include <vector>
#include "gameObject.h"

class PolygonRendererComponent;

class Life : public GameObject
{
private:
	static constexpr int MaxValue = 5; //最大ライフ

	//ハートの大きさとスペース
	static constexpr float HeartWidth = 70.0f;
	static constexpr float HeartHeight = 70.0f;
	static constexpr float HeartSpacing = 8.0f;

	static constexpr const char* FilledTexture = "asset\\texture\\life.png";
	static constexpr const char* EmptyTexture = "asset\\texture\\life_empty.png";

	std::vector<PolygonRendererComponent*> m_HeartRenderers;
	int m_Value = 0;

	void RefreshHearts(); //ライフ表示更新
	void SetValue(int value);

public:
	void Init() override;

	void Add(int value);
	int GetValue() const { return m_Value; }
	int GetMaxValue() const { return MaxValue; }
};
