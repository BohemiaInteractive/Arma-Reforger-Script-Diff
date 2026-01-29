[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_STOP_ENGINEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_STOP_ENGINE : SCR_BaseTutorialStage
{
	protected bool m_bEngineOff;
	protected ScriptInvokerVoid m_OnEngineStartedInvoker;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		SCR_HelicopterControllerComponent comp = SCR_HelicopterControllerComponent.Cast(helicopter.FindComponent(SCR_HelicopterControllerComponent));
		if (!comp)
			return;
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 14);
		
		m_OnEngineStartedInvoker = comp.GetOnEngineStop();
		if (m_OnEngineStartedInvoker)
			comp.GetOnEngineStop().Insert(GetOnEngineStop);
		
	}
	//------------------------------------------------------------------------------------------------
	protected void GetOnEngineStop()
	{
		m_bEngineOff = true;
		
		if (!m_OnEngineStartedInvoker)
			m_OnEngineStartedInvoker.Remove(GetOnEngineStop);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		return m_bEngineOff;
	}
};