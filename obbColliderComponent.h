//-----------------------------------------------------------
//  [ obbColliderComponent.h ]
//  SATを用いたOBB同士の衝突判定と押し戻し処理
//-----------------------------------------------------------

#pragma once

#include "objectComponent.h"
#include "vector3.h"

class OBBColliderComponent : public ObjectComponent
{
private:
	Vector3 m_HalfSize{ 0.5f, 0.5f, 0.5f }; //箱の半分サイズ
	Vector3 m_CenterOffset{ 0.0f, 0.0f, 0.0f }; //中心位置のずらし量
	bool m_UseScale = true; //オブジェクトのスケールを判定に使うか
	bool m_UseRotation = true; //箱の回転を判定に使うか

public:
	//ObjectComponentのコンストラクタを引き継ぐ
	using ObjectComponent::ObjectComponent;

	//判定に使用するOBBの中心・向き・半サイズをワールド座標で取得
	void GetWorldOBB(Vector3& center, Vector3 axis[3], Vector3& half) const;

	void Update() override;
	void Draw() override;

	//SATでOBB同士が交差しているかのみ判定
	bool Intersects(const OBBColliderComponent* other) const;
	//SATでOBB同士の交差を判定し押し戻し量を計算
	//preferHorizontalOnDownwardがtrueの場合、側面寄りの下向き補正を水平補正へ置き換える
	bool ResolveCollision(const OBBColliderComponent* other, Vector3& outCorrection, bool preferHorizontalOnDownward = false) const;

	void SetHalfSize(const Vector3& halfSize) { m_HalfSize = halfSize; }
	const Vector3& GetHalfSize() const { return m_HalfSize; }
	void SetCenterOffset(const Vector3& centerOffset) { m_CenterOffset = centerOffset; }
	const Vector3& GetCenterOffset() const { return m_CenterOffset; }
	void SetUseScale(bool useScale) { m_UseScale = useScale; }
	bool GetUseScale() const { return m_UseScale; }
	void SetUseRotation(bool useRotation) { m_UseRotation = useRotation; }
	bool GetUseRotation() const { return m_UseRotation; }
};
