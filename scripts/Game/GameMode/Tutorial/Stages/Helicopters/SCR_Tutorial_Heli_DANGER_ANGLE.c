[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_DANGER_ANGLEClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_DANGER_ANGLE : SCR_BaseTutorialStage
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		PlayNarrativeCharacterStage("HELICOPTER_Copilot", 17);
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;
		
		float angle = m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("PitchAngle"));
		float roll = m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("RollAngle"));
		
		return (angle >= -30 && angle <= 40) && (roll >= -30 && roll <= 30);
	}
};