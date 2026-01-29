[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_ExitBuilding_2Class : SCR_BaseTutorialStageClass
{
}

class SCR_Tutorial_CombatEngineering_ExitBuilding_2 : SCR_BaseTutorialStage
{
	protected IEntity m_SpawnedEntity;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_SpawnedEntity = GetGame().GetWorld().FindEntityByName("BUILDING_GUNNEST");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SpawnedEntity)
		{
			m_TutorialComponent.SetStage("SPAWN_ENTITY_2");
			return false;
		}
		
		return !IsBuildingModeOpen();
	}
}