//------------------------------------------------------------------------------------------------
//! Type of a players current VON transmission
enum EVONTransmitType
{
	NONE,
	DIRECT,
	CHANNEL,
	LONG_RANGE
}

//------------------------------------------------------------------------------------------------
// invoker typedefs
void OnVONActiveToggled(bool isToggledDirect, bool isToggledChannel);	// when direct toggle activates
typedef func OnVONActiveToggled;

void OnEntriesChanged(SCR_VONEntry entry, bool state);			
typedef func OnEntriesChanged;

//------------------------------------------------------------------------------------------------
class SCR_VONControllerClass: ScriptGameComponentClass
{}

//------------------------------------------------------------------------------------------------
//! Scripted VON input and control, attached to SCR_PlayerController
class SCR_VONController : ScriptComponent
{
	protected const string VON_MENU_OPENING_CONTEXT = "VONMenuOpeningContext";
	
	[Attribute("", UIWidgets.Object)]
	protected ref SCR_VONMenu m_VONMenu;
	
	[Attribute()]
	protected ref SCR_VONAutoTune m_VONAutoTune;

	bool m_bIsEditorRadioAdded;	// True if editor radio transceivers are added to m_aEntries
		
	protected const string VON_CONTEXT              = "VONContext";
	protected const string ACTION_DIRECT            = "VONDirect";
	protected const string ACTION_DIRECT_TOGGLE     = "VONDirectToggle";
	protected const string ACTION_CHANNEL           = "VONChannel";
	protected const string ACTION_TRANSCEIVER_CYCLE = "VONTransceiverCycle";
	protected const string ACTION_LONG_RANGE_TOGGLE = "VONLongRangeToggle";

	protected const float TOGGLE_OFF_DELAY = 0.5;		// seconds, delay before toggle state can get canceled, to avoid collision of double click and press
	
	protected static bool s_bUnconsciousVONPermitted = true;
	protected static bool s_bIsInit;		// component init done, static so its only done once
	protected bool m_bIsDisabled; 			// VON control is disabled
	protected bool m_bIsPauseDisabled;		// disabled by pause menu
	protected bool m_bIsUnconscious; 		// Character is unconscious --> VON control is disabled
	protected ECharacterLifeState m_eLifeState;	// Life state of controlled character
	protected bool m_bIsActive;				// VON is active
	protected bool m_bIsToggledDirect;		// VON direct speech toggle is active
	protected bool m_bIsLongRangeToggled;	// VON long range radio is selected
	protected float m_fToggleOffDelay;		// used to track delay before toggle can be cancelled
	protected string m_sActiveHoldAction;	// tracker for ending the hold action VON 
	protected string m_sLocalEncryptionKey;	// local players faction encryption key
	protected EVONTransmitType m_eVONType;	// currently active VON type
	protected InputManager m_InputManager;

