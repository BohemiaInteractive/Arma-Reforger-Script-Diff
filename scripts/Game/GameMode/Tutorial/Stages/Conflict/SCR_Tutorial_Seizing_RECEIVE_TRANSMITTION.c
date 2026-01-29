[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_RECEIVE_TRANSMITTIONClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_RECEIVE_TRANSMITTION : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("CONFLICT_Instructor_B", 7);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_CAPTURING_POSITIONS_RADIO_ACTIVE_INSTRUCTOR_B_09" || GetDuration() > 36000;
	}
};