[EntityEditorProps(insertable: false)]
class SCR_Tutorial_SquadLeadership_ORDER_GETIN_CARClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_SquadLeadership_ORDER_GETIN_CAR : SCR_BaseTutorialStage
{
	IEntity m_Soldier;
	Vehicle m_Vehicle;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		BaseWorld world = GetGame().GetWorld();

		m_Vehicle = Vehicle.Cast(world.FindEntityByName("PlayerVehicle"));
		
		RegisterWaypoint(m_Vehicle, "", "GETIN");
		
		PlayNarrativeCharacterStage("SQUADLEADERSHIP_Instructor", 12);
	}

	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Vehicle)
			return false;
		
		IEntity driver = m_Vehicle.GetPilot();
		return driver && driver != m_Player;
	}
}