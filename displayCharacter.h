//-----------------------------------------------------------
//  [ displayCharacter.h ]
//  各画面の演出として表示するユニティちゃん
//-----------------------------------------------------------
#pragma once

#include "gameObject.h"

class AnimationModel;

class DisplayCharacter : public GameObject
{
public:
	//画面に応じたアニメーションの再生方法
	enum class AnimationPreset
	{
		Idle,
		Salute, //敬礼
		KneelDown, //跪く
	};

private:
	//アニメーション識別名
	static constexpr const char* IdleAnimationName = "Idle";
	static constexpr const char* SaluteAnimationName = "Salute";
	static constexpr const char* KneelDownAnimationName = "KneelDown";

	static constexpr float ModelScale = 0.01f;
	static constexpr float AnimationFrameAdvance = 0.5f; //1フレームあたりのアニメーション進行量

	//アニメーションの最終フレーム
	static constexpr float SaluteLastFrame = 70.0f;
	static constexpr float KneelDownLastFrame = 106.0f;

	//専用ライト
	static constexpr float CharacterLightAmbient = 0.5f; //環境光
	static constexpr float CharacterLightDiffuse = 0.9f; //拡散光

	AnimationModel* m_AnimationModel = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11InputLayout* m_VertexLayout = nullptr;
	AnimationPreset m_AnimationPreset = AnimationPreset::Idle;
	float m_AnimationFrame = 0.0f;

	static const char* GetAnimationName(AnimationPreset preset);
	static float GetAnimationLastFrame(AnimationPreset preset);
	static bool IsLoopingAnimation(AnimationPreset preset);
	void UpdateAnimation();
	//アニメーションの再生フレームを進める
	void AdvanceAnimationFrame();

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	//フェード中もアニメーション更新
	void TransitionUpdate() override;
	void Draw() override;

	void SetAnimationPreset(AnimationPreset preset);
};
