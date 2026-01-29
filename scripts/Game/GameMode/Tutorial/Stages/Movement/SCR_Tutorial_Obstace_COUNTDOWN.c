[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_COUNTDOWNClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_COUNTDOWN : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_OBSTACLE_START");
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 2);
		
		SCR_TutorialLogic_Obstacle.Cast(m_Logic).m_bObstacleStartedEarly = false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		if (vector.DistanceSqXZ(m_Player.GetOrigin(), GetGame().GetWorld().FindEntityByName("WP_OBSTACLE_START").GetOrigin()) > 1.5)
		{
			m_fDelay = 0;
			SCR_TutorialLogic_Obstacle.Cast(m_Logic).m_bObstacleStartedEarly = true;
			PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 3);
			return true;
		}
		
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_OBSTACLE_COURSE_IN_INSTRUCTOR_A_03" || GetDuration() > 5000;
	}
};