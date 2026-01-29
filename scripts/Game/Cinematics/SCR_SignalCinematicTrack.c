[CinematicTrackAttribute(name:"Signal Track", description:"Track used for animating signals of entities")]
class SCR_SignalCinematicTrack : SCR_CinematicTrackBase
{
	[Attribute("")]
	string m_sSignalName;
	
	[Attribute("0.0")]
	float m_fValue;
	
	[Attribute("")]
	bool m_bForceUpdate;
	
	protected IEntity m_owner;
	protected float m_fValueLast;
	
	override void OnInit(World world)
	{
		super.OnInit(world);
		
		m_owner = FindOwner();
	}
		
	override void OnApply(float time)
	{
		if (!m_bForceUpdate && float.AlmostEqual(m_fValue, m_fValueLast, 0.001))
		{
			return;
		}
		
		if (!m_owner)
		{
			m_owner = FindOwner();
		}
		
		if (!m_owner)
		{
			return;
		}
		
		SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(m_owner.FindComponent(SignalsManagerComponent));
		if (!signalsManagerComponent)
		{
			return;
		}
		
		const int signalIdx = signalsManagerComponent.AddOrFindSignal(m_sSignalName);
		signalsManagerComponent.SetSignalValue(signalIdx, m_fValue);
		
		m_fValueLast = m_fValue;
	}
}
