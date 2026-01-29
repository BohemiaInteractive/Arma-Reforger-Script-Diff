[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_CANPICKUPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_CANPICKUP : SCR_BaseTutorialStage
{
	private bool m_bIsFuelComp;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		RegisterWaypoint("Repair_Jerrycan", "", "PICKUP");
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 12);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.IsEntityInPlayerInventory("Repair_Jerrycan");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		SCR_CharacterInventoryStorageComponent characterStorage = SCR_CharacterInventoryStorageComponent.Cast(m_Player.FindComponent(SCR_CharacterInventoryStorageComponent));
		if (!characterStorage)
		{
			super.OnStageFinished();
			return;
		}
		
		IEntity jerryCan = GetGame().GetWorld().FindEntityByName("Repair_Jerrycan");
		if (jerryCan)
			characterStorage.StoreItemToQuickSlot(jerryCan, 5, true);
		
		super.OnStageFinished();
	}
};