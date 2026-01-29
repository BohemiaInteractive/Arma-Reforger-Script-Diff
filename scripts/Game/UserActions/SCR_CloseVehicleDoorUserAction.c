class SCR_CloseVehicleDoorUserAction : VehicleDoorUserAction
{
	[Attribute("1", desc: "Should the door open on their own, or should they be opened by the character?")]
	protected bool m_bAnimateCharacter;
	
	//------------------------------------------------------------------------------------------------
	override bool GetActionNameScript(out string outName)
	{
		UIInfo uiInfo = GetUIInfo();
		if (!uiInfo)
		{
			outName = "Missing UIInfo";
			return true;
		}
		
		outName = uiInfo.GetName();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity || !pUserEntity)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(pUserEntity);
		if (!character)
			return;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return;
		
		ECharacterDoorAnimType animType = ECharacterDoorAnimType.INVALID;
		if (compartmentAccess.IsInCompartment())
		{
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			array<int> compartmentDoors = {};
			if (compartment)
				compartment.GetAvailableDoorIndices(compartmentDoors);

			if (compartmentDoors.Contains(GetDoorIndex()))
				animType = ECharacterDoorAnimType.FROM_INSIDE;
			else
				animType = ECharacterDoorAnimType.NO_CHARACTER_ANIM;
		}
		else
		{
			animType = ECharacterDoorAnimType.FROM_OUTSIDE;
		}
		
		if (!compartmentAccess.CloseDoor(pOwnerEntity, animType, GetDoorIndex()))
			return;
		
		super.PerformAction(pOwnerEntity, pUserEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		IEntity owner = GetOwner();//no null check here as it was already done in CanBeShownScript
		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;
		
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

		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return false;

		IEntity currentDoorUser = compartmentManager.GetDoorUser(GetDoorIndex());
		if (currentDoorUser && currentDoorUser != user)
		{
			SetCannotPerformReason("#AR-UserAction_SeatOccupied");
			return false;
		}
		
		if (!compartmentAccess.CanAccessDoor(vehicle, compartmentManager, GetDoorIndex()))
		{
			SetCannotPerformReason("#AR-UserAction_SeatObstructed");
			return false;
		}
		
		return true;
	}	
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		IEntity owner = GetOwner();
		if (!owner)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;
		
		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;
		
		if (compartmentAccess.IsDoorFromAnotherVehicle(owner, GetDoorIndex()))
			return false;

		BaseCompartmentManagerComponent compartmentManager = BaseCompartmentManagerComponent.Cast(owner.FindComponent(BaseCompartmentManagerComponent));
		if (!compartmentManager)
			return false;
		
		return compartmentManager.IsDoorOpen(GetDoorIndex());
	}	
};
