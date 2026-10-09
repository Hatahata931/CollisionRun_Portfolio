//-----------------------------------------------------------
//  [ playerAnimation.cpp ]
//  プレイヤーのアニメーション制御
//-----------------------------------------------------------
#include "main.h"
#include "player.h"
#include "animationModel.h"

//次に再生するアニメーションを決定
void Player::UpdateAnimationState(bool isMoving, bool landedThisFrame)
{
	//開始要求は1フレームだけ使用するため、読み取った直後に解除する
	const bool jumpAnimationRequested = m_JumpAnimationRequested;
	m_JumpAnimationRequested = false;

	AnimationState nextState = AnimationState::Idle;
	if (isMoving)
		nextState = m_IsDashing ? AnimationState::Run : AnimationState::Walk;

	if (landedThisFrame)
		nextState = AnimationState::Landing;
	//接地していない
	else if (!m_IsGrounded)
	{
		const bool jumpSequenceActive =
			IsAirborneAnimation(m_NextAnimationState);
		const bool confirmedAirborne =
			m_AirborneFrames >= MinLandingAirFrames;

		//ジャンプアニメーションの開始要求がある、ジャンプアニメーションを
		//再生中、またはMinLandingAirFrames以上空中にいる
		if (jumpAnimationRequested || jumpSequenceActive || confirmedAirborne)
		{
			if (m_Velocity.y > Gravity)
				nextState = AnimationState::JumpUp;
			else if (m_Velocity.y < -Gravity)
				nextState = AnimationState::JumpDown;
			else
				nextState = AnimationState::JumpAir;
		}
	}
	//接地している
	else if (m_NextAnimationState == AnimationState::Landing)
	{
		const bool landingFinished =
			m_NextAnimationFrame >= LandingLastFrame;
		const bool canCancelLanding =
			m_NextAnimationFrame >= LandingCancelableFrame;

		//停止中は最後まで再生し、移動中も着地の膝曲げが見えるまでは維持する
		if (!landingFinished && (!isMoving || !canCancelLanding))
			nextState = AnimationState::Landing;
	}

	if (m_NextAnimationState != nextState)
		ChangeAnimationState(nextState); //アニメーション状態変更
}

void Player::TransitionUpdate()
{
	//入力や物理状態は変えず、描画に使うアニメーションだけを進める
	UpdateAnimationPlayback();
}

//モデルの姿勢、再生フレーム、切り替え補間を更新
void Player::UpdateAnimationPlayback()
{
	//再生方法
	const AnimationPlaybackMode currentPlaybackMode = IsLoopingAnimation(m_CurrentAnimationState)
		? AnimationPlaybackMode::Loop : AnimationPlaybackMode::Clamp;
	const AnimationPlaybackMode nextPlaybackMode = IsLoopingAnimation(m_NextAnimationState)
		? AnimationPlaybackMode::Loop : AnimationPlaybackMode::Clamp;

	//ルート位置を固定するか
	const AnimationPositionMode currentPositionMode = IsAirborneAnimation(m_CurrentAnimationState)
		? AnimationPositionMode::Locked : AnimationPositionMode::Animated;
	const AnimationPositionMode nextPositionMode = IsAirborneAnimation(m_NextAnimationState)
		? AnimationPositionMode::Locked : AnimationPositionMode::Animated;

	m_AnimationModel->Update(GetAnimationName(m_CurrentAnimationState), m_CurrentAnimationFrame,
		GetAnimationName(m_NextAnimationState), m_NextAnimationFrame, m_AnimationBlend,
		currentPlaybackMode, nextPlaybackMode, currentPositionMode, nextPositionMode);

	AdvanceAnimationFrames(); //アニメーションの再生位置を進める

	m_AnimationBlend += AnimationBlendSpeed;
	if (m_AnimationBlend >= 1.0f)
	{
		m_AnimationBlend = 1.0f;
		m_CurrentAnimationState = m_NextAnimationState;
		m_CurrentAnimationFrame = m_NextAnimationFrame;
	}
}

void Player::ChangeAnimationState(AnimationState nextState)
{
	const bool jumpUpToAir = m_NextAnimationState == AnimationState::JumpUp &&
		nextState == AnimationState::JumpAir;

	//ジャンプの頂点では上昇中の姿勢をそのまま維持する
	if (jumpUpToAir)
	{
		m_CurrentAnimationState = nextState;
		m_NextAnimationState = nextState;
		m_CurrentAnimationFrame = m_NextAnimationFrame;
		m_NextAnimationFrame = m_CurrentAnimationFrame;
		m_AnimationBlend = 1.0f;
		return;
	}

	//通常の切り替え
	m_CurrentAnimationState = m_NextAnimationState;
	m_CurrentAnimationFrame = m_NextAnimationFrame;
	m_NextAnimationState = nextState;
	m_NextAnimationFrame = GetAnimationStartFrame(nextState);
	m_AnimationBlend = 0.0f;
}

//状態に応じて現在と次のアニメーション再生位置を進める
void Player::AdvanceAnimationFrames()
{
	const auto advanceFrame = [](AnimationState state, float& frame)
	{
		switch (state)
		{
		case AnimationState::Idle:
		case AnimationState::Walk:
		case AnimationState::Run:
			frame += GroundAnimationFrameAdvance;
			break;

		//ジャンプ・着地は最終フレームで止める
		case AnimationState::JumpUp:
			frame += JumpUpFrameAdvance;
			if (frame > JumpUpLastFrame)
				frame = JumpUpLastFrame;
			break;

		case AnimationState::JumpDown:
			frame += JumpDownFrameAdvance;
			if (frame > JumpDownLastFrame)
				frame = JumpDownLastFrame;
			break;

		case AnimationState::Landing:
			frame += LandingFrameAdvance;
			if (frame > LandingLastFrame)
				frame = LandingLastFrame;
			break;

		//JumpUpの最終フレームを使うので何もしない
		case AnimationState::JumpAir:
			break;
		}
	};

	advanceFrame(m_CurrentAnimationState, m_CurrentAnimationFrame);
	advanceFrame(m_NextAnimationState, m_NextAnimationFrame);
}

const char* Player::GetAnimationName(AnimationState state)
{
	switch (state)
	{
	case AnimationState::Walk:
		return "Walk";
	case AnimationState::Run:
		return "Run";
	case AnimationState::JumpUp:
	case AnimationState::JumpAir:
		return "JumpUp";
	case AnimationState::JumpDown:
		return "JumpDown";
	case AnimationState::Landing:
		return "Landing";
	case AnimationState::Idle:
	default:
		return "Idle";
	}
}

float Player::GetAnimationStartFrame(AnimationState state)
{
	return state == AnimationState::JumpAir
		? JumpUpLastFrame : 0.0f;
}

//指定したアニメーションがループ再生か確認
bool Player::IsLoopingAnimation(AnimationState state)
{
	return state == AnimationState::Idle || state == AnimationState::Walk ||
		state == AnimationState::Run;
}

//指定したアニメーションが空中用か確認
bool Player::IsAirborneAnimation(AnimationState state)
{
	return state == AnimationState::JumpUp || state == AnimationState::JumpAir ||
		state == AnimationState::JumpDown;
}
