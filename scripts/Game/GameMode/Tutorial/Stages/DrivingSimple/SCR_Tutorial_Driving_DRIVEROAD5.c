[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Driving_DRIVEROAD5Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Driving_DRIVEROAD5 : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_DRIVING_5");
		PlayNarrativeCharacterStage("DRIVING_Copilot", 9);
		
		SCR_HintManagerComponent.HideHint();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_Player.IsInVehicle();
	}
};