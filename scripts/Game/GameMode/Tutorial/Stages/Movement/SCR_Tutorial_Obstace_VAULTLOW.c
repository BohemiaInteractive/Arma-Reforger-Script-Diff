[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_VAULTLOWClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_VAULTLOW : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_VAULT", "", "JUMP").SetOffsetVector("0 0.2 0");
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 7);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.CLIMB);
	}
};