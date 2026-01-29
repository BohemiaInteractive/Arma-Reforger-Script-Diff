[BaseContainerProps()]
class SCR_TutorialLogic_FreeRoam : SCR_BaseTutorialCourseLogic
{
	protected const int FAST_TRAVEL_SIGN_DISTANCE = 25;
	//------------------------------------------------------------------------------------------------
	void SetUpAmbulance()
	{
		IEntity ambulance = GetGame().GetWorld().FindEntityByName("RespawnAmbulance");
		if (!ambulance)
			return;
			
		SCR_VehicleSpawnProtectionComponent protectionComp = SCR_VehicleSpawnProtectionComponent.Cast(ambulance.FindComponent(SCR_VehicleSpawnProtectionComponent));
		if (protectionComp)
		{
			protectionComp.SetProtectOnlyDriverSeat(true);
			protectionComp.SetVehicleOwner(-2);
		}
		
		IEntity soldier = GetGame().GetWorld().FindEntityByName("Ambulance_Doctor");
		if (!soldier)
			return;
		
		BaseCompartmentSlot doctorCompartment;
		SCR_BaseCompartmentManagerComponent compartmentMan = SCR_BaseCompartmentManagerComponent.Cast(ambulance.FindComponent(SCR_BaseCompartmentManagerComponent));
		if (compartmentMan)
		{
			array <BaseCompartmentSlot> compartments = {};
			compartmentMan.GetCompartments(compartments);
			foreach (BaseCompartmentSlot compartment : compartments)
			{
				if (compartment.Type() == SCR_DoctorCompartmentSlot)
					doctorCompartment = compartment;
			}
		}
		
		SCR_CompartmentAccessComponent compartmentAccess = SCR_CompartmentAccessComponent.Cast(soldier.FindComponent(SCR_CompartmentAccessComponent));
		if (!compartmentAccess)
			return;
		
		//compartmentAccess.OpenDoor(ambulance, ECharacterDoorAnimType.INVALID, 4);
		compartmentAccess.GetInVehicle(ambulance, doctorCompartment, true, 4, ECloseDoorAfterActions.LEAVE_OPEN, true);
		GetGame().GetCallqueue().CallLater(compartmentAccess.OpenDoor, 1000, false, ambulance, ECharacterDoorAnimType.INVALID, 4);
		
		SCR_NarrativeComponent narrativeComponent = SCR_NarrativeComponent.Cast(soldier.FindComponent(SCR_NarrativeComponent));
		
		if (!SCR_NarrativeComponent)
			return;
		
		narrativeComponent.Initialize(soldier);
	}
	
	//------------------------------------------------------------------------------------------------
	void PrepareFastTravelSigns()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		SCR_BaseTutorialStage stage = tutorial.GetCurrentStage();
		if (!stage)
			return;
		
		IEntity ent = GetGame().GetWorld().FindEntityByName("FastTravel_Signs");
		if (!ent)
			return;
		
		SCR_Waypoint wp;
		ent = ent.GetChildren();
		while (ent)
		{
			wp = stage.RegisterWaypoint(ent, "", "SERPENTINE");
			if (wp)
			{
				wp.EnableFading(true);
				wp.m_iMaximumDrawDistance = FAST_TRAVEL_SIGN_DISTANCE;
				wp.SetOffsetVector("0 2.5 0");
			}
			
			ent = ent.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void PrepareHelicopter()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity spawnPos = GetGame().GetWorld().FindEntityByName("SpawnPos_UH1COURSE");
		if (!spawnPos)
			return;
		
		SCR_VehicleDamageManagerComponent damageManager;
		
		IEntity helicopter = GetGame().GetWorld().FindEntityByName("UH1COURSE");
		if (!helicopter)
		{
			tutorial.SpawnAsset("UH1COURSE", "{70BAEEFC2D3FEE64}Prefabs/Vehicles/Helicopters/UH1H/UH1H.et", spawnPos);
			return;
		}
		
		
		if (vector.Distance(helicopter.GetOrigin(), spawnPos.GetOrigin()) > 11 || IsHelicopterDamaged())
			tutorial.SpawnAsset("UH1COURSE", "{70BAEEFC2D3FEE64}Prefabs/Vehicles/Helicopters/UH1H/UH1H.et", spawnPos);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsHelicopterDamaged()
	{
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return true;
		
		SCR_DamageManagerComponent damageManager = helicopter.GetDamageManager();
		if (damageManager && damageManager.GetState() != EDamageState.UNDAMAGED)
			return true;
	
		SlotManagerComponent slotManager = SlotManagerComponent.Cast(helicopter.FindComponent(SlotManagerComponent));
		if (!slotManager)
			return false;
		
		IEntity rotor;
		SCR_RotorDamageManagerComponent rotorDmgComp;
		EntitySlotInfo slot = slotManager.GetSlotByName("RotorMain");
		if (slot)
		{
			rotor = slot.GetAttachedEntity();
			rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			if (rotorDmgComp && rotorDmgComp.GetState() != EDamageState.UNDAMAGED)
				return true;
		}
		
		slot = slotManager.GetSlotByName("RotorTail");
		if (slot)
		{
			rotor = slot.GetAttachedEntity();
			rotorDmgComp = SCR_RotorDamageManagerComponent.Cast(rotor.FindComponent(SCR_RotorDamageManagerComponent));
			if (rotorDmgComp && rotorDmgComp.GetState() != EDamageState.UNDAMAGED)
				return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void PrepareDrivingCourseInstructor()
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity spawnPos = GetGame().GetWorld().FindEntityByName("SpawnPos_SmallJeep");
		
		IEntity car = tutorial.SpawnAsset("SmallJeep", "{94DE32169691AC34}Prefabs/Vehicles/Wheeled/M151A2/M151A2_transport_MERDC.et", spawnPos);
		
		IEntity instructor = tutorial.SpawnAsset("DRIVING_Copilot", "{F9C424E7C6598A7E}Prefabs/Characters/Tutorial/Instructors/Driving_Instructor_Copilot.et", spawnPos);

		if (!instructor || !car)
			return;
		
		/*tutorial.ChangeVehicleLockState(car, true);	
		
		SCR_VehicleSpawnProtectionComponent comp = SCR_VehicleSpawnProtectionComponent.Cast(car.FindComponent(SCR_VehicleSpawnProtectionComponent));
			
		if (comp)
			comp.SetReasonText("#AR-Tutorial_Action_Instructor");*/
		
		SCR_CompartmentAccessComponent compAccess = SCR_CompartmentAccessComponent.Cast(instructor.FindComponent(SCR_CompartmentAccessComponent));
		if (compAccess)
			compAccess.MoveInVehicle(car, ECompartmentType.CARGO);
	}
}