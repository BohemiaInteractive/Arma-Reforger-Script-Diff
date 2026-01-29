[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_SUPPLIESBAGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_SUPPLIESBAG : SCR_BaseTutorialStage
{
	protected const ResourceName BACKPACK_RESOURCENAME = "{06B68C58B72EAAC6}Prefabs/Items/Equipment/Backpacks/Backpack_ALICE_Medium.et";
	protected EquipedLoadoutStorageComponent m_LoadoutStorage;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("BagForSupplies", "", "PICKUP");
	
		m_LoadoutStorage = EquipedLoadoutStorageComponent.Cast(m_Player.FindComponent(EquipedLoadoutStorageComponent));
		
		SCR_InventoryStorageManagerComponent storageManager = m_TutorialComponent.GetPlayerInventory();
		if (storageManager)	
			storageManager.m_OnInventoryOpenInvoker.Insert(OnInventoryClosedHint);
		
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 9);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_LoadoutStorage)
			return false;
		
		IEntity backpackEnt = m_LoadoutStorage.GetClothFromArea(LoadoutBackpackArea);
		if (!backpackEnt)
			return false;
		
		EntityPrefabData prefabData = backpackEnt.GetPrefabData();
		if (!prefabData)
			return false;
			
		return prefabData.GetPrefabName() == BACKPACK_RESOURCENAME; 
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInventoryClosedHint(bool isClosed)
	{
		if (isClosed)
			return;

		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowHint, 500, false, m_StageInfo.GetHint(), false, false);
		
		SCR_InventoryStorageManagerComponent storageManager = m_TutorialComponent.GetPlayerInventory();
		if (storageManager)
			storageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryClosedHint);
			
	}
};