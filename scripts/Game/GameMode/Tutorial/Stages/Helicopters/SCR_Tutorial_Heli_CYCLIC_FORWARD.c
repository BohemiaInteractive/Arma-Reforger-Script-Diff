[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_CYCLIC_FORWARDClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_CYCLIC_FORWARD : SCR_BaseTutorialStage
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
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 6);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;
		
		return m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("PitchAngle")) <= -10;
	}
};