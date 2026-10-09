//-----------------------------------------------------------
//  [ box.h ]
//  汎用箱クラス
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Box : public GameObject
{
private:
	bool m_CollisionEnabled = true;
	bool m_DamageEnabled = false;
	Vector3 m_RotationVelocity{ 0.0f, 0.0f, 0.0f };

	//回転する箱に乗ったプレイヤーの追従量計算に使う前フレーム情報
	Vector3 m_PreviousPosition{ 0.0f, 0.0f, 0.0f };
	Vector3 m_PreviousRotation{ 0.0f, 0.0f, 0.0f };

protected:
	virtual void UpdateRotation();

public:
	void Init() override;
	void Update() override;
	void SetModel(const char* filename);
	void SetTexture(const char* filename);
	void SetColliderHalfSize(const Vector3& halfSize);
	//モデル原点から見たOBB中心のローカルオフセットを設定
	void SetColliderCenterOffset(const Vector3& centerOffset);

	void SetCollisionEnabled(bool enabled) { m_CollisionEnabled = enabled; }
	bool IsCollisionEnabled() const { return m_CollisionEnabled; }
	void SetDamageEnabled(bool enabled) { m_DamageEnabled = enabled; }
	bool IsDamageEnabled() const { return m_DamageEnabled; }
	void SetRotationVelocity(const Vector3& velocity) { m_RotationVelocity = velocity; }
	virtual bool HasPlatformMotion() const { return m_RotationVelocity != Vector3(0.0f, 0.0f, 0.0f); }

	//指定位置における前フレームからの足場移動量を取得
	Vector3 GetPlatformDeltaAtPosition(const Vector3& position) const;
};