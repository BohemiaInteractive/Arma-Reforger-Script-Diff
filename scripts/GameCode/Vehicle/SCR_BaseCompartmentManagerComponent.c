class SCR_BaseCompartmentManagerComponentClass : BaseCompartmentManagerComponentClass
{
	[Attribute("0", desc: "Will block supply actions if one or more compartments are occupied as well as blocking player get in actions if there are supplies. This is a hotfix until proper solution as it simply disables some player actions on the surface rather than with proper logic and does not effect AI")]
	protected bool m_bBlockSuppliesIfOccupied;

	//------------------------------------------------------------------------------------------------
	bool ShouldBlockSuppliesIfOccupied()
	{
		return m_bBlockSuppliesIfOccupied;
	}
}

class SCR_BaseCompartmentManagerComponent : BaseCompartmentManagerComponent
{
	//Compartment slots associated with crew
	static const ref array<ECompartmentType> CREW_COMPARTMENT_TYPES = {ECompartmentType.PILOT, ECompartmentType.TURRET};
	
	//Compartment slots associated with passengers
	static const ref array<ECompartmentType> PASSENGER_COMPARTMENT_TYPES = {ECompartmentType.CARGO};
	
	//~ Default Occupant spawning
	protected ref array<BaseCompartmentSlot> m_aCompartmentsToSpawnDefaultOccupant;
	protected ref array<IEntity> m_aSpawnedDefaultOccupants;
	protected AIGroup m_SpawnedOccupantsAIGroup;
	protected bool m_bIsSpawningDefaultOccupants;
	protected ref ScriptInvoker Event_OnDoneSpawningDefaultOccupants; //~ Send over SCR_BaseCompartmentManagerComponent, array of spawned characters (IEntity) as well as bool WasCanceled
	protected ref array<ref array<ECompartmentType>> m_aDefaultOccupantsCompartmentTypesToSpawn = {}; //~ Keeps track of the default occupants types that need to be spawned. The types grouped together will be spawned in the same group
	
	//------------------------------------------------------------------------------------------------
	//! TODO: HOT FIX - Will block supply actions if one or more compartments are occupied as well as blocking player get in actions if there are supplies. This is a hotfix until proper solution as it simply disables some player actions on the surface rather than with proper logic and does not effect AI
	//! \return True if the get in actions and supply actions care if the compartments are occupied or if there are supplies stored in the vehicle
	bool BlockSuppliesIfOccupied()
	{
		SCR_BaseCompartmentManagerComponentClass data = SCR_BaseCompartmentManagerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!data)
			return false;

