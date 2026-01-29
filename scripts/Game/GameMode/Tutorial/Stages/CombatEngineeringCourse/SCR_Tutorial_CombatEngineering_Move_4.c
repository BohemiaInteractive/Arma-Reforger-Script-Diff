[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_Move_4Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_Move_4 : SCR_BaseTutorialStage
{	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CE_PATH4");		

		IEntity wpEnt = GetGame().GetWorld().FindEntityByName("WP_CE_PATH6");
		if (wpEnt)
			UnregisterWaypoint(wpEnt);
		
		SCR_Waypoint wp = RegisterWaypoint(wpEnt);
		if (wp)
			wp.m_iMaximumDrawDistance = 50
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnStageFinished()
	{
		if (!m_TutorialComponent)
			return;
		
		IEntity wpPos = m_ReachedWaypoint.GetAttachedEntity();
		if (wpPos.GetName() == "WP_CE_PATH6")
			m_TutorialComponent.SetStage("StopAndGetOut");
		else
			m_TutorialComponent.FinishStage();
	}
};