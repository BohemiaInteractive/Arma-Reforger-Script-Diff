[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_PICKUPTOOLSClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_PICKUPTOOLS : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("toolBox", "", "PICKUP");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_TutorialComponent.IsEntityInPlayerInventory("Repair_Wrench"))
			return false;
		
		if (!m_TutorialComponent.IsEntityInPlayerInventory("Repair_Jerrycan"))
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		IEntity jerryCan = GetGame().GetWorld().FindEntityByName("Repair_Jerrycan");
		IEntity wrench = GetGame().GetWorld().FindEntityByName("Repair_Wrench"); 
		if (!characterStorage || !jerryCan || !wrench)
		{
			super.OnStageFinished();
			return;
		}
		
		characterStorage.StoreItemToQuickSlot(wrench, 4, true);
		characterStorage.StoreItemToQuickSlot(jerryCan, 5, true);
		
		super.OnStageFinished();
	}
};