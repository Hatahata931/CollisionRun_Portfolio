//-----------------------------------------------------------
//  [ gateRequirementDisplay.h ]
//  次のGateに必要なStarの数を表示するUI
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class NumberRendererComponent;
class PolygonRendererComponent;

class GateRequirementDisplay : public GameObject
{
private:
	static constexpr int NoRequiredCount = -1; //表示対象のGateがない状態

	//数字スプライトの分割数と表示桁数
	static constexpr int SpriteColumns = 5;
	static constexpr int SpriteRows = 5;
	static constexpr int DisplayDigits = 3;

	//数字とアイコンの表示サイズ・配置
	static constexpr float DigitWidth = 50.0f;
	static constexpr float DigitHeight = 50.0f;
	static constexpr float DigitSpacing = 35.0f;
	static constexpr float IconSize = 60.0f;
	static constexpr float IconOffsetY = -5.0f;
	static constexpr float NumberOffsetX = 56.0f;

	PolygonRendererComponent* m_IconRenderer = nullptr;
	NumberRendererComponent* m_NumberRenderer = nullptr;

	//残っているGateのうち最小の必要数を次の目標として取得
	int FindNextRequiredCount() const;
	//残っているGateに合わせてアイコンと必要数の表示を更新
	void UpdateDisplay();

public:
	void Init() override;
	void Update() override;
	//フェード中もGateの必要数表示を更新
	void TransitionUpdate() override;
};
