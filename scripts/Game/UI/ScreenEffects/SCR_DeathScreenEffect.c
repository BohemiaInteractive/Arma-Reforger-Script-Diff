class SCR_DeathScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of DeathEffect()
	protected const float DEATHEFFECT_START_OPACITY						= 0.7;
	protected const float DEATHEFFECT_FADEIN_OPACITY_DURATION 			= 8;
	protected const float DEATHEFFECT_FADEIN_PROGRESSION_DURACTION		= 5;

	// Play Animation of InstaDeathEffect()
	protected const float INSTADEATHEFFECT_START_OPACITY				= 0.4;
	protected const float INSTADEATHEFFECT_START_PROGRESSION			= 0.4;
	protected const float INSTADEATHEFFECT_FADEIN_OPACITY_DURATION 	= 8.5;
	protected const float INSTADEATHEFFECT_FADEIN_PROGRESSION_DURACTION= 10;
	
	protected const float DEATHEFFECT_FADEIN_OPACITY_TARGET		= 1;
	protected const float DEATHEFFECT_FADEIN_PROGRESSION_TARGET 	= 1;

	// Widgets
	protected ImageWidget							m_wDeath;
	protected ImageWidget							m_wBlackOut;

	// Character
	protected ChimeraCharacter						m_pCharacterEntity;
	protected SCR_HitZone 							m_pHeadHitZone;
	protected SCR_CharacterDamageManagerComponent m_pDamageManager;


	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_pCharacterEntity = ChimeraCharacter.Cast(owner);

		m_wDeath = ImageWidget.Cast(m_wRoot.FindAnyWidget("DeathOverlay"));
		m_wBlackOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("DeathBlackOut"));
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		if (!to)
			return;

		m_pCharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_pCharacterEntity)
			return;

		//Don't remove effects until new entity was aquired, so death effects persist through death
		ClearEffects();

		m_pDamageManager = SCR_CharacterDamageManagerComponent.Cast(m_pCharacterEntity.GetDamageManager());
		if (!m_pDamageManager)
			return;

		// Invoker for momentary damage events and DOT damage events
		m_pHeadHitZone = m_pDamageManager.GetHeadHitZone();

		if (m_pHeadHitZone)
			m_pHeadHitZone.GetOnDamageStateChanged().Insert(InstaDeathEffect);

		m_pDamageManager.GetOnDamageStateChanged().Insert(OnDamageStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	private void OnDamageStateChanged()
	{
		if (m_pDamageManager.GetState() != EDamageState.DESTROYED)
			return;

		if (m_pHeadHitZone.GetDamageState() != EDamageState.DESTROYED)
			DeathEffect();
		else
			InstaDeathEffect();
	}

	//------------------------------------------------------------------------------------------------
	protected void DeathEffect()
	{
		if (!m_wDeath)
			return;

		AnimateWidget.StopAllAnimations(m_wDeath);
		
		m_wDeath.SetOpacity(DEATHEFFECT_START_OPACITY);		
		AnimateEffectVisibility(m_wDeath, DEATHEFFECT_FADEIN_OPACITY_TARGET, DEATHEFFECT_FADEIN_PROGRESSION_TARGET, DEATHEFFECT_FADEIN_OPACITY_DURATION, DEATHEFFECT_FADEIN_PROGRESSION_DURACTION);
	}

	//------------------------------------------------------------------------------------------------
	protected void InstaDeathEffect()
	{
		if (!m_wDeath)
			return;

		if (m_pHeadHitZone.GetDamageState() != EDamageState.DESTROYED)
			return;

		if (m_pDamageManager.GetState() != EDamageState.DESTROYED)
			return;

		AnimateWidget.StopAllAnimations(m_wDeath);
		
		m_wDeath.SetOpacity(INSTADEATHEFFECT_START_OPACITY);
		m_wDeath.SetMaskProgress(INSTADEATHEFFECT_START_PROGRESSION);		
		AnimateEffectVisibility(m_wDeath, DEATHEFFECT_FADEIN_OPACITY_TARGET, DEATHEFFECT_FADEIN_PROGRESSION_TARGET, INSTADEATHEFFECT_FADEIN_OPACITY_DURATION, INSTADEATHEFFECT_FADEIN_PROGRESSION_DURACTION);
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		if (m_pDamageManager)
			m_pDamageManager.GetOnDamageStateChanged().Remove(OnDamageStateChanged);

		if (m_pHeadHitZone)
			m_pHeadHitZone.GetOnDamageStateChanged().Remove(InstaDeathEffect);

		if (m_wDeath)
			HideSingleEffect(m_wDeath);
	}
}
