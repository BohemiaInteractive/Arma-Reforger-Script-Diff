[BaseContainerProps(configRoot: true), SCR_PlaceableEntitiesRegistryTitleField()]
class SCR_PlaceableEntitiesGameModeRegistry : SCR_PlaceableEntitiesRegistry
{
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		SCR_CampaignBuildingManagerComponent buildingManager = SCR_CampaignBuildingManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManager)
			return;

		array<ResourceName> prefabsOverride = buildingManager.GetPlaceablePrefabs();
		if (prefabsOverride)
			SetPrefabs(prefabsOverride);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_PlaceableEntitiesGameModeRegistry()
	{		
		if (SCR_Global.IsEditMode()) 
			return;
		
		Init();
	}
}