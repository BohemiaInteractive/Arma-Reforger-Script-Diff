[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_M72Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_M72 : SCR_BaseTutorialStage
{	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (m_TutorialComponent.FindPrefabInPlayerInventory("{9C5C20FB0E01E64F}Prefabs/Weapons/Launchers/M72/Launcher_M72A3.et"))
		{
			m_bFinished = true;
			return;
		}
		
		m_TutorialComponent.SpawnAsset("M72LAW", "{9C5C20FB0E01E64F}Prefabs/Weapons/Launchers/M72/Launcher_M72A3.et");
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);
		
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 6);

		RegisterWaypoint("M72LAW", "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		if (item && item.GetPrefabData().GetPrefabName() != "{9C5C20FB0E01E64F}Prefabs/Weapons/Launchers/M72/Launcher_M72A3.et")
			return false;
		
		m_bFinished = true;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_Tutorial_SW_PICKUP_M72()
	{
		if (!m_Player)
			return;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
	}
}