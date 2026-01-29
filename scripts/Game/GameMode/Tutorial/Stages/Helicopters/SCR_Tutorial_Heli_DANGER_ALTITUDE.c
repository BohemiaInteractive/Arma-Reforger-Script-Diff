[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_DANGER_ALTITUDEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_DANGER_ALTITUDE : SCR_BaseTutorialStage
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 16);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;
		
		return m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("AltitudeAGL")) > 25;
	}
};