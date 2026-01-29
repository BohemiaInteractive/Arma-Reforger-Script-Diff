[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_SpawnEntity_2Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_SpawnEntity_2 : SCR_BaseTutorialStage
{
	protected string m_sSpawnedEntityName = "BUILDING_GUNNEST";
	protected SCR_Waypoint m_SuggestedWP;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_SuggestedWP = RegisterWaypoint("WP_SUGGESTED_GUNPOST_SPAWN");
		
		PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_E", 2);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return GetGame().GetWorld().FindEntityByName(m_sSpawnedEntityName);
	}
}