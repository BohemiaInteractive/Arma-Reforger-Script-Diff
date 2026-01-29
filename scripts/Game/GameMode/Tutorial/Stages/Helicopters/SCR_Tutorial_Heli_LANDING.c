[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_LANDINGClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_LANDING : SCR_BaseTutorialStage
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		SCR_TutorialLogic_Heli logic = SCR_TutorialLogic_Heli.Cast(m_TutorialComponent.GetActiveConfig().GetCourseLogic());
		if (!logic)
			return;
		
		logic.m_bCheckAngle = false;
		logic.m_bCheckAltitude = false;

		RegisterWaypoint("SpawnPos_UH1COURSE");
		CreateMarkerCustom("SpawnPos_UH1COURSE", SCR_EScenarioFrameworkMarkerCustom.OBJECTIVE_MARKER, SCR_EScenarioFrameworkMarkerCustomColor.REFORGER_ORANGE);
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 13);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;
		
		return m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("AltitudeAGL")) < 2;
	}
};