[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_D", 1);
		
		CreateMarkerCustom("COMBATENGINEERING_Instructor_D", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_ENGINEERING_COURSE_IN_INSTRUCTOR_D_03" || GetDuration() > 15000;
	}
}