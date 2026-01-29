class SCR_ToggleHandbrakeAction : SCR_ScriptedUserAction
{
	[Attribute(desc: "When anything is defined in here, this action will only be visible if the compartment section of the compartment the player is in is defined in here.")]
	protected ref array<int> m_aDefinedCompartmentSectionsOnly;

	[Attribute(desc: "When aynthing is defined in here, this action won't be visible if the compartment section of the compartment the player is in is defined in here.")]
	protected ref array<int> m_aExcludeDefinedCompartmentSections;

	protected const LocalizedString CONTROLLED_BY_DRIVER = "#AR-UserAction_ControlledByDriver";
	protected const LocalizedString OCCUPIED_BY_ENEMY = "#AR-UserAction_SeatHostile";

	//------------------------------------------------------------------------------------------------
	//! Can this entity be shown in the UI by the provided user entity?
	override bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		if ((m_aDefinedCompartmentSectionsOnly && !m_aDefinedCompartmentSectionsOnly.IsEmpty()) || (m_aExcludeDefinedCompartmentSections && !m_aExcludeDefinedCompartmentSections.IsEmpty()))
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(user);
			return SCR_InteractionHandlerComponent.CanBeShownInVehicle(
				character,
				this,
				false,
				m_eShownInVehicleState != EUserActionInVehicleState.IN_VEHICLE_PILOT,
				m_eShownInVehicleState != EUserActionInVehicleState.NOT_IN_VEHICLE,
				m_aDefinedCompartmentSectionsOnly,
				m_aExcludeDefinedCompartmentSections
			);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		Vehicle vehicleEntity = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicleEntity)
			return false;

		if (!vehicleEntity.IsOccupied())
			return true;

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);
		if (!character)
			return false;

		Faction characterFaction = character.GetFaction();
		Faction vehicleFaction = vehicleEntity.GetFaction();
		if (characterFaction && vehicleFaction && characterFaction.IsFactionEnemy(vehicleFaction))
		{
			SetCannotPerformReason(OCCUPIED_BY_ENEMY);
			return false;
		}

		if (!SCR_InteractionHandlerComponent.CanBeShownInVehicle(
			character,
			this,
			true,
			m_eShownInVehicleState != EUserActionInVehicleState.IN_VEHICLE_PILOT,
			m_eShownInVehicleState != EUserActionInVehicleState.NOT_IN_VEHICLE,
			m_aDefinedCompartmentSectionsOnly,
			m_aExcludeDefinedCompartmentSections
		))
		{
			SetCannotPerformReason(CONTROLLED_BY_DRIVER);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when someone tries to perform the action, user entity is typically character
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		Vehicle vehicleEntity = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicleEntity)
			return;

		CarControllerComponent controller = CarControllerComponent.Cast(vehicleEntity.GetVehicleController());
		if (!controller)
			return;

		VehicleWheeledSimulation simulation = controller.GetWheeledSimulation();
		if (!simulation)
			return;

		bool handBrake = controller.GetPersistentHandBrake();
		controller.SetPersistentHandBrake(!handBrake);
		simulation.SetBreak(!handBrake, !handBrake);

		//get number of all gears where neautral is also one of them and it is the middle one
		int gears = simulation.GearboxGearsCount();
		//find neutral gear id by removing all forward gears
		if (handBrake)
			gears = gears - simulation.GearboxForwardGearsCount() - 1;//neutral
		else
			gears = gears - simulation.GearboxForwardGearsCount();//first

		//set gear to neutral
		simulation.SetGear(gears);
	}

	//------------------------------------------------------------------------------------------------
	//! If overridden and true is returned, outName is returned when BaseUserAction.GetActionName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetActionNameScript(out string outName)
	{
		Vehicle vehicleEntity = Vehicle.Cast(GetOwner().GetRootParent());
		if (!vehicleEntity)
			return false;

		UIInfo info = GetUIInfo();
		if (!info)
			return false;

		CarControllerComponent carControllerComp = CarControllerComponent.Cast(vehicleEntity.GetVehicleController());
		if (!carControllerComp)
			return false;

		if (carControllerComp.GetPersistentHandBrake())
			outName = info.GetDescription();
		else
			outName = info.GetName();

		return true;
	}
}
