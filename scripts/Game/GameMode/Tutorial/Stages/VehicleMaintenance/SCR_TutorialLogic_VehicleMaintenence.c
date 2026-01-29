[BaseContainerProps()]
class SCR_TutorialLogic_VehicleMaintenance : SCR_BaseTutorialCourseLogic
{	
	protected VehicleControllerComponent vehicleController;
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{	
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;

		PrepareSupplies();
		DamageVehicle();
		
		MoveIntoStorage("Repair_Wrench");
		MoveIntoStorage("Repair_Jerrycan");
		
		IEntity jerryCan = GetGame().GetWorld().FindEntityByName("Repair_Jerrycan");
		if (!jerryCan)
			return;
		
		SCR_FuelManagerComponent fuelMan = SCR_FuelManagerComponent.Cast(jerryCan.FindComponent(SCR_FuelManagerComponent));
		if (fuelMan)
			fuelMan.SetTotalFuelPercentage(0);
		
		Vehicle veh = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("RepairingVehicle"));
		if (veh)
		{
			vehicleController = veh.GetVehicleController();
			GetGame().GetCallqueue().CallLater(IsVehicleDrowned, 1000, true);
		}
	};
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputChanged);
		GetGame().GetCallqueue().Remove(IsVehicleDrowned);
	};
	
	//------------------------------------------------------------------------------------------------
	protected void IsVehicleDrowned()
	{
		if (!vehicleController)
		{
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputChanged);
			return;
		}
		
		if (!vehicleController.GetEngineDrowned())
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.RequestBreakCourse(SCR_ETutorialBreakType.FORCED);
	}
	
	//------------------------------------------------------------------------------------------------
	void MoveIntoStorage(string entityName)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName(entityName);
		if (!ent)
			return;
		
		IEntity storageEnt = GetGame().GetWorld().FindEntityByName("toolBox");
		if (!storageEnt)
			return;
		
		SCR_InventoryStorageManagerComponent inventoryStorageComp = SCR_InventoryStorageManagerComponent.Cast(storageEnt.FindComponent(SCR_InventoryStorageManagerComponent));
		if (!inventoryStorageComp)
			return;
		
		inventoryStorageComp.InsertItem(ent);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void PrepareSupplies()
	{
		IEntity cargo = GetGame().GetWorld().FindEntityByName("StorageDepo");
		if (!cargo)
			return;
		
		SCR_ResourceComponent resourceComponent = SCR_ResourceComponent.FindResourceComponent(cargo);
		if (!resourceComponent)
			return;
		
		SCR_ResourceConsumer resourceConsumer = resourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		if (resourceConsumer)
			resourceConsumer.RequestConsumtion(resourceConsumer.GetAggregatedResourceValue());
		
		SCR_ResourceGenerator resourceGenerator = resourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT_STORAGE, EResourceType.SUPPLIES);
		if (resourceGenerator)
			resourceGenerator.RequestGeneration(250);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void DamageVehicle()
	{
		Vehicle courseVehicle = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("RepairingVehicle"));
		if (!courseVehicle)
			return;
		
		SCR_DamageManagerComponent damageManagerBroken = SCR_DamageManagerComponent.GetDamageManager(courseVehicle);
		if (damageManagerBroken)
		{
			HitZone engineHitZone = damageManagerBroken.GetHitZoneByName("Engine_01");
			
			if (engineHitZone)
			{
				// This will work only if hitzone belongs to this damage manager
				vector transform[3];
				transform[0] = courseVehicle.GetOrigin();
				transform[1] = vector.Forward;
				transform[2] = vector.Up;

				DamageManagerComponent engineDamageManager = DamageManagerComponent.Cast(engineHitZone.GetHitZoneContainer());
				if (engineDamageManager)
				{
					SCR_DamageContext damageContext1 = new SCR_DamageContext(EDamageType.TRUE, engineHitZone.GetMaxHealth(), transform, engineDamageManager.GetOwner(), engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);
					SCR_DamageContext damageContext2 = new SCR_DamageContext(EDamageType.COLLISION, 50, transform, engineDamageManager.GetOwner(), engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);

					engineDamageManager.HandleDamage(damageContext1);
					engineDamageManager.HandleDamage(damageContext2);
				}
			}
			
			engineHitZone = damageManagerBroken.GetHitZoneByName("FuelTank_01");
			
			if (engineHitZone)
			{
				// This will work only if hitzone belongs to this damage manager
				vector transform[3];
				transform[0] = courseVehicle.GetOrigin();
				transform[1] = vector.Forward;
				transform[2] = vector.Up;

				DamageManagerComponent engineDamageManager = DamageManagerComponent.Cast(engineHitZone.GetHitZoneContainer());
				if (engineDamageManager)
				{
					SCR_DamageContext damageContext1 = new SCR_DamageContext(EDamageType.TRUE, engineHitZone.GetMaxHealth(), transform, engineDamageManager.GetOwner(), engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);
					SCR_DamageContext damageContext2 = new SCR_DamageContext(EDamageType.COLLISION, 50, transform, engineDamageManager.GetOwner(), engineHitZone, Instigator.CreateInstigator(null), null, -1, -1);

					engineDamageManager.HandleDamage(damageContext1);
					engineDamageManager.HandleDamage(damageContext2);
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputChanged(bool gamepad)
	{
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		if (gamepad)
		{
			if (tutorial.GetStageIndexByName("GAMEPAD") == -1)
				tutorial.InsertStage("GAMEPAD", tutorial.GetActiveStageIndex());
			
			tutorial.SetStage("GAMEPAD");
			return;
		}
		
		if (tutorial.GetStageIndexByName("KMB") == -1)
			tutorial.InsertStage("KMB", tutorial.GetActiveStageIndex());
		
		tutorial.SetStage("KMB");
	}
}