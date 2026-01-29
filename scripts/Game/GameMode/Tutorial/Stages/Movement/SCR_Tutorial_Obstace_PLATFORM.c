[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_PLATFORMClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_PLATFORM : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_PLATFORM");
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 13);
	}
};