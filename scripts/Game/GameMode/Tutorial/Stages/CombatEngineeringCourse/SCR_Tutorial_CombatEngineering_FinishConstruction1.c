[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_FinishConstruction1Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_FinishConstruction1 : SCR_BaseTutorialStage
{
	SCR_CampaignBuildingCompositionComponent m_CampaignBuildingComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		IEntity bunker = GetGame().GetWorld().FindEntityByName("BUILDING_GUNNEST");
		if (!bunker)
			return;
		
		PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_E", 4);
		
		RegisterWaypoint(bunker, "", "BUILD");
		
		m_CampaignBuildingComponent = SCR_CampaignBuildingCompositionComponent.Cast(bunker.FindComponent(SCR_CampaignBuildingCompositionComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_CampaignBuildingComponent)
		{
			m_TutorialComponent.SetStage("ENTER_BUILDING_2");
			return false;
		}
		
		return m_CampaignBuildingComponent.IsCompositionSpawned();
	}
};