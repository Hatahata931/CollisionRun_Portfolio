//-----------------------------------------------------------
//  [ gateRequirementDisplay.cpp ]
//  次のGateに必要なStarの数を表示するUI
//-----------------------------------------------------------
#include "main.h"
#include <limits>
#include <vector>
#include "gateRequirementDisplay.h"
#include "gate.h"
#include "manager.h"
#include "numberRendererComponent.h"
#include "polygonRendererComponent.h"
#include "scene.h"

void GateRequirementDisplay::Init()
{
	GameObject::Init();

	m_IconRenderer = AddComponent<PolygonRendererComponent>();
	//Gateが残っているときだけ、アイコンと必要数をセットで表示
	m_IconRenderer->InitScreenSpaceRect(0.0f, IconOffsetY, IconSize, IconSize, "asset\\texture\\gate_icon.png");
	m_IconRenderer->SetVisible(false);

	m_NumberRenderer = AddComponent<NumberRendererComponent>();
	m_NumberRenderer->Init("asset\\texture\\number.png", SpriteColumns, SpriteRows);
	m_NumberRenderer->SetSpriteSize(DigitWidth, DigitHeight);
	m_NumberRenderer->SetSpacing(DigitSpacing);
	m_NumberRenderer->SetOffset(NumberOffsetX, 0.0f);
	//StarCounterと見た目を揃えるため、必要数も3桁固定で表示する
	m_NumberRenderer->SetMinimumDigits(DisplayDigits);
	m_NumberRenderer->SetVisible(false);
}

void GateRequirementDisplay::Update()
{
	GameObject::Update();
	UpdateDisplay();
}

//フェード中もGateの必要数表示を更新
void GateRequirementDisplay::TransitionUpdate()
{
	UpdateDisplay();
}

//残っているGateに合わせてアイコンと必要数の表示を更新
void GateRequirementDisplay::UpdateDisplay()
{
	const int requiredCount = FindNextRequiredCount();
	//残っているGateがなければ表示しない
	if (requiredCount == NoRequiredCount)
	{
		m_NumberRenderer->SetVisible(false);
		m_IconRenderer->SetVisible(false);
		return;
	}

	m_NumberRenderer->SetValue(requiredCount);
	m_NumberRenderer->SetVisible(true);
	m_IconRenderer->SetVisible(true);
}

//残っているGateのうち最小の必要数を次の目標として取得
int GateRequirementDisplay::FindNextRequiredCount() const
{
	Scene* scene = Manager::GetScene();
	if (!scene)
		return NoRequiredCount;

	int nextRequiredCount = std::numeric_limits<int>::max();
	const auto gates = scene->GetGameObjects<Gate>();

	for (const Gate* gate : gates)
	{
		//今のフレームで開いたGateは、シーンから削除される前でも候補から外す
		if (gate->IsDestroyRequested())
			continue;

		const int requiredCount = gate->GetRequiredItemCount();
		if (requiredCount < nextRequiredCount)
			nextRequiredCount = requiredCount;
	}

	if (nextRequiredCount == std::numeric_limits<int>::max())
		return NoRequiredCount;

	return nextRequiredCount;
}
