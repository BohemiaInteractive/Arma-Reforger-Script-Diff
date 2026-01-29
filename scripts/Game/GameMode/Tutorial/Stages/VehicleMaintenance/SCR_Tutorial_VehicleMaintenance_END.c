[EntityEditorProps(insertable: false)]
class SCR_Tutorial_VehicleMaintenance_ENDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_VehicleMaintenance_END : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();

		PlayNarrativeCharacterStage("VEHICLEMAINTENANCE_Instructor", 23);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_JEEP_FULLY_REFUELLED_INSTRUCTOR_P_03" || GetDuration() > 12000;
	}
};