enum ECharacterHitZoneGroup : EHitZoneGroup
{
	HEAD = 10,
	UPPERTORSO = 20,
	LOWERTORSO = 30,
	LEFTARM = 40,
	RIGHTARM = 50,
	LEFTLEG = 60,
	RIGHTLEG = 70,
}

//------------------------------------------------------------------------------------------------
class SCR_CharacterDamageManagerComponentClass: SCR_ExtendedDamageManagerComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CharacterDamageManagerComponent : SCR_ExtendedDamageManagerComponent
{
	// 1000 ms timer for bloody clothes update
	static const int BLOOD_CLOTHES_UPDATE_PERIOD = 1000;
	
	// bleeding rate multiplier after death - used to stop particles sooner
	const float DEATH_BLEEDOUT_SCALE = 4;

	//equivalent of colliding with s105 while it is moving at around ~3.5m/s (~10km/h)
	protected const int MIN_OTHER_MOMENTUM = 3000;
	
	// time for resetting minimum impulse. 
	protected const int MINIMUM_IMPULSE_RESET_TIME = 2500;
	
	// Physics variables
	protected float m_fHighestContact = 0;
	protected float m_fMinImpulse;
	protected float m_fWaterFallDamageMultiplier = 0.33;
	protected int m_fMinWaterFallDamageVelocity = 10;

	// Static array for all limbs
	static ref array<ECharacterHitZoneGroup> LIMB_GROUPS;
	static const ref array<ECharacterHitZoneGroup> EXTREMITY_LIMB_GROUPS = {ECharacterHitZoneGroup.LEFTARM, ECharacterHitZoneGroup.RIGHTARM, ECharacterHitZoneGroup.LEFTLEG, ECharacterHitZoneGroup.RIGHTLEG};
	
	//replicated arrays for clients
	protected ref array<ECharacterHitZoneGroup> m_aTourniquettedGroups;
	protected ref array<ECharacterHitZoneGroup> m_aSalineBaggedGroups;
	protected ref array<int> m_aBeingHealedGroup;
	protected ref array<float> m_aGroupBleedingRates;
	
	protected ref map<SCR_CharacterHitZone, ref SCR_ArmoredClothItemData> m_mClothItemDataMap;
	protected ref map<HitZone, ParticleEffectEntity> m_mBleedingParticles;
	protected ref array<HitZone> m_aBleedingHitZones;
	protected SCR_CharacterBloodHitZone m_pBloodHitZone;
	protected SCR_CharacterResilienceHitZone m_pResilienceHitZone;
	protected SCR_CharacterHeadHitZone m_pHeadHitZone;

	// audio
	protected SCR_CommunicationSoundComponent m_CommunicationSound;

	static SCR_GameModeHealthSettings s_HealthSettings;
 	
	protected bool m_bDOTScaleChangedByGM;
	protected bool m_bRegenScaleChangedByGM;
	protected bool m_bUnconsciousnessSettingsChangedByGM;
	protected bool m_bOverrideCharacterMedicalGMAttribute;
	
