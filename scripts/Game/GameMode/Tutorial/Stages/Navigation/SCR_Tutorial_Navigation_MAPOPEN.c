[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Navigation_MAPOPENClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Navigation_MAPOPEN : SCR_BaseTutorialStage
{
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{			
		PlayNarrativeCharacterStage("NAVIGATION_Instructor", 2);

		CreateMarkerCustom("NAVIGATION_Instructor", SCR_EScenarioFrameworkMarkerCustom.POINT_OF_INTEREST, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);

		SCR_MapEntity.GetOnMapOpen().Remove(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(m_TutorialComponent.OnMapClose);
		SCR_MapEntity.GetOnMapOpen().Insert(m_TutorialComponent.OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(m_TutorialComponent.OnMapClose);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_TutorialComponent.GetIsMapOpen();
	}
};