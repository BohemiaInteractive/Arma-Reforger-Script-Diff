[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Obstacle_LADDERUPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Obstacle_LADDERUP : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_LADDER_UP", "", "LADDER");

		PlayNarrativeCharacterStage("OBSTACLECOURSE_Instructor", 12);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.CheckCharacterStance(ECharacterCommandIDs.LADDER);
	}
};