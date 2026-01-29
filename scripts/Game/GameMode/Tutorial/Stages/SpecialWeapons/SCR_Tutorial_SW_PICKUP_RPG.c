[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_PICKUP_RPGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_PICKUP_RPG : SCR_BaseTutorialStage
{	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		if (m_TutorialComponent.IsEntityInPlayerInventory("COURSE_RPG"))
		{
			m_bFinished = true;
			return;
		}
		
		m_TutorialComponent.SpawnAsset("COURSE_RPG", "{7A82FE978603F137}Prefabs/Weapons/Launchers/RPG7/Launcher_RPG7.et");
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Insert(OnItemAdded);

		RegisterWaypoint("COURSE_RPG", "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool OnItemAdded(IEntity item, BaseInventoryStorageComponent storageComponent)
	{
		if (item && item.GetName() != "COURSE_RPG")
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
	void ~SCR_Tutorial_SW_PICKUP_RPG()
	{
		if (!m_Player)
			return;
		
		SCR_InventoryStorageManagerComponent storageManComp = SCR_InventoryStorageManagerComponent.Cast(m_Player.FindComponent(SCR_InventoryStorageManagerComponent));
		if (storageManComp)	
			storageManComp.m_OnItemAddedInvoker.Remove(OnItemAdded);
	}
}