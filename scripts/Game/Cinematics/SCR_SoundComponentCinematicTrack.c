[CinematicTrackAttribute(name:"Sound Component Track", description:"Track used for triggering sound on SoundComponent")]
class SCR_SoundComponentCinematicTrack : SCR_CinematicTrackBase
{
	[Attribute("")]
	protected string m_sSoundEvent;
	
	[Attribute("")]
	protected string m_sBoneName;
	
	protected IEntity m_owner;
	
	override void OnInit(World world)
	{
		super.OnInit(world);
		
		m_owner = FindOwner();
	}
		
	[CinematicEventAttribute()]
	void PlayEvent()
	{
		if (m_sSoundEvent.IsEmpty())
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
					
		SoundComponent soundComponent = SoundComponent.Cast(m_owner.FindComponent(SoundComponent));			
		if (!soundComponent)
		{
			return;
		}
		
		if (m_sBoneName.IsEmpty())
		{
			soundComponent.SoundEvent(m_sSoundEvent);
		}
		else
		{
			soundComponent.SoundEventBone(m_sSoundEvent, m_sBoneName)
		}
	}
}