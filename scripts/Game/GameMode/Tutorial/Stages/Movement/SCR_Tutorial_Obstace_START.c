[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_OBSTACLE_START");
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 1);
		
		CreateMarkerCustom("OBSTACLECOURSE_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return GetDuration() > 5000;
	}
};