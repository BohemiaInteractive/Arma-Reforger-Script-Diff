[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		CreateMarkerCustom("SQUADLEADERSHIP_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 2);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_SQUAD_REQUISITION_INSTRUCTOR_O_01" || GetDuration() > 4000;
	}
}