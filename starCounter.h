//-----------------------------------------------------------
//  [ starCounter.h ]
//  取得したStar数を表示するUI
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class NumberRendererComponent;

class StarCounter : public GameObject
{
private:
	//数字スプライトの分割設定
	static constexpr int SpriteColumns = 5; //列数
	static constexpr int SpriteRows = 5; //行数
	static constexpr int SlashSpriteIndex = 10; //「/」のスプライト番号

	//大きさ・桁数
	static constexpr int DisplayDigits = 3;
	static constexpr float DigitWidth = 50.0f;
	static constexpr float DigitHeight = 50.0f;
	static constexpr float DigitSpacing = 35.0f; //数字を並べる間隔

	//Starアイコンと数字の配置
	static constexpr float IconSize = 60.0f;
	static constexpr float IconOffsetY = -5.0f; //数字に合わせるためのアイコンのY位置調整
	static constexpr float NumberOffsetX = 56.0f; //アイコンから数字をずらす距離

	//通常時と全取得時の数字色
	static const XMFLOAT4 NormalNumberColor;
	static const XMFLOAT4 CompleteNumberColor;

	NumberRendererComponent* m_NumberRenderer = nullptr;
	int m_Value = 0;
	int m_MaxValue = 0;
	bool m_IsComplete = false;

	//取得数と総数を数字描画コンポーネントへ反映
	void RefreshDisplay();
	//全取得状態と数字の色を更新し、必要な場合だけ達成SEを再生
	void UpdateCompletionState(bool playCompletionSound);

public:
	void Init() override;

	void Add(int value);
	int GetValue() const { return m_Value; }
	void SetValue(int value);
	void SetMaxValue(int value);
	int GetMaxValue() const { return m_MaxValue; }
};
