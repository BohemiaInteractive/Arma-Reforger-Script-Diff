[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_WINDOWClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_WINDOW : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_WINDOW", "", "JUMP").SetOffsetVector("0 0.6 0");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 8);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};