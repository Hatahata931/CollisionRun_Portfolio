//-----------------------------------------------------------
//  [ gateRequirementNotice.cpp ]
//  Gate接近時に不足しているStar数を表示するUI
//-----------------------------------------------------------
#include "main.h"
#include <limits>
#include <vector>
#include "gateRequirementNotice.h"
#include "audioManager.h"
#include "gate.h"
#include "manager.h"
#include "numberRendererComponent.h"
#include "player.h"
#include "polygonRendererComponent.h"
#include "scene.h"
#include "starCounter.h"

void GateRequirementNotice::Init()
{
	GameObject::Init();

	//数字(3桁)全体の横幅
	const float numberDisplayWidth = DigitWidth + DigitSpacing * (DisplayDigits - 1);
	//全体の横幅
	const float totalWidth = PrefixWidth + IconSize +
		numberDisplayWidth + SuffixWidth + ElementSpacing * 3.0f;
	const float prefixY = (IconSize - PrefixHeight) * 0.5f;
	const float numberY = (IconSize - DigitHeight) * 0.5f;
	const float suffixY = (IconSize - SuffixHeight) * 0.5f;

	//通知全体がGameObjectの原点を中心に並ぶよう、左端から順に配置
	float cursorX = -totalWidth * 0.5f;

	//「あと」の文字
	m_PrefixRenderer = AddComponent<PolygonRendererComponent>();
	m_PrefixRenderer->InitScreenSpaceRect(cursorX, prefixY,
		PrefixWidth, PrefixHeight, "asset\\texture\\text_ato.png");
	cursorX += PrefixWidth + ElementSpacing;

	//Starアイコン
	m_IconRenderer = AddComponent<PolygonRendererComponent>();
	m_IconRenderer->InitScreenSpaceRect(cursorX, 0.0f,
		IconSize, IconSize, "asset\\texture\\star_icon.png");
	cursorX += IconSize + ElementSpacing;

	//Star数
	m_NumberRenderer = AddComponent<NumberRendererComponent>();
	m_NumberRenderer->Init("asset\\texture\\number.png", SpriteColumns, SpriteRows);
	m_NumberRenderer->SetSpriteSize(DigitWidth, DigitHeight);
	m_NumberRenderer->SetSpacing(DigitSpacing);
	m_NumberRenderer->SetOffset(cursorX, numberY);
	m_NumberRenderer->SetMinimumDigits(DisplayDigits);
	cursorX += numberDisplayWidth + ElementSpacing;

	//「個必要です」の文字
	m_SuffixRenderer = AddComponent<PolygonRendererComponent>();
	m_SuffixRenderer->InitScreenSpaceRect(cursorX, suffixY,
		SuffixWidth, SuffixHeight, "asset\\texture\\text_kohitsuyoudesu.png");

	//不足中のGateへ近づくまではUIを表示しない
	SetVisible(false);
}

void GateRequirementNotice::Update()
{
	GameObject::Update();

	const int remainingCount = FindNearestRemainingCount();
	if (remainingCount == NoRemainingCount)
	{
		SetVisible(false);
		return;
	}

	m_NumberRenderer->SetValue(remainingCount);
	SetVisible(true);
}

//表示範囲内にある最寄りの不足中Gateから、残りのStar数を取得する
int GateRequirementNotice::FindNearestRemainingCount() const
{
	Scene* scene = Manager::GetScene();
	if (!scene)
		return NoRemainingCount;

	Player* player = scene->GetGameObject<Player>();
	StarCounter* starCounter = scene->GetGameObject<StarCounter>();
	if (!player || !starCounter || player->IsRespawning())
		return NoRemainingCount;

	const int currentStarCount = starCounter->GetValue();
	float nearestDistanceSq = std::numeric_limits<float>::max();
	int nearestRemainingCount = NoRemainingCount;

	const auto gates = scene->GetGameObjects<Gate>();
	//表示範囲内に複数のGateがある場合は、プレイヤーに最も近いものを選ぶ
	for (Gate* gate : gates)
	{
		//今のフレームで開いたGateは、シーンから削除される前でも候補から外す
		if (!gate || gate->IsDestroyRequested())
			continue;

		const int remainingCount =
			gate->GetRequiredItemCount() - currentStarCount;
		if (remainingCount <= 0)
			continue;

		const Vector3 direction = player->GetPosition() - gate->GetPosition();
		const float distanceSq = Vector3::dot(direction, direction);
		//表示開始距離をGate本体が開く距離と揃える
		const float noticeDistance = gate->GetOpenDistance();
		if (distanceSq > noticeDistance * noticeDistance)
			continue;

		if (distanceSq >= nearestDistanceSq)
			continue;

		nearestDistanceSq = distanceSq;
		nearestRemainingCount = remainingCount;
	}

	return nearestRemainingCount;
}

//通知を構成する要素の表示状態をまとめて切り替え
void GateRequirementNotice::SetVisible(bool visible)
{
	const bool becameVisible = visible && !m_IsVisible;
	m_IsVisible = visible;

	if (m_PrefixRenderer)
		m_PrefixRenderer->SetVisible(visible);
	if (m_IconRenderer)
		m_IconRenderer->SetVisible(visible);
	if (m_NumberRenderer)
		m_NumberRenderer->SetVisible(visible);
	if (m_SuffixRenderer)
		m_SuffixRenderer->SetVisible(visible);

	//非表示から表示へ変わった瞬間だけ再生
	if (becameVisible)
		AudioManager::PlaySE(SEId::GateRequirementNotice);
}
