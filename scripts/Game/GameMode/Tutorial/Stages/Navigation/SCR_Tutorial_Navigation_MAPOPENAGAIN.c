[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MAPOPENAGAINClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MAPOPENAGAIN : SCR_BaseTutorialStage
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
		
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 9);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.GetIsMapOpen();
	}
};