	// TODO: Move these attributes to prefab data to save some memory
	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourceNamePicker, desc: "Bleeding particle effect", params: "ptc", precision: 3, category: "Bleeding")]
	protected ResourceName m_sBleedingParticle;

	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Bleeding particle effect rate scale", params: "0 5 0.001", precision: 3, category: "Bleeding")]
	protected float m_fBleedingParticleRateScale;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001", precision: 3, category: "Bleeding")]
	protected float m_fDOTScale;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001", precision: 3, category: "Regeneration")]
	protected float m_fRegenScale;
	
	[Attribute("0.3", UIWidgets.Auto, "Resilience regeneration scale while unconscious\n[x * 100%]")]
	protected float m_fUnconsciousRegenerationScale;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed", category: "Unconsciousness")]
	protected bool m_bPermitUnconsciousness;
	
	[Attribute(defvalue: "0.05", uiwidget: UIWidgets.Slider, desc: "Affects how much the bleeding is reduced", params: "0 1 0.001", precision: 3, category: "Bleeding")]
	protected float m_fTourniquetStrengthMultiplier;	
	
	[Attribute(defvalue: "4", uiwidget: UIWidgets.EditBox, desc: "When higher, less damage is needed to reach max aimingDamage", category: "Damage penalties")]
	protected float m_fAimingDamageMultiplier;	
	
	[Attribute(defvalue: "3", uiwidget: UIWidgets.EditBox, desc: "When higher, less damage is needed to reach m_fMaxMovementDamage", category: "Damage penalties")]
	protected float m_fMovementDamageMultiplier;	
	
	[Attribute(defvalue: "0.8", uiwidget: UIWidgets.EditBox, desc: "Maximum movement damage. 1 Means max-damage/immobilized", category: "Damage penalties")]
	protected float m_fMaxMovementDamage;
	
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Allow this character to show bleeding effects on any clothing items when damaged or bleeding", category: "Bleeding")]
	protected bool m_bAllowBloodyClothes;

	[Attribute(defvalue: "0.1", desc: "Poison buildup speed factor", params: "0.01 1 0.01")]
	protected float m_fPoisonBuildupFactor;

	//-----------------------------------------------------------------------------------------------------------
	float GetPoisonBuildupFactor()
	{
		return m_fPoisonBuildupFactor;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Check whether character health state meets requirements for consciousness
	protected bool ShouldBeUnconscious()
	{
		HitZone bloodHZ = GetBloodHitZone();
		if (!bloodHZ)
			return false;
		
		ECharacterBloodState bloodState = bloodHZ.GetDamageState();
		if (bloodHZ.GetDamageStateThreshold(bloodState) <= bloodHZ.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS))	
			return true;
				
		HitZone resilienceHZ = GetResilienceHitZone();
		if (!resilienceHZ)
			return false;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return false;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return false;

		ECharacterResilienceState resilienceState = resilienceHZ.GetDamageState();

		if (controller.IsUnconscious())
		{		
			if (resilienceHZ.GetDamageStateThreshold(resilienceState) <= resilienceHZ.GetDamageStateThreshold(ECharacterResilienceState.WEAKENED))
				return true;
		}
		else
		{
			if (resilienceHZ.GetDamageStateThreshold(resilienceState) <= resilienceHZ.GetDamageStateThreshold(ECharacterResilienceState.UNCONSCIOUS))
				return true;
		}
			
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateConsciousness()
	{
		bool unconscious = ShouldBeUnconscious();
		
		// If unconsciousness is not allowed, kill character
		// Also kill the character if the blood state is not high enough for being unconsciousness
		if (unconscious)
		{
			if (!GetPermitUnconsciousness() || (m_pBloodHitZone && m_pBloodHitZone.GetDamageState() == ECharacterBloodState.DESTROYED))
			{
				Kill(GetInstigator());
				return;
			}
			else if (s_HealthSettings && s_HealthSettings.GetIfKillIndefiniteUnconscious() && IsIndefinitelyUnconscious())
			{
				Kill(GetInstigator());
				return;
			}
		}
		
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		controller.SetUnconscious(unconscious);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Force unconsciousness regardless of health
	//! \param resilienceHealth Desired healthScaled of resilienceHZ. Must be below unconscious-threshold to work
	void ForceUnconsciousness(float resilienceHealth = 0)
	{
		if (!GetPermitUnconsciousness())
			return;
		
		HitZone resilienceHZ = GetResilienceHitZone();
		if (!resilienceHZ)
			return;
		
		resilienceHZ.HandleDamage(resilienceHZ.GetMaxHealth() - (resilienceHZ.GetMaxHealth() * resilienceHealth), EDamageType.TRUE, null);
		UpdateConsciousness();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState, bool isJIP)
	{
		if(isJIP)
			return;
				
		if (newLifeState == ECharacterLifeState.INCAPACITATED)
		{
			SoundKnockout();
		}
		else if (newLifeState == ECharacterLifeState.DEAD)
		{
			SoundDeath(previousLifeState);
			RemoveAllBleedingParticlesAfterDeath();
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Insert armor data from clothing item prefab into map, create map if null
	void InsertArmorData(notnull SCR_CharacterHitZone charHitZone, SCR_ArmoredClothItemData attributes)
	{
		if (!m_mClothItemDataMap)
			m_mClothItemDataMap = new map<SCR_CharacterHitZone, ref SCR_ArmoredClothItemData>();
		
		m_mClothItemDataMap.Insert(charHitZone, attributes);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Remove armor data from clothing item prefab from map, delete map if empty
	void RemoveArmorData(notnull SCR_CharacterHitZone charHitZone)
	{
		if (!m_mClothItemDataMap)
			return;
		
		if (m_mClothItemDataMap.Contains(charHitZone))
			m_mClothItemDataMap.Remove(charHitZone);
		
		if (m_mClothItemDataMap.IsEmpty())
			m_mClothItemDataMap = null;
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	//! If !remove, take data from prefab and insert to map as class. If remove, remove this hitZone's stored data from map
	void UpdateArmorDataMap(notnull SCR_ArmoredClothItemData armorAttr, bool remove)
	{
		foreach (string hitZoneName : armorAttr.m_aProtectedHitZones)
		{
			SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(GetHitZoneByName(hitZoneName));
			if (hitZone)
			{
				if (remove)
				{
					RemoveArmorData(hitZone);
					continue;
				}
				
				SCR_ArmoredClothItemData armoredClothItemData;
				armoredClothItemData = new SCR_ArmoredClothItemData(
					armorAttr.m_eProtection,
					armorAttr.m_aProtectedHitZones,
					armorAttr.m_aProtectedDamageTypes,
					armorAttr.m_MaterialResourceName
				);

				InsertArmorData(hitZone, armoredClothItemData);
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override GameMaterial OverrideHitMaterial(HitZone struckHitzone)
	{
		SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(struckHitzone);
		if (!charHitZone)
			return null;
		
		SCR_ArmoredClothItemData armorData = GetArmorData(charHitZone);
		if (!armorData)
			return null;
		
		if (!armorData.m_Material)
			return null;
		
		return armorData.m_Material.GetResource().ToBaseContainer();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Get data from m_mClothItemDataMap
	SCR_ArmoredClothItemData GetArmorData(notnull SCR_CharacterHitZone charHitZone)
	{
		if (!m_mClothItemDataMap)
			return null;
		
		if (m_mClothItemDataMap.Contains(charHitZone))
			return m_mClothItemDataMap.Get(charHitZone);
		
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Get protection value of particular hitZone stored on m_mClothItemDataMap
	float GetArmorProtection(notnull SCR_CharacterHitZone charHitZone, EDamageType damageType)
	{
		SCR_ArmoredClothItemData localArmorData = GetArmorData(charHitZone);
		if (!localArmorData)
			return 0;

		foreach (EDamageType protectedType : localArmorData.m_aProtectedDamageTypes)
		{
			if (damageType == protectedType)
				return localArmorData.m_eProtection;
		}
		
		return 0;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Function called from SCR_ArmorDamageManagerComponent in case player is shot in armor. Applies effects
	void ArmorHitEventEffects(float damage)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;
		
		CharacterInputContext context = controller.GetInputContext();
		if (context)
			context.SetHit(EHitReactionType.HIT_REACTION_LIGHT, 0);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Function called from SCR_ArmorDamageManagerComponent in case player is shot in armor. Applies damage
	void ArmorHitEventDamage(EDamageType type, float damage, IEntity instigator)
	{
		if (m_pResilienceHitZone)
			m_pResilienceHitZone.HandleDamage(damage, type, instigator);
	}

	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		super.OnDamageEffectAdded(dmgEffect);

		if (dmgEffect.GetAffectedHitZone().IsProxy())
			return;
		
		if (dmgEffect.GetDamageType() == EDamageType.HEALING)
			SoundHeal();
			
		if (dmgEffect.GetDamageType() == EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
		{
			if (GetState() == EDamageState.DESTROYED)
				return;
			
			//Terminate passive regeneration on bloodHZ when bleeding effect is added
			array<ref SCR_PersistentDamageEffect> effects = {};
			FindAllDamageEffectsOfTypeOnHitZone(SCR_PassiveHitZoneRegenDamageEffect, m_pBloodHitZone, effects);
			
			foreach (SCR_PersistentDamageEffect effect : effects)
			{
				effect.Terminate();
			}
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		super.OnDamageEffectRemoved(dmgEffect);

		if (dmgEffect.GetAffectedHitZone().IsProxy())
			return;
		
		EDamageType localDamageType = dmgEffect.GetDamageType();
					
		if (localDamageType == EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
		{
			//If no longer bleeding, plan blood regeneration
			if (!IsBleeding())
				RegenVirtualHitZone(m_pBloodHitZone);
		}
		
		// if a healing effect is removed, it may mean all damage was healed. So try to clear the history in this case
		if (localDamageType == EDamageType.HEALING || localDamageType == EDamageType.REGENERATION)
			TryClearDamageHistory();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hijack collisiondamage in case it's applied to defaultHZ, since that means it's falldamage.
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		// Handle falldamage. Falldamage is applied to defaultHZ, so it's propegated down to physical hitZones manually, then back up to the health HZ like normal damage.
		if (damageContext.damageType == EDamageType.COLLISION && damageContext.struckHitZone == GetDefaultHitZone())
		{
			if (damageContext.damageValue > damageContext.struckHitZone.GetDamageThreshold())
				HandleAnimatedFallDamage(damageContext.damageValue);
	
			return true;
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Method used to attempt to detach player's helmet when his head is hit
	//! \param[in] damageContext
	protected void KnockOffTheHelmet(notnull BaseDamageContext damageContext)
	{
		SCR_CharacterHeadHitZone headHZ = SCR_CharacterHeadHitZone.Cast(damageContext.struckHitZone);
		if (!headHZ)
			return;

		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;

		CharacterControllerComponent controller = character.GetCharacterController();
		if (!controller)
			return;

		SCR_InventoryStorageManagerComponent storageMgr = SCR_InventoryStorageManagerComponent.Cast(controller.GetInventoryStorageManager());
		if (!storageMgr)
			return;

		SCR_CharacterInventoryStorageComponent characterStorage = storageMgr.GetCharacterStorage();
		if (!characterStorage)
			return;

		SCR_HeadgearInventoryItemComponent hatIIC = SCR_HeadgearInventoryItemComponent.Cast(characterStorage.GetItemFromLoadoutSlot(new LoadoutHeadCoverArea()));
		if (hatIIC)
			hatIIC.DetachHelmet(damageContext.damageValue, damageContext.damageType, damageContext.hitDirection, damageContext.hitPosition, character);
	}

	//------------------------------------------------------------------------------------------------
	//! Method used by the authority to synchronize an execution of the sound playback for a damage effect
	//! \param[in] effectType for which sound playback should be executed
	//! \return true if playback was successful, otherwise false
	bool SynchronizedSoundEvent(typename effectType)
	{
		array<ref SCR_PersistentDamageEffect> persistentEffects = {};
		GetPersistentEffects(persistentEffects);
		foreach (int i, SCR_PersistentDamageEffect effect : persistentEffects)
		{
			if (effect.Type() != effectType)
				continue;

			return SynchronizedSoundEvent(effect, i);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Method used by the authority to synchronize an execution of the sound playback for a damage effect
	//! \param[in] broadcastingEffect from which sound playback will be triggered locally
	//! \param[in] effectId optionally if id is already known
	//! \return true if playback was successful, otherwise false
	bool SynchronizedSoundEvent(notnull SCR_PersistentDamageEffect broadcastingEffect, int effectId = -1)
	{
		if (effectId < 0)
		{
			array<ref SCR_PersistentDamageEffect> persistentEffects = {};
			GetPersistentEffects(persistentEffects);
			foreach (int i, SCR_PersistentDamageEffect effect : persistentEffects)
			{
				if (effect != broadcastingEffect)
					continue;
	
				effectId = i;
				break;
			}
		}

		if (effectId < 0)
			return false;
		
		if (!broadcastingEffect.ExecuteSynchronizedSoundPlayback(this))
			return false;

		Rpc(Do_SynchronizedSoundEvent, effectId);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] effectId id of the damage effect that should be used for the sound playback
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Do_SynchronizedSoundEvent(int effectId)
	{
		array<ref SCR_PersistentDamageEffect> persistentEffects = {};
		GetPersistentEffects(persistentEffects);
		foreach (int i, SCR_PersistentDamageEffect effect : persistentEffects)
		{
			if (i != effectId)
				continue;

			effect.ExecuteSynchronizedSoundPlayback(this);
			break;
		}
	}

	//------------------------------------------------------------------------------------------------
	void SoundHit(bool critical, EDamageType damageType)
	{
		// Ignore if knocked out or dead
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (!character)
			return;
		
		if (character.GetCharacterController().GetLifeState() != ECharacterLifeState.ALIVE)
			return;

		if (m_CommunicationSound)
			m_CommunicationSound.SoundEventHit(critical, damageType);
	}

	//------------------------------------------------------------------------------------------------
	//! Tell m_CommunicationSound to stop character from screaming, otherwise ignore
	void SoundKnockout()
	{
		if (m_CommunicationSound && !m_CommunicationSound.IsPlaying())
			m_CommunicationSound.SoundEventDeath(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Tell m_CommunicationSound that this character is now dead
	void SoundDeath(int previousLifestate)
	{
		bool silent;

		// Silent if killed with headshot
		if (m_pHeadHitZone && m_pHeadHitZone.GetDamageState() == EDamageState.DESTROYED)
			silent = true;

		// Silent if already knocked out
		if (previousLifestate == ECharacterLifeState.INCAPACITATED)
			silent = true;

		if (m_CommunicationSound)
			m_CommunicationSound.SoundEventDeath(silent);
	}

	//------------------------------------------------------------------------------------------------
	void SoundHeal()
	{
		// Ignore if knocked out or dead
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (character)
		{
			if (character.GetCharacterController().GetLifeState() != ECharacterLifeState.ALIVE)
				return;
		}
		
		if (m_CommunicationSound)
			m_CommunicationSound.DelayedSoundEventPriority(SCR_SoundEvent.SOUND_VOICE_PAIN_RELIEVE, SCR_ECommunicationSoundEventPriority.SOUND_PAIN_RELIEVE, SCR_CommunicationSoundComponent.DEFAULT_EVENT_PRIORITY_DELAY);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//!	Clean up blood particle sources after death
	void RemoveAllBleedingParticlesAfterDeath()
	{
		float bleedingRate;
		if (m_pBloodHitZone)
			bleedingRate = DEATH_BLEEDOUT_SCALE * m_pBloodHitZone.GetTotalBleedingAmount();

		float delay;
		if (bleedingRate > 0)
			delay = m_pBloodHitZone.GetHealth() / bleedingRate;

		// Damage over time is not simulated after character death
		GetGame().GetCallqueue().Remove(RemoveAllBleedingParticles);
		GetGame().GetCallqueue().CallLater(RemoveAllBleedingParticles, delay * 1000);
	}
	
	//! \param[in] hitZone Hitzone used to get bleeding areas to visualize
	//! \param[in] immediateBloodEffect Intensity of effect. Intensity will be rounded to nearest round number. Must be => 1
	//-----------------------------------------------------------------------------------------------------------
	void AddBloodToClothes(notnull SCR_CharacterHitZone hitZone, float immediateBloodEffect)
	{
		if (!m_bAllowBloodyClothes || immediateBloodEffect < 1)
			return;
		
		EquipedLoadoutStorageComponent loadoutStorage = EquipedLoadoutStorageComponent.Cast(GetOwner().FindComponent(EquipedLoadoutStorageComponent));
		if (!loadoutStorage)
			return;
		
		array<ref LoadoutAreaType> bleedingAreas = hitZone.GetBleedingAreas();
		
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
			
			float newBloodValue = materialComponent.GetUserParam2() + Math.Ceil(immediateBloodEffect * 0.1);
			materialComponent.SetUserParam2(Math.Clamp(newBloodValue, 1, 255));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeHealed(bool ignoreHealingDOT = true)
	{
		array<ref SCR_PersistentDamageEffect> effects = {};
		GetPersistentEffects(effects);
		
		// if there is any physical damage or a bleeding, you can be healed
		array<HitZone> hitZones = {};
		GetAllHitZonesInHierarchy(hitZones);
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
				return true;
		}
		
		if (IsBleeding())
			return true;
		
		// if there is no physical damage or bleeding, healing DOT could still classify you as injured if ignoreHealingDOT is false
		if (ignoreHealingDOT)
			return false;
		
		if (IsDamageEffectPresent(SCR_PhysicalHitZonesRegenDamageEffect) || IsDamageEffectPresent(SCR_PassiveHitZoneRegenDamageEffect))
			return true;
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! will fully heal the character 
	//! \param ignoreHealingDOT Enabling this bool will remove all persistent damage effects, including the ones spawned by healthItems. Otherwise just bleeding is removed and regeneration effects are retained
	override void FullHeal(bool ignoreHealingDOT = true)
	{
		if (!ignoreHealingDOT)
		{
			array<ref SCR_PersistentDamageEffect> effects = {};
			GetPersistentEffects(effects);
			
			foreach(SCR_PersistentDamageEffect effect : effects)
			{
				effect.Terminate();	
			}
		}
		else
		{
			RemoveAllBleedings();
		}
		
		super.FullHeal(ignoreHealingDOT);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	protected void TryClearDamageHistory()
	{
		// Upon death, do not clear damage history. It will instead be cleared OnDelete()
		if (GetState() == EDamageState.DESTROYED)
		{
			// Dead bodies keep their history for as long as they aren't despawned. This is memory expensive. To prevent it, we clear history after x seconds
			GetGame().GetCallqueue().CallLater(ClearDamageHistory, 1500);
		}
		else if (GetState() == EDamageState.UNDAMAGED)
		{
			// do not clear damage history if there are positive OR negative effects left on the character
			if (CanBeHealed(false))
				return;
			
			ClearDamageHistory();
		}
	}

	//-----------------------------------------------------------------------------------------------------------
	void SetBloodHitZone(HitZone hitZone)
	{
		m_pBloodHitZone = SCR_CharacterBloodHitZone.Cast(hitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	SCR_CharacterBloodHitZone GetBloodHitZone()
	{
		return m_pBloodHitZone;
	}
	
	//------------------------------------------------------------------------------------------------
	// Scale for effects that become worsened with blood level  
	float GetResilienceRegenScale()
	{
		if (!m_pBloodHitZone)
			return 1;

		float criticalBloodLevel = m_pBloodHitZone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS);
		if (criticalBloodLevel >= 1)
			return 0;

		float currentBloodLevel = m_pBloodHitZone.GetDamageStateThreshold(m_pBloodHitZone.GetDamageState());
		if (currentBloodLevel < criticalBloodLevel)
			return 0;

		float resilienceRegenScale = (currentBloodLevel - criticalBloodLevel) / (1 - criticalBloodLevel);
		
		// Change regen scale when unconscious
		ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
		if (character)
		{
			if (character.GetCharacterController().GetLifeState() == ECharacterLifeState.INCAPACITATED)
				resilienceRegenScale *= m_fUnconsciousRegenerationScale;
		}
		
		return resilienceRegenScale;
	}

	//-----------------------------------------------------------------------------------------------------------
	void SetBleedingScale(float rate, bool changed)
	{
		m_fDOTScale = rate;
		m_bDOTScaleChangedByGM = changed;
	}
	
	//------------------------------------------------------------------------------------------------
 	float GetBleedingScale()
 	{
		if (!s_HealthSettings || m_bDOTScaleChangedByGM)
			return m_fDOTScale;
		
		return s_HealthSettings.GetBleedingScale();
 	}		
 	
 	//-----------------------------------------------------------------------------------------------------------
	void SetRegenScale(float rate, bool changed)
	{
		m_fRegenScale = rate;
		m_bRegenScaleChangedByGM = changed;
	}
	
	//------------------------------------------------------------------------------------------------
	// Return base regen scale
 	float GetRegenScale()
 	{
		if (!s_HealthSettings || m_bRegenScaleChangedByGM)
			return m_fRegenScale;
		
		return s_HealthSettings.GetRegenScale();
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether unconsciousness is permitted. Character prefab has highest authority, then gamemaster, then gamemode configuration
 	bool GetPermitUnconsciousness()
 	{
		if (!m_bPermitUnconsciousness)
			return false;
		
		if (!s_HealthSettings || m_bUnconsciousnessSettingsChangedByGM)
			return m_bPermitUnconsciousness;
		
		return s_HealthSettings.IsUnconsciousnessPermitted();
 	}		
	
	//-----------------------------------------------------------------------------------------------------------
	//! Returns whether this character will remain unconscious with no chance to wake up without intervention
	protected bool IsIndefinitelyUnconscious(bool onlyPlayers = true)
 	{
		if (onlyPlayers && !EntityUtils.IsPlayer(GetOwner()))
			return false;
		
		if (!m_pBloodHitZone || !m_pResilienceHitZone)
			return false;
		
		// if bloodHitZone is below unconscious threshold you will not wake up without a medic
		if (m_pBloodHitZone.GetDamageState() >= ECharacterBloodState.UNCONSCIOUS)
			return true;
		
		// if unconscious and bleeding, you will not wake if your blood reaches below the ECharacterBloodState.UNCONSCIOUS treshold.
		if (IsBleeding())
		{
			float timeToWake;
			float timeToUnconscious = (m_pBloodHitZone.GetHealth() - (m_pBloodHitZone.GetDamageStateThreshold(ECharacterBloodState.UNCONSCIOUS) * m_pBloodHitZone.GetMaxHealth())) / m_pBloodHitZone.GetTotalBleedingAmount();
			
			DotDamageEffect regenEffect = DotDamageEffect.Cast(FindDamageEffectOnHitZone(SCR_PassiveHitZoneRegenDamageEffect, GetResilienceHitZone()));
			if (regenEffect)
				timeToWake = ((m_pResilienceHitZone.GetDamageStateThreshold(ECharacterResilienceState.WEAKENED) * m_pResilienceHitZone.GetMaxHealth()) - m_pResilienceHitZone.GetHealth()) / -regenEffect.GetDPS();

			if (timeToUnconscious < timeToWake)
				return true;
		}
		
		return false;
 	}	
 	
	//-----------------------------------------------------------------------------------------------------------
	void SetPermitUnconsciousness(bool permit, bool changed)
 	{
 		m_bPermitUnconsciousness = permit;
		m_bUnconsciousnessSettingsChangedByGM = changed;
 	}

	//-----------------------------------------------------------------------------------------------------------
	void SetOverrideCharacterMedicalGMAttribute(bool permit)
	{
		m_bOverrideCharacterMedicalGMAttribute = permit;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetOverrideCharacterMedicalGMAttribute()
	{
		return m_bOverrideCharacterMedicalGMAttribute;
	}

	//-----------------------------------------------------------------------------------------------------------
	void SetResilienceHitZone(HitZone hitZone)
	{
		m_pResilienceHitZone = SCR_CharacterResilienceHitZone.Cast(hitZone);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	SCR_CharacterResilienceHitZone GetResilienceHitZone()
	{
		return m_pResilienceHitZone;
	}

	//-----------------------------------------------------------------------------------------------------------
	void SetHeadHitZone(HitZone hitZone)
	{
		m_pHeadHitZone = SCR_CharacterHeadHitZone.Cast(hitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	SCR_CharacterHeadHitZone GetHeadHitZone()
	{
		return m_pHeadHitZone;
	}
		
	//-----------------------------------------------------------------------------------------------------------
	float GetTourniquetStrengthMultiplier()
	{
		return m_fTourniquetStrengthMultiplier;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Add Special contact effect to hitZone
	//! \param[1] effect Special contact effect to be added to character
	void AddSpecialContactEffect(notnull SCR_SpecialCollisionDamageEffect effect)
	{
		AddDamageEffect(effect);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Add bleeding effect to hitzone
	\param hitZone Hitzone to get bleeding rate from and add effect to
	\param colliderDescriptorIndex Collider descriptor index
	*/
	void AddBleedingEffectOnHitZone(notnull SCR_CharacterHitZone hitZone, int colliderDescriptorIndex = -1)
	{
		// This code is handled on authority only
		if (hitZone.IsProxy())
			return;
		
		// In case bleeding is started outside of normal context, full health will prevent DOT. This block will circumvent this issue
		float hitZoneDamageMultiplier = hitZone.GetHealthScaled();
		float bleedingRate = hitZone.GetMaxBleedingRate() - hitZone.GetMaxBleedingRate() * hitZoneDamageMultiplier;

		SCR_BleedingDamageEffect hitZoneBleeding = new SCR_BleedingDamageEffect();
		if (colliderDescriptorIndex == -1)
		{
			int descriptorCnt = hitZone.GetNumColliderDescriptors();
			if (descriptorCnt == 1)
				colliderDescriptorIndex = 0;
			else if (descriptorCnt > 1)
				colliderDescriptorIndex = Math.RandomInt(0, descriptorCnt - 1);
		}
			
		hitZoneBleeding.m_iColliderDescriptorIndex = colliderDescriptorIndex;
		hitZoneBleeding.SetDPS(bleedingRate * GetBleedingScale());
		hitZoneBleeding.SetMaxDuration(0);
		hitZoneBleeding.SetDamageType(EDamageType.BLEEDING);
		hitZoneBleeding.SetAffectedHitZone(hitZone);
		hitZoneBleeding.SetInstigator(GetInstigator());
		AddDamageEffect(hitZoneBleeding);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Register hitzone in array as bleeding
	void AddBleedingToArray(notnull HitZone hitZone)
	{
		if (!m_aBleedingHitZones)
			m_aBleedingHitZones = {};

		if (!m_aBleedingHitZones.Contains(hitZone))
			m_aBleedingHitZones.Insert(hitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Unregister hitzone from array
	void RemoveBleedingFromArray(notnull HitZone hitZone)
	{
		if (m_aBleedingHitZones)
		{
			m_aBleedingHitZones.RemoveItem(hitZone);
			if (m_aBleedingHitZones.IsEmpty())
				m_aBleedingHitZones = null;
		}
	}
	
	array<HitZone> GetBleedingHitZones()
	{
		return m_aBleedingHitZones;
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Called to start bleeding effect on a specificied bone
	void CreateBleedingParticleEffect(notnull HitZone hitZone, int colliderDescriptorIndex)
	{
		if (System.IsConsoleApp())
			return;
		
		// Play Bleeding particle
		if (m_sBleedingParticle.IsEmpty())
			return;

		RemoveBleedingParticleEffect(hitZone);

		// TODO: Blood traces on ground that should be left regardless of clothing, perhaps just delayed
		SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
		if (!characterHitZone || characterHitZone.IsCovered())
			return;
		
		array<HitZone> groupHitZones = {};
		GetHitZonesOfGroup(characterHitZone.GetHitZoneGroup(), groupHitZones);
		float bleedingRate;
			
		foreach (HitZone groupHitZone : groupHitZones)
		{
			SCR_RegeneratingHitZone regenHitZone = SCR_RegeneratingHitZone.Cast(groupHitZone);
			if (regenHitZone)
				bleedingRate +=	regenHitZone.GetHitZoneDamageOverTime(EDamageType.BLEEDING);
		}
		
		if (bleedingRate == 0 || m_fBleedingParticleRateScale == 0)
			return;

		// Get bone node
		vector transform[4];
		int boneIndex;
		int boneNode;
		if (!hitZone.TryGetColliderDescription(GetOwner(), colliderDescriptorIndex, transform, boneIndex, boneNode))
			return;

		// Create particle emitter
		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.Parent = GetOwner();
		spawnParams.PivotID = boneNode;
		ParticleEffectEntity particleEmitter = ParticleEffectEntity.SpawnParticleEffect(m_sBleedingParticle, spawnParams);
		if (System.IsConsoleApp())
			return;		
		
		if (!particleEmitter)
		{
			Print("Particle emitter: " + particleEmitter.ToString() + " There was a problem with creating the particle emitter: " + m_sBleedingParticle, LogLevel.WARNING);
			return;
		}

		// Track particle emitter in array
		if (!m_mBleedingParticles)
			m_mBleedingParticles = new map<HitZone, ParticleEffectEntity>;

		m_mBleedingParticles.Insert(hitZone, particleEmitter);

		// Play particles
		Particles particles = particleEmitter.GetParticles();
		if (particles)
			particles.MultParam(-1, EmitterParam.BIRTH_RATE, bleedingRate * m_fBleedingParticleRateScale);
		else
			Print("Particle: " + particles.ToString() + " Bleeding particle likely not created properly: " + m_sBleedingParticle, LogLevel.WARNING);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Remove all bleeding particle effects from all bleeding hitZones
	void RemoveAllBleedingParticles()
	{
		if (IsBleeding())
			return;
		
		HitZone hitZone;
		array<HitZone> hitZones = {};
		array<ref SCR_PersistentDamageEffect> damageEffects = GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect);
		foreach (SCR_PersistentDamageEffect effect : damageEffects)
		{
			hitZone = effect.GetAffectedHitZone();
			if (!hitZones.Contains(hitZone))
				hitZones.Insert(hitZone);
		}
		
		foreach (HitZone hz : hitZones)
		{
			RemoveBleedingParticleEffect(hz);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Get particle effect on a specified hitZone
	\param hitZone Bleeding HitZone to get particle effect for
	*/
	void RemoveBleedingParticleEffect(HitZone hitZone)
	{
		if (!m_mBleedingParticles)
			return;

		ParticleEffectEntity particleEmitter = m_mBleedingParticles.Get(hitZone);
		if (particleEmitter)
		{
			particleEmitter.StopEmission();
			m_mBleedingParticles.Remove(hitZone);
		}

		if (m_mBleedingParticles.IsEmpty())
			m_mBleedingParticles = null;
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Stop all bleeding
	void RemoveAllBleedings()
	{
		TerminateDamageEffectsOfType(SCR_BleedingDamageEffect);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Terminate all bleeding effects on every hitZone of a group
	void RemoveGroupBleeding(ECharacterHitZoneGroup charHZGroup)
	{
		if (!IsBleeding())
			return;
		
		if (charHZGroup == ECharacterHitZoneGroup.VIRTUAL)
			return;

		array<ref SCR_PersistentDamageEffect> effects = {};
		array<HitZone> hitZones = {};
		GetHitZonesOfGroup(charHZGroup, hitZones);
		
		foreach (HitZone hitZone : hitZones)
		{
			FindDamageEffectsOnHitZone(hitZone, effects);
			foreach (SCR_PersistentDamageEffect effect : effects)
			{
				if (effect.GetDamageType() == EDamageType.BLEEDING)
					TerminateDamageEffect(effect)
			}
			
			effects.Clear();
		}
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Add bleeding to a particular physical hitZone
	void AddParticularBleeding(string hitZoneName = "Chest", ECharacterDamageState intensityEnum = ECharacterDamageState.WOUNDED, float intensityFloat = -1)
	{
		SCR_CharacterHitZone targetHitZone = SCR_CharacterHitZone.Cast( GetHitZoneByName(hitZoneName));

		if (!targetHitZone || targetHitZone.IsProxy())
			return;
		
		// If someone wants to add bleeding to undamaged hitZone, return
		if (intensityEnum == ECharacterDamageState.UNDAMAGED)
			return;
		
		// if intensityFloat is unset use the enum instead
		if (intensityFloat < 0)
			targetHitZone.SetHealthScaled(targetHitZone.GetDamageStateThreshold(ECharacterDamageState.WOUNDED));
		else
			targetHitZone.SetHealthScaled(intensityFloat);

		AddBleedingEffectOnHitZone(targetHitZone);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Get a random physical hitZone
	SCR_CharacterHitZone GetRandomPhysicalHitZone(bool includeBleedingHZs = false)
	{
		array<HitZone> hitZones = {};
		array<HitZone> validHitZones = {};
		GetPhysicalHitZones(hitZones);

		foreach (HitZone hitZone: hitZones)
		{
			//Is character hz
			SCR_CharacterHitZone characterHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (!characterHitZone)
				continue;
			
			// skip over bleeding hitzones if includeBleedingHZs is true
			if (!includeBleedingHZs)
			{
				SCR_RegeneratingHitZone regenHitZone = SCR_RegeneratingHitZone.Cast(hitZone);
				if (!regenHitZone || regenHitZone.GetHitZoneDamageOverTime(EDamageType.BLEEDING))
					continue;
			}
			
			validHitZones.Insert(characterHitZone);
		}

		if (validHitZones.IsEmpty())
			return null;
		
		SCR_CharacterHitZone hitZone = SCR_CharacterHitZone.Cast(validHitZones.GetRandomElement());
		return hitZone;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Add bleeding to a random physical hitZone
	void AddRandomBleeding()
	{
		AddBleedingEffectOnHitZone(GetRandomPhysicalHitZone());
	}
	
	//-----------------------------------------------------------------------------------------------------------
	ECharacterHitZoneGroup FindAssociatedHitZoneGroup(EBandagingAnimationBodyParts bodyPartToBandage)
	{
		array<HitZone> hitZones = {};
		
		GetPhysicalHitZones(hitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : hitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (charHitZone && charHitZone.GetBodyPartToHeal() == bodyPartToBandage)
				return charHitZone.GetHitZoneGroup();		
		}
				
		return null;
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	EBandagingAnimationBodyParts FindAssociatedBandagingBodyPart(ECharacterHitZoneGroup hitZoneGroup)
	{
		array<HitZone> hitZones = {};
		
		GetPhysicalHitZones(hitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : hitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (charHitZone && charHitZone.GetHitZoneGroup() == hitZoneGroup)
				return charHitZone.GetBodyPartToHeal();		
		}
				
		return null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void GetHealingAnimHitzones(EBandagingAnimationBodyParts eBandagingAnimBodyParts, out notnull array<HitZone> GroupHitZones)
	{
		array<HitZone> allGroupedHitZones = {};
		GetAllHitZones(allGroupedHitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : allGroupedHitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (charHitZone && charHitZone.GetBodyPartToHeal() == eBandagingAnimBodyParts)
				GroupHitZones.Insert(hitZone);
		}
	}
		
	//-----------------------------------------------------------------------------------------------------------
	float GetGroupHealthScaled(ECharacterHitZoneGroup hitZoneGroup)
	{
		float totalGroupHealth;
		float hitZoneQuantity;
		
		array<HitZone> allGroupedHitZones = {};
		GetAllHitZones(allGroupedHitZones);
		SCR_CharacterHitZone charHitZone;
		
		foreach (HitZone hitZone : allGroupedHitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (!charHitZone || charHitZone.GetHitZoneGroup() != hitZoneGroup)
				continue;
			
			totalGroupHealth += charHitZone.GetHealthScaled();	
			hitZoneQuantity ++;
		}
		
		if (hitZoneQuantity <= 0)
			return 0;
		
		return totalGroupHealth / hitZoneQuantity;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override float GetGroupDamageOverTime(ECharacterHitZoneGroup hitZoneGroup, EDamageType damageType)
	{
		float totalGroupDOT;
		array<HitZone> groupHitZones = {};
		GetHitZonesOfGroup(hitZoneGroup, groupHitZones);
		DotDamageEffect dotEffect;
		
		array<ref PersistentDamageEffect> effects = {};
		foreach (HitZone hitZone : groupHitZones)
		{
			SCR_RegeneratingHitZone hz = SCR_RegeneratingHitZone.Cast(hitZone);
			if (hz)
				totalGroupDOT += hz.GetHitZoneDamageOverTime(damageType);
		}

		return totalGroupDOT;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	bool GetGroupTourniquetted(ECharacterHitZoneGroup hitZoneGroup)
	{
		return m_aTourniquettedGroups && m_aTourniquettedGroups.Contains(hitZoneGroup);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetTourniquettedGroup(ECharacterHitZoneGroup hitZoneGroup, bool setTourniquetted)
	{
		if (setTourniquetted)
		{
			if (!m_aTourniquettedGroups)
				m_aTourniquettedGroups = {};
			else if (m_aTourniquettedGroups.Contains(hitZoneGroup))
				return;
			
			m_aTourniquettedGroups.Insert(hitZoneGroup);
		}
		else
		{
			if (!m_aTourniquettedGroups || !m_aTourniquettedGroups.Contains(hitZoneGroup))
				return;
			else
				m_aTourniquettedGroups.RemoveItem(hitZoneGroup);
	
			if (m_aTourniquettedGroups.IsEmpty())
				m_aTourniquettedGroups = null;
		}
		
		UpdateCharacterGroupDamage(hitZoneGroup);
	}

	//-----------------------------------------------------------------------------------------------------------
	bool GetGroupIsBeingHealed(ECharacterHitZoneGroup hitZoneGroup)
	{
		return m_aBeingHealedGroup && m_aBeingHealedGroup.Contains(hitZoneGroup);
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupIsBeingHealed(ECharacterHitZoneGroup hitZoneGroup, bool setIsBeingHealed)
	{
		if (setIsBeingHealed)
		{
			if (!m_aBeingHealedGroup)
				m_aBeingHealedGroup = {};
			else if (m_aBeingHealedGroup.Contains(hitZoneGroup))
				return;

			m_aBeingHealedGroup.Insert(hitZoneGroup);
		}
		else
		{
			if (!m_aBeingHealedGroup || !m_aBeingHealedGroup.Contains(hitZoneGroup))
				return;
			else
				m_aBeingHealedGroup.RemoveItem(hitZoneGroup);

			if (m_aBeingHealedGroup.IsEmpty())
				m_aBeingHealedGroup = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	bool GetGroupSalineBagged(ECharacterHitZoneGroup hitZoneGroup)
	{
		return m_aSalineBaggedGroups && m_aSalineBaggedGroups.Contains(hitZoneGroup);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] hitZoneGroup
	//! \param[in] setSalineBagged
	void SetSalineBaggedGroup(ECharacterHitZoneGroup hitZoneGroup, bool setSalineBagged)
	{
		if (setSalineBagged)
		{
			if (!m_aSalineBaggedGroups)
				m_aSalineBaggedGroups = {};
			else if (m_aSalineBaggedGroups.Contains(hitZoneGroup))
				return;

			m_aSalineBaggedGroups.Insert(hitZoneGroup);
		}
		else
		{
			if (!m_aSalineBaggedGroups || !m_aSalineBaggedGroups.Contains(hitZoneGroup))
				return;
			else
				m_aSalineBaggedGroups.RemoveItem(hitZoneGroup);

			if (m_aSalineBaggedGroups.IsEmpty())
				m_aSalineBaggedGroups = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! the official and globally used way of checking if bleeding tests for any PERSISTENT damage effects of EDamageType.BLEEDING. 
	bool IsBleeding()
	{
		array<ref SCR_PersistentDamageEffect> effects = {};
		GetPersistentEffects(effects);
		foreach (SCR_PersistentDamageEffect effect : effects)
		{
			if (effect.GetDamageType() == EDamageType.BLEEDING)
				return true;
		}
		
		return false;
	}
		
	//------------------------------------------------------------------------------------------------
	HitZone GetMostDOTHitZone(EDamageType damageType, bool includeVirtualHZs = false, array<EHitZoneGroup> allowedGroups = null)
	{
		float highestDOT;
		float localDOT;
		HitZone highestDOTHitZone;
		array<HitZone> hitZones = {};
		
		if (includeVirtualHZs)
			GetAllHitZones(hitZones);
		else
			GetPhysicalHitZones(hitZones);
		
		foreach (HitZone hitZone : hitZones)
		{
			if (allowedGroups)
			{
				SCR_CharacterHitZone charHitZone = SCR_CharacterHitZone.Cast(hitZone);
				if (!charHitZone)
					continue;
				
				if (!allowedGroups.Contains(charHitZone.GetHitZoneGroup()))
					continue;
			}
			
			DotDamageEffect dotEffect;
			array<ref SCR_PersistentDamageEffect> damageEffects = {};
			FindDamageEffectsOnHitZone(hitZone, damageEffects);
			
			foreach (SCR_PersistentDamageEffect effect : damageEffects)	
			{
				if (damageType != effect.GetDamageType())
					continue;
				
				dotEffect = DotDamageEffect.Cast(effect);
				if (!dotEffect)
					continue;
				
				localDOT += dotEffect.GetDPS();
			}
			
			if (localDOT > highestDOT)
			{
				highestDOT = localDOT;
				highestDOTHitZone = hitZone;
			}
		}
		return highestDOTHitZone;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	static void GetAllLimbs(notnull out array<ECharacterHitZoneGroup> limbs)
	{
		limbs.Copy(LIMB_GROUPS);
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	static void GetAllExtremities(notnull out array<ECharacterHitZoneGroup> limbs)
	{
		limbs.Copy(EXTREMITY_LIMB_GROUPS);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateCharacterGroupDamage(ECharacterHitZoneGroup hitZoneGroup)
	{
		const float TOURNIQUETTED_LEG_DAMAGE = 0.7;
		const float TOURNIQUETTED_ARMS_DAMAGE = 0.7;
		// Updated OnDamageStateChanged, movement damage is equal to the accumulative damage of all hitZones in the limb divided by the amount of limbs of type
		if (hitZoneGroup == ECharacterHitZoneGroup.LEFTLEG || hitZoneGroup == ECharacterHitZoneGroup.RIGHTLEG)
		{
			float leftLegDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTLEG))
				leftLegDamage = TOURNIQUETTED_LEG_DAMAGE;
			else
				leftLegDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.LEFTLEG));
			
			float rightLegDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.RIGHTLEG))
				rightLegDamage = TOURNIQUETTED_LEG_DAMAGE;
			else
				rightLegDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.RIGHTLEG));

			float legDamage = (leftLegDamage + rightLegDamage) * 0.5 * m_fMovementDamageMultiplier;
			SetMovementDamage(Math.Clamp(legDamage, 0, m_fMaxMovementDamage));
			return;
		}

		if (hitZoneGroup == ECharacterHitZoneGroup.LEFTARM || hitZoneGroup == ECharacterHitZoneGroup.RIGHTARM)
		{
			float leftArmDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.LEFTARM))
				leftArmDamage = TOURNIQUETTED_ARMS_DAMAGE;
			else
				leftArmDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.LEFTARM));

			float rightArmDamage;
			if (GetGroupTourniquetted(ECharacterHitZoneGroup.RIGHTARM))
				rightArmDamage = TOURNIQUETTED_ARMS_DAMAGE;
			else
				rightArmDamage = (1 - GetGroupHealthScaled(ECharacterHitZoneGroup.RIGHTARM));

			float armDamage = (leftArmDamage + rightArmDamage) * 0.5 * m_fAimingDamageMultiplier;
			SetAimingDamage(armDamage);
		}
	}
	
	//-----------------------------------------------------------------------------------------------------------
	/*! Determine which hitZones group is taking highest DOT
	\param damageTypes Which damagetype will be checked on the hitZones for highest damage amount
	\param onlyExtremities Only compare the 4 extremities (both arms, both legs)
	\param ignoreTQdHitZones Ignore tourniquetted limbs when looking for the one most bleeding one
	*/
	ECharacterHitZoneGroup GetCharMostDOTHitzoneGroup(EDamageType damageType, bool onlyExtremities = false, bool ignoreTQdHitZones = false, bool ignoreIfBeingTreated = false)
	{
		if (!m_aBleedingHitZones)
			return null;

		array<float> DOTValues = {};
		typename groupEnum = ECharacterHitZoneGroup;
		int groupCount = groupEnum.GetVariableCount();

		for (int i; i < groupCount; i++)
			DOTValues.Insert(0);

		float highestDOT;
		array<HitZone> hitZones = {}; 
		
		GetPhysicalHitZones(hitZones);
		SCR_CharacterHitZone charHitZone;
		ECharacterHitZoneGroup group;
		
		foreach (HitZone hitZone : hitZones)
		{
			charHitZone = SCR_CharacterHitZone.Cast(hitZone);
			if (!charHitZone)
				continue;
		
			// Virtual hitZones don't count to the accumulation of bleedings
			group = charHitZone.GetHitZoneGroup();
			if (group == EHitZoneGroup.VIRTUAL)
				continue;
			
			// If only extremities are desired for checking, continue if group is not an extremity
			if (onlyExtremities && !EXTREMITY_LIMB_GROUPS.Contains(group))
				continue;
			
			float DOT = charHitZone.GetHitZoneDamageOverTime(EDamageType.BLEEDING);
			if (DOT == 0)
				continue;
			
			// GetHitZoneBleedingDPS() is unaware of tourniquet status, so it is reduced seperately here.
			if (GetGroupTourniquetted(group))
			{
				DOT *= m_fTourniquetStrengthMultiplier;
				if (ignoreTQdHitZones)
				{
					DOTValues[LIMB_GROUPS.Find(group)] = 0;
					continue;
				}
			}
			
			// if desired, bleedingHitzones that are being treated are skipped so another hitZone will be healed by this inquiry
			if (GetGroupIsBeingHealed(group))
			{
				if (ignoreIfBeingTreated)
				{
					DOTValues[LIMB_GROUPS.Find(group)] = 0;
					continue;
				}
			}
			
			DOTValues[LIMB_GROUPS.Find(group)] = DOTValues[LIMB_GROUPS.Find(group)] + DOT;
		}
		
		ECharacterHitZoneGroup mostDOTHitZoneGroup;
		
		for (int i; i < groupCount; i++)
		{
			if (DOTValues[i] > highestDOT)
			{
				highestDOT = DOTValues[i];
				mostDOTHitZoneGroup = i;
			}
		}
		
		return LIMB_GROUPS.Get(mostDOTHitZoneGroup);
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Get bone name for given hz group
	string GetBoneName(ECharacterHitZoneGroup hzGroup)
	{
		switch (hzGroup)
		{
			case ECharacterHitZoneGroup.HEAD:
			{
				return "Head";
			} break;

			case ECharacterHitZoneGroup.UPPERTORSO:
			{
				return "Spine5";
			} break;

			case ECharacterHitZoneGroup.LOWERTORSO:
			{
				return "Spine1";
			} break;

			case ECharacterHitZoneGroup.LEFTARM:
			{
				return "LeftForeArm";
			} break;

			case ECharacterHitZoneGroup.RIGHTARM:
			{
				return "RightForeArm";
			} break;

			case ECharacterHitZoneGroup.LEFTLEG:
			{
				return "LeftKnee";
			} break;

			case ECharacterHitZoneGroup.RIGHTLEG:
			{
				return "RightKnee";
			}
		}

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------ 
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SCR_BaseGameMode baseGameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (baseGameMode)
			s_HealthSettings = baseGameMode.GetGameModeHealthSettings();
		
		LIMB_GROUPS = {};
		SCR_Enum.GetEnumValues(ECharacterHitZoneGroup, LIMB_GROUPS);
		
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (!character)
			return;
		
		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(owner.FindComponent(SCR_CharacterControllerComponent));
		if (controller)
			controller.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			
		RplComponent rpl = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (rpl)
		{
			auto hz = GetDefaultHitZone();
			if (hz && !hz.IsProxy())
			{
				SCR_CharacterBuoyancyComponent charBuoyancyComp = SCR_CharacterBuoyancyComponent.Cast(owner.FindComponent(SCR_CharacterBuoyancyComponent));
				if (charBuoyancyComp)
					charBuoyancyComp.GetOnWaterEnter().Insert(OnWaterEnter);
			}
		}
		
		m_CommunicationSound = SCR_CommunicationSoundComponent.Cast(owner.FindComponent(SCR_CommunicationSoundComponent));
		
		// CallLater because GetOwner().GetPhysics() returns empty until GC finishes onPostInit
		Physics physics = owner.GetPhysics();
		GetGame().GetCallqueue().CallLater(SetExactMinImpulse, param1:owner);
		
#ifdef ENABLE_DIAG
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_LOG_PLAYER_DAMAGE,"","Log player damage","GameCode");
		
		if (System.IsCLIParam("logPlayerDamage"))
			DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_CHARACTER_LOG_PLAYER_DAMAGE, true);
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	void RegenPhysicalHitZones(bool skipRegenDelay = false)
	{
		SCR_PhysicalHitZonesRegenDamageEffect regen = new SCR_PhysicalHitZonesRegenDamageEffect();
		
		regen.SetInstigator(Instigator.CreateInstigator(GetOwner()));
		regen.SetAffectedHitZone(GetDefaultHitZone());
		regen.SetDPS(m_fRegenScale);
		regen.SetDamageType(EDamageType.REGENERATION);
		regen.SkipRegenDelay(skipRegenDelay);
		AddDamageEffect(regen);
	}	
	
	//------------------------------------------------------------------------------------------------
	void RegenVirtualHitZone(SCR_RegeneratingHitZone targetHitZone, float dps = -1, bool skipRegenDelay = false)
	{
		if (!targetHitZone)
			return;
		
		if (IsDamageEffectPresentOnHitZones(SCR_PassiveHitZoneRegenDamageEffect, {targetHitZone}))
			return;
		
		SCR_PassiveHitZoneRegenDamageEffect regen = new SCR_PassiveHitZoneRegenDamageEffect();
		
		regen.SetInstigator(Instigator.CreateInstigator(GetOwner()));
		regen.SetAffectedHitZone(targetHitZone);
		regen.SetDPS(dps);
		regen.SetDamageType(EDamageType.REGENERATION);
		regen.SkipRegenDelay(skipRegenDelay);
		AddDamageEffect(regen);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetExactMinImpulse(IEntity owner)
	{
		if (!owner)
			return;
		
		m_fMinImpulse = 80;
		Physics physics = owner.GetPhysics();
		if (physics)
			m_fMinImpulse = physics.GetMass();
	}
		
	//------------------------------------------------------------------------------------------------
	protected void OnWaterEnter()
	{
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return;

		// Don't try to apply waterFallDamage when going less than X m/s. Only consider vertical speed
		vector totalVelocity = physics.GetVelocity();
		if (Math.AbsFloat(totalVelocity[1]) < m_fMinWaterFallDamageVelocity)
			return;
		
		vector ownerTransform[4];
		physics.GetDirectWorldTransform(ownerTransform);
		
		m_fHighestContact = totalVelocity.Length() * m_fWaterFallDamageMultiplier;
		CalculateCollisionDamage(GetOwner(), null, ownerTransform[3], m_fHighestContact, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Filter out any contacts under a reasonable speed for damage
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		if (Math.AbsFloat(contact.GetRelativeNormalVelocityBefore()) <= 3)
			return false;

		// Do not recompute collisiondamage within the impulseDelay if it's lower than the highest one thus far
		float relativeNormalVelocityBefore = Math.AbsFloat(contact.GetRelativeNormalVelocityBefore());
		if (m_fHighestContact > relativeNormalVelocityBefore)
			return false;

		if (!super.FilterContact(owner, other, contact))
			return false;

		Physics otherPhys = other.GetPhysics();
		if (!otherPhys)
			return false;

		if (!otherPhys.IsDynamic())
			return false;//if player just run into a wall then lets not make his life worse

		float otherMomentum = contact.VelocityBefore2.Length() * otherPhys.GetMass();
		if (otherMomentum < MIN_OTHER_MOMENTUM)
			return false;//dont harm the player just for running into things

		Physics ownerPhys = owner.GetPhysics();
		if (!ownerPhys)
			return false;

		vector otherPositionBeforeContact  = contact.Position - (contact.VelocityBefore2.Normalized());
		vector ownerPositionBeforeContact  = contact.Position - (contact.VelocityBefore1.Normalized());
		
		vector otherDirectionToCollisionPoint = vector.Direction(otherPositionBeforeContact, contact.Position).Normalized();
		vector ownerDirectionToCollisionPoint = vector.Direction(ownerPositionBeforeContact, contact.Position).Normalized();

		float otherDot = vector.Dot(contact.VelocityBefore2.Normalized(), otherDirectionToCollisionPoint);
		if (otherDot > 0.7)
			otherDot = 1;//close enough to being direct frontal impact, thus consider it as such

		otherMomentum *= otherDot;

		float ownerDot = vector.Dot(contact.VelocityBefore1.Normalized(), ownerDirectionToCollisionPoint);
		float ownerMomentum = contact.VelocityBefore1.Length() * ownerPhys.GetMass() * ownerDot;

		float directionalMomentum = otherMomentum + ownerMomentum;

		//if momentum was not directed at the collision then it shouldnt matter, but this isnt perfect!
		//Because if you would have a 'M' shaped object moving right, then other object which would happen to be inside of it, would not receive damage because of this
		if (directionalMomentum < MIN_OTHER_MOMENTUM)
			return false;

		m_fHighestContact = relativeNormalVelocityBefore;
		
		int remainingTime = GetGame().GetCallqueue().GetRemainingTime(ResetContact);
		if (remainingTime == -1)
		{
			GetGame().GetCallqueue().CallLater(ResetContact, MINIMUM_IMPULSE_RESET_TIME, false);
		}
		else
		{
			GetGame().GetCallqueue().Remove(ResetContact);
			GetGame().GetCallqueue().CallLater(ResetContact, MINIMUM_IMPULSE_RESET_TIME, false);
		}
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetContact()
	{
		m_fHighestContact = 0;
		GetGame().GetCallqueue().Remove(ResetContact);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Calculate damage from collision event based on contact data
	//! \param owner Owner is entity that is parent of this damageManager
	//! \param other Other is the entity that collided with the owner
	//! \param contact Contact data class should contain all collisiondata needed to compute damage
	override protected void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{		
		float relativeNormalVelocityBefore = Math.AbsFloat(contact.GetRelativeNormalVelocityBefore());

		CalculateCollisionDamage(owner, other, contact.Position, relativeNormalVelocityBefore);
	}

	//------------------------------------------------------------------------------------------------
	//
	protected void CalculateCollisionDamage(IEntity owner, IEntity other, vector collisionPosition, float highestCachedImpulse = 0, bool instantUnconsciousness = true)
	{
		HitZone defaultHitZone = GetDefaultHitZone();
		if (!defaultHitZone || defaultHitZone.GetDamageState() == EDamageState.DESTROYED)
			return;

		if (instantUnconsciousness)
			ForceUnconsciousness(GetResilienceHitZone().GetDamageStateThreshold(ECharacterDamageState.STATE3));
		
		float momentumCharacterThreshold = m_fMinImpulse * 1 * Physics.KMH2MS;
		float momentumCharacterDestroy = m_fMinImpulse * 100 * Physics.KMH2MS;
		float damageScaleToCharacter = (momentumCharacterDestroy - momentumCharacterThreshold) * 0.0001;
		
		float impactMomentum = Math.AbsFloat(m_fMinImpulse * highestCachedImpulse);
		
		float damageValue = damageScaleToCharacter * (impactMomentum - momentumCharacterThreshold);
		if (damageValue <= 0)
			return;
		
		// We apply the collisiondamage only every 200 ms at most. 
		// If a bigger collision happens within this time, this collisions Contact data will overwrite the previous collision, but does not reset the remaining time until it is applied.
		const int impulseDelay = 200;
		int remainingTime = GetGame().GetCallqueue().GetRemainingTime(ApplyCollisionDamage);
		if (remainingTime == -1)
		{
			GetGame().GetCallqueue().CallLater(ApplyCollisionDamage, impulseDelay, false, other, collisionPosition, damageValue);
		}
		else
		{
			GetGame().GetCallqueue().Remove(ApplyCollisionDamage);
			GetGame().GetCallqueue().CallLater(ApplyCollisionDamage, remainingTime, false, other, collisionPosition, damageValue);
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	override void OnHandleFallDamage(EFallDamageType fallDamageType, vector velocityVector)
	{
		HitZone defaultHitZone = GetDefaultHitZone();
		if (!defaultHitZone || defaultHitZone.GetDamageState() == EDamageState.DESTROYED)
			return;

		DamageManagerComponentClass prefabData = DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		
		float damage = Math.InverseLerp(prefabData.GetFallDamageLimitNoDamage(), prefabData.GetFallDamageLimitFullDamage(), velocityVector.Length());
		if (damage <= 0)
			return;
		
		if (fallDamageType == EFallDamageType.RAGDOLL)
			HandleRagdollFallDamage(GetRandomPhysicalHitZone(true), damage);

		if (fallDamageType == EFallDamageType.LANDING)
			HandleAnimatedFallDamage(GetDefaultHitZone().GetMaxHealth() * Math.Pow(damage, 3));
		
		if (m_CommunicationSound && GetDefaultHitZone().GetDamageState() == EDamageState.DESTROYED)
			m_CommunicationSound.SoundEventDeath(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleAnimatedFallDamage(float damage)
	{
		array<HitZone> targetHitZones = {};
		GetHitZonesOfGroup(ECharacterHitZoneGroup.LEFTLEG, targetHitZones, true);
		GetHitZonesOfGroup(ECharacterHitZoneGroup.RIGHTLEG, targetHitZones, false);
		
		if (targetHitZones.IsEmpty())
			return;
		
		const float overDamageCutOff = 50;
		if (damage > overDamageCutOff)
			GetHitZonesOfGroup(ECharacterHitZoneGroup.LOWERTORSO, targetHitZones, false);
		
		damage *= 2;
		vector hitPosDirNorm[3];
		SCR_DamageContext context = new SCR_DamageContext(EDamageType.COLLISION, damage/targetHitZones.Count(), hitPosDirNorm, GetOwner(), null, Instigator.CreateInstigator(GetOwner()), null, -1, -1);
		context.damageEffect = new SCR_AnimatedFallDamageEffect();
		
		foreach (HitZone hitZone : targetHitZones)
		{
			context.struckHitZone = hitZone;
			HandleDamage(context);
		}
		
		// Give character impulse when dying from animated falldamage to make the ragdoll transition more seamless and smooth
		SCR_CharacterAnimationComponent animComp = SCR_CharacterAnimationComponent.Cast(GetOwner().FindComponent(SCR_CharacterAnimationComponent));
		if (animComp)
		{		
			float randomDirectional = Math.RandomFloat(-0.5, 0.3);
			animComp.AddRagdollEffectorDamage(Vector(randomDirectional, 1, 0), Vector(randomDirectional, -0.5, 0.3).Normalized(), 82, 15, 2);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleRagdollFallDamage(notnull HitZone contactingHitZone, float damage)
	{
		array<int> colliderIDs = {};
		contactingHitZone.GetColliderIDs(colliderIDs);
		if (colliderIDs.IsEmpty())
			return;

		vector colliderTransform[4];
		Physics physics = GetOwner().GetPhysics();

		physics.GetGeomWorldTransform(colliderIDs[0], colliderTransform);
		
		array<HitZone> targetHitZones = {};
		GetPhysicalHitZones(targetHitZones);
		GetNearestHitZones(colliderTransform[3], targetHitZones, 5);
		
		if (targetHitZones.IsEmpty())
			return;
		
		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = colliderTransform[3];
		
		SCR_DamageContext context = new SCR_DamageContext(EDamageType.COLLISION, (damage*100) / targetHitZones.Count(), hitPosDirNorm, GetOwner(), null, GetInstigator(), null, -1, -1);
		context.damageEffect = new SCR_RagdollFallDamageEffect();
		
		foreach (HitZone hitZone : targetHitZones)
		{
			context.struckHitZone = hitZone;
			HandleDamage(context);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyCollisionDamage(IEntity other, vector collisionPosition, float damageValue)
	{
		if (!GetOwner())
			return;

		HitZone defaultHitZone = GetDefaultHitZone();
		if (!defaultHitZone || defaultHitZone.GetDamageState() == EDamageState.DESTROYED)
			return;

		array<HitZone> characterHitZones = {};
		GetAllHitZones(characterHitZones);
		
		// Apply collisionDamage only to the 6 nearest hitzones to the contactpoint
		const int hitZonesReturnAmount = 6;
		GetNearestHitZones(collisionPosition, characterHitZones, hitZonesReturnAmount);
		
		Vehicle vehicle = Vehicle.Cast(other);
		if (vehicle && vehicle.GetPilot())
			other = vehicle.GetPilot();
		
		vector hitPosDirNorm[3];
		hitPosDirNorm[0] = collisionPosition;

		SCR_DamageContext context = new SCR_DamageContext(EDamageType.COLLISION, damageValue/hitZonesReturnAmount, hitPosDirNorm, GetOwner(), characterHitZones[0], Instigator.CreateInstigator(other), null, -1, -1);
		
		foreach (HitZone characterHitZone : characterHitZones)
		{
			if (characterHitZone.GetDamageState() == EDamageState.DESTROYED)
				continue;
			
			context.struckHitZone = characterHitZone;
			HandleDamage(context);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Sound event playback
	//! \param[in] soundEventName
	void PlaySoundEvent(string soundEventName)
	{
		if (soundEventName.IsEmpty())
			return;

		CharacterSoundComponent soundComp = CharacterSoundComponent.Cast(GetOwner().FindComponent(CharacterSoundComponent));
		if (!soundComp)
			return;

		soundComp.SoundEvent(soundEventName);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get nearest hitZones to the point of impact
	//! \param worldPosition Position of the point of impact
	//! \param[inout] nearestHitZones Array of hitZones which colliders' positions will be compared with worldPosition. Returns the nearest hitZones to worldPosition in descending order of distance
	//! \param hitZonesReturnAmount Amount of hitZones that are returned in nearestHitZones. IF all are wanted, set nearestHitZones.Count().
	void GetNearestHitZones(vector worldPosition, notnull inout array<HitZone> nearestHitZones, int hitZonesReturnAmount)
	{
		if (nearestHitZones.IsEmpty())
			return;
		
		array<vector> colliderDistances  = {};
		array<int> IDs = {};
		Physics physics = GetOwner().GetPhysics();
		
		vector colliderTransform[4];
		vector relativePosition;
		vector colliderDistance;
		
		foreach (HitZone hitZone : nearestHitZones)
		{
			if (!hitZone.HasColliderNodes())
				continue;
			
			IDs.Clear();
			hitZone.GetColliderIDs(IDs);
			foreach (int ID : IDs)
			{
				physics.GetGeomWorldTransform(ID, colliderTransform);
				relativePosition = worldPosition - colliderTransform[3];
				colliderDistance = {relativePosition.LengthSq(), ID, 0};
				colliderDistances.Insert(colliderDistance);
			}
		}
		
		// Order colliderDistances by distance from close to furthest
		colliderDistances.Sort();
		
		array<int> closestIDs = {};
		for (int i; i < hitZonesReturnAmount; i++)
		{
			closestIDs.InsertAt(colliderDistances[i][1], i);
		}
		
		nearestHitZones.Clear();
		GetHitZonesByColliderIDs(nearestHitZones, closestIDs);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called whenever an instigator is going to be set.
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		//If the new instigator is self and owner is unconscious, don't override it. Instigator remains cause of unconsciousness.
		if (newInstigator.GetInstigatorEntity() == GetOwner())
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(GetOwner());
			if (character && character.GetCharacterController().GetLifeState() == ECharacterLifeState.INCAPACITATED)
				return false;
		}
		
		return super.ShouldOverrideInstigator(currentInstigator, newInstigator);
	}
	
	//------------------------------------------------------------------------------------------------
	//!	Invoked when damage state changes.
	protected override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		
		if (newState == EDamageState.UNDAMAGED || newState == EDamageState.DESTROYED)
			TryClearDamageHistory();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	protected override void OnDamage(notnull BaseDamageContext damageContext)
	{		
		super.OnDamage(damageContext);

		if (damageContext.damageValue > 0)
			RegenPhysicalHitZones();

		KnockOffTheHelmet(damageContext);
		#ifdef ENABLE_DIAG	
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_CHARACTER_LOG_PLAYER_DAMAGE))
		{
			SCR_HitZone scriptedHz = SCR_HitZone.Cast(damageContext.struckHitZone);
			if (!scriptedHz)
				return;

			IEntity hzOwner = scriptedHz.GetOwner();
			if (!hzOwner)
				return;
			
			string instigatorName;
			int instigatorID = damageContext.instigator.GetInstigatorPlayerID();
			IEntity instigatorEntity = damageContext.instigator.GetInstigatorEntity();
			
			if (instigatorID > 0)
			{
				instigatorName = GetGame().GetPlayerManager().GetPlayerName(instigatorID);
			}
			else
			{
				ResourceName prefabName;
				if (instigatorEntity)
				{
					EntityPrefabData prefabData = instigatorEntity.GetPrefabData();
					if (prefabData)
						prefabName = prefabData.GetPrefabName();
				}
				
				if (prefabName.IsEmpty())
				{
					if (instigatorEntity)
					{
						instigatorName = ((instigatorEntity.GetID()).ToString());
					}
					else
					{
						instigatorName = instigatorEntity.ToString();
					}
				}
				else
				{
					TStringArray strs = new TStringArray;
					prefabName.Split("/", strs, true);
					instigatorName = ((instigatorEntity.GetID()).ToString()) + strs[strs.Count() - 1];
				}
			}
			
			string hzOwnerName;
			int hzOwnerID = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(hzOwner);
			if (hzOwnerID > 0)
			{
				hzOwnerName = GetGame().GetPlayerManager().GetPlayerName(hzOwnerID);
			}
			else
			{
				EntityPrefabData prefabData = hzOwner.GetPrefabData();
				ResourceName prefabName = prefabData.GetPrefabName();
				
				if (prefabName.IsEmpty())
				{
					hzOwnerName = ((hzOwner.GetID()).ToString());
				}
				else
				{
					TStringArray strs = new TStringArray;
					prefabName.Split("/", strs, true);
					hzOwnerName = ((hzOwner.GetID()).ToString()) + strs[strs.Count() - 1];
				}
			}
			
			if (EntityUtils.IsPlayer(instigatorEntity) || EntityUtils.IsPlayer(hzOwner))
				PrintFormat("HIT LOG: (%1) damaged (%2) - [Damage = %3, Speed = %4]", instigatorName, hzOwnerName, damageContext.damageValue, damageContext.impactVelocity);
		}
		#endif
	}

	#ifdef ENABLE_DIAG
	//-----------------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{		
		super.OnDelete(owner);
	}

	override void DiagOnlyIfPossessedByPlayerController(notnull IEntity owner)
	{
		ProcessDebug(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	void ProcessDebug(IEntity owner)
	{			
		DbgUI.Text("See: GameCode >> Hit Zones >> Player HitZones for damages");
		DbgUI.Text("Applying damage from client is NOT possible");
		DbgUI.Text("Only conventional DOT damage are shown in debug");
		DbgUI.Text("Y to damage every hitZone for 4 damage");
		DbgUI.Text("U to reduce chest health by 10");
		DbgUI.Text("I to reduce right thigh health by 10");
		DbgUI.Text("O to reduce left thigh health by 10");
		DbgUI.Text("P to add bleeding to right thigh");
		DbgUI.Text("T to add bleeding to left thigh");
		DbgUI.Text("K to reset damage effect");
	
		vector hitPosDirNorm[3];
		
		if (Debug.KeyState(KeyCode.KC_Y))
		{
			Debug.ClearKey(KeyCode.KC_Y);
			AddParticularBleeding("RThigh", intensityFloat: Math.RandomFloat(0.9, 0.99));
			AddParticularBleeding("LThigh", intensityFloat: Math.RandomFloat(0.9, 0.99));
			AddParticularBleeding("Chest", intensityFloat: Math.RandomFloat(0.9, 0.99));
			AddParticularBleeding("Head", intensityFloat: Math.RandomFloat(0.9, 0.99));
			AddParticularBleeding("LArm", intensityFloat: Math.RandomFloat(0.9, 0.99));
			AddParticularBleeding("Abdomen", intensityFloat: Math.RandomFloat(0.9, 0.99));
			AddParticularBleeding("RArm", intensityFloat: Math.RandomFloat(0.9, 0.99));
		}
		if (Debug.KeyState(KeyCode.KC_U))
		{
			Debug.ClearKey(KeyCode.KC_U);
			Print(HealHitZones(10, false, true));	
		}
		if (Debug.KeyState(KeyCode.KC_I))
		{
			Debug.ClearKey(KeyCode.KC_I);
			
			SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, 10, hitPosDirNorm, owner, GetHitZoneByName("RThigh"), Instigator.CreateInstigator(owner), null, -1, -1);
			HandleDamage(damageContext);
		}
		if (Debug.KeyState(KeyCode.KC_O))
		{
			Debug.ClearKey(KeyCode.KC_O);
			
			SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, 10, hitPosDirNorm, owner, GetHitZoneByName("LThigh"), Instigator.CreateInstigator(owner), null, -1, -1);
			HandleDamage(damageContext);
		}
		if (Debug.KeyState(KeyCode.KC_P))
		{
			Debug.ClearKey(KeyCode.KC_P);
			AddParticularBleeding("RThigh");
		}
		if (Debug.KeyState(KeyCode.KC_T))
		{
			Debug.ClearKey(KeyCode.KC_T);
			AddParticularBleeding("LThigh");
		}
		if (Debug.KeyState(KeyCode.KC_K))
		{
			Debug.ClearKey(KeyCode.KC_K);
			FullHeal();
		}
	}
#endif
};