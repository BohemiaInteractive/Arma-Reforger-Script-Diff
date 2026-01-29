[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_RPM_WAITClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_RPM_WAIT : SCR_BaseTutorialStage
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{	
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ClearLatestHint();
		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 2);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;
		
		return m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("MainRotorRPM")) >= 324;
	}
};