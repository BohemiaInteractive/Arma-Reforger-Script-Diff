[EntityEditorProps(insertable: false)]
class SCR_Tutorial_Heli_CYCLIC_ANTITORQUE_RIGHTClass: SCR_BaseTutorialStageClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_Tutorial_Heli_CYCLIC_ANTITORQUE_RIGHT : SCR_BaseTutorialStage
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	//------------------------------------------------------------------------------------------------
	override protected void Setup()
	{		
		Vehicle helicopter = Vehicle.Cast(GetGame().GetWorld().FindEntityByName("UH1COURSE"));
		if (!helicopter)
			return;
		
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(helicopter.FindComponent(SignalsManagerComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool GetIsFinished()
	{
		if (!m_SignalsManagerComponent)
			return false;
		
		return m_SignalsManagerComponent.GetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("TurnRate")) >= 30;
	}
};