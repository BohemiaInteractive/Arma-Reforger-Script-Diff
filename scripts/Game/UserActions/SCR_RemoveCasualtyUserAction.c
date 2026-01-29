class SCR_RemoveCasualtyUserAction : SCR_CompartmentUserAction
{
	const string m_sCannotPerformHostile = "#AR-UserAction_SeatHostile";
	const string m_sCannotPerformObstructed = "#AR-UserAction_SeatObstructed";

	[Attribute(desc: "If true, the action will be only shown if player is inside a vehicle")]
	protected bool m_bInteriorOnly;

	[Attribute(desc: "If true, the action will be only shown if player is not inside a vehicle")]
	protected bool m_bExteriorOnly;

	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!pOwnerEntity)
			return;

		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return;

		ChimeraCharacter casualty = ChimeraCharacter.Cast(compartment.GetOccupant());
		if (!casualty)
			return;

		ChimeraCharacter user = ChimeraCharacter.Cast(pUserEntity);
		if (!user)
			return;

		CompartmentAccessComponent userCompartmentAccess = user.GetCompartmentAccessComponent();
		CompartmentAccessComponent casualtyCompartmentAccess = casualty.GetCompartmentAccessComponent();
		if (casualtyCompartmentAccess && userCompartmentAccess)
		{
			int doorIdx = GetRelevantDoorIndex(pUserEntity);
			casualtyCompartmentAccess.KickFromVehicle(doorIdx);
		}

		super.PerformAction(pOwnerEntity, pUserEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		// Restrict removing casualty from another compartment in case the section does not match
		BaseCompartmentSlot characterCompartment = compartmentAccess.GetCompartment();
		if (characterCompartment && characterCompartment.GetCompartmentSection() != compartment.GetCompartmentSection())
			return false;

		IEntity owner = compartment.GetOwner();
		Vehicle vehicle = Vehicle.Cast(owner.GetRootParent());
		if (vehicle)
		{
			Faction characterFaction = character.GetFaction();
			Faction vehicleFaction = vehicle.GetFaction();

			if (characterFaction && vehicleFaction && characterFaction.IsFactionEnemy(vehicleFaction))
			{
				bool isActive = true;
				SCR_VehicleFactionAffiliationComponent vehicleAffiliation = vehicle.GetFactionAffiliation();
				if (vehicleAffiliation)
					isActive = vehicleAffiliation.IsVehicleActive();

				if (isActive)
				{
					SetCannotPerformReason(m_sCannotPerformHostile);
					return false;
				}
			}
		}

		BaseCompartmentManagerComponent managerComponent = compartment.GetManager();
		if (!managerComponent)
			return false;

		// Make sure vehicle can be enter via provided door, if not, set reason.
		if (!character.IsInVehicle() && !compartmentAccess.CanGetInVehicleViaDoor(owner, managerComponent, GetRelevantDoorIndex(user)))
		{
			SetCannotPerformReason(m_sCannotPerformObstructed);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user)
	{
		BaseCompartmentSlot compartment = GetCompartmentSlot();
		if (!compartment)
			return false;

		ChimeraCharacter character = ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
		if (!compartmentAccess)
			return false;

		if (compartmentAccess.IsGettingIn() || compartmentAccess.IsGettingOut())
			return false;

		// Restrict removing casualty from another compartment in case the section does not match
		BaseCompartmentSlot characterCompartment = compartmentAccess.GetCompartment();
		if (characterCompartment && characterCompartment.GetCompartmentSection() != compartment.GetCompartmentSection())
			return false;

		ChimeraCharacter casualty = ChimeraCharacter.Cast(compartment.GetOccupant());
		if (!casualty)
			return false;

		CharacterControllerComponent controller = casualty.GetCharacterController();
		if (!controller)
			return false;

		if (m_bInteriorOnly && !SCR_InteractionHandlerComponent.CanBeShownInVehicle(character, this, interiorOnly: m_bInteriorOnly))
			return false;

		if (m_bExteriorOnly && character.IsInVehicle())
			return false;

		return controller.GetLifeState() != ECharacterLifeState.ALIVE;
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return false;
	}
}
