class SCR_StaminaBlurEffect : SCR_BaseScreenEffect
{
	// PP constants
	// Play Animation of StaminaEffects()
	protected const float STAMINAEFFECT_INITIAL_OPACITY_TARGET			= 0.75;
	protected const float STAMINAEFFECT_FADEIN_PROGRESSION_TARGET		= 0.35;
	protected const float STAMINAEFFECT_FADEIN_PROGRESSION_DURATION	 	= 1;
	protected const float STAMINA_CLEAREFFECT_DELAY 					= 2000;
	protected const float STAMINA_EFFECT_THRESHOLD 						= 0.45;

	[Attribute(defvalue: "0.1111", uiwidget: UIWidgets.EditBox, desc: "Blur intensity multiplier")]
	protected float m_fStaminaBlurMultiplier;

	//Blurriness
	private static float s_fBlurriness;
	private static bool s_bEnableRadialBlur;
	protected const string RADIAL_BLUR_EMAT 							= "{B011FE0AD21E2447}UI/Materials/ScreenEffects_BlurPP.emat";

	// Widgets
	private ImageWidget 												m_wSupression;

	// Variables connected to a material, need to be static
	static const int RADIAL_BLUR_PRIORITY								= 6;

	// Stamina
	protected int m_iStaminaSignal 										= -1;
	protected bool m_bStaminaEffectActive;

	// Owner data
	protected SignalsManagerComponent m_pSignalsManager;

	//------------------------------------------------------------------------------------------------
	protected override void DisplayStartDraw(IEntity owner)
	{
		m_wSupression = ImageWidget.Cast(m_wRoot.FindAnyWidget("SuppressionVignette"));
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ChimeraCharacter characterEntity = ChimeraCharacter.Cast(to);
		if (!characterEntity)
			return;

		// Audio components for damage-related audio effects
		m_pSignalsManager = SignalsManagerComponent.Cast(characterEntity.FindComponent(SignalsManagerComponent));
		if (m_pSignalsManager)
			m_iStaminaSignal = m_pSignalsManager.FindSignal("Exhaustion");

		characterEntity.GetWorld().SetCameraPostProcessEffect(characterEntity.GetWorld().GetCurrentCameraId(), RADIAL_BLUR_PRIORITY, PostProcessEffectType.RadialBlur, RADIAL_BLUR_EMAT);
	}

	//------------------------------------------------------------------------------------------------
	protected void FindStaminaValues()
	{
		if (!m_pSignalsManager || m_iStaminaSignal == -1)
			return;

		float stamina = m_pSignalsManager.GetSignalValue(m_iStaminaSignal);
		s_fBlurriness = m_wSupression.GetMaskProgress() * m_fStaminaBlurMultiplier;

		if (stamina > STAMINA_EFFECT_THRESHOLD && !m_bStaminaEffectActive)
		{
			s_bEnableRadialBlur = true;
			StaminaEffects(true);
		}
		else if (m_bStaminaEffectActive && stamina < STAMINA_EFFECT_THRESHOLD)
		{
			s_bEnableRadialBlur = false;
			GetGame().GetCallqueue().Remove(StaminaEffects);
			GetGame().GetCallqueue().Remove(ClearStaminaEffect);
			ClearStaminaEffect(false);
			m_bStaminaEffectActive = false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StaminaEffects(bool repeat)
	{
		if (!m_wSupression)
			return;

		m_bStaminaEffectActive = true;

		m_wSupression.SetOpacity(STAMINAEFFECT_INITIAL_OPACITY_TARGET);
		AnimateWidget.AlphaMask(m_wSupression, STAMINAEFFECT_FADEIN_PROGRESSION_TARGET, STAMINAEFFECT_FADEIN_PROGRESSION_DURATION);

		UpdateEffectVisibility(m_wSupression);

		GetGame().GetCallqueue().CallLater(ClearStaminaEffect, STAMINA_CLEAREFFECT_DELAY, false, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearStaminaEffect(bool repeat)
	{
		if (!m_wSupression)
			return;

		AnimateWidget.AlphaMask(m_wSupression, 0, 1);
		if (repeat && m_bStaminaEffectActive && m_wSupression && !m_wSupression.GetOpacity() == 0)
			GetGame().GetCallqueue().CallLater(StaminaEffects, STAMINA_CLEAREFFECT_DELAY, false, repeat);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnSuspended()
	{
		s_bEnableRadialBlur = false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnResumed()
	{
		s_bEnableRadialBlur = true;
	}

	//------------------------------------------------------------------------------------------------
	protected override void UpdateEffect(float timeSlice)
	{
		if (m_pSignalsManager)
			FindStaminaValues();
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_wSupression)
			HideSingleEffect(m_wSupression);
	}
}
