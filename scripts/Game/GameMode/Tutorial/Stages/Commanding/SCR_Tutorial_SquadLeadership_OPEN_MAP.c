[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_OPEN_MAPClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_OPEN_MAP : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_MapEntity.GetOnMapOpen().Remove(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
		SCR_MapEntity.GetOnMapOpen().Insert(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(m_TutorialComponent.OnMapClose);

		CreateMarkerCustom("WP_SquadLeadership_CROSSROAD", SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 13);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.GetIsMapOpen();
	}
}