[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_GETINClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_GETIN : SCR_BaseTutorialStage
{
	protected VehicleControllerComponent m_VehicleController;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle veh = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("RepairingVehicle"));
		if (!veh)
			return;
		
		m_VehicleController = VehicleControllerComponent.Cast(veh.FindComponent(VehicleControllerComponent));
		
		RegisterWaypoint(veh, "", "MOUNTCAR");
		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 19);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_VehicleController)
			return false;
		
		m_VehicleController.SetEngineStartupChance(100);
		
		return m_VehicleController.IsEngineOn() && m_Player.IsInVehicle();
	}
};