[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_ENDINGClass: SCR_BaseTutorialStageClass
{
};

class SCR_Tutorial_SW_ENDING : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override void Setup()
	{
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorN", 3);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_ROADBLOCK_DESTROYED_INSTRUCTOR_N_01" || GetDuration() > 5000;
	}
}