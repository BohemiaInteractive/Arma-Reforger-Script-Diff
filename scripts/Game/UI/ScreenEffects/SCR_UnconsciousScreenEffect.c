class SCR_UnconsciousScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of UnconsciousnessEffect()
	protected const float UNCONSCIOUS_EFFECT_DURATION = 0.5;
	protected const float UNCONSCIOUS_FADEOUT_DURATION = 0.8;
	protected const float UNCONSCIOUS_OSCILIATION_OVERSHOOT = 0.2;
	protected const float UNCONSCIOUS_ALPHAMASK_MIN = 0.4;
	protected const float UNCONSCIOUS_ALPHAMASK_MAX = 0.7;
	protected const float UNCONSCIOUS_OPACITY_DIFF = 0.05;

	// Widgets
	protected bool m_bOverUnder;
	protected WidgetAnimationBase m_UnconMaskAnim;
	protected ImageWidget m_wDeath;

	//Character
	protected SCR_CharacterDamageManagerComponent m_DamageManager;
	protected ChimeraCharacter m_CharacterEntity;

	[Attribute("0", UIWidgets.ComboBox, "Which curve shape should be used for lerping the effect in", "", ParamEnumArray.FromEnum(EAnimationCurve)) ]
	EAnimationCurve m_eUnconsciousEffectAnimIn;

	[Attribute("0", UIWidgets.ComboBox, "Which curve shape should be used for lerping the effect out", "", ParamEnumArray.FromEnum(EAnimationCurve)) ]
	EAnimationCurve m_eUnconsciousEffectAnimOut;

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_CharacterEntity = ChimeraCharacter.Cast(owner);
		m_wDeath = ImageWidget.Cast(m_wRoot.FindAnyWidget("UnconOverlay"));
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();

		m_CharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_CharacterEntity)
			return;

		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(m_CharacterEntity.GetDamageManager());

		// In case player became unconscious before invokers were established, check if already bleeding
		CharacterControllerComponent controller = m_CharacterEntity.GetCharacterController();
		if (controller && controller.GetLifeState() == ECharacterLifeState.INCAPACITATED)
			DisplayConsciousnessChanged(false);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayConsciousnessChanged(bool conscious, bool init = false)
	{
		UnconsciousnessEffect(conscious);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnconsciousnessEffect(bool conscious)
	{
		if (!conscious)
			PlayUnconAnim(UNCONSCIOUS_EFFECT_DURATION);
		else
			StopUnconAnim();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayUnconAnim(float speed)
	{
		if (!m_wDeath || !m_CharacterController)
			return;

		if (m_CharacterController.IsDead())
			return;

		m_bOverUnder = !m_bOverUnder;

		float unconProgress = 1 - GetUnconProgress();
		float targetProgress;
		EAnimationCurve curve;

		if (m_bOverUnder)
		{
			curve = m_eUnconsciousEffectAnimIn;
			targetProgress = unconProgress + (unconProgress * UNCONSCIOUS_OSCILIATION_OVERSHOOT);
			AnimateWidget.Opacity(m_wDeath, 1, speed, true);
		}
		else
		{
			curve = m_eUnconsciousEffectAnimOut;
			targetProgress = unconProgress - (unconProgress * UNCONSCIOUS_OSCILIATION_OVERSHOOT);
			AnimateWidget.Opacity(m_wDeath, 1 - UNCONSCIOUS_OPACITY_DIFF, speed, true);
		}

		m_UnconMaskAnim = AnimateWidget.AlphaMask(m_wDeath, Math.Lerp(UNCONSCIOUS_ALPHAMASK_MIN, UNCONSCIOUS_ALPHAMASK_MAX, targetProgress), speed);
		m_UnconMaskAnim.SetCurve(curve);

		if (m_UnconMaskAnim)
			m_UnconMaskAnim.GetOnCompleted().Insert(OnAnimCycleComplete);
		
		SCR_LogitechLEDManager.ActivateState(ELogitechLEDState.UNCONSCIOUS);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAnimCycleComplete(WidgetAnimationBase anim)
	{
		if (anim)
		{
			anim.Stop();
			anim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);
		}

		GetGame().GetCallqueue().CallLater(PlayUnconAnim, param1: UNCONSCIOUS_EFFECT_DURATION);
	}

	//------------------------------------------------------------------------------------------------
	protected float GetUnconProgress()
	{
		if (!m_DamageManager)
			return 0;

		SCR_CharacterResilienceHitZone resilience = m_DamageManager.GetResilienceHitZone();
		if (!resilience)
			return 0;

		SCR_CharacterBloodHitZone blood = m_DamageManager.GetBloodHitZone();
		if (blood && blood.GetHealthScaled() < blood.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))
			return 0;

		return Math.Lerp(0, resilience.GetDamageStateThreshold(ECharacterResilienceState.WEAKENED), resilience.GetHealthScaled());
	}

	//------------------------------------------------------------------------------------------------
	protected void StopUnconAnim()
	{
		if (m_UnconMaskAnim)
			m_UnconMaskAnim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);

		if (!m_wDeath)
			return;

		AnimateWidget.AlphaMask(m_wDeath, 0, UNCONSCIOUS_FADEOUT_DURATION);
		AnimateWidget.Opacity(m_wDeath, 0, UNCONSCIOUS_FADEOUT_DURATION, true);
		SCR_LogitechLEDManager.ActivateState(ELogitechLEDState.DEFAULT);
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_UnconMaskAnim)
			m_UnconMaskAnim.GetOnCycleCompleted().Remove(OnAnimCycleComplete);

		if (m_wDeath)
		{
			AnimateWidget.StopAllAnimations(m_wDeath);
			m_wDeath.SetOpacity(0);
			m_wDeath.SetMaskProgress(0);
		}
	}
}
