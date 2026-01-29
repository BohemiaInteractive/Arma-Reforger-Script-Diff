[BaseContainerProps()]
class SCR_TutorialLogic_CombatEngineering : SCR_BaseTutorialCourseLogic
{
	bool m_bBuildingVoiceLinePlayed;
	protected const ResourceName CONSTRUCTION_TRUCK_PREFAB = "{F54F833E747C1B77}Prefabs/Vehicles/Wheeled/M923A1/Tutorial_Engineering_Truck.et";
	protected const ResourceName MACHINEGUN_NEST_PREFAB = "{6E9DCCF936BF17B9}PrefabsEditable/Auto/Compositions/Slotted/SlotFlatSmall/E_MachineGunNest_S_US_02.et";
	
	//------------------------------------------------------------------------------------------------
	void HandleArlevilleSupplies(bool refill)
	{
		IEntity ent = GetGame().GetWorld().FindEntityByName("ARLEVILLE_SUPPLY_STORAGE");
		if (!ent)
			return;

		SCR_ResourceComponent resComp = SCR_ResourceComponent.FindResourceComponent(ent);
		if (!resComp)
			return;

		SCR_ResourceContainer resourceContainer = resComp.GetContainer(EResourceType.SUPPLIES);
		if (!resourceContainer)
			return;
		
		resourceContainer.DepleteResourceValue();
		
		if (refill)
			resourceContainer.IncreaseResourceValue(810);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntitySpawned(IEntity ent)
	{
		if (!ent)
			return;
		
		ResourceName spawnedEntityResourceName = ent.GetPrefabData().GetPrefabName();
		if (spawnedEntityResourceName == MACHINEGUN_NEST_PREFAB)
		{
			ent.SetName("BUILDING_GUNNEST");
			return;
		}
		
		if (spawnedEntityResourceName != CONSTRUCTION_TRUCK_PREFAB)
			return;
		
		ent.SetName("BUILDING_VEHICLE");
		SlotManagerComponent slotMan = SlotManagerComponent.Cast(ent.FindComponent(SlotManagerComponent));
		if (!slotMan)
			return;
		
		EntitySlotInfo slot = slotMan.GetSlotByName("EngineerBox");
		if (!slot)
			return;
		
		IEntity box = slot.GetAttachedEntity();
		if (!box)
			return;
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (tutorial)
			tutorial.BlockBuildingModeAccess(box, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseStart()
	{	
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		IEntity service = GetGame().GetWorld().FindEntityByName("VEHICLE_REQUESTING_BOARD");
		if (service)
		{
			SCR_CampaignBuildingProviderComponent buildingComponent = SCR_CampaignBuildingProviderComponent.Cast(service.FindComponent(SCR_CampaignBuildingProviderComponent));
			if (buildingComponent)
				buildingComponent.RemovePlayerCooldowns(1);
		}
		
		SCR_EntityHelper.DeleteEntityAndChildren(GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE"));
		SCR_EntityHelper.DeleteEntityAndChildren(GetGame().GetWorld().FindEntityByName("BUILDING_GUNNEST"));
		
		tutorial.GetOnEntitySpawned().Insert(OnEntitySpawned);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnCourseEnd()
	{	
		m_bBuildingVoiceLinePlayed = false;
		HandleArlevilleSupplies(false);
		
		SCR_TutorialGamemodeComponent tutorial = SCR_TutorialGamemodeComponent.GetInstance();
		if (!tutorial)
			return;
		
		tutorial.GetOnEntitySpawned().Remove(OnEntitySpawned);
		
		IEntity service = GetGame().GetWorld().FindEntityByName("VEHICLE_REQUESTING_BOARD");
		if (service)
			tutorial.BlockBuildingModeAccess(service, false);
		
		IEntity vehicle = GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE");
		if (!vehicle)
			return;
		
		tutorial.ChangeVehicleLockState(vehicle, true);
		tutorial.InsertIntoGarbage(vehicle);
	}
}