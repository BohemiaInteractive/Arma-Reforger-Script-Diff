[EntityEditorProps(insertable: false)]
class SCR_Tutorial_ShootingRange_FIRINGPOS_ONEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_ShootingRange_FIRINGPOS_ONE : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		RegisterWaypoint("WP_FIREPOZ_1", "", "GUNREADY");
		PlayNarrativeCharacterStage("SHOOTINGRANGE_Instructor", 5);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_TO_POSITION_INSTRUCTOR_L_03" || GetDuration() > 8000;
	}
};