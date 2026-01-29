[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Seizing_STARTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Seizing_START : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		CreateMarkerCustom("CONFLICT_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);

		//TODO> We are checking this twice
		SCR_TutorialLogic_SeizingCourse logic = SCR_TutorialLogic_SeizingCourse.Cast(m_Logic);
		if (logic && !logic.HasPlayerAnyFirearm())
			m_bFinished = true;
		else
			PlayNarrativeCharacterStage("CONFLICT_Instructor_B", 2);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_bFinished || m_sLastFinishedEvent == "SOUND_TUTORIAL_EX_PLAYER_HAS_WEAPON_INSTRUCTOR_B_01" || GetDuration() > 6000;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStageFinished()
	{
		SCR_TutorialLogic_SeizingCourse logic = SCR_TutorialLogic_SeizingCourse.Cast(m_Logic);
		if (logic && !logic.HasPlayerAnyFirearm())
			m_TutorialComponent.InsertStage("MISSING_WEAPON");

		super.OnStageFinished();
	}
}