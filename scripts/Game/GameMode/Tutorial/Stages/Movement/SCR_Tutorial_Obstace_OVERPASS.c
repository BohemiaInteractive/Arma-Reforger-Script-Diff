[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_OVERPASSClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_OVERPASS : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_OVERPASS");
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 5);
	}
};