		return data.ShouldBlockSuppliesIfOccupied();
	}

	//------------------------------------------------------------------------------------------------
	//! Get compartments of specific type
	//! \param[in,out] outCompartments
	//! \param compartmentType
	void GetCompartmentsOfType(inout array<BaseCompartmentSlot> outCompartments, ECompartmentType compartmentType)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && compartment.GetType() == compartmentType)
				outCompartments.Insert(compartment);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in,out] outCompartments
	//! \param compartmentTypes
	void GetCompartmentsOfTypes(inout array<BaseCompartmentSlot> outCompartments, notnull array<ECompartmentType> compartmentTypes)
	{
		outCompartments.Clear();
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && compartmentTypes.Contains(compartment.GetType()))
				outCompartments.Insert(compartment);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get all free compartment slots of the given type
	//! \param[in,out] outCompartments array of all free compartments of type
	//! \param compartmentType The compartment type to check
	//! \param checkHasValidOccupantPrefab if true checks if compartment has valid default occupant prefab
	void GetFreeCompartmentsOfType(inout array<BaseCompartmentSlot> outCompartments, ECompartmentType compartmentType, bool checkHasValidOccupantPrefab = false)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && compartment.GetType() == compartmentType && !compartment.IsOccupied() && compartment.IsCompartmentAccessible() && (!checkHasValidOccupantPrefab || (checkHasValidOccupantPrefab && !compartment.GetDefaultOccupantPrefab().IsEmpty())))
				outCompartments.Insert(compartment);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get first free compartment slots of the given type
	//! \param compartmentType The compartment type to check
	//! \param checkHasValidOccupantPrefab if true checks if compartment has valid default occupant prefab
	//! \return first found compartment. Will return null if none found.
	BaseCompartmentSlot GetFirstFreeCompartmentOfType(ECompartmentType compartmentType, bool checkHasValidOccupantPrefab = false)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment && compartment.GetType() == compartmentType && !compartment.IsOccupied() && compartment.IsCompartmentAccessible() && (!checkHasValidOccupantPrefab || (checkHasValidOccupantPrefab && !compartment.GetDefaultOccupantPrefab().IsEmpty())))
				return compartment;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get first free compartment slots of the given class
	//! \param compartmentClass The compartment clas to check
	//! \param checkHasValidOccupantPrefab if true checks if compartment has valid default occupant prefab
	//! \return first found compartment. Will return null if none found.
	BaseCompartmentSlot GetFirstFreeCompartmentOfType(typename compartmentClass, bool checkHasValidOccupantPrefab = false)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment.Type() != compartmentClass)
				continue;
			
			if (!compartment.IsOccupied() && compartment.IsCompartmentAccessible() && (!checkHasValidOccupantPrefab || !compartment.GetDefaultOccupantPrefab().IsEmpty()))
				return compartment;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Go over all compartments of given type and checks if at least one compartment is free
	//! \param compartmentTypes
	//! \return true if at least one compartment of given types is free
	bool HasFreeCompartmentOfTypes(notnull array<ECompartmentType> compartmentTypes)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		
		foreach (ECompartmentType type: compartmentTypes)
		{
			compartments.Clear();
			GetCompartmentsOfType(compartments, type);
			
			foreach (BaseCompartmentSlot compartment: compartments)
			{
				if (!compartment.IsOccupied() && compartment.IsCompartmentAccessible())
					return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Count of all occupants
	int GetOccupantCount()
	{
		array<IEntity> occupants = {};
		GetOccupants(occupants);
		
		return occupants.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get occupants of all managed compartments
	//! \param[in,out] occupants
	void GetOccupants(inout array<IEntity> occupants)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartments(compartments);
		IEntity occupant;
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			if (occupant)
				occupants.Insert(occupant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get occupants of all managed compartments of specified type.
	//! \param[out] occupants
	//! \param compartmentType
	void GetOccupantsOfType(inout array<IEntity> occupants, ECompartmentType compartmentType)
	{
		array<BaseCompartmentSlot> compartments = {}; GetCompartmentsOfType(compartments, compartmentType);
		IEntity occupant;
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			if (occupant)
				occupants.Insert(occupant);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Damage random hitzones depending on amount of damage to be applied. Must be called on authority.
	//! \param damage
	//! \param damageType
	//! \param instigator
	//! \param gettingIn
	//! \param gettingOut
	void DamageOccupants(float damage, EDamageType damageType, notnull Instigator instigator, bool damageWhileGetIn = false, bool damageWhileGetOut = false)
	{
		if (damage == 0)
			return;
		
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			if (compartment)
			{
				compartment.DamageOccupant(damage, damageType, instigator, damageWhileGetIn, damageWhileGetOut);
				if (damageType == EDamageType.COLLISION)
					compartment.ScreenShakeOccupant(damage);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Kill all the occupants and eject them if requested. Must be called on authority.
	//! \param instigator
	//! \param eject
	//! \param gettingIn
	//! \param gettingOut
	void KillOccupants(notnull Instigator instigator, bool eject = false, bool gettingIn = false, bool gettingOut = false)
	{
		array<BaseCompartmentSlot> compartments = {};
		GetCompartments(compartments);

		foreach (BaseCompartmentSlot compartment : compartments)
		{
			if (compartment)
				compartment.KillOccupant(instigator, eject, gettingIn, gettingOut);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Kill all the occupants and eject them if requested. Must be called on authority.
	//! \param[in] forceEject Chance the passengers get ejected. -1 Will use the same value that is configured on the compartment. 1 will force eject. In-between is custom chance.
	//! \param[in] ejectUnconscious
	//! \param[out] allEjectedImmediately informs if this client was able to make characters leave their compartments
	//! \param[in] ejectOnTheSpot if characters should be ejected in the position where they currently are
	//! \return true if any character was ejected
	bool EjectRandomOccupants(float ejectionChance = -1, bool ejectUnconscious = false, out bool allEjectedImmediately = true, bool ejectOnTheSpot = false)
	{
		array<BaseCompartmentSlot> compartments = {};
		GetCompartments(compartments);

		bool characterEjected;
		bool ejectedImmediately;
		allEjectedImmediately = true;
		foreach (BaseCompartmentSlot compartment : compartments)
		{
			// if ejectionChance is <0, eject if random > compartmentChance
			float random = Math.RandomFloat01();
			if (ejectionChance < 0)
			{
				if (random > compartment.GetRandomEjectionChance())
					continue;
			}// if ejectionChance is 0>&&<1, eject if random > ejectionChance 
			else if (ejectionChance > 0 && ejectionChance < 1)
			{
				if (random > ejectionChance)
					continue;
			}
			// else if (ejectionChance ==  1) force eject
			if (compartment)
			{
				characterEjected = compartment.EjectOccupant(ejectionChance == 1, ejectUnconscious, ejectedImmediately, ejectOnTheSpot) || characterEjected;
				allEjectedImmediately = allEjectedImmediately && ejectedImmediately;
			}
		}

		return characterEjected;
	}

	//------------------------------------------------------------------------------------------------
	//! Set AI compartment access for all compartments
	//! \param accessible
	void SetCompartmentsAccessibleForAI(bool accessible)
	{
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment: compartments)
			compartment.SetCompartmentAccessible(accessible);
	}

	//------------------------------------------------------------------------------------------------
	//! Get event called when spawning of occupants is done.
	//! Gives an array of IEntity (Spawned characters) and bool if spawning was canceled or not
	//! \return Spawn done scriptinvoker
	ScriptInvoker GetOnDoneSpawningDefaultOccupants()
	{
		if (!Event_OnDoneSpawningDefaultOccupants)
			Event_OnDoneSpawningDefaultOccupants = new ScriptInvoker;
		
		return Event_OnDoneSpawningDefaultOccupants;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Lock/Unlock all compartment slots of given types for AI
	//! \param compartmentTypes Types to lock
	//! \param setAccessible If true sets locked else set unlocked
	void SetAllCompartmentsAccessibleOfTypes(notnull array<ECompartmentType> compartmentTypes, bool setAccessible)
	{
		array<BaseCompartmentSlot> compartments = {};
		GetCompartmentsOfTypes(compartments, compartmentTypes);
		
		foreach(BaseCompartmentSlot compartment: compartments)
		{			
			compartment.SetCompartmentAccessible(setAccessible);
		}
	}
	
	//--------------------------------------------------- Check can occupy with characters ---------------------------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Check if can spawn characters in the vehicle.
	//! \param compartmentTypes Given compartment types that need to be checked if can be filled.
	//! \param checkHasDefaultOccupantsData
	//! \param friendlyFaction
	//! \param checkOccupyingFaction If true function will check if the faction of the vehicle is null (empty) or the same/friendly to the static faction as given in the editableUIInfo. Function will return false if faction is hostile or neutral
	//! \param checkForFreeCompartments If true will check if there are compartments free for the given compartments types. Function will return false if there are no free compartments of given type
	//! \return Will return true if a character can be spawned in at least one compartment
	//! \param checkOccupyingFaction
	//! \param checkForFreeCompartments
	//! \return true if can occupy, false otherwise
	bool CanOccupy(array<ECompartmentType> compartmentTypes, bool checkHasDefaultOccupantsData = true, FactionKey friendlyFaction = string.Empty, bool checkOccupyingFaction = true, bool checkForFreeCompartments = true)
	{
		if (m_bIsSpawningDefaultOccupants)
			return false;
		
		//~ Check if default character data is defined
		if (checkHasDefaultOccupantsData && !HasDefaultOccupantsDataForTypes(compartmentTypes))
			return false;
		
		//~ Check if all occupants in vehicle are friendly. If not return false
		if (checkOccupyingFaction && !IsOccupiedByFriendlies(friendlyFaction, true))
			return false;

		//~ Check if there is at least one free compartment. If not return false
		if (checkForFreeCompartments && !HasFreeCompartmentOfTypes(compartmentTypes))
			return false;
		
		return true;
	}	
	
	//--------------------------------------------------- Spawn default occupants ---------------------------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Spawns default characters as defined in BaseComparmentSlots inside the vehicle of given compartment types (Server only)
	//! Places all spawned characters in the same group
	//! Note that it can only spawn the same compartment type once and will fail if you try to do it twice.
	//! If the system is already spawning default characters and you call it again for other department types it will send them to a queue and execute them after the current compartment types are filled.
	//! \param compartmentTypes Given compartment types that need to be filled with characters
	//! \return True if start spawning occupants
	bool SpawnDefaultOccupants(notnull array<ECompartmentType> compartmentTypes)
	{
		//~ No compartment types given to spawn
		if (compartmentTypes.IsEmpty())
		{
			if (Event_OnDoneSpawningDefaultOccupants)
				Event_OnDoneSpawningDefaultOccupants.Invoke(this, null, true);
			
			return false;
		}
		
		//~ Check if is already spawning
		for(int i = 0; i < m_aDefaultOccupantsCompartmentTypesToSpawn.Count(); i++)
		{
			if (!m_aDefaultOccupantsCompartmentTypesToSpawn[i] || m_aDefaultOccupantsCompartmentTypesToSpawn[i].IsEmpty())
			{
				m_aDefaultOccupantsCompartmentTypesToSpawn.RemoveOrdered(i);
				i--;
				continue;
			}
			
			foreach(ECompartmentType type: compartmentTypes)
			{
				//~ Is already spawning the same type
				if (m_aDefaultOccupantsCompartmentTypesToSpawn[i].Contains(type))
				{
					Print(string.Format("'SCR_BaseCompartmentManagerComponent' is already spawning default occupants of type %1!", typename.EnumToString(ECompartmentType, type)), LogLevel.WARNING);
					return false; 
				}
			}
		}
		
		//~ Add to spawn list
		m_aDefaultOccupantsCompartmentTypesToSpawn.Insert(compartmentTypes);
		
		//~ Directly spawn in vehicle as not currently spawning anything
		if (!m_bIsSpawningDefaultOccupants)
		{
			return InitiateSpawnDefaultOccupants(compartmentTypes);
		}
		//~ Already spawning so wait until done
		else 
		{
			m_aCompartmentsToSpawnDefaultOccupant = {};
			foreach (ECompartmentType compartmentType: compartmentTypes)
			{
				GetFreeCompartmentsOfType(m_aCompartmentsToSpawnDefaultOccupant, compartmentType, true);
			}
			
			//~ Nothing to spawn in so cancel
			if (m_aCompartmentsToSpawnDefaultOccupant.IsEmpty())
				return false;
		}
		
		return true; 
	}
	
	//~ Actually initia
	protected bool InitiateSpawnDefaultOccupants(notnull array<ECompartmentType> compartmentTypes)
	{	
		m_bIsSpawningDefaultOccupants = true;
		m_aCompartmentsToSpawnDefaultOccupant = {};
		
		foreach (ECompartmentType compartmentType: compartmentTypes)
		{
			GetFreeCompartmentsOfType(m_aCompartmentsToSpawnDefaultOccupant, compartmentType, true);
		}
		
		//~ Nothing to spawn in so cancel
		if (m_aCompartmentsToSpawnDefaultOccupant.IsEmpty())
		{
			FinishedSpawningDefaultOccupants(true);			
			return false;
		}
			
		m_aSpawnedDefaultOccupants = {};
		m_SpawnedOccupantsAIGroup = null;
		
		GetGame().GetCallqueue().CallLater(SpawnDefaultOccupantEachFrame, 0, true);
		return true;
	}
	
	//~ Spawns a character each frame to take the load of the server
	protected void SpawnDefaultOccupantEachFrame()
	{
		BaseCompartmentSlot compartmentToFill = null;
		
		for(int i = 0; i < m_aCompartmentsToSpawnDefaultOccupant.Count(); i++)
        {
			//~ Invalid compartment so remove it from the list
           	if (!m_aCompartmentsToSpawnDefaultOccupant[i] || m_aCompartmentsToSpawnDefaultOccupant[i].IsOccupied() || !m_aCompartmentsToSpawnDefaultOccupant[i].IsCompartmentAccessible())
			{
				m_aCompartmentsToSpawnDefaultOccupant.RemoveOrdered(i);
				i--;
				continue;
			}
			
			compartmentToFill = m_aCompartmentsToSpawnDefaultOccupant[i];
			m_aCompartmentsToSpawnDefaultOccupant.RemoveOrdered(i);
			break;
        }
		
		//~ No compartments found
		if (!compartmentToFill)
		{
			FinishedSpawningDefaultOccupants(false);
			return;
		}
		
		IEntity spawnedCharacter = compartmentToFill.SpawnDefaultCharacterInCompartment(m_SpawnedOccupantsAIGroup);
		if (!spawnedCharacter)
			return;
		
		
		m_aSpawnedDefaultOccupants.Insert(spawnedCharacter);
		
		if (m_aCompartmentsToSpawnDefaultOccupant.IsEmpty())
			FinishedSpawningDefaultOccupants(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Cancel spawning if in progress
	void CancelSpawningCharacters()
	{
		if (!m_aDefaultOccupantsCompartmentTypesToSpawn.IsEmpty() || m_bIsSpawningDefaultOccupants)
			FinishedSpawningDefaultOccupants(true);
	}

	//------------------------------------------------------------------------------------------------
	//! \return True if asynchronous operation of spawning default occupants is underway.
	bool IsSpawningDefaultOccupants()
	{
		return m_bIsSpawningDefaultOccupants;
	}
	
	//~ Called when finished spawning. Note that it will spawn more passengers if it has a queue
	protected void FinishedSpawningDefaultOccupants(bool wasCanceled)
	{
		m_bIsSpawningDefaultOccupants = false;
		SCR_CompartmentAccessComponent compartmentAccess;
		
		if (Event_OnDoneSpawningDefaultOccupants)
			Event_OnDoneSpawningDefaultOccupants.Invoke(this, m_aSpawnedDefaultOccupants, wasCanceled);
		
		m_aSpawnedDefaultOccupants = null;
		m_aCompartmentsToSpawnDefaultOccupant = null;
		GetGame().GetCallqueue().Remove(SpawnDefaultOccupantEachFrame);
		
		//~ Clear queue
		if (wasCanceled)
		{
			m_aDefaultOccupantsCompartmentTypesToSpawn.Clear();
		}
		//~ next in queue
		else 
		{		
			//~ Delete the one just spawned		
			m_aDefaultOccupantsCompartmentTypesToSpawn.RemoveOrdered(m_aDefaultOccupantsCompartmentTypesToSpawn.Count() -1);
			//~ Spawn the entities that where waiting to be spawned
			if (!m_aDefaultOccupantsCompartmentTypesToSpawn.IsEmpty())
				InitiateSpawnDefaultOccupants(m_aDefaultOccupantsCompartmentTypesToSpawn[0]);
		}
	}
	
	//--------------------------------------------------- Vehicle occupied by friendly ---------------------------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Checks if vehicle is occupied by characters that are friendly towards the given faction
	//! \param vehicleFactionKey the vehicle's faction
	//! \param allFriendly If true will find all occupants and check if all of them are friendly. If false it will only check the first character it finds, mixed factions might not return a valid value.
	//! \param trueIfEmpty If this is true then the function will return true if the vehicle is empty. Else it will return false
	//! \return true if the vehicle is occupied by a friendly/friendlies, false otherwise
	bool IsOccupiedByFriendlies(FactionKey vehicleFactionKey, bool allFriendly, bool trueIfEmpty = true)
	{
		//~ No given string so cannot check
		if (vehicleFactionKey.IsEmpty())
			return false;
		
		array<BaseCompartmentSlot> compartments = {}; 
		GetCompartments(compartments);
		
		if (compartments.IsEmpty())
			return false;
		
		//~ No faction manager so return false
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;
		
		IEntity occupant; 
		FactionAffiliationComponent factionAffiliationComponent;
		Faction occupantFaction;
		Faction vehicleFaction = factionManager.GetFactionByKey(vehicleFactionKey);
		bool friendlyFound = false;
		bool occupantFound = false;
		
		//~ Go over each compartment and see if it is occupied and if the enetity is an enemy or neutral to the given faction
		foreach (BaseCompartmentSlot compartment: compartments)
		{
			occupant = compartment.GetOccupant();
			
			if (!occupant)
				continue;
			
			occupantFound = true;
			
			factionAffiliationComponent = FactionAffiliationComponent.Cast(occupant.FindComponent(FactionAffiliationComponent));
			if (!factionAffiliationComponent)
				continue;
			
			occupantFaction = factionAffiliationComponent.GetAffiliatedFaction();
			
			//~ Same faction so is friendly
			if (occupantFaction.GetFactionKey() == vehicleFactionKey)
				return true;
			
			//~ Occupent is friendly
			if (vehicleFaction.IsFactionFriendly(occupantFaction))
			{
				if (allFriendly)
					friendlyFound = true;
				else 
					return true;
			}
			//~ Occupent is not friendly!
			else 
			{
				return false;
			}
		}
		
		//~ No occupants found
		if (!occupantFound)
			return trueIfEmpty;
		
		//~ Multiple occupants found check if all must be friendly
		if (allFriendly)
			return friendlyFound;
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Returns true if vehicle is occupied by any character or a compartment is locked
	bool AnyCompartmentsOccupiedOrLocked()
	{		
		//~ If vehicle check if it is occupied
		array<IEntity> occupants = {};
		GetOccupants(occupants);
			
		//~ Entity is occupied by characters
		if (!occupants.IsEmpty())
			return true;
			
		array<BaseCompartmentSlot> compartments = {};
		GetCompartments(compartments);
		
		foreach (BaseCompartmentSlot compartment : compartments)
		{
			if (!compartment)
				continue;
			
			//~ Vehicle currently has a locked compartment so is in use
			if (compartment.IsGetInLocked())
				return true;
		}
		
		return false;
	}
	
	//--------------------------------------------------- Has default occupent data set of type ---------------------------------------------------\\

	//------------------------------------------------------------------------------------------------
	//! Check if there is at least one default occupant prefab set for the given compartment types
	//! \param compartmentTypes Given compartment types that need to be filled with characters
	//! \return True if at least one prefab data is set
	bool HasDefaultOccupantsDataForTypes(array<ECompartmentType> compartmentTypes)
	{
		array<BaseCompartmentSlot> compartments = new array<BaseCompartmentSlot>;
		SCR_DefaultOccupantData defaultOccupantData;
		
		foreach (ECompartmentType compartmentType: compartmentTypes)
		{
			compartments.Clear();
			GetCompartmentsOfType(compartments, compartmentType);
			
			foreach (BaseCompartmentSlot compartment: compartments)
			{
				defaultOccupantData = compartment.GetDefaultOccupantData();
				
				if (defaultOccupantData && defaultOccupantData.IsValid())
					return true;
			}
		}
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_BaseCompartmentManagerComponent()
	{
		//~ Component destroyed before it could finish spawning all characters
		if (m_bIsSpawningDefaultOccupants)
			FinishedSpawningDefaultOccupants(true);
	}
}
