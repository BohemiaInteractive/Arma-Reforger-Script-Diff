[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		PlayNarrativeCharacterStage("HELICOPTER_Instructor", 2);

		CreateMarkerCustom("HELICOPTER_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "Tutorial_TA_Helicopter_Course_In_Instructor_I_04" || GetDuration() > 16000;
	}
};