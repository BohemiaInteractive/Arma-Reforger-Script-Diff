class SCR_BleedingDamageEffect : SCR_DotDamageEffect
{
	SCR_CharacterBloodHitZone m_BloodHitZone;
	int m_iColliderDescriptorIndex = -1;
	
	protected static SCR_BloodOnClothesSystem s_System;
	
	//------------------------------------------------------------------------------------------------
	protected override void HandleConsequences(SCR_ExtendedDamageManagerComponent dmgManager, DamageEffectEvaluator evaluator)
	{
		super.HandleConsequences(dmgManager, evaluator);

		evaluator.HandleEffectConsequences(this, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectAdded(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectAdded(dmgManager);

		dmgManager.TerminateDamageEffectsOfType(SCR_PhysicalHitZonesRegenDamageEffect);

		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		if (!characterDamageManager)
			return;

		m_BloodHitZone = characterDamageManager.GetBloodHitZone();

		characterDamageManager.CreateBleedingParticleEffect(GetAffectedHitZone(), m_iColliderDescriptorIndex);
		
		TryAddBleedingToCloth(dmgManager);
		
		characterDamageManager.AddBleedingToArray(GetAffectedHitZone());
	}

	//------------------------------------------------------------------------------------------------
	override void OnEffectRemoved(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnEffectRemoved(dmgManager);

		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);
		characterDamageManager.RegenPhysicalHitZones();

		HitZone hitZone = GetAffectedHitZone();

		// Don't remove particle effect if there is still a bleeding on the hitZoneGroup
		SCR_HitZone scriptedHitZone = SCR_HitZone.Cast(hitZone);

		characterDamageManager.RemoveBleedingFromArray(hitZone);

		// the rest of this function removes bleeding particle effects only if no bleeding effects remain on any hitzones in the group
		array<HitZone> hitZones = {};
		array<ref SCR_PersistentDamageEffect> effects = {};
		characterDamageManager.GetHitZonesOfGroup(scriptedHitZone.GetHitZoneGroup(), hitZones);
		
		//Unsubscribe from SCR_BloodOnClothesSystem if registered
		if (s_System)
			s_System.Unregister(this);
		
		foreach (HitZone groupHitZone : hitZones)
		{
			characterDamageManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_BleedingDamageEffect, groupHitZone, effects);
			if (!effects.IsEmpty())
				return;
			
			effects.Clear();
		}

		characterDamageManager.RemoveBleedingParticleEffect(hitZone);
	}

	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(float timeSlice, SCR_ExtendedDamageManagerComponent dmgManager)
	{
		timeSlice = GetAccurateTimeSlice(timeSlice);
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(dmgManager);

		float damageAmount = GetDPS() * timeSlice;

		//Check if bleeding hitzone also has a tourniquet on it
		SCR_CharacterHitZone affectedHitZone = SCR_CharacterHitZone.Cast(GetAffectedHitZone());
		if (affectedHitZone && characterDmgManager.GetGroupTourniquetted(affectedHitZone.GetHitZoneGroup()))
			damageAmount *= characterDmgManager.GetTourniquetStrengthMultiplier();

		DotDamageEffectTimerToken token = UpdateTimer(timeSlice, dmgManager);
		DealCustomDot(m_BloodHitZone, damageAmount, token, dmgManager);
	}

	//------------------------------------------------------------------------------------------------
	event override bool HijackDamageEffect(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		array<ref SCR_PersistentDamageEffect> persistentEffects = {};
		dmgManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_BleedingDamageEffect, GetAffectedHitZone(), persistentEffects);
		
		//if the effect is already present on the hitZone, we dont add a second one.
		if (!persistentEffects.IsEmpty())
		{
			SCR_BleedingDamageEffect actualEffect = SCR_BleedingDamageEffect.Cast(persistentEffects[0]);

			actualEffect.SetDPS(actualEffect.CalculateBleedingRate());
			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	float CalculateBleedingRate()
	{
		SCR_CharacterHitZone affectedHitZone = SCR_CharacterHitZone.Cast(GetAffectedHitZone());
		if (!affectedHitZone)
		{
			Print("SCR_BleedingDamageEffect instance without hitZone exists", LogLevel.ERROR);
			return 0;
		}

		float hitZoneDamageMultiplier = affectedHitZone.GetHealthScaled();
		return affectedHitZone.GetMaxBleedingRate() - affectedHitZone.GetMaxBleedingRate() * hitZoneDamageMultiplier;
	}
		
	//------------------------------------------------------------------------------------------------
	protected void TryAddBleedingToCloth(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		SCR_CharacterHitZone charHZ = SCR_CharacterHitZone.Cast(GetAffectedHitZone());
		if (!charHZ)
			return;
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(dmgManager.GetOwner().FindComponent(EquipedLoadoutStorageComponent));
		if (!loadoutStorage)
			return;
		
		array<ref LoadoutAreaType> bleedingAreas = charHZ.GetBleedingAreas();
		if (bleedingAreas.IsEmpty())
			return;
		
		IEntity clothEntity;
		ParametricMaterialInstanceComponent materialComponent;
		foreach (LoadoutAreaType bleedingArea : bleedingAreas)
		{
			clothEntity = loadoutStorage.GetClothFromArea(bleedingArea.Type());
			if (!clothEntity)
				continue;
			
			materialComponent = ParametricMaterialInstanceComponent.Cast(clothEntity.FindComponent(ParametricMaterialInstanceComponent));
			if (!materialComponent)
				continue;
		
			World world = GetGame().GetWorld();
			s_System = SCR_BloodOnClothesSystem.Cast(world.FindSystem(SCR_BloodOnClothesSystem));
			if (s_System)
				s_System.Register(this);
			
			// A damageEffect only needs to be registered once even if more bleedingareas are valid
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool Save(ScriptBitWriter w)
	{
		super.Save(w);

		w.WriteInt(m_iColliderDescriptorIndex);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Load(ScriptBitReader r)
	{
		super.Load(r);

		r.ReadInt(m_iColliderDescriptorIndex);
		HitZone hz = GetAffectedHitZone();
		if (!hz)
			return true;

		SCR_ExtendedDamageManagerComponent dmgManager = SCR_ExtendedDamageManagerComponent.Cast(hz.GetHitZoneContainer());
		if (dmgManager)
			OnEffectAdded(dmgManager);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override EDamageType GetDefaultDamageType()
	{
		return EDamageType.BLEEDING;
	}

	//------------------------------------------------------------------------------------------------
	override void OnDiag(SCR_ExtendedDamageManagerComponent dmgManager)
	{
		super.OnDiag(dmgManager);

		string text;
		text += text.Format("  ColliderIndex: %1 \n", m_iColliderDescriptorIndex);
		DbgUI.Text(text);
	}
}
