class SCR_VehicleFactionAffiliationComponentClass: SCR_FactionAffiliationComponentClass
{
	
};

class SCR_VehicleFactionAffiliationComponent: SCR_FactionAffiliationComponent
{
	protected int m_iOccupantCount;
	protected int m_iAliveOccupantCount;
	
	//--------------------------------------------------------------------------------------------------------------------------
	override event void OnPostInit(IEntity owner)
	{
		ClearAffiliatedFaction();
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	//! Check if there are any occupants regardless of life state
	bool IsVehicleOccupied()
	{
		return m_iOccupantCount > 0;
	}	
	
	//--------------------------------------------------------------------------------------------------------------------------
	//! Check if there are any conscious and alive occupants among the occupants 
	bool IsVehicleActive()
	{
		return m_iAliveOccupantCount > 0;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	override protected void OnCompartmentEntered(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move)
	{
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(occupant);
		if(!character)
			return;
	
		// Test if the faction of the vehicle and the newly mounted passenger match.
		// If not, we'll eject the new passenger
		Faction characterFaction = character.GetFaction();
		Faction vehicleFaction = GetAffiliatedFaction();
		
		if (characterFaction && !vehicleFaction) 
		{
			// No faction on the vehicle, just set the occupant's faction
			SetAffiliatedFaction(characterFaction);
		}
		else if (characterFaction && !characterFaction.IsFactionFriendly(GetAffiliatedFaction()))
		{
			// There is vehicleFaction set. Only overwrite it when there are no further occupants
			if (!IsVehicleOccupied())
			{
				SetAffiliatedFaction(characterFaction);
			}
		};
		
		if (vehicleFaction && characterFaction && characterFaction.IsFactionEnemy(vehicleFaction) && !characterFaction.IsFactionEnemy(characterFaction))	
		{
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			compartmentAccess.GetOutVehicle(EGetOutType.TELEPORT, -1, ECloseDoorAfterActions.INVALID, false);
			// Drop through, we need to account for this new occupant still
		};
		
		// Make us count.
		m_iOccupantCount++;
		UpdateOccupantsCount();
	}
		
		
	//--------------------------------------------------------------------------------------------------------------------------
	override protected void OnCompartmentLeft(IEntity vehicle, IEntity occupant, BaseCompartmentSlot compartment, bool move)
	{
		if (move) // moving only between compartments
			return;

		m_iOccupantCount--;
		UpdateOccupantsCount(occupant);
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	void UpdateOccupantsCount(IEntity ignoreOccupant = null)
	{
		SCR_BaseCompartmentManagerComponent baseCompMan = SCR_BaseCompartmentManagerComponent.Cast(GetOwner().FindComponent(SCR_BaseCompartmentManagerComponent));
		if (!baseCompMan)
			return;
		
		array<IEntity> occupants = {};
		baseCompMan.GetOccupants(occupants);
		int aliveOccupants;
		int allOccupants;
		ECharacterLifeState state;
		foreach(IEntity occupant : occupants)
		{
			if ((ignoreOccupant == occupant))
				continue;
			
			ChimeraCharacter character = ChimeraCharacter.Cast(occupant);
			if (!character)
				continue;
			
			CharacterControllerComponent contr = character.GetCharacterController();
			if (!contr)
				continue;
			
			state = contr.GetLifeState();
			if (state == ECharacterLifeState.ALIVE)
				aliveOccupants++;
			
			if (state != ECharacterLifeState.DEAD)
				allOccupants++;
		}
		
		m_iAliveOccupantCount = aliveOccupants;
		m_iOccupantCount = allOccupants;
		
		if (m_iOccupantCount < 1)
			ClearAffiliatedFaction();
	}
	
	//--------------------------------------------------------------------------------------------------------------------------
	void OnOccupantLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		UpdateOccupantsCount();
	}	
};