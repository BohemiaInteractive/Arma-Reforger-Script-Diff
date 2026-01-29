[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_StopAndGetOutClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_StopAndGetOut : SCR_BaseTutorialStage
{
	protected Vehicle m_Car;
	protected vector m_vWpPos;
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Car = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("BUILDING_VEHICLE"));
		IEntity wpEnt = GetGame().GetWorld().FindEntityByName("WP_CE_PATH6");
		
		UnregisterWaypoint(wpEnt);
		
		SCR_Waypoint wp = RegisterWaypoint(wpEnt,"", "GETOUT");
		if (wp)
			m_vWpPos = wp.GetPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Car || m_vWpPos == vector.Zero)
			return false;

		return (m_Car.GetPilot() != m_Player) && (vector.Distance(m_Car.GetOrigin(), m_vWpPos) <= 6);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		if (m_TutorialComponent && m_Car)
			m_TutorialComponent.ChangeVehicleLockState(m_Car, true);
		
		super.OnStageFinished();
	}
};