//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// TODO: Move enum definitions to separate file possibly in Damage folder
enum ECharacterDamageState: EDamageState
{
	WOUNDED = 3
};

enum ECharacterHealthState: EDamageState
{
	MODERATE = 3,
	SERIOUS = 4,
	CRITICAL = 5
};

enum ECharacterBloodState: EDamageState
{
	STRONG = 3,
	FAINTING = 4,
	UNCONSCIOUS = 5
};

enum ECharacterResilienceState : EDamageState
{
	WEAKENED = 3,
	FAINTING = 4,
	UNCONSCIOUS = 5
};

enum EBandagingAnimationBodyParts
{
	Invalid = 0,
	UpperHead = 1,
	LowerHead = 2,
	UpperTorso = 3,
	LowerTorso = 4,
	LeftHand = 5,
	RightHand = 6,
	LeftLeg = 7,
	RightLeg = 8
};

enum SCR_EForcedWoundedSubmesh
{
	DISABLED = 0,
	FORCED_WOUNDED = 1,
	FORCED_NOT_WOUNDED = 2,
};

//---- REFACTOR NOTE END ----

class SCR_BleedingHitZoneParameters : Managed
{
	void SCR_BleedingHitZoneParameters(
		HitZone hitZone,
		float DamageOverTime	
	)
	{
		m_hHitZone = hitZone;
		m_fDamageOverTime = DamageOverTime;
	}
	HitZone m_hHitZone;
	float m_fDamageOverTime;
};

//------------------------------------------------------------------------------------------------
class SCR_RegeneratingHitZone : SCR_HitZone
{
	[Attribute(defvalue: "200", desc: "Time required to fully regenerate this hit zone\n[s]")]
	protected float m_fFullRegenerationTime;
	
	protected bool m_bAutoRegenOnHealthSet = true;
	
	//-----------------------------------------------------------------------------------------------------------
	/*!
	Returns the desired regeneration speed in damage per second for this hitzone
	\param considerRegenScale Consider the Gamemaster tweaked modifier for the regenerationDPS in this getter
	*/
	float CalculatePassiveRegenDPS(bool considerRegenScale = true)
	{
		if (m_fFullRegenerationTime <= 0)
			return 0;
		
		if (considerRegenScale)
		{
			SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
			if (damageMgr && damageMgr.s_HealthSettings)
			{
				if (SCR_CharacterResilienceHitZone.Cast(this))
					return ((GetMaxHealth() / m_fFullRegenerationTime) * damageMgr.s_HealthSettings.GetResilienceHzRegenScale());
			
				return (GetMaxHealth() / m_fFullRegenerationTime) * damageMgr.GetRegenScale();
			}
		}
		
		return GetMaxHealth() / m_fFullRegenerationTime;
	}
		
