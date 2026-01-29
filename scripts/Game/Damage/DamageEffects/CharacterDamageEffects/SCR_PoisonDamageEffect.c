class SCR_PoisonDamageEffect : SCR_DotDamageEffect
{
	protected bool m_bIsDecaying;
	protected float m_fLocalDPSValue;

	// Delay in ms between next hit sound playback
	protected static const int SOUND_COOLDOWN = 1500;
	protected const int CRITICAL_DMG_THRESHOLD = 4;
	protected const int MAX_DMG_VALUE = 100;

	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		// same as with bleeding, this should prevent us from regenerating
		dmgManager.TerminateDamageEffectsOfType(SCR_PhysicalHitZonesRegenDamageEffect);

		array<ref SCR_PersistentDamageEffect> damageEffects = {};
		dmgManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_PoisonDamageEffect, GetAffectedHitZone(), damageEffects);

		SCR_PoisonDamageEffect poisonEffect;
		IEntity instigatorEnt = GetInstigator().GetInstigatorEntity();

		int otherMaxDuration;
		const int thisMaxDuration = GetMaxDuration();
		foreach (SCR_PersistentDamageEffect dmgEffect : damageEffects)
		{
			poisonEffect = SCR_PoisonDamageEffect.Cast(dmgEffect);
			if (!poisonEffect || poisonEffect == this)
				continue;

			if (poisonEffect.GetInstigator().GetInstigatorEntity() != instigatorEnt)
				continue;

			// combine same effects from the same entity
			poisonEffect.SetDPS(poisonEffect.GetDPS() + GetDPS());

			// is that effect supposed to end at some point?
			otherMaxDuration = poisonEffect.GetMaxDuration();
			if (otherMaxDuration > 0)
			{
				if (thisMaxDuration > 0)
					poisonEffect.SetMaxDuration(otherMaxDuration + thisMaxDuration); // if both of us are meant to expire, then lets combine that time
				else
					poisonEffect.SetMaxDuration(0); // if this new effect is not supposed to expire, then we make that old one not expire

				poisonEffect.ApplyEffect(dmgManager);
			}

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override /*static*/ bool UseBatchProcessing()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override /*static*/ void BatchData(inout SCR_BatchedDamageEffects batchedDataContainer, notnull SCR_PersistentDamageEffect effect)
	{
		if (!batchedDataContainer)
		{
			batchedDataContainer = new SCR_BatchedPoisonDamageEffects(effect.GetCustomDamageValue());
			return;
		}

		SCR_BatchedPoisonDamageEffects poisonBatch = SCR_BatchedPoisonDamageEffects.Cast(batchedDataContainer);
		if (!poisonBatch)
			return;

		poisonBatch.m_fDamageValue += effect.GetCustomDamageValue();
		poisonBatch.m_iNumberOfEffects++;
	}

	//------------------------------------------------------------------------------------------------
	override /*static*/ void BatchProcessing(notnull SCR_ExtendedDamageManagerComponent dmgManager, notnull SCR_BatchedDamageEffects batchedDataContainer, bool isAuthority)
	{
		if (!isAuthority)
			return;

		SCR_CharacterDamageManagerComponent characterDamageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!characterDamageMgr)
			return;

		SCR_BatchedPoisonDamageEffects poisonBatchData = SCR_BatchedPoisonDamageEffects.Cast(batchedDataContainer);
		if (!poisonBatchData)
			return;

		WorldTimestamp currentTs = dmgManager.GetOwner().GetWorld().GetTimestamp();
		if (!poisonBatchData.m_NextSoundEventTime)
		{
			poisonBatchData.m_NextSoundEventTime = currentTs.PlusMilliseconds(SOUND_COOLDOWN);
			return;
		}

		if (poisonBatchData.m_NextSoundEventTime.Greater(currentTs))
			return;

		poisonBatchData.m_NextSoundEventTime = currentTs.PlusMilliseconds(SOUND_COOLDOWN);
		characterDamageMgr.SynchronizedSoundEvent(Type());
	}

	//------------------------------------------------------------------------------------------------
	override bool ExecuteSynchronizedSoundPlayback(notnull SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterDamageManagerComponent characterDamageMgr = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!characterDamageMgr)
			return false;

		float damageValue = m_fLocalDPSValue;
		bool decaying = m_bIsDecaying;
		SCR_BatchedPoisonDamageEffects batchedData = SCR_BatchedPoisonDamageEffects.Cast(SCR_DamageSufferingSystem.GetInstance().GetBatchedDataOfType(dmgManager, Type()));
		if (batchedData)
		{
			damageValue = batchedData.m_fDamageValue;
			decaying = decaying && batchedData.m_iNumberOfEffects < 2;
		}

		characterDamageMgr.SoundHit(damageValue > CRITICAL_DMG_THRESHOLD && !decaying, EDamageType.INCENDIARY);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		m_fLocalDPSValue = GetDPS();
		ExecuteSynchronizedSoundPlayback(dmgManager);
		SCR_DamageSufferingSystem.GetInstance().RegisterEffect(dmgManager, this);
		SCR_PoisonScreenEffect poisonScreenEffect = SCR_PoisonScreenEffect.Cast(GetScreenEffect(dmgManager, SCR_PoisonScreenEffect));
		if (!poisonScreenEffect)
			return;

		poisonScreenEffect.OnDamageEffectAdded(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectRemoved(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		ExecuteSynchronizedSoundPlayback(dmgManager);
		SCR_DamageSufferingSystem.GetInstance().UnregisterEffect(dmgManager, this);
		SCR_PoisonScreenEffect poisonScreenEffect = SCR_PoisonScreenEffect.Cast(GetScreenEffect(dmgManager, SCR_PoisonScreenEffect));
		if (!poisonScreenEffect)
			return;

		poisonScreenEffect.OnDamageEffectRemoved(this);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] dmgManager
	//! \param[in] effectType
	//! \return
	BaseInfoDisplay GetScreenEffect(notnull SCR_ExtendedDamageManagerComponent dmgManager, typename effectType)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(dmgManager.GetOwner());
		if (!character)
			return null;

		if (SCR_PlayerController.GetLocalControlledEntity() != character)
			return null;

		PlayerController controller = GetGame().GetPlayerController();
		if (!controller)
			return null;

		SCR_HUDManagerComponent hudMgr = SCR_HUDManagerComponent.Cast(controller.FindComponent(SCR_HUDManagerComponent));
		if (!hudMgr)
			return null;

		array<BaseInfoDisplay> outInfoDisplays = {};
		hudMgr.GetInfoDisplays(outInfoDisplays);
		SCR_ScreenEffectsManager screenEffectsDisplay;
		foreach (BaseInfoDisplay display : outInfoDisplays)
		{
			if (!display)
				continue;

			screenEffectsDisplay = SCR_ScreenEffectsManager.Cast(display);
			if (!screenEffectsDisplay)
				continue;

			return screenEffectsDisplay.GetEffect(effectType);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	override float GetCustomDamageValue()
	{
		return m_fLocalDPSValue;
	}

	//------------------------------------------------------------------------------------------------
	override void RecalculateDPS(float timeSlice, notnull SCR_ExtendedDamageManagerComponent dmgManager)
	{
		const SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);

		int decayFactor = 1;
		if (m_bIsDecaying)
			decayFactor = -1;//if it shouldnt expire then we are constantly exposed to the source of the poison

		m_fLocalDPSValue = Math.Clamp(GetDPS() * (1 + decayFactor * characterDmgManager.GetPoisonBuildupFactor() * timeSlice), 0, MAX_DMG_VALUE);
		SetDPS(m_fLocalDPSValue); // poison buildup
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		timeSlice = GetAccurateTimeSlice(timeSlice);

		const bool decaying = GetMaxDuration() != 0;
		if (decaying != m_bIsDecaying)
		{
			m_bIsDecaying = decaying;
			ApplyEffect(dmgManager);
		}

		DotDamageEffectTimerToken token = UpdateTimer(timeSlice, dmgManager);
		DealCustomDot(GetAffectedHitZone(), m_fLocalDPSValue * timeSlice, token, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override bool SaveApplyEffect(ScriptBitWriter w)
	{
		const int remainingTime = Math.Ceil(GetMaxDuration() - GetCurrentDuration());

		bool decaying = remainingTime > 0;
		w.WriteBool(decaying);
		if (decaying)
			w.WriteInt(remainingTime);

		w.WriteFloat01(GetDPS() * 0.01); // divide by MAX_DMG_VALUE to compress it to the 0-1 range
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool LoadApplyEffect(ScriptBitReader r)
	{
		r.ReadBool(m_bIsDecaying);
		int remainingTime;
		if (m_bIsDecaying)
			r.ReadInt(remainingTime);

		SetMaxDuration(remainingTime); // always set it, as 0 means that this is not going expire

		r.ReadFloat01(m_fLocalDPSValue);
		m_fLocalDPSValue *= MAX_DMG_VALUE; // uncompress from 0-1
		SetDPS(m_fLocalDPSValue);

		return true;
	}
}
