[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_ZIGZAGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_ZIGZAG : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_ZIGZAG_2");
		
		if (!SCR_TutorialLogic_Obstacle.Cast(m_Logic).m_bObstacleStartedEarly)
			PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 4);
	}
};