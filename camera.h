//-----------------------------------------------------------
//  [ camera.h ]
//  プレイヤーを追従するカメラ
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class Player;

class Camera : public GameObject
{
private:
	//角度
	static constexpr float DefaultPitch = XM_PI * 25.0f / 180.0f;
	static constexpr float MinPitch = -XM_PI * 10.0f / 180.0f;
	static constexpr float MaxPitch = XM_PI * 60.0f / 180.0f;

	//距離
	static constexpr float DefaultDistance = 5.5f;
	static constexpr float MinDistance = 4.0f;
	static constexpr float MaxDistance = 7.0f;

	static constexpr float TargetHeight = 1.5f; //プレイヤーの位置から注視点までの高さ

	//感度
	static constexpr float MouseRotationSensitivity = 0.0035f; //マウスでの感度
	static constexpr float KeyboardRotationSpeed = 0.03f; //矢印キーでの感度
	static constexpr float ZoomStep = 0.5f; //ホイール1目盛りあたりの距離変化

	//追従
	static constexpr float HorizontalFollowRate = 0.22f; //水平方向の追従率
	static constexpr float VerticalFollowRate = 0.12f; //上下方向の追従率
	static constexpr float ZoomFollowRate = 0.18f; //ズーム距離の補間率

	//描画範囲
	static constexpr float FieldOfView = XM_PI / 3.0f; //縦方向の画角(60度)
	static constexpr float NearClip = 0.1f; //手前側の描画開始距離
	static constexpr float FarClip = 1000.0f; //奥側の描画終了距離

	XMMATRIX m_View;
	XMMATRIX m_Projection;

	Vector3 m_Target{ 0.0f, 0.0f, 0.0f }; //注視点

	//プレイヤーを中心に周回するための現在角度と距離
	float m_Yaw = 0.0f;
	float m_Pitch = DefaultPitch;
	float m_CurrentDistance = DefaultDistance;
	float m_TargetDistance = DefaultDistance;
	bool m_FollowInitialized = false; //カメラの追従位置を初期化したか
	bool m_WasRespawning = false; //前のフレームまでプレイヤーが復帰処理中か

	//カメラ回転・ズーム入力処理
	void HandleInput(Player* player);
	//注視点を即座にプレイヤーに合わせる
	void SnapTargetToPlayer(Player* player);
	//注視点を少しずつプレイヤーに合わせる
	void UpdateFollowTarget(Player* player);
	//注視点を中心としたカメラ位置を計算
	void UpdateOrbitPosition();
	//ビュー行列、プロジェクション行列を更新
	void UpdateMatrices();

public:
	void Init() override;
	void Update() override;
	void LateUpdate() override;
	void Draw() override;

	XMMATRIX GetViewMatrix() const { return m_View; }
	XMMATRIX GetProjectionMatrix() const { return m_Projection; }

	void SetTarget(Vector3 target) { m_Target = target; }

	//カメラの向いている方向を取得
	Vector3 GetForward() override;
};
