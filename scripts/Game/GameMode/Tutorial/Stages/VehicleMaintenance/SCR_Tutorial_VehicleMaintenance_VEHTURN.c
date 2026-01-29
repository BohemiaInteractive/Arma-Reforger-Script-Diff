[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_VEHTURNClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_VEHTURN : SCR_BaseTutorialStage
{	
	protected IEntity m_Vehicle;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_Vehicle = GetGame().GetWorld().FindEntityByName("RepairingVehicle");
		if (m_Vehicle)
			RegisterWaypoint(m_Vehicle, "", "UNFLIP");

		SCR_InventoryStorageManagerComponent storageManComp = m_TutorialComponent.GetPlayerInventory();
		if (storageManComp)
			storageManComp.m_OnInventoryOpenInvoker.Insert(OnInventoryClosedHint);
		
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 14);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Vehicle)
			return false;
		
		vector angles = m_Vehicle.GetAngles();
		if (float.AlmostEqual(0, angles[0], 45) && float.AlmostEqual(0, angles[2], 45))
		{
			SCR_HintManagerComponent.HideHint();
			SCR_HintManagerComponent.ClearLatestHint();
			PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 15);
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInventoryClosedHint(bool isClosed)
	{
		if (isClosed)
			return;
		
		SCR_HintManagerComponent.ClearLatestHint();
		SCR_HintManagerComponent.HideHint();
			
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowHint, 500, false, m_StageInfo.GetHint(), false, false);
		
		SCR_InventoryStorageManagerComponent storageManComp = m_TutorialComponent.GetPlayerInventory();
		if (storageManComp)
			storageManComp.m_OnInventoryOpenInvoker.Remove(OnInventoryClosedHint);
	}
};