	//-----------------------------------------------------------------------------------------------------------
	//! Get current total damage per second of this hitZone
	float GetHitZoneDamageOverTime(EDamageType targetDamageType)
	{
		array<ref SCR_PersistentDamageEffect> effects = {};
		float dot;
		
		// if damageManager is not cast, throw assert
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (damageMgr == null)
			return 0;
		
		damageMgr.FindDamageEffectsOnHitZone(this, effects);
		
		// if wanted dps is physical hitZone passive regen, return the exact value. If the regen is still in delaytimer, return 0
		if (targetDamageType == EDamageType.REGENERATION && SCR_CharacterHitZone.Cast(this))
		{
			array<ref SCR_PersistentDamageEffect> persistentEffects = damageMgr.GetAllPersistentEffectsOfType(SCR_PhysicalHitZonesRegenDamageEffect);
			if (!persistentEffects.IsEmpty())
			{
				SCR_PhysicalHitZonesRegenDamageEffect regenDamageEffect = SCR_PhysicalHitZonesRegenDamageEffect.Cast(persistentEffects[0]);
				if (regenDamageEffect.DelayTimerRanOut())
					return CalculatePassiveRegenDPS();
				else
					return 0;
			}
		}
		
		// if wanted dps is physical hitZone healing, add defaultHitZone to checked hitZones
		if (SCR_CharacterHitZone.Cast(this) && targetDamageType == EDamageType.HEALING)
			damageMgr.FindDamageEffectsOnHitZone(damageMgr.GetDefaultHitZone(), effects);
		
		foreach (SCR_PersistentDamageEffect effect : effects)
		{
			DotDamageEffect dotEffect = DotDamageEffect.Cast(effect);
			if (dotEffect && dotEffect.GetDamageType() == targetDamageType)
				dot += dotEffect.GetDPS();
		}
		
		return dot;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! HealthSet could have decreased health, so passive regeneration is planned
	override void OnHealthSet()
	{
		super.OnHealthSet();
		
		if (!m_bAutoRegenOnHealthSet)
			return;
		
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!damageMgr)
			return;
		
		// OnHealthSet, call start relevant passive regeneration depending on the hitzone type
		if (SCR_CharacterHitZone.Cast(this))
			damageMgr.RegenPhysicalHitZones();
		else
			damageMgr.RegenVirtualHitZone(this);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Permanently disable/enable the automatic start of regeneration on this hitzone when manually setting it's health
	//! Use this when setting health on characters that should not heal, such as scripted gameplay events.
	void ToggleHitZoneAutoRegenOnHealthSet(bool doRegen)
	{
		m_bAutoRegenOnHealthSet = doRegen;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_CharacterHitZone : SCR_RegeneratingHitZone
{
	protected static const float DAMAGE_TO_BLOOD_MULTIPLIER = 15; //Const value is based on previous testing by caliber
	
	[Attribute("0.65", UIWidgets.Auto, "Multiplier of hitzone health that will be applied as bleeding damage over time when damaged")]
	protected float m_fBleedingRateScale;
	
	[Attribute()]
	protected ref array<string> m_aDamageSubmeshes;
	
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref LoadoutAreaType> m_aBleedingAreas;

	[Attribute(ECharacterHitZoneGroup.VIRTUAL.ToString(), UIWidgets.ComboBox, enumType: ECharacterHitZoneGroup)]
	protected ECharacterHitZoneGroup m_eHitZoneGroup;	
	
	[Attribute(EBandagingAnimationBodyParts.Invalid.ToString(), UIWidgets.ComboBox, enumType: EBandagingAnimationBodyParts)]
	protected EBandagingAnimationBodyParts m_eBandageAnimationBodyPart;	
	
	[Attribute(SCR_EForcedWoundedSubmesh.DISABLED.ToString(), UIWidgets.ComboBox, enumType: SCR_EForcedWoundedSubmesh)]
	protected SCR_EForcedWoundedSubmesh m_eForcedWoundedSubmesh;
	
	protected bool m_bIsWounded;
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		if (m_eForcedWoundedSubmesh != SCR_EForcedWoundedSubmesh.DISABLED)
			GetGame().GetCallqueue().CallLater(UpdateSubmeshes);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);
		
		if (this != damageContext.struckHitZone)
			return;
				
		// Request impact sound from damage manager
		bool critical = damageContext.damageType == EDamageType.FIRE || damageContext.damageValue >= GetCriticalHealthThreshold();
		
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!manager)
			return;
		
		manager.SoundHit(critical, damageContext.damageType);
		
		// FireDamage shouldn't make character look bloody
		if (damageContext.damageType == EDamageType.FIRE)
			return;
		
		float bloodAddition = damageContext.damageValue / GetMaxHealth() * DAMAGE_TO_BLOOD_MULTIPLIER;
		
		// Adding immediately some blood to the clothes - currently it's based on the damage dealt.
		manager.AddBloodToClothes(this, Math.Clamp(bloodAddition , 0, 255));
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override float ComputeEffectiveDamage(notnull BaseDamageContext damageContext, bool isDOT)
	{
		if (isDOT)
			return super.ComputeEffectiveDamage(damageContext, isDOT);
		
		BaseDamageContext hack = BaseDamageContext.Cast(damageContext.Clone());
		float damage = damageContext.damageValue * GetBaseDamageMultiplier();
		damage *= GetDamageMultiplier(damageContext.damageType);
	
		if (damage > 0)
		{
			float protectionValue = GetArmorProtectionValue(damageContext.damageType);
			float damageScale = ((damage - protectionValue) / damage);
			damageScale = Math.Max(damageScale, 0);
			
			hack.damageValue *= damageScale;
		}
		
		return super.ComputeEffectiveDamage(hack, isDOT);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Get strength of protection from armor attribute on prefab of clothing item
	float GetArmorProtectionValue(EDamageType damageType)
	{
		SCR_CharacterDamageManagerComponent damageMgr = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!damageMgr)
			return 0;

		return damageMgr.GetArmorProtection(this, damageType);
	}	
	
	//-----------------------------------------------------------------------------------------------------------
	//! Whether hitzone submeshes are hidden with clothing
	bool IsCovered()
	{
		CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetOwner().FindComponent(CharacterIdentityComponent));
		if (!identity)
			return false;
		
		foreach (string submesh: m_aDamageSubmeshes)
		{
			if (identity.IsCovered(submesh))
				return true;
		}
		
		return false;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		UpdateSubmeshes();

		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.UpdateCharacterGroupDamage(GetHitZoneGroup());
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void SetWoundedSubmesh(bool wounded)
	{
		m_bIsWounded = wounded;
		
		CharacterIdentityComponent identity = CharacterIdentityComponent.Cast(GetOwner().FindComponent(CharacterIdentityComponent));
		if (!identity)
			return;
		
		foreach (string submesh: m_aDamageSubmeshes)
			identity.SetWoundState(submesh, wounded);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! Manage wounds submeshes
	void UpdateSubmeshes()
	{
		bool isWounded;
		switch (m_eForcedWoundedSubmesh)
		{
			case SCR_EForcedWoundedSubmesh.DISABLED:
				isWounded = GetDamageStateThreshold(GetDamageState()) <= GetDamageStateThreshold(ECharacterDamageState.WOUNDED);
				break;
			
			case SCR_EForcedWoundedSubmesh.FORCED_WOUNDED:
				isWounded = true;
				break;
			
			case SCR_EForcedWoundedSubmesh.FORCED_NOT_WOUNDED:
				isWounded = false;
				break;
		}
		
		if (m_bIsWounded != isWounded)
			SetWoundedSubmesh(isWounded);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! \param[in] considerRegenScale consider the Gamemaster's tweaked modifier for the regenerationDPS
	//! \return the desired regeneration speed in damage per second for this hit zone
	float GetMaxBleedingRate()
	{
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!manager)
			return 1;
		
		float bleedingRate = m_fBleedingRateScale * GetMaxHealth();
		
		bleedingRate *= manager.GetBleedingScale();
		
		return bleedingRate;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	//! \return the absolute amount of health at which the health will be considered critical
	float GetCriticalHealthThreshold()
	{
		return GetCriticalDamageThreshold() * GetMaxHealth();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	override EHitZoneGroup GetHitZoneGroup()
	{
		return m_eHitZoneGroup;
	}	

	//-----------------------------------------------------------------------------------------------------------
	array<ref LoadoutAreaType> GetBleedingAreas()
	{
		return m_aBleedingAreas;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	EBandagingAnimationBodyParts GetBodyPartToHeal()
	{
		return m_eBandageAnimationBodyPart;
	}
};

//-----------------------------------------------------------------------------------------------------------
//! health hitzone - Receives damage from physical hitzones
class SCR_CharacterHealthHitZone : SCR_HitZone
{
}

//-----------------------------------------------------------------------------------------------------------
//! Resilience - incapacitation or death, depending on game mode settings
class SCR_CharacterResilienceHitZone : SCR_RegeneratingHitZone
{
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamage(notnull BaseDamageContext damageContext)
	{
		super.OnDamage(damageContext);
		
		SCR_CharacterDamageManagerComponent characterDmgManager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!characterDmgManager)
			return;
		
		// passive healing on resilienceHitZone should stop when resilience is damaged
		array<ref SCR_PersistentDamageEffect> effects = {};
		characterDmgManager.FindAllDamageEffectsOfTypeOnHitZone(SCR_PassiveHitZoneRegenDamageEffect, this, effects);
		foreach (SCR_PersistentDamageEffect effect : effects)
		{
			effect.Terminate();
		}
		
		characterDmgManager.RegenVirtualHitZone(this);
	}
		
	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		
		UpdateConsciousness();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateConsciousness()
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (damageManager)
			damageManager.UpdateConsciousness();
	}
		
	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		SCR_CharacterDamageManagerComponent characterDamageManager = SCR_CharacterDamageManagerComponent.Cast(pManagerComponent);
		if (characterDamageManager)
			characterDamageManager.SetResilienceHitZone(this);
	}
};

//-----------------------------------------------------------------------------------------------------------
//! Blood - does not receive damage directly, only via scripted events.
class SCR_CharacterBloodHitZone : SCR_RegeneratingHitZone
{
	ref map<SCR_CharacterHitZone, ref SCR_BleedingHitZoneParameters> m_mHitZoneDOTMap;
	ref array<float> m_aHitZoneGroupBleedings;

	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		
		UpdateConsciousness();
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void UpdateConsciousness()
	{
		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (damageManager)
			damageManager.UpdateConsciousness();
	}

	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.SetBloodHitZone(this);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	float GetTotalBleedingAmount()
	{
		float dps;
		
		SCR_ExtendedDamageManagerComponent extendedDmgManager = SCR_ExtendedDamageManagerComponent.Cast(GetHitZoneContainer());
		if (!extendedDmgManager)
			return dps;
		
		DotDamageEffect dotEffect;
		array<ref SCR_PersistentDamageEffect> bleedingEffects = extendedDmgManager.GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect);
		foreach (SCR_PersistentDamageEffect effect : bleedingEffects)
		{
			dotEffect = DotDamageEffect.Cast(effect);
			if (!dotEffect)
				continue;
			
			dps += dotEffect.GetDPS();
		}
		
		return dps;
	}
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//-----------------------------------------------------------------------------------------------------------
	bool AddBleedingHZToMap(SCR_CharacterHitZone hitZone, SCR_BleedingHitZoneParameters localBleedingHZParams)
	{
		if (!m_mHitZoneDOTMap)
			m_mHitZoneDOTMap = new map<SCR_CharacterHitZone, ref SCR_BleedingHitZoneParameters>();

		if (m_mHitZoneDOTMap.Contains(hitZone))
		{
			m_mHitZoneDOTMap.Set(hitZone, localBleedingHZParams);
			return true;
		}

		return m_mHitZoneDOTMap.Insert(hitZone, localBleedingHZParams);
	}
	
	//-----------------------------------------------------------------------------------------------------------
	void RemoveBleedingHZFromMap(SCR_CharacterHitZone hitZone)
	{
		if (!m_mHitZoneDOTMap)
			return;
		
		m_mHitZoneDOTMap.Remove(hitZone);
		
		if (m_mHitZoneDOTMap.IsEmpty())
			m_mHitZoneDOTMap = null;
	}
	
	//-----------------------------------------------------------------------------------------------------------
	map<SCR_CharacterHitZone, ref SCR_BleedingHitZoneParameters> GetHitZoneDOTMap()
	{
		return m_mHitZoneDOTMap;
	}
	//---- REFACTOR NOTE END ----
};

//-----------------------------------------------------------------------------------------------------------
class SCR_CharacterHeadHitZone : SCR_CharacterHitZone
{
	//-----------------------------------------------------------------------------------------------------------
	override void OnInit(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.OnInit(pOwnerEntity, pManagerComponent);
		
		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(pManagerComponent);
		if (manager)
			manager.SetHeadHitZone(this);
	}

	//-----------------------------------------------------------------------------------------------------------
	override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);
		SoundKnockout();
	}

	//-----------------------------------------------------------------------------------------------------------
	//! Interrupt screaming when shot in head
	void SoundKnockout()
	{
		if (GetDamageState() != EDamageState.DESTROYED)
			return;

		SCR_CharacterDamageManagerComponent manager = SCR_CharacterDamageManagerComponent.Cast(GetHitZoneContainer());
		if (manager)
			manager.SoundKnockout();
	}
};
