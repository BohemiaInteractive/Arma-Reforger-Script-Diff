[CinematicTrackAttribute(name:"Character Animation Event Track", description:"Uses character animation events to do stuff")]
class SCR_CharacterAnimationEventCinematicTrack : SCR_CinematicTrackBase
{
	static const string EVENT_NAME_SIGNAL = "SignalCinematicTimeline";
	static const string EVENT_NAME_SOUND = "SoundCinematicTimeline";
	
	[Attribute("")]
	protected string m_sSourceEntityName;
	
	[Attribute("")]
	protected bool m_bUsePlayerCharacter;
	
	[Attribute("")]
	protected string m_sTargerEntityName;
	
	bool m_bIsFirstFrame = true;
			
	//------------------------------------------------------------------------------------------------
	//! Callback called when animation event happens.
	//! \param[in] animEventType ID of animation event.
	//! \param[in] animUserString String parameter of animation event
	//! \param[in] intParam int parameter of animation event
	//! \param[in] timeFromStart time from Start of animation event
	//! \param[in] timeToEnd time to end of animation event
	protected void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		const string eventName = GameAnimationUtils.GetEventString(animEventType);
		
		// Sets signal on animation event
		// animUserString = signal name
		// intParam = signal value
		if (eventName == EVENT_NAME_SIGNAL)
		{
			IEntity entity = FindOwner(m_sTargerEntityName);
			if (!entity)
			{
				return;
			}
			
			SignalsManagerComponent signalsManagerComponent = SignalsManagerComponent.Cast(entity.FindComponent(SignalsManagerComponent));
			if (!signalsManagerComponent)
			{
				return;
			}
			
			const string signalName = GameAnimationUtils.GetEventString(animUserString);
			signalsManagerComponent.SetSignalValue(signalsManagerComponent.AddOrFindSignal(signalName), intParam);
		}
		// Plays sound on linked entity
		// animUserString = sound event name
		else if(eventName == EVENT_NAME_SOUND)
		{
			IEntity entity = FindOwner(m_sTargerEntityName);
			if (!entity)
			{
				return;
			}
			
			SoundComponent soundComponent = SoundComponent.Cast(entity.FindComponent(SoundComponent));
			if (!soundComponent)
			{
				return;
			}
			
			const string soundEventName = GameAnimationUtils.GetEventString(animUserString);
			soundComponent.SoundEvent(soundEventName);
		}
	}
	
	IEntity GetSourceEntity()
	{	
		if (m_bUsePlayerCharacter)
		{			
			PlayerController playerController =  GetGame().GetPlayerController();
			if (!playerController)
			{
				return null;
			}
			
			return playerController.GetControlledEntity();
		}
		else
		{
			return FindOwner(m_sSourceEntityName);
		}
	}
	
	override void OnApply(float time)
	{
		super.OnApply(time);
		
		if (m_bIsFirstFrame)
		{
			// Register to OnAnimation event		
			const IEntity sourceEntity = GetSourceEntity();
			if (!sourceEntity)
			{
				return;
			}
			
			const SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(sourceEntity);
			if (!chimeraCharacter)
			{
				return;
			}
						
			SCR_CharacterControllerComponent characterController =  SCR_CharacterControllerComponent.Cast(chimeraCharacter.FindComponent(SCR_CharacterControllerComponent));
			if (!characterController)
			{
				return;
			}
			
			characterController.GetOnAnimationEvent().Insert(OnAnimationEvent);
			
			m_bIsFirstFrame = false;
		}
	}
			
	override void OnFinish()
	{
		super.OnFinish();
		
		// Unregister OnAnimation event
		const IEntity sourceEntity = GetSourceEntity();
		if (!sourceEntity)
		{
			return;
		}
		
		const SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(sourceEntity);
		if (!chimeraCharacter)
		{
			return;
		}
					
		SCR_CharacterControllerComponent characterController =  SCR_CharacterControllerComponent.Cast(chimeraCharacter.FindComponent(SCR_CharacterControllerComponent));
		if (!characterController)
		{
			return;
		}
		
		characterController.GetOnAnimationEvent().Remove(OnAnimationEvent);
	}
}
