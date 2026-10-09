//-----------------------------------------------------------
//  [ effectManager.cpp ]
//  ゲーム中のエフェクト生成をまとめて扱う
//-----------------------------------------------------------
#include "main.h"
#include "effectManager.h"
#include "effectSprite.h"
#include "manager.h"
#include "scene.h"

//エフェクト種別に対応する設定を作成
EffectSpriteSetting EffectManager::CreateSetting(EffectId id)
{
	EffectSpriteSetting setting;

	//見た目の調整値はここに集約して各ギミック側の処理をシンプルに保つ
	switch (id)
	{
	case EffectId::PlayerLanding:
		setting.TexturePath = "asset\\texture\\effect_player_landing_dust.png";
		setting.LifeFrames = 12;
		setting.StartScale = { 0.65f, 0.65f, 0.65f };
		setting.EndScale = { 1.4f, 0.85f, 1.4f };
		setting.StartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		setting.EndColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		break;

	case EffectId::PlayerDash:
		setting.TexturePath = "asset\\texture\\effect_player_dash_dust.png";
		setting.LifeFrames = 12;
		setting.StartScale = { 0.5f, 0.5f, 0.5f };
		setting.EndScale = { 1.2f, 0.83f, 1.2f };
		setting.StartColor = { 1.0f, 1.0f, 1.0f, 0.8f };
		setting.EndColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		break;

	case EffectId::StarGet:
		setting.TexturePath = "asset\\texture\\effect_spark.png";
		setting.LifeFrames = 22;
		setting.StartScale = { 0.35f, 0.35f, 0.35f };
		setting.EndScale = { 1.2f, 1.2f, 1.2f };
		setting.StartColor = { 1.0f, 0.95f, 0.25f, 1.0f };
		setting.EndColor = { 1.0f, 0.75f, 0.10f, 0.0f };
		setting.RotationSpeed = 0.12f;
		setting.Additive = true;
		break;

	case EffectId::HeartGet:
		setting.TexturePath = "asset\\texture\\effect_ring.png";
		setting.LifeFrames = 26;
		setting.StartScale = { 0.45f, 0.45f, 0.45f };
		setting.EndScale = { 1.25f, 1.25f, 1.25f };
		setting.StartColor = { 1.0f, 0.35f, 0.55f, 0.9f };
		setting.EndColor = { 1.0f, 0.55f, 0.75f, 0.0f };
		setting.Additive = true;
		break;

	case EffectId::JumpPad:
		setting.TexturePath = "asset\\texture\\effect_ring.png";
		setting.LifeFrames = 18;
		setting.StartScale = { 0.5f, 0.5f, 0.5f };
		setting.EndScale = { 1.6f, 1.6f, 1.6f };
		setting.StartColor = { 0.35f, 0.95f, 1.0f, 0.85f };
		setting.EndColor = { 0.35f, 0.95f, 1.0f, 0.0f };
		setting.Additive = true;
		break;

	case EffectId::Checkpoint:
		setting.TexturePath = "asset\\texture\\effect_ring.png";
		setting.LifeFrames = 34;
		setting.StartScale = { 0.7f, 0.7f, 0.7f };
		setting.EndScale = { 2.1f, 2.1f, 2.1f };
		setting.StartColor = { 0.45f, 1.0f, 0.75f, 0.9f };
		setting.EndColor = { 0.45f, 1.0f, 0.75f, 0.0f };
		setting.Additive = true;
		break;

	case EffectId::GateOpen:
		setting.TexturePath = "asset\\texture\\effect_ring.png";
		setting.LifeFrames = 30;
		setting.StartScale = { 1.5f, 1.5f, 1.5f };
		setting.EndScale = { 4.0f, 4.0f, 4.0f };
		setting.StartColor = { 1.0f, 0.85f, 0.25f, 0.9f };
		setting.EndColor = { 1.0f, 0.65f, 0.15f, 0.0f };
		setting.Additive = true;
		break;

	case EffectId::Goal:
		setting.TexturePath = "asset\\texture\\effect_spark.png";
		setting.LifeFrames = 45;
		setting.StartScale = { 1.0f, 1.0f, 1.0f };
		setting.EndScale = { 3.2f, 3.2f, 3.2f };
		setting.StartColor = { 1.0f, 0.95f, 0.35f, 1.0f };
		setting.EndColor = { 1.0f, 0.75f, 0.15f, 0.0f };
		setting.RotationSpeed = 0.08f;
		setting.Additive = true;
		break;

	case EffectId::BulletHit:
		setting.TexturePath = "asset\\texture\\explosion.png";
		setting.Columns = 4;
		setting.Rows = 4;
		setting.TotalFrames = 16;
		setting.FrameInterval = 1;
		setting.LifeFrames = 16;
		setting.StartScale = { 0.7f, 0.7f, 0.7f };
		setting.EndScale = { 1.1f, 1.1f, 1.1f };
		setting.StartColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		setting.EndColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		break;

	default:
		break;
	}

	return setting;
}

EffectSprite* EffectManager::Play(EffectId id, const Vector3& position)
{
	Scene* scene = Manager::GetScene();
	if (!scene)
		return nullptr;

	EffectSprite* effect = scene->AddGameObject<EffectSprite>(Scene::EFFECT);
	effect->SetPosition(position);
	effect->Configure(CreateSetting(id));

	return effect;
}
