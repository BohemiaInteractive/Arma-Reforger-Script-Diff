class SCR_GetInUserAction : SCR_CompartmentUserAction
{	
	protected const LocalizedString OCCUPIED_BY_SUPPLIES = "#AR-UserAction_SeatOccupied";
	
	protected SCR_BaseLockComponent m_pLockComp;
	protected DamageManagerComponent m_DamageManager;
	protected SCR_ResourceComponent m_ResourceComp;
	protected SCR_BaseCompartmentManagerComponent m_CompartmentManager;

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		IEntity vehicle = SCR_EntityHelper.GetMainParent(pOwnerEntity, true);
		if (!vehicle)
			return;

		m_pLockComp = SCR_BaseLockComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseLockComponent));
		m_DamageManager = DamageManagerComponent.Cast(vehicle.FindComponent(DamageManagerComponent));
		
		//~ Hotfix to prevent get in action if the vehicle has supplies
		m_CompartmentManager = SCR_BaseCompartmentManagerComponent.Cast(pOwnerEntity.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (m_CompartmentManager && m_CompartmentManager.BlockSuppliesIfOccupied())
			m_ResourceComp = SCR_ResourceComponent.FindResourceComponent(pOwnerEntity, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;
		
		BaseCompartmentSlot targetCompartment = GetCompartmentSlot();
		if (!targetCompartment)
			return;
		
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;
		
		if (!compartmentAccess.GetInVehicle(pOwnerEntity, targetCompartment, false, GetRelevantDoorIndex(pUserEntity), ECloseDoorAfterActions.RETURN_TO_PREVIOUS_STATE, false))
			return;
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (m_DamageManager && m_DamageManager.GetState() == EDamageState.DESTROYED)
			return false;

		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;
		
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;
		
		//~ TODO: Hotfix until proper solution, only blocks player does not block AI or Editor actions
		float storedResources;
		if (m_ResourceComp && m_CompartmentManager && m_CompartmentManager.BlockSuppliesIfOccupied())
		{
			if (SCR_ResourceSystemHelper.GetStoredResources(m_ResourceComp, storedResources) && storedResources > 0)
			{
				SetCannotPerformReason(OCCUPIED_BY_SUPPLIES);
				return false;
			}
		}
		
		IEntity owner = compartment.GetOwner();
		Vehicle vehicle = Vehicle.Cast(SCR_EntityHelper.GetMainParent(owner, true));
		if (vehicle)
		{
			Faction characterFaction = character.GetFaction();
			Faction vehicleFaction = vehicle.GetFaction();
			if (characterFaction && vehicleFaction && characterFaction.IsFactionEnemy(vehicleFaction))
			{
				SetCannotPerformReason("#AR-UserAction_SeatHostile");
				return false;
			}
		}
		
		if (compartment.GetOccupant() || m_CompartmentManager.IsGetInAndOutBlockedByDoorUser(GetRelevantDoorIndex(user)))
		{
			SetCannotPerformReason("#AR-UserAction_SeatOccupied");
			return false;
		}
		
		// Check if the position isn't lock.
		if (m_pLockComp && m_pLockComp.IsLocked(user, compartment))
		{
			SetCannotPerformReason(m_pLockComp.GetCannotPerformReason(user));
			return false;
		}

		if (compartment.IsReserved() && !compartment.IsReservedBy(user))
		{
			SetCannotPerformReason("#AR-Campaign_Action_CannotEnterVehicle-UC");
			return false;
		}

		// Make sure vehicle can be enter via provided door, if not, set reason.
		if (!compartmentAccess.CanGetInVehicleViaDoor(owner, m_CompartmentManager, GetRelevantDoorIndex(user)))
		{
			SetCannotPerformReason("#AR-UserAction_SeatObstructed");
			return false;
		}
		
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		if (m_DamageManager && m_DamageManager.GetState() == EDamageState.DESTROYED)
			return false;

		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		ChimeraCharacter occupant = ChimeraCharacter.Cast(compartment.GetOccupant());
		if (occupant)
		{
			CharacterControllerComponent controller = occupant.GetCharacterController();
			if (controller && controller.GetLifeState() != ECharacterLifeState.ALIVE)
				return false;
		}
		
		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (character && character.IsInVehicle())
			return false;
		
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;
		
		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;
		
		return true;
	}	
};
