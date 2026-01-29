[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_ENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_END : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("OBSTACLECOURSE_Instructor");
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 15);
	}
};