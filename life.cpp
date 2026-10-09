//-----------------------------------------------------------
//  [ life.cpp ]
//  プレイヤーのライフ
//-----------------------------------------------------------
#include "main.h"
#include "life.h"
#include "polygonRendererComponent.h"

void Life::Init()
{
	GameObject::Init();

	m_HeartRenderers.clear();
	//ライフアイコンを横一列に並べる
	for (int i = 0; i < MaxValue; i++)
	{
		const float x = (HeartWidth + HeartSpacing) * i;
		auto renderer = AddComponent<PolygonRendererComponent>();
		renderer->InitScreenSpaceRect(x, 0.0f, HeartWidth, HeartHeight, EmptyTexture);
		m_HeartRenderers.push_back(renderer);
	}

	m_Value = MaxValue;
	RefreshHearts();
}

//ライフ表示更新
void Life::RefreshHearts()
{
    for (int i = 0; i < static_cast<int>(m_HeartRenderers.size()); i++)
    {
        if (i < m_Value)
            m_HeartRenderers[i]->SetTexture(FilledTexture);
        else
            m_HeartRenderers[i]->SetTexture(EmptyTexture);
    }
}

void Life::Add(int value)
{
    SetValue(m_Value + value);
}

void Life::SetValue(int value)
{
	if (value < 0)
		value = 0;
	if (value > MaxValue)
		value = MaxValue;

    if (m_Value == value)
        return;

    m_Value = value;
    RefreshHearts();
}
