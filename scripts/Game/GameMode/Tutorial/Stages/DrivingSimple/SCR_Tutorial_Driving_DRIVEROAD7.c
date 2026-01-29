[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_DRIVEROAD7Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_DRIVEROAD7 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_7", "", "RIGHT");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};