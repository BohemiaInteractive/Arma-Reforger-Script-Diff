[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_SpawnEntity_1Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_SpawnEntity_1 : SCR_BaseTutorialStage
{
	protected string m_sSpawnedEntityName = "BUILDING_VEHICLE";
	protected SCR_Waypoint m_SuggestedWP;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_SuggestedWP = RegisterWaypoint("WP_SUGGESTED_VEHICLE_SPAWN");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return GetGame().GetWorld().FindEntityByName(m_sSpawnedEntityName);
	}
}