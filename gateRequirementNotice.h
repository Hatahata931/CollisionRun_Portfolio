//-----------------------------------------------------------
//  [ gateRequirementNotice.h ]
//  Gate接近時に不足しているStar数を表示するUI
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class NumberRendererComponent;
class PolygonRendererComponent;

class GateRequirementNotice : public GameObject
{
private:
	static constexpr int NoRemainingCount = -1; //表示対象のGateがない状態

	//数字スプライトの分割数と表示桁数
	static constexpr int SpriteColumns = 5;
	static constexpr int SpriteRows = 5;
	static constexpr int DisplayDigits = 3;

	//数字の大きさとスペース
	static constexpr float DigitWidth = 60.0f;
	static constexpr float DigitHeight = 60.0f;
	static constexpr float DigitSpacing = 42.0f;

	static constexpr float IconSize = 70.0f;

	//「あと」の文字
	static constexpr float PrefixWidth = 69.2f;
	static constexpr float PrefixHeight = 36.4f;

	//「個必要です」の文字
	static constexpr float SuffixWidth = 216.0f;
	static constexpr float SuffixHeight = 45.0f;

	static constexpr float ElementSpacing = 2.0f; //各要素の間のスペース

	//Starアイコン、残り数、文字画像の描画をそれぞれ担当
	PolygonRendererComponent* m_PrefixRenderer = nullptr;
	PolygonRendererComponent* m_IconRenderer = nullptr;
	NumberRendererComponent* m_NumberRenderer = nullptr;
	PolygonRendererComponent* m_SuffixRenderer = nullptr;

	bool m_IsVisible = false;

	//表示範囲内にある最寄りの不足中Gateから残りのStar数を取得
	int FindNearestRemainingCount() const;
	//通知を構成する要素の表示状態をまとめて切り替え
	void SetVisible(bool visible);

public:
	void Init() override;
	void Update() override;
};
