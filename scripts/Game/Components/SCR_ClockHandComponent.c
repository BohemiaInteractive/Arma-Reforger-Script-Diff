#define ENABLE_BASE_DESTRUCTION
[EntityEditorProps(category: "Components/SCR_ClockHandComponentClass", description: "Component handlinng move of clock hands.")]
class SCR_ClockHandComponentClass : ScriptComponentClass
{
}

class SCR_ClockHandComponent : ScriptComponent
{
	private TimeAndWeatherManagerEntity m_TimeManager;
	private SignalsManagerComponent m_SignalManager;
	private SCR_DestructionMultiPhaseComponent m_MultiPhaseDestComp;
	private int m_iClockSignalIndex;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(owner.GetWorld());
		if (!world)
			return;
		
		m_TimeManager = world.GetTimeAndWeatherManager();
		if (!m_TimeManager)
			return;
		
		m_SignalManager = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (!m_SignalManager)
			return;
		
		m_iClockSignalIndex = m_SignalManager.AddOrFindSignal("ClockHand");
		m_MultiPhaseDestComp = SCR_DestructionMultiPhaseComponent.Cast(owner.FindComponent(SCR_DestructionMultiPhaseComponent));
				
		if (m_iClockSignalIndex != -1)
			GetGame().GetCallqueue().CallLater(ClockHandStep, 1000, true);	
		
		ClearEventMask(owner,EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ClockHandStep()
	{
#ifdef ENABLE_BASE_DESTRUCTION
		if (m_MultiPhaseDestComp && m_MultiPhaseDestComp.GetDestroyed())
			GetGame().GetCallqueue().Remove(ClockHandStep);
#endif
		
		m_SignalManager.SetSignalValue(m_iClockSignalIndex, m_TimeManager.GetTimeOfTheDay());
	}
}
