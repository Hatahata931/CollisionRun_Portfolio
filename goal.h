//-----------------------------------------------------------
//  [ goal.h ]
//  乗るとクリア画面に移動するゴール
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Player;

class Goal : public GameObject
{
private:
	static constexpr float LandingHeightTolerance = 0.25f; //上面とプレイヤーの足元の許容差
	static constexpr int ClearDelayFrames = 110; //クリア画面へ遷移するまでの待機時間

	//クリア文字の表示位置と大きさ
	static constexpr float ClearTextPositionX = 640.0f;
	static constexpr float ClearTextPositionY = 200.0f;
	static constexpr float ClearTextWidth = 600.0f;
	static constexpr float ClearTextHeight = 121.21212f;

	bool m_IsClearSequenceActive = false;
	int m_ClearSequenceTimer = 0;

	//クリア画面へ渡すStar数
	int m_CollectedStars = 0;
	int m_TotalStars = 0;

	//プレイヤーが乗っているか判定
	bool IsPlayerOnGoal(Player* player);
	//クリア演出を開始
	void StartClearSequence(Player* player);
	//クリア演出を進めて遷移
	void UpdateClearSequence();

public:
	void Init() override;
	void Update() override;
};
