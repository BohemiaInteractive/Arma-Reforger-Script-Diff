[EntityEditorProps(insertable: false)]
class SCR_TutorialStageMedical_CHECK_EXPLAINClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_TutorialStageMedical_CHECK_EXPLAIN: SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		PlayNarrativeCharacterStage("FIRSTAID_Instructor", 4);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool GetIsFinished()
	{
		return m_sLastFinishedEvent == "SOUND_TUTORIAL_TA_INJURIES_ASSESSED_INSTRUCTOR_H_03" || GetDuration() > 17000;
	}

	//------------------------------------------------------------------------------------------------
	override void OnStageFinished()
	{
		if (!m_TutorialComponent.FindPrefabInPlayerInventory("{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et"))
			m_TutorialComponent.InsertStage("PickupTourniquet");
		
		super.OnStageFinished();
	}
};