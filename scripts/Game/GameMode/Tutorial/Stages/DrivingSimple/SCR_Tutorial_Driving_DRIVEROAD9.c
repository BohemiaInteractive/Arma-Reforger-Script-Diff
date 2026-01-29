[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_DRIVEROAD9Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_DRIVEROAD9 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_9");
		SCR_HintManagerComponent.HideHint();
		
		PlayNarrativeCharacterStage("DRIVING_Copilot", 12);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};