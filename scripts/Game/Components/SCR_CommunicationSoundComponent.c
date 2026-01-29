[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "")]
class SCR_CommunicationSoundComponentClass : CommunicationSoundComponentClass
{
}

enum ECP_VehicleRoles
{
	DRIVER,
	GUNNER,
	COMMANDER,
	PILOT,
	COPILOT,
	OPERATOR,
	PASSANGER	// TODO: PASSENGER
}

enum ECP_Characters
{
	MAN,
	SOLDIER,
	MACHINE_GUNNER,
	SNIPER,
	RECON,
	AT_SOLDIER,
	OFFICER,
	RADIO_MAN
}

enum ECP_VehicleTypes
{
	VEHICLE,
	CAR,
	TRUCK,
	APC,
	FUEL_TRUCK,
	COMM_TRUCK,
	SUPPLY_TRUCK,
	MORTAR,
	HELICOPTER,
}

enum SCR_ECommunicationSoundEventPriority
{
	SOUND_NONE = 0,
	SOUND_BREATH = 10,
	SOUND_PAIN_RELIEVE = 80,
	SOUND_HIT = 90,
	SOUND_DEATH = 98,
	SOUND_KNOCKOUT = 99
}

void OnSoundEventStartedDelegate(SCR_CommunicationSoundComponent component, string eventName, AudioHandle handle, int priority);
typedef func OnSoundEventStartedDelegate;
typedef ScriptInvokerBase<OnSoundEventStartedDelegate> OnSoundEventStartedInvoker;

void OnSoundEventFinishedDelegate(SCR_CommunicationSoundComponent component, string eventName, AudioHandle handle, int priority, bool terminated);
typedef func OnSoundEventFinishedDelegate;
typedef ScriptInvokerBase<OnSoundEventFinishedDelegate> OnSoundEventFinishedInvoker;

class SCR_CommunicationSoundComponent : CommunicationSoundComponent
{
	protected SignalsManagerComponent m_SignalsManagerComponent;
	private static const int SUBTITLES_MAX_DISTANCE = 20;
	static const int DEFAULT_EVENT_PRIORITY_DELAY = 100;
	private static const string DAMAGE_TYPE_SIGNAL_NAME = "DamageType";
	private static const string HIT_SCREAM_INTENSITY_SIGNAL_NAME = "HitScreamIntensity";

	protected string m_sDelayedSoundEvent;
	protected SCR_ECommunicationSoundEventPriority m_eDelayedSoundEventPriority;
	protected EDamageType m_eEDamageType;
	protected int m_iHitScreamIntensity;
	protected ref OnSoundEventStartedInvoker m_OnSoundEventStarted;
	protected ref OnSoundEventFinishedInvoker m_OnSoundEventFinished;
	
	protected int m_iDamageTypeSignalIdx;
	protected int m_iHitScreamIntensitySignalIdx;
	
