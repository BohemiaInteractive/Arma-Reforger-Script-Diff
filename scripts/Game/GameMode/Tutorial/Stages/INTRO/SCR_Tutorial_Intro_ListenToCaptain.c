[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Intro_ListenToCaptainClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Intro_ListenToCaptain : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_INTRO_BR_CAPTAIN_BRIEFING_CAPTAIN_11" || m_sLastFinishedEvent == "SOUND_TUTORIAL_INTRO_AM_CAPTAIN_RESUMING_CAPTAIN_01" || GetDuration() > 18000;
	}
}