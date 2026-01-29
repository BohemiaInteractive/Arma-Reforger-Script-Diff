[EntityEditorProps(insertable: false)]
class SCR_Tutorial_CombatEngineering_Move_1Class: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_CombatEngineering_Move_1 : SCR_BaseTutorialStage
{
	
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		RegisterWaypoint("WP_CE_PATH1");

		SCR_Waypoint wp = RegisterWaypoint("WP_CE_PATH6");
		if (wp)
			wp.m_iMaximumDrawDistance = 50;
		
		CreateMarkerPath("CE_PATH_DOTS");
		CreateMarkerCustom("WP_CE_PATH6", SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
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