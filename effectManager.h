//-----------------------------------------------------------
//  [ effectManager.h ]
//  ゲーム中のエフェクト生成をまとめて扱う
//-----------------------------------------------------------
#pragma once

#include "vector3.h"

struct EffectSpriteSetting;
class EffectSprite;

enum class EffectId
{
	PlayerLanding,
	PlayerDash,
	StarGet,
	HeartGet,
	JumpPad,
	Checkpoint,
	GateOpen,
	Goal,
	BulletHit
};

class EffectManager
{
private:
	//エフェクト種別に対応する設定を作成
	static EffectSpriteSetting CreateSetting(EffectId id);

public:
	static EffectSprite* Play(EffectId id, const Vector3& position);
};
