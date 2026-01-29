[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_DRIVEROAD8Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_DRIVEROAD8 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_8");
		PlayNarrativeCharacterStage("DRIVING_Copilot", 11);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};