	protected static bool m_bShowSubtitles;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] metadata
	void ShowSubtitles(array<string> metadata)
	{
		int size = metadata.Count();
		
		if (size > 10)
		{
			Print("Too many metadata. Metadata size = " + size.ToString(), LogLevel.ERROR);
			return;
		}
		
		// Get text		
		int textIdx = -1;
		
		for (int i = size - 1; i >= 0; i--)
		{
			if (!metadata[i].IsEmpty())
			{
				textIdx = i;
				break;
			}
		}
		
		// Do not show subtitles if metadata is empty
		if (textIdx == -1)
			return;
		
		// Get parameters		
		string param[9];		
		int paramIndex;
			
		for (int i = 0; i < textIdx; i++)
		{
			if (!metadata[i].IsEmpty())
			{
				param[paramIndex] = metadata[i];
				paramIndex++;
			}
		}

		// Show subtitles
		// Subtitles printed to the chat - old way
		string localizedText = WidgetManager.Translate(metadata[textIdx], param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7], param[8]);

		SCR_ChatComponent.RadioProtocolMessage(localizedText);
	}
	
	//------------------------------------------------------------------------------------------------
	// Inserted to OnUserSettingsChangedInvoker() on SCR_UISoundEntity
	static void SetSubtitiles()
	{	
		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		if (settings)
			settings.Get("m_bShowRadioProtocolText", m_bShowSubtitles);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] company
	//! \param[in] platoon
	//! \param[in] squad
	//! \param[in] character
	//! \param[in] characterRole
	void SetCallsignSignals(int company, int platoon, int squad, int character, int characterRole)
	{
		if (!m_SignalsManagerComponent)
			return;	
		
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("CompanyCaller"), company);
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("PlattonCaller"), platoon);
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("SquadCaller"), squad);
		m_SignalsManagerComponent.SetSignalValue(m_SignalsManagerComponent.AddOrFindSignal("SoldierCaller"), character -1);
	}
	
	//------------------------------------------------------------------------------------------------
	//!  Schedule and prioritise appropriate injury sounds to be played. The sound scheduled will skip the internal queue
	//! \param[in] critical Whether current hit event is considered a critical hit. Multiple critical hits will increase intensity of hit scream.
	//! \param[in] damageType
	void SoundEventHit(bool critical, EDamageType damageType)
	{
		if (critical)
			m_iHitScreamIntensity += 1;

		SCR_ECommunicationSoundEventPriority priority = SCR_ECommunicationSoundEventPriority.SOUND_HIT + m_iHitScreamIntensity;
		if (priority >= SCR_ECommunicationSoundEventPriority.SOUND_DEATH)
			priority = SCR_ECommunicationSoundEventPriority.SOUND_DEATH - 1;

		DelayedSoundEventPriority(SCR_SoundEvent.SOUND_HIT, priority, DEFAULT_EVENT_PRIORITY_DELAY, damageType);
	}

	//------------------------------------------------------------------------------------------------
	//!  Schedule and prioritise appropriate injury sounds to be played. The sound scheduled will skip the internal queue
	//! \param[in] eventName Name of the sound event to be played after delayMS.
	//! \param[in] priority Priority of the sound event. The event with higher priority will override the current delayed event.
	//! \param[in] delayMS Miliseconds delay for determining the sound event to be played
	//! \param[in] damageType
	void DelayedSoundEventPriority(string eventName, SCR_ECommunicationSoundEventPriority priority, int delayMS, EDamageType damageType = EDamageType.TRUE)
	{
		if (priority < m_eDelayedSoundEventPriority)
			return;

		m_sDelayedSoundEvent = eventName;
		m_eDelayedSoundEventPriority = priority;
		m_eEDamageType = damageType;

		ScriptCallQueue queue = GetGame().GetCallqueue();

		int remainingTime = queue.GetRemainingTime(PlayDelayedSoundEventPriority);

		if (delayMS < remainingTime)
		{
			queue.Remove(PlayDelayedSoundEventPriority);
			remainingTime = 0;
		}

		if (remainingTime <= 0)
			queue.CallLater(PlayDelayedSoundEventPriority, delayMS);
	}

	//------------------------------------------------------------------------------------------------
	//! Plays the delayed sound event requested via DelayedSoundEventPriority.
	//! Skips internal queue of the communication sound component.
	void PlayDelayedSoundEventPriority()
	{
		if (m_SignalsManagerComponent)
		{
			m_SignalsManagerComponent.SetSignalValue(m_iDamageTypeSignalIdx, m_eEDamageType);
			m_SignalsManagerComponent.SetSignalValue(m_iHitScreamIntensitySignalIdx, m_iHitScreamIntensity);
		}
		
		SoundEventPriority(m_sDelayedSoundEvent, m_eDelayedSoundEventPriority, true);

		m_sDelayedSoundEvent = string.Empty;
		m_eDelayedSoundEventPriority = SCR_ECommunicationSoundEventPriority.SOUND_NONE;
		m_eEDamageType = EDamageType.TRUE;

		// Also reset hit scream intensity
		m_iHitScreamIntensity = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Called by damage manager when character dies.
	//! Depending on silent parameter, character will cancel current delayed sound event and then either scream or not
	//! \param[in] silent When true, character will play SOUND_KNOCKOUT event. When false, SOUND_DEATH will be played instead.
	void SoundEventDeath(bool silent)
	{
		GetGame().GetCallqueue().Remove(DelayedSoundEventPriority);

		if (silent)
			SoundEvent(SCR_SoundEvent.SOUND_KNOCKOUT);
		else
			SoundEvent(SCR_SoundEvent.SOUND_DEATH);
	}
	
	//------------------------------------------------------------------------------------------------
	OnSoundEventStartedInvoker GetOnSoundEventStarted()
	{
		if (!m_OnSoundEventStarted)
			m_OnSoundEventStarted = new OnSoundEventStartedInvoker();

		return m_OnSoundEventStarted;
	}
	
	//------------------------------------------------------------------------------------------------
	OnSoundEventFinishedInvoker GetOnSoundEventFinished()
	{
		if (!m_OnSoundEventFinished)
			m_OnSoundEventFinished = new OnSoundEventFinishedInvoker();

		return m_OnSoundEventFinished;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SCR_CallsignBaseComponent callsignBaseComponent = SCR_CallsignBaseComponent.Cast(owner.FindComponent(SCR_CallsignBaseComponent));
		if (callsignBaseComponent)
			callsignBaseComponent.GetOnCallsignChanged().Insert(SetCallsignSignals);
				
		m_SignalsManagerComponent = SignalsManagerComponent.Cast(owner.FindComponent(SignalsManagerComponent));
		if (m_SignalsManagerComponent)
		{
			m_iDamageTypeSignalIdx = m_SignalsManagerComponent.AddOrFindSignal(DAMAGE_TYPE_SIGNAL_NAME);
			m_iHitScreamIntensitySignalIdx = m_SignalsManagerComponent.AddOrFindSignal(HIT_SCREAM_INTENSITY_SIGNAL_NAME);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleMetadata(array<string> metadata, int priority, float distance)
	{
		// Hotfix: Disabled "subtitles"
		/*
		if (m_bShowSubtitles && distance < SUBTITLES_MAX_DISTANCE)
			ShowSubtitles(metadata);
		*/
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSoundEventStarted(string eventName, AudioHandle handle, int priority)
	{
		if (m_OnSoundEventStarted)
			m_OnSoundEventStarted.Invoke(this, eventName, handle, priority);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnSoundEventFinished(string eventName, AudioHandle handle, int priority, bool terminated)
	{
		if (m_OnSoundEventFinished)
			m_OnSoundEventFinished.Invoke(this, eventName, handle, priority, terminated);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_CommunicationSoundComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetScriptedMethodsCall(true);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_CommunicationSoundComponent()
	{
		SCR_CallsignBaseComponent callsignBaseComponent = SCR_CallsignBaseComponent.Cast(GetOwner().FindComponent(SCR_CallsignBaseComponent));
		if (callsignBaseComponent)
			callsignBaseComponent.GetOnCallsignChanged().Remove(SetCallsignSignals);
	}
}
