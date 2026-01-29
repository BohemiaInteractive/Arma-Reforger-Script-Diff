class SCR_DamageManagerComponentClass : DamageManagerComponentClass
{
	[Attribute()]
	protected ref SCR_SecondaryExplosions m_SecondaryExplosions;

	[Attribute()]
	protected ref SCR_SecondaryExplosions m_SecondaryFires;

	[Attribute(category: "Repairs")]
	protected ref SCR_RepairConfig m_RepairConfig;

	//------------------------------------------------------------------------------------------------
	SCR_SecondaryExplosions GetSecondaryExplosions()
	{
		return m_SecondaryExplosions;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SecondaryExplosions GetSecondaryFires()
	{
		return m_SecondaryFires;
	}

	//------------------------------------------------------------------------------------------------
	//! Retreives burn state for provided health 
	//! \param[in] health percenatage of total health (0.0 to 1.0)
	//! \param[out] state output that will contain state that coresponds to the provided health or SCR_EBurningState.NONE if there is no such state 
	//! \return true if coresponding state was found
	bool GetBurnStateForHealth(float health, out SCR_EBurningState state)
	{
		if (!m_RepairConfig)
		{
			state = SCR_EBurningState.NONE;
			return false;
		}

		return m_RepairConfig.GetBurnStateForHealth(health, state);
	}
}

class SCR_DamageManagerComponent : DamageManagerComponent
{
	[Attribute(defvalue: SCR_EBurningState.SMOKING_LIGHT.ToString(), uiwidget: UIWidgets.ComboBox, desc: "State from which vehicle will be considered on fire thus players will be able to extinguish it", enums: ParamEnumArray.FromEnum(SCR_EBurningState))]
	protected SCR_EBurningState m_iMinimumBurningState;

	protected static const int MIN_MOMENTUM_RESPONSE_INDEX = 1;
	protected static const int MAX_MOMENTUM_RESPONSE_INDEX = 5;
	protected static const int MIN_DESTRUCTION_RESPONSE_INDEX = 6;
	protected const float SIMULATION_IMPRECISION_MULTIPLIER = 1.1;
	static const int MAX_DESTRUCTION_RESPONSE_INDEX = 10;
	static const string MAX_DESTRUCTION_RESPONSE_INDEX_NAME = "HugeDestructible";
	private static int s_iFirstFreeDamageManagerData = -1;
	protected static ref array<ref SCR_DamageManagerData> s_aDamageManagerData = {};
	
	static const ref array<EDamageType> HEALING_DAMAGE_TYPES = {EDamageType.HEALING, EDamageType.REGENERATION};

	protected int m_iTimetickInstigator = System.GetTickCount();
	protected int m_iTimeThresholdInstigatorReplacement = 180000; //180000 miliseconds = 3 minutes
	protected int m_iPlayerId = 0;

	protected int m_iDamageManagerDataIndex = -1;


	//------------------------------------------------------------------------------------------------
	//! Return hit zones with colliders assigned
	void GetPhysicalHitZones(out notnull array<HitZone> physicalHitZones)
	{
		array<HitZone> hitZones = {};
		GetAllHitZonesInHierarchy(hitZones);

		HitZoneContainerComponent container;
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone.HasColliderNodes())
			{
				physicalHitZones.Insert(hitZone);
				continue;
			}

			// Ignore own default hit zone
			container = hitZone.GetHitZoneContainer();
			if (container == null || container == this)
				continue;

			// Allow subordinate default hit zone
			if (container.GetDefaultHitZone() == hitZone)
				physicalHitZones.Insert(hitZone);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return hit zones with passive regeneration enabled
	void GetRegeneratingHitZones(out notnull array<SCR_RegeneratingHitZone> regeneratingHitZones)
	{
		array<HitZone> hitZones = {};
		GetAllHitZonesInHierarchy(hitZones);

		foreach (HitZone hitZone : hitZones)
		{
			SCR_RegeneratingHitZone regenHitZone = SCR_RegeneratingHitZone.Cast(hitZone);
			if (regenHitZone)
				regeneratingHitZones.Insert(regenHitZone);
		}
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	/*!
	Set fire rate of a flammable hitzone
	\param hitZoneIndex Index of the hitzone to set fire rate for
	\param fireRate Rate of fire to be applied
	*/
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetFireState_(int hitZoneIndex, SCR_EBurningState fireState)
	{
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);
		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(hitZones.Get(hitZoneIndex));
		if (flammableHitZone)
			flammableHitZone.SetFireState(fireState);
	}
	void RpcDo_SetFireState(int hitZoneIndex, SCR_EBurningState fireState)
	{
		Rpc(RpcDo_SetFireState_, hitZoneIndex, fireState);
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	//! No contacts for destroyed entities
	override bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{
		return GetState() != EDamageState.DESTROYED;
	}
	
	//------------------------------------------------------------------------------------------------
	// This method uses similar logic to the logic of DamageSurroundingHitzones, but not the same.
	int GetSurroundingHitzones(vector origin, Physics physics, float maxDistance, out array<HitZone> outHitzones)
	{
		array<HitZone> hitZones = {};
		outHitzones = {};
		int hitZonesCount;

		int count = GetAllHitZonesInHierarchy(hitZones);
		float maxDistanceSq = maxDistance * maxDistance; //SQUARE it for faster calculations of distance
		array<string> hitZoneColliderNames = {};

		float minDistance, currentDistance;
		int colliderCount, geomIndex;
		vector mat[4];
		for (int i = count - 1; i >= 0; i--)
		{
			minDistance = float.MAX;
			colliderCount = hitZones[i].GetAllColliderNames(hitZoneColliderNames); //The array is cleared inside the GetAllColliderNames method

			if (colliderCount == 0)
				continue;

			for (int y = colliderCount - 1; y >= 0; y--)
			{
				geomIndex = physics.GetGeom(hitZoneColliderNames[y]);
				if (geomIndex == -1)
					continue;

				physics.GetGeomWorldTransform(geomIndex, mat);
				currentDistance = vector.DistanceSq(origin, mat[3]);

				if (currentDistance < minDistance)
					minDistance = currentDistance;
			}

			if (minDistance > maxDistanceSq)
				continue;

			minDistance = Math.Sqrt(minDistance);

			hitZonesCount++;
			outHitzones.Insert(hitZones[i]);
		}

		return hitZonesCount;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\ param damageType determinese which damage multipliers are taken into account
	//! Made specifically for cases where hitzones are not parented
	*/
	float GetMinDestroyDamage(EDamageType damageType, array<HitZone> hitzones)
	{
		float damage;
		float damageMultiplier;
		HitZone defaultHitzone = GetDefaultHitZone();
		if (!IsDamageHandlingEnabled() || defaultHitzone.GetDamageMultiplier(damageType) * defaultHitzone.GetBaseDamageMultiplier() == 0)
			return -1; // invalid damage value, because this vehicle cannot be destroyed

		foreach (HitZone hitZone : hitzones)
		{
			damageMultiplier = hitZone.GetBaseDamageMultiplier() * hitZone.GetDamageMultiplier(damageType);
			if (damageMultiplier == 0)
				continue;

			damage += (hitZone.GetMaxHealth() + hitZone.GetDamageReduction()) / damageMultiplier;

			if (damage < hitZone.GetDamageThreshold())
				damage += hitZone.GetDamageThreshold();
		}

		return damage;
	}

	//------------------------------------------------------------------------------------------------
	protected notnull SCR_DamageManagerData GetScriptedDamageManagerData()
	{
		if (m_iDamageManagerDataIndex == -1)
			m_iDamageManagerDataIndex = AllocateScriptedDamageManagerData();

		return s_aDamageManagerData[m_iDamageManagerDataIndex];
	}

	//------------------------------------------------------------------------------------------------
	private int AllocateScriptedDamageManagerData()
	{
		if (s_iFirstFreeDamageManagerData == -1)
			return s_aDamageManagerData.Insert(new SCR_DamageManagerData());
		else
		{
			int returnIndex = s_iFirstFreeDamageManagerData;
			SCR_DamageManagerData data = s_aDamageManagerData[returnIndex];
			s_iFirstFreeDamageManagerData = data.m_iNextFreeIndex;
			data.m_iNextFreeIndex = -1;
			return returnIndex;
		}
	}

	//------------------------------------------------------------------------------------------------
	private void FreeScriptedDamageManagerData(int index)
	{
		s_aDamageManagerData[index].Reset();
		s_aDamageManagerData[index].m_iNextFreeIndex = s_iFirstFreeDamageManagerData;
		s_iFirstFreeDamageManagerData = index;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamage()
	{
		return GetScriptedDamageManagerData().GetOnDamage();
	}
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeAdded()
	{
		return GetScriptedDamageManagerData().GetOnDamageOverTimeAdded();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageOverTimeRemoved()
	{
		return GetScriptedDamageManagerData().GetOnDamageOverTimeRemoved();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnDamageStateChanged()
	{
		return GetScriptedDamageManagerData().GetOnDamageStateChanged();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDamage(notnull BaseDamageContext damageContext)
	{
		if (!s_aDamageManagerData.IsIndexValid(m_iDamageManagerDataIndex))
			return;

		ScriptInvoker invoker = s_aDamageManagerData[m_iDamageManagerDataIndex].GetOnDamage(false);
		if (invoker)
			invoker.Invoke(damageContext);
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: Move to Character Damage Manager, use simplified instigator tracking for everything else
	//------------------------------------------------------------------------------------------------
	/*!
	Called whenever an instigator is going to be set.
	\param currentInstigator: This damage manager's last instigator
	\param newInstigator: The new instigator for this damage manager
	\return If it returns true, newInstigator will become the new current instigator for the damage manager and it will receive kill credit.
	*/
	protected override bool ShouldOverrideInstigator(notnull Instigator currentInstigator, notnull Instigator newInstigator)
	{
		//If time difference since last instigator set is small, this kill was a suicide, and the previous instigator was an enemy, do not override
		int currentTimeTick = System.GetTickCount();

		if (m_iPlayerId == 0)
		{
			m_iPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(GetOwner());
		}

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (m_iPlayerId <= 0 || !factionManager)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}

		int newId = newInstigator.GetInstigatorPlayerID();
		int oldId = currentInstigator.GetInstigatorPlayerID();

		Faction factionKiller = factionManager.GetPlayerFaction(newId);
		if (!factionKiller)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}

		Faction factionPrevInstigator = factionManager.GetPlayerFaction(oldId);
		if (!factionPrevInstigator)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}
		Faction factionPlayer = factionManager.GetPlayerFaction(m_iPlayerId);
		if (!factionPlayer)
		{
			m_iTimetickInstigator = currentTimeTick;
			return true;
		}

		if (newId == m_iPlayerId && (currentTimeTick - m_iTimetickInstigator) < m_iTimeThresholdInstigatorReplacement && !factionPrevInstigator.IsFactionFriendly(factionPlayer))
		{
			return false;
		}

		m_iTimetickInstigator = currentTimeTick;
		return true;
	}
	//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	void ~SCR_DamageManagerComponent()
	{
		if (m_iDamageManagerDataIndex != -1)
			FreeScriptedDamageManagerData(m_iDamageManagerDataIndex);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get damage manager from given owner
	\param owner Entity to get damage manager from
	\return Damage Manager if any is found
	*/
	static SCR_DamageManagerComponent GetDamageManager(notnull IEntity owner)
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(owner);
		if (character)
			return character.GetDamageManager();

		BaseVehicle vehicle = BaseVehicle.Cast(owner);
		if (vehicle)
			return vehicle.GetDamageManager();

		return SCR_DamageManagerComponent.Cast(owner.FindComponent(SCR_DamageManagerComponent));
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get a list of all hitzones of specific hitzone group
	\param hitZoneGroup Hitzone group to get hitzones of
	\param[out] groupHitZones List of group hitzones found
	\return Count of found group hitzones
	*/
	int GetHitZonesOfGroup(EHitZoneGroup hitZoneGroup, out notnull array<HitZone> groupHitZones, bool clearArray = true)
	{
		if (clearArray)
			groupHitZones.Clear();

		array<HitZone> allHitZones = {};
		GetAllHitZonesInHierarchy(allHitZones);
		SCR_HitZone scrHitZone;

		foreach (HitZone hitzone : allHitZones)
		{
			scrHitZone = SCR_HitZone.Cast(hitzone);
			if (scrHitZone && scrHitZone.GetHitZoneGroup() == hitZoneGroup)
				groupHitZones.Insert(hitzone);
		}

		return groupHitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get a list of all hitzones of specific hitzone group
	\param hitZoneGroups List of Hitzone groups to get hitzones of
	\param[out] groupHitZones List of group hitzones found
	\return Count of found group hitzones
	*/
	int GetHitZonesOfGroups(notnull array<EHitZoneGroup> hitZoneGroups, out notnull array<HitZone> groupHitZones)
	{
		groupHitZones.Clear();

		if (hitZoneGroups.IsEmpty())
			return 0;

		array<HitZone> allHitZones = {};
		GetAllHitZonesInHierarchy(allHitZones);
		SCR_HitZone scrHitZone;

		foreach (HitZone hitzone : allHitZones)
		{
			scrHitZone = SCR_HitZone.Cast(hitzone);
			if (scrHitZone && hitZoneGroups.Contains(scrHitZone.GetHitZoneGroup()))
				groupHitZones.Insert(hitzone);
		}

		return groupHitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Get total damage over time of given hitzone group
	\param hitZoneGroup Hitzone group to get damage over time from
	\param damageType Damage type to get damage over time from
	\return Damage over time value
	*/
	float GetGroupDamageOverTime(ECharacterHitZoneGroup hitZoneGroup, EDamageType damageType)
	{
		float totalGroupDamage;
		array<HitZone> groupedHitZones = {};
		GetHitZonesOfGroup(hitZoneGroup, groupedHitZones);

		foreach (HitZone hitzone : groupedHitZones)
		{
			totalGroupDamage += hitzone.GetDamageOverTime(damageType);
		}

		return totalGroupDamage;
	}

	//------------------------------------------------------------------------------------------------
	/*! Neutralize the entity with a specific damage type, registering the killer entity.
	\param instigator Source of the damage
	*/
	void Kill(notnull Instigator instigator)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return;

		HitZone hitZone = GetDefaultHitZone();
		if (!hitZone)
			return;

		vector hitPosDirNorm[3];

		SCR_DamageContext damageContext = new SCR_DamageContext(EDamageType.TRUE, hitZone.GetMaxHealth(), hitPosDirNorm, owner, hitZone, instigator, null, -1, -1);
		HandleDamage(damageContext);
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Damage random physical hit zones up to damage amount
	\param damage Total damage to be applied
	\param type Type of damage
	\param instigator Entity that fired the projectile or otherwise caused the damage
	\param onlyPhysical Whether only physical hit zones should be damaged
	\param outMat [hitPosition, hitDirection, hitNormal]
	\param damageDefault Whether to damage default hit zone as well
	*/
	void DamageRandomHitZones(float damage, EDamageType type, notnull Instigator instigator, bool onlyPhysical = true, vector outMat[3] = {}, bool damageDefault = false)
	{
		array<HitZone> hitZones = {};

		if (onlyPhysical)
			GetPhysicalHitZones(hitZones);
		else
			GetAllHitZonesInHierarchy(hitZones);

		if (!damageDefault)
			hitZones.RemoveItem(GetDefaultHitZone());

		HitZone hitZone;
		DamageManagerComponent damageManager;
		float hitZoneDamage;

		while (damage > 0 && !hitZones.IsEmpty())
		{
			hitZone = hitZones.GetRandomElement();
			if (!hitZone)
			{
				hitZones.RemoveItem(hitZone);
				continue;
			}

			damageManager = DamageManagerComponent.Cast(hitZone.GetHitZoneContainer());
			if (!damageManager)
			{
				hitZones.RemoveItem(hitZone);
				continue;
			}

			// Check actual damage that will be dealt
			SCR_DamageContext hitZoneContext = new SCR_DamageContext(type, damage, outMat, damageManager.GetOwner(), hitZone, instigator, null, -1, -1);
			hitZoneDamage = Math.Min(hitZone.ComputeEffectiveDamage(hitZoneContext, false), hitZone.GetHealth());

			// Remove hit zones that cannot be damaged with this loop
			if (hitZoneDamage <= 0 || float.AlmostEqual(hitZoneDamage, 0))
			{
				hitZones.RemoveItem(hitZone);
				continue;
			}

			//damage is the remaining amount of damage to split between future hit zones
			damage -= hitZoneDamage;

			damageManager.HandleDamage(hitZoneContext);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if there is damage that can be repaired
	//! \param[in] ignoreHealingDOT If this is false the method will return that the entity can be healed also if the damageType is positive, such as HEALING
	bool CanBeHealed(bool ignoreHealingDOT = true)
	{
		// Any damage to default hitzone
		if (GetDefaultHitZone().GetDamageState() == EDamageState.UNDAMAGED)
			return true;

		// Check if has damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);
		
		foreach (EDamageType type : damageTypes)
		{
			if (!ignoreHealingDOT && HEALING_DAMAGE_TYPES.Contains(type))
				continue;
			
			if (IsDamagedOverTime(type))
				return true;
		}
		
		// No need to check fire on characters
		if (SCR_ChimeraCharacter.Cast(GetOwner()))
			return false;

		// Check if any hitzone is damaged
		array<HitZone> hitZones = {};
		GetAllHitZonesInHierarchy(hitZones);
		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
				return true;

			// Flammable hitzone may be smoking
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone && IsOnFire(flammableHitZone))
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Fix all the damage
	void FullHeal(bool ignoreHealingDOT = true)
	{
		array<HitZone> hitZones = {};
		GetAllHitZonesInHierarchy(hitZones);

		// Extinguish flammable hitzones
		SCR_FlammableHitZone flammableHitZone;
		foreach (HitZone hitZone : hitZones)
		{
			// Put out fire and smoke
			flammableHitZone = SCR_FlammableHitZone.Cast(hitZone);
			if (flammableHitZone)
				flammableHitZone.SetFireRate(0);
		}

		// Heal remaining damage over time
		array<EDamageType> damageTypes = {};
		SCR_Enum.GetEnumValues(EDamageType, damageTypes);

		foreach (EDamageType type : damageTypes)
		{
			// Prevent removing of healing DOTs so morphine and saline effects aren't stopped by healing action
			if (ignoreHealingDOT && (type == EDamageType.REGENERATION || type == EDamageType.HEALING))
				continue;

			if (IsDamagedOverTime(type))
				RemoveDamageOverTime(type);
		}

		// Fix hitzones
		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone && hitZone.GetDamageState() != EDamageState.UNDAMAGED)
				hitZone.SetHealthScaled(1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Partially heal the hitZones of an entity
	//! \param healthToDistribute The total value to be distributed among the hitzones
	//! \param sequential If true, apply health sequentially, if false apply health in parallel
	//! \param maxHealThresholdScaled When set, hitzone is healed only up to this threshold. Must be between 0 and 1.
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \return Returns float of health that was not distributed. Value above 0 means not all health was distributed
	float HealHitZones(float healthToDistribute, bool sequential = false, float maxHealThresholdScaled = 1, array<HitZone> alternativeHitZones = null)
	{
		if (healthToDistribute <= 0)
			return healthToDistribute;

		if (maxHealThresholdScaled < 0 || maxHealThresholdScaled > 1)
			maxHealThresholdScaled = 1;

		array<HitZone> targetHitZones = {};
		if (alternativeHitZones)
			targetHitZones.Copy(alternativeHitZones);
		else
			GetPhysicalHitZones(targetHitZones);

		if (sequential)
			healthToDistribute = HealHitZonesInSequence(healthToDistribute, maxHealThresholdScaled, targetHitZones);
		else // if in Parallel
			healthToDistribute = HealHitZonesInParallel(healthToDistribute, maxHealThresholdScaled, targetHitZones);

		ReduceSmoke();
		return healthToDistribute;
	}

	//------------------------------------------------------------------------------------------------
	protected float HealHitZonesInSequence(float healthToDistribute, float maxHealThresholdScaled, array<HitZone> targetHitZones)
	{
		foreach (HitZone hitZone : targetHitZones)
		{
			if (healthToDistribute <= 0)
				break;

			// If hitzone is undamaged, go onto the next one
			float healthToAdd = (hitZone.GetMaxHealth() * maxHealThresholdScaled) - hitZone.GetHealth();
			if (healthToAdd <= 0)
				continue;

			// If health to distribute runs out, heal hitzone with whats left and exit
			if (healthToAdd > healthToDistribute)
			{
				hitZone.HandleDamage(-healthToDistribute, EDamageType.HEALING, null);
				healthToDistribute = 0;
				break;
			}

			// Heal hitZone and substract distributable health value
			hitZone.HandleDamage(-healthToAdd, EDamageType.HEALING, null);
			healthToDistribute -= healthToAdd;
		}

		return healthToDistribute;
	}

	//------------------------------------------------------------------------------------------------
	protected float HealHitZonesInParallel(float healthToDistribute, float maxHealThresholdScaled, array<HitZone> targetHitZones)
	{
		array<HitZone> damagedHitZones = {};

		while (healthToDistribute > 0)
		{
			foreach (HitZone hitZone : targetHitZones)
			{
				if (hitZone.GetHealth() < (hitZone.GetMaxHealth() * maxHealThresholdScaled))
					damagedHitZones.Insert(hitZone);
			}

			if (damagedHitZones.IsEmpty())
				break;

			float healthToDistributeHitZone = healthToDistribute / damagedHitZones.Count();
			foreach (HitZone hitZone : damagedHitZones)
			{
				if (healthToDistribute <= 0)
					break;

				// If hitzone is undamaged, go onto the next one
				float healthToAdd = (hitZone.GetMaxHealth() * maxHealThresholdScaled) - hitZone.GetHealth();
				if (healthToAdd <= 0)
					continue;

				// If health to distribute is more than needed, apply what's needed and save whats' left
				if (healthToDistributeHitZone > healthToAdd)
				{
					hitZone.HandleDamage(-healthToAdd, EDamageType.HEALING, null);
					healthToDistribute -= healthToAdd;
					continue;
				}
				else// If health to distribute is less than needed, reduce healthToDistribute by health applied and continue
				{
					hitZone.HandleDamage(-healthToDistributeHitZone, EDamageType.HEALING, null);
					healthToDistribute -= healthToDistributeHitZone;
					continue;
				}
			}

			damagedHitZones.Clear();
		}

		return healthToDistribute;
	}

	//------------------------------------------------------------------------------------------------
	//! Reduce smoke output based on the total health of hit zones
	void ReduceSmoke()
	{
		if (IsOnFire())
			return;

		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return;

		HitZone defaultHZ = GetDefaultHitZone();
		if (!defaultHZ)
			return;

		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);

		float averageHP;
		int numberOfValidHZ;
		foreach (HitZone hz : hitZones)
		{
			if (hz == defaultHZ)
				continue;

			numberOfValidHZ++;
			averageHP += hz.GetHealthScaled();
		}

		if (numberOfValidHZ > 0)
		{
			averageHP /= numberOfValidHZ;
			if (averageHP > defaultHZ.GetHealthScaled())
				averageHP = defaultHZ.GetHealthScaled();
		}
		else
		{
			averageHP = defaultHZ.GetHealthScaled();
		}

		SCR_EBurningState desiredState;
		if (!prefabData.GetBurnStateForHealth(averageHP, desiredState))
			return;

		SCR_FlammableHitZone flammableHZ;
		foreach (HitZone hz : hitZones)
		{
			flammableHZ = SCR_FlammableHitZone.Cast(hz);
			if (!flammableHZ)
				continue;

			//make sure that we are not going to make it worse than it is now
			if (flammableHZ.GetFireState() <= desiredState)
				continue;

			flammableHZ.SetFireState(desiredState);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get all damage dealt to hitzones. Maxhealth minus current health is damage.
	//! \param untilThresholdScaled When set damage until this scaled threshold is returned.
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \return Returns float of all health missing from checked hitZones
	float GetHitZonesDamage(float untilThresholdScaled = 1, array<HitZone> alternativeHitZones = null)
	{
		array<HitZone> hitZones = {};
		if (!alternativeHitZones || alternativeHitZones.IsEmpty())
			GetPhysicalHitZones(hitZones);
		else
			hitZones.Copy(alternativeHitZones);

		if (!hitZones)
			return 0;

		float totalDamage, addedDamage;

		foreach (HitZone hitZone : hitZones)
		{
			//~ Ignore undamaged
			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
				continue;

			addedDamage = (hitZone.GetMaxHealth() * untilThresholdScaled) - hitZone.GetHealth();
			if (addedDamage > 0)
				totalDamage += addedDamage;
		}

		return totalDamage;
	}

	//------------------------------------------------------------------------------------------------
	//! Get Health scaled of all hitzones
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \return Returns float of all hitzones scaled devided by the amount of hitzones
	float GetHitZonesHealthScaled(array<HitZone> alternativeHitZones = null)
	{
		array<HitZone> hitZones = {};
		if (!alternativeHitZones || alternativeHitZones.IsEmpty())
			GetPhysicalHitZones(hitZones);
		else
			hitZones.Copy(alternativeHitZones);

		//~ No hitzones so health is full
		if (!hitZones || hitZones.IsEmpty())
			return 1;

		float totalHealthScaled;

		foreach (HitZone hitZone : hitZones)
		{
			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
			{
				totalHealthScaled += 1;
				continue;
			}

			totalHealthScaled += hitZone.GetHealthScaled();
		}

		return totalHealthScaled / hitZones.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get single Health scaled from hitzones. Will return lowest or highest value
	//! \param alternativeHitZones When filled, function will use these hitzones instead of the default, physicalHitZones
	//! \param getLowestHealth If true it will return the hitzone health that is lowest, else it will return the hitzone health that is highest
	//! \return Returns float of all hitzones scaled devided by the amount of hitzones
	float GetSingleHitZonesHealthScaled(array<HitZone> alternativeHitZones = null, bool getLowestHealth = true)
	{
		array<HitZone> hitZones = {};
		if (!alternativeHitZones || alternativeHitZones.IsEmpty())
			GetPhysicalHitZones(hitZones);
		else
			hitZones.Copy(alternativeHitZones);

		//~ No hitzones so health is full
		if (!hitZones || hitZones.IsEmpty())
			return 1;

		float returnValue = -1;
		float healthScaled;

		foreach (HitZone hitZone : hitZones)
		{
			if (returnValue < 0)
			{
				if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
					returnValue = 1;
				else
					returnValue = hitZone.GetHealthScaled();

				continue;
			}

			if (hitZone.GetDamageState() == EDamageState.UNDAMAGED)
				healthScaled = 1;
			else
				healthScaled = hitZone.GetHealthScaled();

			if ((getLowestHealth && returnValue > healthScaled) || (!getLowestHealth && returnValue < healthScaled))
				returnValue = healthScaled;
		}

		return returnValue;
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion value, type and resource type if defined
	ResourceName GetSecondaryExplosion(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES, bool fire = false, out bool hasData = false)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return ResourceName.Empty;

		SCR_SecondaryExplosions secondaries;
		if (fire)
			secondaries = prefabData.GetSecondaryFires();
		else
			secondaries = prefabData.GetSecondaryExplosions();

		if (!secondaries)
			return ResourceName.Empty;

		hasData = true;
		return secondaries.GetExplosionPrefab(value, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion scale, type and resource type if defined
	SCR_SecondaryExplosion GetSecondaryExplosionForScale(SCR_ESecondaryExplosionScale scale, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		SCR_SecondaryExplosions secondaries = prefabData.GetSecondaryFires();
		if (!secondaries)
			return null;

		return secondaries.GetSecondaryExplosionForScale(scale, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion value, type and resource type if defined
	SCR_ESecondaryExplosionScale GetSecondaryExplosionScale(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return SCR_ESecondaryExplosionScale.NONE;

		SCR_SecondaryExplosions secondaries = prefabData.GetSecondaryFires();
		if (!secondaries)
			return SCR_ESecondaryExplosionScale.NONE;

		return secondaries.GetSecondaryExplosionScale(value, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine secondary explosion prefab based on explosion value, type and resource type if defined
	ResourceName GetSecondaryFireParticle(float value, SCR_ESecondaryExplosionType explosionType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		SCR_DamageManagerComponentClass prefabData = SCR_DamageManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return ResourceName.Empty;

		SCR_SecondaryExplosions secondaries = prefabData.GetSecondaryFires();
		if (!secondaries)
			return ResourceName.Empty;

		return secondaries.GetFireParticles(value, explosionType, resourceType);
	}

	//------------------------------------------------------------------------------------------------
	//! Determine if provided or any hit zone from this damage manager is on fire
	//! \param[in] hitZone zone that will be checked if it is burning and if null then will check all hit zones
	bool IsOnFire(HitZone hitZone = null)
	{
		if (!hitZone)
			return IsOnFire(new array<HitZone>);
		
		SCR_FlammableHitZone flammableHZ = SCR_FlammableHitZone.Cast(hitZone);
		if (!flammableHZ)
			return false;

		return flammableHZ.GetFireState() >= m_iMinimumBurningState;
	}

	//------------------------------------------------------------------------------------------------
	//! Determine if any hit zone from this damage manager is on fire
	//! \param[in] hitZones list of hit zones that should be checked and if empty then all hit zones are going to be checked
	bool IsOnFire(notnull array<HitZone> hitZones)
	{
		if (hitZones.IsEmpty())
		{
			GetAllHitZones(hitZones);
			if (hitZones.IsEmpty())
				return false;
		}

		SCR_FlammableHitZone flammableHZ;
		foreach (HitZone hz : hitZones)
		{
			flammableHZ = SCR_FlammableHitZone.Cast(hz);
			if (!flammableHZ)
				continue;

			if (flammableHZ.GetFireState() >= m_iMinimumBurningState)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn secondary explosion
	void SecondaryExplosion(ResourceName prefabName, notnull Instigator instigator, notnull EntitySpawnParams spawnParams)
	{
		if (GetDefaultHitZone() && GetDefaultHitZone().IsProxy())
			return;

		Resource secondaryResource = Resource.Load(prefabName);
		if (!secondaryResource.IsValid())
			return;

		if (!spawnParams.Parent)
			spawnParams.Parent = GetOwner();

		IEntity explosion = GetGame().SpawnEntityPrefab(secondaryResource, spawnParams.Parent.GetWorld(), spawnParams);
		if (!explosion)
			return;

		// Set instigator on trigger component
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(explosion.FindComponent(BaseTriggerComponent));
		if (!trigger)
			return;

		trigger.SetInstigator(GetInstigator());
		// Ignore own vehicle
		array<IEntity> ignoreList = {GetOwner().GetRootParent()};

		// Ignore all hierarchy members as well
		SCR_EntityHelper.GetHierarchyEntityList(GetOwner().GetRootParent(), ignoreList);

		// Remove possible character entites from ignorelist
		SCR_BaseCompartmentManagerComponent baseCompartment = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (baseCompartment)
		{
			array<IEntity> vehicleOccupants = {};
			baseCompartment.GetOccupants(vehicleOccupants);
			foreach (IEntity entity : vehicleOccupants)
			{
				ignoreList.RemoveItem(entity);
			}
		}
			
		array<BaseProjectileEffect> explosionContainers = {};
		trigger.GetProjectileEffects(ExplosionDamageContainer, explosionContainers);
		foreach (BaseProjectileEffect effect : explosionContainers)
		{
			ExplosionDamageContainer.Cast(effect).SetIgnoreList(ignoreList);
		}
	}

	//------------------------------------------------------------------------------------------------
	SCR_ResourceEncapsulator GetResourceEncapsulator(EResourceType suppliesType = EResourceType.SUPPLIES)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return null;

		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		if (!resourceComponent)
			return null;

		SCR_ResourceContainer container = resourceComponent.GetContainer(suppliesType);
		if (!container)
			return null;

		return container.GetResourceEncapsulator();
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn supply secondary explosion when vehicle becomes destroyed
	void SupplySecondaryExplosion(notnull Instigator instigator)
	{
		SCR_ResourceEncapsulator encapsulator = GetResourceEncapsulator();
		if (!encapsulator)
			return;

		// First check, if there are any supplies at all
		// If there is no explosion, then do not spend time trying to figure out where it should be originated from
		float resourceValue = encapsulator.GetAggregatedResourceValue();
		if (resourceValue <= 0)
			return;

		IEntity owner = encapsulator.GetOwner();
		if (!owner)
			owner = GetOwner();

		if (!owner)
			return;

		bool hasData;
		ResourceName secondaryExplosionPrefab = GetSecondaryExplosion(resourceValue, SCR_ESecondaryExplosionType.RESOURCE, EResourceType.SUPPLIES, false, hasData);
		if (!hasData)
			return;

		SCR_ResourceContainerQueueBase containerQueue = encapsulator.GetContainerQueue();
		int containerCount = encapsulator.GetContainerCount();
		SCR_ResourceContainer container;
		EntitySpawnParams spawnParams = new EntitySpawnParams();

		// Get the weighed average position of explosion relative to encapsulator
		// Only calculate if we actually have an explosion to spawn
		if (!secondaryExplosionPrefab.IsEmpty())
		{
			float weight;
			vector position;
			vector averagePosition = owner.CoordToLocal(encapsulator.GetOwnerOrigin());
			spawnParams.Parent = owner;

			for (int i; i < containerCount; i++)
			{
				container = containerQueue.GetContainerAt(i);
				if (!container)
					continue;

				// Determine secondary explosion position
				weight = container.GetResourceValue() / resourceValue;
				if (weight <= 0)
					continue;

				position = owner.CoordToLocal(container.GetOwnerOrigin());
				averagePosition += position * weight;
			}
			spawnParams.Transform[3] = averagePosition;
		}


		// Destroy the resources and deny further use
		for (int i; i < containerCount; i++)
		{
			container = containerQueue.GetContainerAt(i);
			if (!container)
				continue;

			container.SetResourceValue(0);
			container.SetResourceRights(EResourceRights.NONE);
		}

		// Spawn explosion if prefab is available
		if (!secondaryExplosionPrefab.IsEmpty())
			SecondaryExplosion(secondaryExplosionPrefab, instigator, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	//! Get weighed average position of explosion for hitzones of specified type
	vector GetSecondaryExplosionPosition(typename hitZoneType, out float totalWeight = 0)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return vector.Zero;

		array<HitZone> hitZones = {};
		GetAllHitZonesInHierarchy(hitZones);
		SCR_DestructibleHitzone destructibleHitZone;
		PointInfo explosionPoint;
		vector position;
		vector weighedAveragePosition;
		vector averagePosition;
		Physics physics = owner.GetPhysics();
		float weight;
		int validHitZones;

		// Get weighed average position of fuel tanks if defined in their hitzones
		foreach (HitZone hitZone : hitZones)
		{
			destructibleHitZone = SCR_DestructibleHitzone.Cast(hitZone);
			if (!destructibleHitZone || !destructibleHitZone.Type().IsInherited(hitZoneType))
				continue;

			explosionPoint = destructibleHitZone.GetSecondaryExplosionPoint();
			if (!explosionPoint && destructibleHitZone != GetDefaultHitZone())
				continue;

			if (explosionPoint)
				position = owner.CoordToLocal(explosionPoint.GetWorldTransformAxis(3));
			else if (physics)
				position = physics.GetCenterOfMass();

			validHitZones++;
			averagePosition += position;
			weight = destructibleHitZone.GetSecondaryExplosionScale();
			if (weight < 0 || float.AlmostEqual(weight, 0))
				continue;

			weighedAveragePosition += position * weight;
			totalWeight += weight;
		}

		if (totalWeight > 0)
			return weighedAveragePosition / totalWeight;
		else if (validHitZones > 0)
			return averagePosition / validHitZones;
		else if (physics)
			return physics.GetCenterOfMass();

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn fuel secondary explosion when vehicle becomes destroyed
	void FuelSecondaryExplosion(notnull Instigator instigator)
	{
		float totalFuel;
		vector explosionPosition = GetSecondaryExplosionPosition(SCR_FuelHitZone, totalFuel);
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.Transform[3] = explosionPosition;
		ResourceName secondaryExplosionPrefab = GetSecondaryExplosion(totalFuel, SCR_ESecondaryExplosionType.FUEL);
		SecondaryExplosion(secondaryExplosionPrefab, instigator, spawnParams);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateFireDamage(float timeSlice);

	//------------------------------------------------------------------------------------------------
	/*! Update visual effects and store result in particles variable
	\param position origin of particle effect entity in owner local space
	\param particles variable to store ParticleEffectEntity
	\param fireType type of fire particles to be created
	\param resourceType type of resource if fireType is RESOURCE, by default SUPPLIES
	*/
	void UpdateFireParticles(vector position, out ParticleEffectEntity particles, SCR_ESecondaryExplosionScale state, SCR_ESecondaryExplosionType fireType, EResourceType resourceType = EResourceType.SUPPLIES)
	{
		if (particles)
		{
			SCR_ParticleHelper.StopParticleEmissionAndLights(particles);
			particles = null;
		}

		SCR_SecondaryExplosion secondaryFire = GetSecondaryExplosionForScale(state, fireType, resourceType);
		if (!secondaryFire)
			return;

		ResourceName fireParticles = secondaryFire.m_sSecondaryFireParticles;
		if (fireParticles.IsEmpty())
			return;

		ParticleEffectEntitySpawnParams params = new ParticleEffectEntitySpawnParams();
		params.FollowParent = GetOwner();
		params.Transform[3] = position;
		params.PlayOnSpawn = true;
		params.DeleteWhenStopped = true;
		params.UseFrameEvent = true;

		particles = ParticleEffectEntity.SpawnParticleEffect(fireParticles, params);
	}

	//------------------------------------------------------------------------------------------------
	protected void ConnectToFireDamageSystem()
	{
		World world = GetOwner().GetWorld();
		FireDamageSystem system = FireDamageSystem.Cast(world.FindSystem(FireDamageSystem));
		if (system)
			system.Register(this);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromFireDamageSystem()
	{
		World world = GetOwner().GetWorld();
		FireDamageSystem system = FireDamageSystem.Cast(world.FindSystem(FireDamageSystem));
		if (system)
			system.Unregister(this);
	}

	//------------------------------------------------------------------------------------------------
	//!	Invoked when damage state changes.
	protected override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		if (m_iDamageManagerDataIndex != -1)
		{
			ScriptInvoker invoker = s_aDamageManagerData[m_iDamageManagerDataIndex].GetOnDamageStateChanged(false);
			if (invoker)
				invoker.Invoke(newState);
		}

		// Only main hitzone can explode supplies
		if (newState == EDamageState.DESTROYED)
		{
			Instigator instigator = GetInstigator();
			SupplySecondaryExplosion(instigator);
			FuelSecondaryExplosion(instigator);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Invoked every time the DoT is added to certain hitzone.
	*/
	override void OnDamageOverTimeAdded(EDamageType dType, float dps, HitZone hz)
	{
		super.OnDamageOverTimeAdded(dType, dps, hz);

		if (m_iDamageManagerDataIndex != -1)
		{
			ScriptInvoker invoker = s_aDamageManagerData[m_iDamageManagerDataIndex].GetOnDamageOverTimeAdded(false);
			if (invoker)
				invoker.Invoke(dType, dps, hz);
		}

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl && rpl.IsProxy())
			return;

		if (dType == EDamageType.FIRE)
			ConnectToFireDamageSystem();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Invoked when provided damage type is removed from certain hitzone.
	*/
	override void OnDamageOverTimeRemoved(EDamageType dType, HitZone hz)
	{
		super.OnDamageOverTimeRemoved(dType, hz);

		if (m_iDamageManagerDataIndex != -1)
		{
			ScriptInvoker invoker = s_aDamageManagerData[m_iDamageManagerDataIndex].GetOnDamageOverTimeRemoved(false);
			if (invoker)
				invoker.Invoke(dType, hz);
		}

		RplComponent rpl = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (rpl && rpl.IsProxy())
			return;

		if (dType == EDamageType.FIRE && !IsDamagedOverTime(dType))
			DisconnectFromFireDamageSystem();
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Called when this DamageManager is about to handle damage.
	Any modifications done to the damageContext will persist for the rest of the damage handling process
	return false if damage handling should proceed with the changes done to the DamageContext.
	return true if damage should be discarded / was fully hijacked and should no longer be applied on this damage manager
	(e.g.: damage was passed to another dmg manager, so we dont handle damage on this manager).
	//! param[in] damageContext
	*/
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		SCR_HitZone hitZone = SCR_HitZone.Cast(damageContext.struckHitZone);
		if (hitZone)
			hitZone.ApplyDamagePassRules(damageContext);

		SCR_FlammableHitZone flammableHitZone = SCR_FlammableHitZone.Cast(damageContext.struckHitZone);
		if (flammableHitZone && damageContext.damageType == EDamageType.INCENDIARY)
			flammableHitZone.HandleIncendiaryDamage(damageContext);

		return false;
	}

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	override int _WB_GetAfterWorldUpdateSpecs(IEntity owner, IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// TODO: DrawDebug should only run when toggled on by user
	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);

		GenericEntity entity = GenericEntity.Cast(owner);
		if (!entity)
			return;

		WorldEditorAPI api = entity._WB_GetEditorAPI();
		if (!api || !api.IsEntitySelected(api.EntityToSource(owner)))
			return;

		//! Hitzone DrawDebug
		array<HitZone> hitZones = {};
		GetAllHitZones(hitZones);

		SCR_HitZone hitzone;
		foreach (HitZone hitZone : hitZones)
		{
			hitzone = SCR_HitZone.Cast(hitZone);
			if (hitzone)
				hitzone.DrawDebug();
		}
	}
	//---- REFACTOR NOTE END ----
#endif
}
