[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MAP_INTROCTUCTIONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MAP_INTROCTUCTION : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_MAP_OPENED_INSTRUCTOR_Q_06" || GetDuration() > 25000;
	}
};