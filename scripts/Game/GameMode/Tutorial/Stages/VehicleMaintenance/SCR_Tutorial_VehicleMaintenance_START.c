[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 7);
		
		CreateMarkerCustom("VEHICLEMAINTENANCE_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
	}

	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_TAKE_WRENCH_AND_JERRY_CAN_INSTRUCTOR_P_05" || GetDuration() > 19000;
	}
};