	protected SCR_VoNComponent m_VONComp;					// VON component used for transmission
	protected SCR_VonDisplay m_VONDisplay;					// VON transmission display
	protected SCR_VONEntry m_ActiveEntry;					// active entry (non direct speech)
	protected SCR_VONEntry m_LongRangeEntry;				// entry for long range radio, if available
	protected SCR_VONEntry m_SavedEntry;					// entry used for switching to different active entry after current one is done, f.e. after VONLongRange
	protected ref SCR_VONEntry m_DirectSpeechEntry;			// separate direct speech entry
	protected ref array<ref SCR_VONEntry> m_aEntries = {};	// all entries except direct
	
	
	protected ref ScriptInvokerBase<OnEntriesChanged> 	m_OnEntriesChanged = new ScriptInvokerBase<OnEntriesChanged>();			// when entry array is modified (SCR_VONEntry entry, bool isAdded)
	protected ref ScriptInvokerBase<OnEntriesChanged> 	m_OnEntriesActiveChanged = new ScriptInvokerBase<OnEntriesChanged>();	// when entry array is modified (SCR_VONEntry entry, bool isActive)
	protected ref ScriptInvokerBase<OnVONActiveToggled> m_OnVONActiveToggled = new ScriptInvokerBase<OnVONActiveToggled>();		// when direct toggle activates (bool isToggledDirect, bool isToggledChannel)
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnEntriesChanged> GetOnEntriesChangedInvoker()
	{
		return m_OnEntriesChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnEntriesChanged> GetOnEntriesActiveChangedInvoker()
	{
		return m_OnEntriesActiveChanged;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerBase<OnVONActiveToggled> GetOnVONActiveToggledInvoker()
	{
		return m_OnVONActiveToggled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDisplay(SCR_VonDisplay display)
	{ 
		m_VONDisplay = display; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VonDisplay GetDisplay()
	{ 
		return m_VONDisplay; 
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VONMenu GetVONMenu()
	{ 
		return m_VONMenu; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Disables ability to transmit and related VON UI
	//! \param state is the target state
	//! \return true if this call changed the state to target, false if the state was target state already
	bool SetVONDisabled(bool state)
	{
		if (m_bIsDisabled == state)
			return false;
		
		m_bIsDisabled = state;
		UpdateSystemState();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Is VON disabled
	bool IsVONDisabled()
	{
		return m_bIsDisabled;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get active VON component for transmit
	SCR_VoNComponent GetVONComponent()
	{ 
		return m_VONComp; 
	}
		
	//------------------------------------------------------------------------------------------------
	//! Set component for transmit of VON
	//! \param VONComp is the subject
	void SetVONComponent(SCR_VoNComponent VONComp)
	{			
		m_VONComp = VONComp;
	}
	
	
	//------------------------------------------------------------------------------------------------
	//! Get current entries
	//! \return count of entries
	int GetVONEntries(inout array<ref SCR_VONEntry> entries)
	{
		int count = m_aEntries.Count();
		for (int i = 0; i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}

		return count;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get number of entries
	int GetVONEntryCount()
	{
		return m_aEntries.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get active entry for outgoing transmission
	//! \return active entry
	SCR_VONEntry GetActiveEntry()
	{
		return m_ActiveEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Using long range radio
	bool IsUsingLRR()
	{
		if (!m_ActiveEntry)
			return false;
		
		return m_LongRangeEntry == m_ActiveEntry;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Long range raido entry is available
	bool IsLRRAvailable()
	{
		return m_LongRangeEntry != null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set entry as active for outgoing transmission
	//! \param entry is subject entry
	//! \param setFromMenu determines whether this was performed by menu selection
	void SetEntryActive(SCR_VONEntry entry, bool setFromMenu = false)
	{
		if (entry == m_ActiveEntry)
			return;
				
		if (m_ActiveEntry)
		{
			m_OnEntriesActiveChanged.Invoke(m_ActiveEntry, false);
			m_ActiveEntry.SetActive(false);
		}
		
		entry.SetActive(true);
		m_ActiveEntry = entry;
		m_OnEntriesActiveChanged.Invoke(entry, true);
		
		if (m_ActiveEntry && !m_ActiveEntry.IsUsable())	// turn off toggle when switching to disabled entry
			DeactivateVON();
		
		if (m_VONDisplay)
			m_VONDisplay.ShowSelectedVONHint(m_ActiveEntry);
		
		SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
		if (radioEntry)
		{
			m_LongRangeEntry = m_ActiveEntry;
			SetVONLongRange(radioEntry.IsLongRange(), false);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Add new VON entry
	//! Set as active entry if m_ActiveEntry is null
	//! \param entry is subject entry
	void AddEntry(SCR_VONEntry entry)
	{
		m_aEntries.Insert(entry);
		entry.InitEntry();
		
		if (SCR_VONEntryRadio.Cast(entry) && SCR_VONEntryRadio.Cast(entry).IsLongRange())	// if long range entry, set it here
			m_LongRangeEntry = entry;
		
		m_OnEntriesChanged.Invoke(entry, true);
		
		if (!m_ActiveEntry)
			SetEntryActive(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove existing VON entry.
	//! Sets m_ActiveEntry to null, when removing active entry
	//! \param entry is subject entry
	void RemoveEntry(SCR_VONEntry entry)
	{
		m_OnEntriesChanged.Invoke(entry, false);
		
		//Deactivate if the entry being removed is active
		if (entry == m_ActiveEntry)
		{
			DeactivateVON();
			
			m_ActiveEntry = null;
		}
		
		if (entry == m_LongRangeEntry)	// unset long range entry
			m_LongRangeEntry = null;
				
		if (m_VONMenu)
		{
			SCR_RadialMenu vonRadial = m_VONMenu.GetRadialMenu();
			if (vonRadial && vonRadial.IsOpened())
				vonRadial.Close();
		}
		
		m_aEntries.RemoveItem(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Finds first unmuted entry and sets it as the active entry
	//! \return EVONTransmitType for selected entry. if no entry selected, return EVONTransmitType.NONE;
	EVONTransmitType SelectFirstUnmutedEntry()
	{
		foreach (SCR_VONEntry entry : m_aEntries)
		{
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (!radioEntry || radioEntry.GetIsMuted())
				continue;
			
			SetEntryActive(entry, true);
			
			bool longRange = radioEntry.IsLongRange();
			if (longRange)
			{
				return EVONTransmitType.LONG_RANGE;
			}
			else
			{
				return EVONTransmitType.CHANNEL; 
			}
		}
		
		return EVONTransmitType.NONE;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetVONProximity(bool activate)
	{
		if (!m_VONComp)
			return;

		if (!m_DirectSpeechEntry.IsUsable())
			return;

		if (activate && m_eVONType == EVONTransmitType.DIRECT)
			return;

		if (activate)
			ActivateVON(EVONTransmitType.DIRECT);
		else
			DeactivateVON(EVONTransmitType.DIRECT);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetVONProximityToggle(bool activate)
	{
		if (!m_VONComp)
			return;

		if (!m_DirectSpeechEntry.IsUsable())
			return;

		if (m_bIsToggledDirect == activate)
			return;

		m_bIsToggledDirect = activate;

		if (activate)
			ActivateVON(EVONTransmitType.DIRECT);
		else
			DeactivateVON(EVONTransmitType.DIRECT);

		m_OnVONActiveToggled.Invoke(m_bIsToggledDirect, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetVONBroadcast(bool activate, EVONTransmitType transmitType = EVONTransmitType.CHANNEL)
	{
		if (!m_VONComp)
			return;
		
		// If active entry is null, attempt to select another.
		if (!m_ActiveEntry)
			transmitType = SelectFirstUnmutedEntry();

		// if active entry disabled or character is incapacitated, use direct instead
		if (m_eLifeState != ECharacterLifeState.ALIVE || !m_ActiveEntry || !m_ActiveEntry.IsUsable())
		{
			SetVONProximity(activate && !m_bIsUnconscious);

			if (m_VONDisplay)
				m_VONDisplay.ShowSelectedVONDisabledHint();

			return;
		}

		if (activate)
			ActivateVON(transmitType);
		else
			DeactivateVON(transmitType);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetVONLongRange(bool longRange, bool restorePrevious = true)
	{
		if (m_bIsLongRangeToggled == longRange)
			return;

		if (!m_LongRangeEntry || !m_LongRangeEntry.IsUsable())
			return;

		m_bIsLongRangeToggled = longRange;

		if (longRange)
		{
			// saved entry to switch back to after using long range mode
			if (!m_SavedEntry && m_ActiveEntry != m_LongRangeEntry)
				m_SavedEntry = m_ActiveEntry;

			SetActiveTransmit(m_LongRangeEntry);

			if (m_VONDisplay)
				m_VONDisplay.ShowSelectedVONHint(m_LongRangeEntry);
		}
		else
		{
			// restore last saved entry
			if (restorePrevious)
			{
				if (!m_SavedEntry)
				{
					// Select first non-longrange entry
					SCR_VONEntryRadio radioEntry;
					foreach (SCR_VONEntry entry : m_aEntries)
					{
						radioEntry = SCR_VONEntryRadio.Cast(entry);
						if (radioEntry && !radioEntry.IsLongRange())
						{
							m_SavedEntry = entry;
							break;
						}
					}
				}

				if (m_VONDisplay)
					m_VONDisplay.ShowSelectedVONHint(m_SavedEntry);

				SetActiveTransmit(m_SavedEntry);
			}
		}
		
		DeactivateVON();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActionVONProximity(float value, EActionTrigger reason = EActionTrigger.UP)
	{
		if (!m_VONComp)
			return;

		bool activate = reason != EActionTrigger.UP;

		// Cancel toggle
		if (activate && m_bIsToggledDirect && m_fToggleOffDelay <= 0)
			SetVONProximityToggle(false);

		SetVONProximity(reason != EActionTrigger.UP);

		if (reason != EActionTrigger.UP)
			m_sActiveHoldAction = ACTION_DIRECT;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActionVONProximityToggle(float value, EActionTrigger reason = EActionTrigger.UP)
	{
		if (!m_VONComp)
			return;

		SetVONProximityToggle(!m_bIsToggledDirect);

		if (m_bIsToggledDirect)
			m_fToggleOffDelay = TOGGLE_OFF_DELAY;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActionVONBroadcast(float value, EActionTrigger reason = EActionTrigger.UP)
	{
		if (!m_VONComp)
			return;

		EVONTransmitType transmitType;

		if (m_bIsLongRangeToggled)
			transmitType = EVONTransmitType.LONG_RANGE;
		else
			transmitType = EVONTransmitType.CHANNEL;

		if (reason != EActionTrigger.UP && m_eVONType == transmitType)
			return;

		SetVONBroadcast(reason != EActionTrigger.UP, transmitType);

		if (reason != EActionTrigger.UP)
			m_sActiveHoldAction = ACTION_CHANNEL;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ActionVONTransceiverCycle(float value, EActionTrigger reason = EActionTrigger.UP)
	{
		TransceiverCycle();
	}

	//------------------------------------------------------------------------------------------------
	protected void TransceiverCycle()
	{
		SCR_VONEntryRadio activeRadioEntry = SCR_VONEntryRadio.Cast(m_ActiveEntry);
		if (!activeRadioEntry)
			return;

		bool isLongRange = activeRadioEntry.IsLongRange();

		BaseRadioComponent radio = activeRadioEntry.GetTransceiver().GetRadio();
		array<SCR_VONEntry> entries = {};

		SCR_VONEntryRadio radioEntry;
		foreach (SCR_VONEntry entry : m_aEntries)
		{
			radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (radioEntry && radioEntry.GetTransceiver().GetRadio() == radio)
				entries.Insert(entry);
		}

		SCR_VONEntry newEntry = m_ActiveEntry;

		if (!entries.IsEmpty())
		{
			int nextEntryID = (entries.Find(m_ActiveEntry) + 1) % entries.Count();
			newEntry = entries[nextEntryID];
			
			if (newEntry != m_ActiveEntry && m_eVONType != EVONTransmitType.DIRECT)
				DeactivateVON(m_eVONType);
		}

		if (!ActivateVON(newEntry))
			return;

		if (m_VONDisplay)
			m_VONDisplay.ShowSelectedVONHint(newEntry);
	}

	//------------------------------------------------------------------------------------------------
	protected void ActionVONLongRangeToggle(float value, EActionTrigger reason = EActionTrigger.UP)
	{
		SetVONLongRange(!m_bIsLongRangeToggled);
	}
	
	//------------------------------------------------------------------------------------------------
	//! VON activation
	//! \param[in] transmitType type of entry to be activated
 	protected void ActivateVON(EVONTransmitType transmitType)
	{
		SCR_VONEntry entry = GetEntryByTransmitType(transmitType);
		if (!entry)
			return;

		ActivateVON(entry, transmitType);
	}

	//------------------------------------------------------------------------------------------------
	//! VON activation
	//! \param[in] entry which is being activated
	//! \param[in] transmitType type of entry to be activated
	//! \return true if activation was successful, false otherwise
 	protected bool ActivateVON(notnull SCR_VONEntry entry, EVONTransmitType transmitType = EVONTransmitType.NONE)
	{
		if (!m_VONComp)
			return false;
		
		if (transmitType == EVONTransmitType.NONE)
		{
			if (entry.GetVONMethod() == ECommMethod.SQUAD_RADIO)
				transmitType = EVONTransmitType.CHANNEL;
			else
				transmitType = EVONTransmitType.DIRECT;
		}

		m_eVONType = transmitType;
		if (transmitType != EVONTransmitType.DIRECT && !GetGame().GetVONCanTransmitCrossFaction() && !SCR_Global.IsAdmin())		// is cross faction transmit disabled
		{
			InitEncryptionKey();
		
			SCR_VONEntryRadio radioEntry = SCR_VONEntryRadio.Cast(entry);
			if (radioEntry && m_sLocalEncryptionKey != string.Empty && radioEntry.GetTransceiver().GetRadio().GetEncryptionKey() != m_sLocalEncryptionKey)
			{
				SetVONProximity(true);
				if (m_VONDisplay)
					m_VONDisplay.ShowSelectedVONDisabledHint(true);

				return false;
			}
		}
					
		SetActiveTransmit(entry);
		m_VONComp.SetCapture(true);
		m_bIsActive = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Current VON deactivation
	protected void DeactivateVON(EVONTransmitType transmitType = EVONTransmitType.NONE)
	{
		if (!m_VONComp)
			return; 
		
		if (transmitType != EVONTransmitType.NONE && transmitType != m_eVONType)	// only deactivate target type, in case other type was activated already
			return;
	
		m_eVONType = EVONTransmitType.NONE;
		m_bIsActive = false;
		m_VONComp.SetCapture(false);
		m_sActiveHoldAction = string.Empty;
					
		// direct toggle is active so ending VON should not end capture
		if (m_bIsToggledDirect)
			ActivateVON(EVONTransmitType.DIRECT);
	}
		
	//------------------------------------------------------------------------------------------------
	protected SCR_VONEntry GetEntryByTransmitType(EVONTransmitType type)
	{
		switch (type)
		{
			case EVONTransmitType.CHANNEL:
				return m_ActiveEntry;
			
			case EVONTransmitType.LONG_RANGE:
				return m_LongRangeEntry;
			
			default:
				return m_DirectSpeechEntry;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set transmission method depending on entry type when starting VON transmit
	protected void SetActiveTransmit(notnull SCR_VONEntry entry)
	{		
		if (entry.GetVONMethod() == ECommMethod.SQUAD_RADIO)
		{
			m_VONComp.SetCommMethod(ECommMethod.SQUAD_RADIO);
			m_VONComp.SetTransmitRadio(SCR_VONEntryRadio.Cast(entry).GetTransceiver());
			SetEntryActive(entry);
		}
		else 
		{
			m_VONComp.SetCommMethod(ECommMethod.DIRECT);
			m_VONComp.SetTransmitRadio(null);
		}
		
		if (entry == m_SavedEntry)
			m_SavedEntry = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Executed on server, sets the active channel of client's VON Controller
	//! \param[in] channelId
	void SetActiveChannel(int channelId)
	{
		Rpc(RpcAsk_SetActiveChannel, channelId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcAsk_SetActiveChannel(int channelId)
	{
		if (!m_aEntries.IsIndexValid(channelId))
			return;

		SetEntryActive(m_aEntries.Get(channelId));
	}
	
	//------------------------------------------------------------------------------------------------
	// Reset VON input states and stop transmission
	protected void ResetVON()
	{
		SetVONProximityToggle(false);
		SetVONLongRange(false, false);
		DeactivateVON();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to reinit VON when new entity is controlled
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{				
		ResetVON();
		
		m_sLocalEncryptionKey = string.Empty;
		
		if (to)
		{
			// Only set new VONComponent if the player is not an active edtior, otherwise the SCR_EditorManagerEntity will take care of this.
			if (!m_VONComp || !m_VONComp.IsLocalActiveEditor())
				SetVONComponent(SCR_VoNComponent.Cast(to.FindComponent(SCR_VoNComponent)));		
		}
		else 
			SetVONComponent(null);	
		
		ChimeraCharacter previous = ChimeraCharacter.Cast(from);
		if (previous)
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(previous.GetCharacterController());
			if (controller)
				controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
		}
		
		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (character)
		{
			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (controller)
			{
				ECharacterLifeState lifeState = controller.GetLifeState();
				OnLifeStateChanged(lifeState, lifeState);
				controller.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
			}
		}

		UpdateSystemState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_BaseGameMode event
	//! Used to unregister VON for deleted players
	protected void OnPlayerDeleted(int playerId, IEntity player)
	{		
		if (playerId != GetGame().GetPlayerController().GetPlayerId())
			return;
		
		ResetVON();
		
		m_ActiveEntry = null;
		m_LongRangeEntry = null;
		
		m_aEntries.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuOpened()
	{
		if (SetVONDisabled(true))
			m_bIsPauseDisabled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI Event
	protected void OnPauseMenuClosed()
	{
		if (m_bIsPauseDisabled)
			SetVONDisabled(false);
		
		m_bIsPauseDisabled = false;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Assign VON comp by fetching it from controlled entity
	bool AssignVONComponent()
	{
		IEntity controlledEnt = SCR_PlayerController.Cast(GetOwner()).GetControlledEntity();
		if (controlledEnt)
			SetVONComponent(SCR_VoNComponent.Cast(controlledEnt.FindComponent(SCR_VoNComponent)));
		
		if(!m_VONComp)
			return false;
		
		return true;			
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		UpdateUnconsciousVONPermitted();

		m_eLifeState = newLifeState;

		if (s_bUnconsciousVONPermitted)
			m_bIsUnconscious = m_eLifeState == ECharacterLifeState.DEAD;
		else
			m_bIsUnconscious = m_eLifeState != ECharacterLifeState.ALIVE;

		if (m_bIsUnconscious)
		{
			// Who has died, does not live. - Franz
			SetVONProximityToggle(false);
			DeactivateVON();
		}
		else if (m_eLifeState == ECharacterLifeState.INCAPACITATED)
		{
			// allow proximity voice chat if incapacitated
			if (m_eVONType != EVONTransmitType.DIRECT)
				DeactivateVON();
		}

		UpdateSystemState();
	}

	//------------------------------------------------------------------------------------------------
	static void UpdateUnconsciousVONPermitted()
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		SCR_GameModeHealthSettings healthSettings = gameMode.GetGameModeHealthSettings();
		if (healthSettings)
			s_bUnconsciousVONPermitted = healthSettings.IsUnconsciousVONPermitted();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitEncryptionKey()
	{
		SCR_FactionManager fManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (fManager)
		{
			SCR_Faction faction = SCR_Faction.Cast(fManager.GetLocalPlayerFaction());
			if (faction)
				m_sLocalEncryptionKey = faction.GetFactionRadioEncryptionKey();
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Initialize component, done once per controller
	protected void Init(IEntity owner)
	{	
		if (s_bIsInit || System.IsConsoleApp())	// hosted server will have multiple controllers, init just the first one // dont init on dedicated server
		{
			Deactivate(owner);
			return;
		}
		
		UpdateUnconsciousVONPermitted();

		m_InputManager = GetGame().GetInputManager();
		if (m_InputManager)
		{
			m_InputManager.AddActionListener(ACTION_DIRECT, EActionTrigger.DOWN, ActionVONProximity);
			m_InputManager.AddActionListener(ACTION_DIRECT, EActionTrigger.UP, ActionVONProximity);
			m_InputManager.AddActionListener(ACTION_DIRECT_TOGGLE, EActionTrigger.DOWN, ActionVONProximityToggle);
			m_InputManager.AddActionListener(ACTION_CHANNEL, EActionTrigger.DOWN, ActionVONBroadcast);
			m_InputManager.AddActionListener(ACTION_CHANNEL, EActionTrigger.UP, ActionVONBroadcast);
			m_InputManager.AddActionListener(ACTION_TRANSCEIVER_CYCLE, EActionTrigger.DOWN, ActionVONTransceiverCycle);
			m_InputManager.AddActionListener(ACTION_LONG_RANGE_TOGGLE, EActionTrigger.DOWN, ActionVONLongRangeToggle);
		}

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetOwner());
		if (playerController)
		{
			OnControlledEntityChanged(null, playerController.GetControlledEntity());
			playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		}

		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDeleted().Insert(OnPlayerDeleted);
		
		m_DirectSpeechEntry = new SCR_VONEntry(); // Init direct speech entry
		
		ConnectToHandleUpdateVONControllersSystem();
		
		s_bIsInit = true;
		
		if (m_VONMenu)
			m_VONMenu.Init(this);
		
		if (m_VONAutoTune)
			m_VONAutoTune.Init(owner, this);

		UpdateSystemState();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Cleanup component
	protected void Cleanup()
	{
		m_VONMenu = null;
		
		UpdateSystemState();
		
		m_InputManager = GetGame().GetInputManager();
		if (m_InputManager)
		{
			m_InputManager.RemoveActionListener(ACTION_DIRECT, EActionTrigger.DOWN, ActionVONProximity);
			m_InputManager.RemoveActionListener(ACTION_DIRECT, EActionTrigger.UP, ActionVONProximity);
			m_InputManager.RemoveActionListener(ACTION_DIRECT_TOGGLE, EActionTrigger.DOWN, ActionVONProximityToggle);
			m_InputManager.RemoveActionListener(ACTION_CHANNEL, EActionTrigger.DOWN, ActionVONBroadcast);
			m_InputManager.RemoveActionListener(ACTION_CHANNEL, EActionTrigger.UP, ActionVONBroadcast);
			m_InputManager.RemoveActionListener(ACTION_TRANSCEIVER_CYCLE, EActionTrigger.DOWN, ActionVONTransceiverCycle);
			m_InputManager.RemoveActionListener(ACTION_LONG_RANGE_TOGGLE, EActionTrigger.DOWN, ActionVONLongRangeToggle);
		}
	
		IEntity ent = PlayerController.Cast(GetOwner()).GetControlledEntity();
		if (ent)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(ent);
			if (character)
			{	
				SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
				if (controller)
					controller.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
			}
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnPlayerDeleted().Remove(OnPlayerDeleted);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Debug
	protected void UpdateDebug()
	{
		DbgUI.Begin("VON debug");
		const string dbg = "VONcomp: %1 | entries: %2";
		DbgUI.Text( string.Format( dbg, m_VONComp, m_aEntries.Count() ) );
		const string dbg2 = "Direct mode: %1 | LRR mode: %2";
		DbgUI.Text( string.Format( dbg2, m_bIsToggledDirect, m_bIsLongRangeToggled ) );
		const string dbg3 = "Activ: %1";
		DbgUI.Text( string.Format( dbg3, m_ActiveEntry ) );
		const string dbg4 = "LRRad: %1";
		DbgUI.Text( string.Format( dbg4, m_LongRangeEntry ) );
		const string dbg5 = "Saved: %1";
		DbgUI.Text( string.Format( dbg5, m_SavedEntry ) );
		
		const string line =  "freq: %2 | active: %3 | class: %4 | type: %1 ";
		foreach ( SCR_VONEntry entry : m_aEntries )
		{
			SCR_VONEntryRadio radio = SCR_VONEntryRadio.Cast(entry);
			DbgUI.Text( string.Format( line, SCR_Enum.GetEnumName(EGadgetType, radio.GetGadget().GetType()), radio.GetEntryFrequency(), radio.IsActive(), radio.ToString()));
		}
		
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPostInit(IEntity owner)
	{
		Init(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnDeactivate(IEntity owner)
	{
		Cleanup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSystemState()
	{
		if ((!m_bIsDisabled && !m_bIsUnconscious) || m_VONMenu)
			ConnectToHandleUpdateVONControllersSystem();
		else
			DisconnectFromHandleUpdateVONControllersSystem();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ConnectToHandleUpdateVONControllersSystem()
	{
		World world = GetOwner().GetWorld();
		HandleUpdateVONControllersSystem updateSystem = HandleUpdateVONControllersSystem.Cast(world.FindSystem(HandleUpdateVONControllersSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Register(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisconnectFromHandleUpdateVONControllersSystem()
	{
		World world = GetOwner().GetWorld();
		HandleUpdateVONControllersSystem updateSystem = HandleUpdateVONControllersSystem.Cast(world.FindSystem(HandleUpdateVONControllersSystem));
		if (!updateSystem)
			return;
		
		updateSystem.Unregister(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		if (!m_bIsDisabled && !m_bIsUnconscious)
		{
			m_InputManager.ActivateContext(VON_CONTEXT);
			m_InputManager.ActivateContext(VON_MENU_OPENING_CONTEXT);
			
			if (m_fToggleOffDelay > 0)
				m_fToggleOffDelay -= timeSlice;
			
			/* 	When non overlaying context such as chat is activated during hold action, we will no longer receive the EActionTrigger.UP callback from the previous context, which is currently intended
				This timeouts it in such case so the VON will not be stuck in an active state	*/
			if (!m_sActiveHoldAction.IsEmpty() && !m_InputManager.IsActionActive(m_sActiveHoldAction))
				DeactivateVON();
		}
		
		if (m_VONMenu)
			m_VONMenu.Update(timeSlice);
		
		if (m_VONAutoTune)
			m_VONAutoTune.Update(timeSlice);

		#ifdef VON_DEBUG
			UpdateDebug();
		#endif
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		s_bIsInit = false;
		Cleanup();
		DisconnectFromHandleUpdateVONControllersSystem();
		
		if (m_VONAutoTune)
			m_VONAutoTune.Deinit();

		super.OnDelete(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_VONAutoTune GetAutoTune()
	{
		return m_VONAutoTune;
	}
};
