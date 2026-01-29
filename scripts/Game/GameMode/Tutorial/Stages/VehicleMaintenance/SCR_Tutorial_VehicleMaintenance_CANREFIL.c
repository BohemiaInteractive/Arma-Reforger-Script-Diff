[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_CANREFILClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_CANREFIL : SCR_BaseTutorialStage
{	
	SCR_FuelManagerComponent m_FuelManager;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		IEntity jerryCan = GetGame().GetWorld().FindEntityByName("Repair_Jerrycan");
		if (!jerryCan)
			return;
		
		m_FuelManager = SCR_FuelManagerComponent.Cast(jerryCan.FindComponent(SCR_FuelManagerComponent));
		RegisterWaypoint(jerryCan, "", "CUSTOM");

		SCR_InventoryStorageManagerComponent storageManager = m_TutorialComponent.GetPlayerInventory();
		if (storageManager)
			storageManager.m_OnInventoryOpenInvoker.Insert(OnInventoryClosedHint);
		
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_FuelManager)
			return false;
		
		return m_FuelManager.GetTotalFuel() > 19;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInventoryClosedHint(bool isClosed)
	{
		if (isClosed)
			return;
		
		SCR_HintManagerComponent.ClearLatestHint();
		SCR_HintManagerComponent.HideHint();
			
		GetGame().GetCallqueue().CallLater(SCR_HintManagerComponent.ShowHint, 500, false, m_StageInfo.GetHint(), false, false);
		
		SCR_InventoryStorageManagerComponent storageManager = m_TutorialComponent.GetPlayerInventory();	
		if (storageManager)
			storageManager.m_OnInventoryOpenInvoker.Remove(OnInventoryClosedHint);
	}
};