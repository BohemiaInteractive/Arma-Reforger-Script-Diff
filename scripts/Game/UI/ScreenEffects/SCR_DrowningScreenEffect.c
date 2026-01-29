class SCR_DrowningScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of drowningVignette
	protected const float DROWNINGEFFECT_OPACITY_FADEOUT_1_DURATION 	= 0.2;
	protected const float DROWNINGEFFECT_PROGRESSION_FADEOUT_1_DURATION = 0.2;

	// Fade out animation of drowningVignette
	protected const float DROWNINGEFFECT_OPACITY_FADEIN_1_DURATION		= 1;
	protected const float DROWNINGEFFECT_PROGRESSION_FADEIN_1_DURATION = 4.5;

	// Play Animation of BlackoutEffect()
	protected const float BLACKOUT_OPACITY_MULTIPLIER					= 0.70;
	
	protected const float DROWNINGEFFECT_STRENGTH = 1.0;

	// Widgets
	protected ImageWidget 							m_wDrowningEffect;
	protected ImageWidget							m_wBlackOut;

	// Character
	protected ChimeraCharacter						m_pCharacterEntity;


	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wDrowningEffect = ImageWidget.Cast(m_wRoot.FindAnyWidget("DrowningVignette"));
		m_wBlackOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("DrowningBlackOut"));
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();
		RemoveInvokers(from);

		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;

		SCR_CharacterControllerComponent m_CharController = SCR_CharacterControllerComponent.Cast(m_pCharacterEntity.GetCharacterController());
		if (!m_CharController)
			return;

		if (m_CharController.m_OnPlayerDrowning)
			m_CharController.m_OnPlayerDrowning.Insert(CreateEffect);

		if (m_CharController.m_OnPlayerStopDrowning)
			m_CharController.m_OnPlayerStopDrowning.Insert(ClearEffect);

		SCR_CharacterDamageManagerComponent damageMan = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (damageMan)
			damageMan.GetOnDamageStateChanged().Insert(OnDeath);

		// In case player started drowning before invokers were established, check if already drowning
		if (m_CharController.IsCharacterDrowning())
			CreateEffect(10, 4);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateEffect(float maxDrowningDuration, float drowningTimeStartFX)
	{
		if (!m_wDrowningEffect)
			return;

		// Add a couple seconds to drowningDuration so the widget still animates when death occurs
		float drowningDuration = (maxDrowningDuration - drowningTimeStartFX) + 2;
		if (drowningDuration <= 0)
			drowningDuration = 1;

		m_wDrowningEffect.SetSaturation(1);
		m_wDrowningEffect.SetMaskTransitionWidth(0.8);

		AnimateEffectVisibility(m_wDrowningEffect, DROWNINGEFFECT_STRENGTH, DROWNINGEFFECT_STRENGTH * 0.6, 1 / drowningDuration, 1 / drowningDuration);

		BlackoutEffect(DROWNINGEFFECT_STRENGTH, drowningDuration);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearEffect()
	{
		AnimateEffectVisibility(m_wDrowningEffect, 0, 0, DROWNINGEFFECT_PROGRESSION_FADEOUT_1_DURATION, DROWNINGEFFECT_OPACITY_FADEOUT_1_DURATION);

		BlackoutEffect(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void BlackoutEffect(float effectStrength, float drowningDuration)
	{
		if (!m_wBlackOut)
			return;

		if (effectStrength <= 0 || drowningDuration <= 0)
		{
			AnimateWidget.AlphaMask(m_wBlackOut, 0, DROWNINGEFFECT_OPACITY_FADEOUT_1_DURATION);
			return;
		}

		m_wBlackOut.SetOpacity(1);
		effectStrength *= BLACKOUT_OPACITY_MULTIPLIER;
		AnimateWidget.AlphaMask(m_wBlackOut, effectStrength, 1 / drowningDuration);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeath(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;

		ClearEffect();
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_wDrowningEffect)
			HideSingleEffect(m_wDrowningEffect);

		if (m_wBlackOut)
			HideSingleEffect(m_wBlackOut);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveInvokers(IEntity prevEntity)
	{
		if (!prevEntity)
			return;

		m_pCharacterEntity = ChimeraCharacter.Cast(prevEntity);
		if (!m_pCharacterEntity)
			return;

		SCR_CharacterControllerComponent m_CharController = SCR_CharacterControllerComponent.Cast(m_pCharacterEntity.GetCharacterController());
		if (!m_CharController)
			return;

		if (m_CharController.m_OnPlayerDrowning)
			m_CharController.m_OnPlayerDrowning.Remove(CreateEffect);

		if (m_CharController.m_OnPlayerStopDrowning)
			m_CharController.m_OnPlayerStopDrowning.Remove(ClearEffect);
	}
}
