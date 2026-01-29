[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SW_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SW_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		PlayNarrativeCharacterStage("SPECIALWEAPONS_InstructorM", 1);
		
		CreateMarkerCustom("SPECIALWEAPONS_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_TAKE_GRENADE_INSTRUCTOR_M_02" || GetDuration() > 32500;
	}
}