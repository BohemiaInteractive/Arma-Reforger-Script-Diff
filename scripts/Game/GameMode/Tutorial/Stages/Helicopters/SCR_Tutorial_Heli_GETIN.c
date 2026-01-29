[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_GETINClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_GETIN : SCR_BaseTutorialStage
{
	protected Vehicle m_Helicopter;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		m_Helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!m_Helicopter)
			return;
		
		RegisterWaypoint(m_Helicopter, "", "GETIN").SetOffsetVector("0 5 0");;
		m_TutorialComponent.ChangeVehicleLockState(m_Helicopter, false);
		
		GetGame().GetCallqueue().CallLater(PlayNarrativeCharacterStage, 500, false, "Soldier_Choppa", 1);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_Helicopter)
			return false;
		
		return m_Helicopter.GetPilot() == m_Player;
	}
};