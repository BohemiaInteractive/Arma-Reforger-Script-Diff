[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_BoardVehicleClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_BoardVehicle : SCR_BaseTutorialStage
{
	protected Vehicle m_Vehicle;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{		
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		m_Vehicle = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE"));
		if (!m_Vehicle)
			return;
		
		PlayNarrativeCharacterStage("COMBATENGINEERING_Instructor_D", 3);

		RegisterWaypoint(m_Vehicle, "", "MOUNTCAR");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Vehicle)
			return false;
		
		return m_Vehicle.GetPilot() == m_Player;
	}
};