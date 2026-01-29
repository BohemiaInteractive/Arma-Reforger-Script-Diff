[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_GETOUTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_GETOUT : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_END", "", "GETOUT");
		PlayNarrativeCharacterStage("DRIVING_Copilot", 13);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return !m_Player.IsInVehicle();
	}
};