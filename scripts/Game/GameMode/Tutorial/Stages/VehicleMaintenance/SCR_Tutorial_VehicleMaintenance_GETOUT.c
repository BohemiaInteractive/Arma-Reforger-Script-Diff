[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_GETOUTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_GETOUT : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_MOVE_DEPOT2", "", "GETOUT");	
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_Player.IsInVehicle();
	}
};