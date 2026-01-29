[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_ENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_END : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("CONFLICT_Instructor_C", 7);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_BUILD_ANTENNA_INSTRUCTOR_C_08" || GetDuration() > 15000;
	}
}