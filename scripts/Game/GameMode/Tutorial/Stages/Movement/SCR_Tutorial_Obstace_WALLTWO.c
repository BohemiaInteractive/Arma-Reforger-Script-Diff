[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_WALLTWOClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_WALLTWO : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WALL2", "", "JUMP").SetOffsetVector("0 0.3 0");
		
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 11);

		GetGame().GetCallqueue().Remove(m_TutorialComponent.Check3rdPersonViewUsed);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};