[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MAP_COMPASS_INTRODUCTIONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MAP_COMPASS_INTRODUCTION : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_COMPASS_SELECTED_INSTRUCTOR_Q_03" || GetDuration() > 11000;
	}
};