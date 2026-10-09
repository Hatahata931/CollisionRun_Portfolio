//-----------------------------------------------------------
//  [ starCounter.cpp ]
//  取得したStar数を表示するUI
//-----------------------------------------------------------
#include "main.h"
#include "starCounter.h"
#include "audioManager.h"
#include "numberRendererComponent.h"
#include "polygonRendererComponent.h"

const XMFLOAT4 StarCounter::NormalNumberColor{ 1.0f, 1.0f, 1.0f, 1.0f };
const XMFLOAT4 StarCounter::CompleteNumberColor{ 1.0f, 0.78f, 0.25f, 1.0f };

void StarCounter::Init()
{
	GameObject::Init();

	auto* iconRenderer = AddComponent<PolygonRendererComponent>();
	//アイコンを左に置き、数字は右へずらして表示
	iconRenderer->InitScreenSpaceRect(0.0f, IconOffsetY, IconSize, IconSize, "asset\\texture\\star_icon.png");

	m_NumberRenderer = AddComponent<NumberRendererComponent>();
	m_NumberRenderer->Init("asset\\texture\\number.png", SpriteColumns, SpriteRows);
	m_NumberRenderer->SetSpriteSize(DigitWidth, DigitHeight);
	m_NumberRenderer->SetSpacing(DigitSpacing);
	m_NumberRenderer->SetOffset(NumberOffsetX, 0.0f);
	m_NumberRenderer->SetMinimumDigits(DisplayDigits); //3桁固定表示

	RefreshDisplay();
	UpdateCompletionState(false);
}

void StarCounter::Add(int value)
{
	m_Value += value;
	RefreshDisplay();
	UpdateCompletionState(true);
}

void StarCounter::SetValue(int value)
{
	m_Value = value;
	RefreshDisplay();
	UpdateCompletionState(false);
}

void StarCounter::SetMaxValue(int value)
{
	m_MaxValue = value;
	RefreshDisplay();
	UpdateCompletionState(false);
}

//取得数と総数を数字描画コンポーネントへ反映
void StarCounter::RefreshDisplay()
{
	if (!m_NumberRenderer)
		return;

	//総数が設定されている場合のみ「取得数 / 総数」で表示
	if (m_MaxValue > 0)
		m_NumberRenderer->SetPairValues(m_Value,m_MaxValue, SlashSpriteIndex);
	else
		m_NumberRenderer->SetValue(m_Value);
}

//全取得状態と数字の色を更新し、必要な場合だけ達成SEを再生
void StarCounter::UpdateCompletionState(bool playCompletionSound)
{
	const bool wasComplete = m_IsComplete;

	//Starが1つもないステージは全取得として扱わない
	m_IsComplete = m_MaxValue > 0 && m_Value >= m_MaxValue;

	//全取得したら数字の色を変える
	if (m_NumberRenderer)
		m_NumberRenderer->SetColor(m_IsComplete ? CompleteNumberColor : NormalNumberColor);

	if (!playCompletionSound || wasComplete || !m_IsComplete)
		return;

	AudioManager::PlaySE(SEId::StarComplete);
}
