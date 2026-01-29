class SCR_BleedingScreenEffect : SCR_BaseScreenEffect
{
	// Play Animation of CreateEffectOverTime()
	protected const int BLEEDING_REPEAT_DELAY 						= 2500;
	
	protected const float BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION 	= 0.2;
	protected const float BLEEDINGEFFECT_PROGRESSION_FADEOUT_1_DURATION = 0.2;
	
	protected const float BLEEDINGEFFECT_OPACITY_FADEOUT_2_DURATION 	= 0.3;
	protected const float BLEEDINGEFFECT_PROGRESSION_FADEOUT_2_DURATION = 3;
	
	protected const float BLEEDINGEFFECT_OPACITY_FADEIN_1_DURATION		= 1;
	protected const float BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION = 4.5;

	// Play Animation of BlackoutEffect()
	protected const float BLACKOUT_OPACITY_MULTIPLIER					= 0.20;

	//Saturation
	private static float s_fSaturation;

	// Widgets
	protected ImageWidget 							m_wBloodEffect1;
	protected ImageWidget 							m_wBloodEffect2;
	protected ImageWidget							m_wBlackOut;
	private bool m_bEffectState;

	// Character
	protected SCR_CharacterDamageManagerComponent	m_DamageManager;
	protected SCR_CharacterBloodHitZone				m_BloodHZ;
	protected ChimeraCharacter						m_CharacterEntity;

	protected bool m_bBleedingEffect;
	protected bool m_bIsBleeding;
	protected bool m_bPlayHeartBeat;


	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		m_wBloodEffect1 = ImageWidget.Cast(m_wRoot.FindAnyWidget("BloodVignette1"));
		m_wBloodEffect2 = ImageWidget.Cast(m_wRoot.FindAnyWidget("BloodVignette2"));
		m_wBlackOut = ImageWidget.Cast(m_wRoot.FindAnyWidget("BleedingBlackOut"));
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayControlledEntityChanged(IEntity from, IEntity to)
	{
		ClearEffects();

		m_CharacterEntity = ChimeraCharacter.Cast(to);
		if (!m_CharacterEntity)
			return;

		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(m_CharacterEntity.GetDamageManager());
		if (!m_DamageManager)
			return;

		// define hitzones for later getting
		m_BloodHZ = m_DamageManager.GetBloodHitZone();

		// Invoker for momentary damage events and DOT damage events
		m_DamageManager.GetOnDamageEffectAdded().Insert(OnDamageEffectAdded);
		m_DamageManager.GetOnDamageEffectRemoved().Insert(OnDamageEffectRemoved);

		// In case player started bleeding before invokers were established, check if already bleeding
		array<ref SCR_PersistentDamageEffect> effects = m_DamageManager.GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect);
		if (effects.IsEmpty())
			return;

		// Pass the first bleeding to the function that is normally invoked when bleeding is added
		OnDamageEffectAdded(effects[0]);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		m_bIsBleeding = m_DamageManager.IsBleeding();
		if (!m_bBleedingEffect && m_bIsBleeding && m_DamageManager.GetState() != EDamageState.DESTROYED)
			CreateEffectOverTime(true);

		m_bBleedingEffect = m_bIsBleeding;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		m_bIsBleeding = m_DamageManager.IsBleeding();
		if (m_bIsBleeding)
			return;

		ClearEffectOverTime(false);
		GetGame().GetCallqueue().Remove(CreateEffectOverTime);
		GetGame().GetCallqueue().Remove(ClearEffectOverTime);

		m_bBleedingEffect = false;
		SCR_LogitechLEDManager.ActivateState(ELogitechLEDState.DEFAULT);
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnSuspended()
	{
		m_bPlayHeartBeat = false;
	}

	//------------------------------------------------------------------------------------------------
	protected override void DisplayOnResumed()
	{
		m_bPlayHeartBeat = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateEffectOverTime(bool repeat)
	{
		if (!m_wBloodEffect1 || !m_wBloodEffect2 || !m_BloodHZ)
			return;

		float effectStrength = 1;
		const float REDUCEDSTRENGTH = 0.7;
		bool playHeartBeat = m_bPlayHeartBeat;

		if (m_BloodHZ.GetTotalBleedingAmount() < 5)
		{
			effectStrength *= REDUCEDSTRENGTH;
			playHeartBeat = false;
		}

		m_wBloodEffect1.SetSaturation(1);
		m_wBloodEffect2.SetSaturation(1);

		UpdateEffectVisibility(m_wBloodEffect1);
		UpdateEffectVisibility(m_wBloodEffect2);

		if (m_bEffectState)
		{
			AnimateEffectVisibility(m_wBloodEffect1, effectStrength, effectStrength * 0.5, BLEEDINGEFFECT_OPACITY_FADEIN_1_DURATION, BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION);
			AnimateEffectVisibility(m_wBloodEffect2, 0, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_2_DURATION, BLEEDINGEFFECT_PROGRESSION_FADEOUT_2_DURATION);
		}
		else
		{
			AnimateEffectVisibility(m_wBloodEffect1, 0, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_2_DURATION, BLEEDINGEFFECT_PROGRESSION_FADEOUT_2_DURATION);
			AnimateEffectVisibility(m_wBloodEffect2, effectStrength, effectStrength * 0.5, BLEEDINGEFFECT_OPACITY_FADEIN_1_DURATION, BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION);
		}

		BlackoutEffect(effectStrength);

		// Play heartbeat sound
		if (playHeartBeat && m_DamageManager.GetDefaultHitZone().GetDamageState() != EDamageState.DESTROYED)
		{
			SCR_UISoundEntity.SetSignalValueStr("BloodLoss", 1 - m_BloodHZ.GetHealthScaled());
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_INJURED_PLAYERCHARACTER);
		}

		GetGame().GetCallqueue().CallLater(ClearEffectOverTime, 1000, false, repeat, m_bEffectState);
		SCR_LogitechLEDManager.ActivateState(ELogitechLEDState.BLEEDING);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearEffectOverTime(bool repeat)
	{
		m_bEffectState = !m_bEffectState;

		AnimateEffectVisibility(m_wBloodEffect1, 0, 0, BLEEDINGEFFECT_PROGRESSION_FADEOUT_1_DURATION, BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION);
		AnimateEffectVisibility(m_wBloodEffect2, 0, 0, BLEEDINGEFFECT_PROGRESSION_FADEOUT_1_DURATION, BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION);

		BlackoutEffect(0);

		if (repeat && m_bBleedingEffect)
			GetGame().GetCallqueue().CallLater(CreateEffectOverTime, BLEEDING_REPEAT_DELAY, false, repeat);
	}

	//------------------------------------------------------------------------------------------------
	protected void BlackoutEffect(float effectStrength)
	{
		if (!m_wBlackOut)
			return;

		if (effectStrength > 0)
		{
			m_wBlackOut.SetOpacity(1);
			effectStrength *= BLACKOUT_OPACITY_MULTIPLIER;
			AnimateWidget.AlphaMask(m_wBlackOut, effectStrength, BLEEDINGEFFECT_PROGRESSION_FADEIN_1_DURATION);
		}
		else
		{
			AnimateWidget.AlphaMask(m_wBlackOut, 0, BLEEDINGEFFECT_OPACITY_FADEOUT_1_DURATION);
		}

		UpdateEffectVisibility(m_wBlackOut);
	}

	//------------------------------------------------------------------------------------------------
	protected override void ClearEffects()
	{
		m_bBleedingEffect = false;

		if (m_wBloodEffect1)
			HideSingleEffect(m_wBloodEffect1);

		if (m_wBloodEffect2)
			HideSingleEffect(m_wBloodEffect2);

		if (m_wBlackOut)
			HideSingleEffect(m_wBlackOut);

		GetGame().GetCallqueue().Remove(CreateEffectOverTime);
		GetGame().GetCallqueue().Remove(ClearEffectOverTime);

		if (!m_DamageManager)
			return;

		m_DamageManager.GetOnDamageEffectAdded().Remove(OnDamageEffectAdded);
		m_DamageManager.GetOnDamageEffectRemoved().Remove(OnDamageEffectRemoved);
	}
}
