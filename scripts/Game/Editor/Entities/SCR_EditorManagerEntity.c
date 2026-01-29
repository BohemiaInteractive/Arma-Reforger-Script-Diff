[EntityEditorProps(category: "GameScripted/Editor", description: "Core Editor manager", color: "251 91 0 255", icon: "WBData/EntityEditorProps/entityEditor.png")]
class SCR_EditorManagerEntityClass : SCR_EditorBaseEntityClass
{
}

/** @ingroup Editor_Entities
*/

/*!
The entity which enables all editor functionality for the player.

- It's **created by SCR_EditorManagerCore** when a player connects.
+ Every player receives one.
- In multiplayer, the entity is **local to the player**.
+ This is required to allow communication with server (see Replication for more details).
+ Requires RplComonent to function.
- This entity handles **only barebone editor functionality**, like access rules, or opening and closing.
- Everything else, like camera or menu handling, must be on specialized editor modes (SCR_EditorModeEntity) and components (SCR_BaseEditorComponent)!
- Editor can have a limitied state, influenced by its modes and checked by IsLimited() function:
+ Editor is **limited** if all its modes are.
+ If at least one mode is unlimited, the editor is **unlimited** as well.
+ The state influences behavior of certain components, e.g., photo mode camera movement may be restricted if the editor is limited.
+ As a rule of thumb, unlimited state means that player can ***cheat*** with the editor.
- Default editor manager prefab is defined in SCR_EditorManagerCore (config is Configs/Core folder).
*/
class SCR_EditorManagerEntity : SCR_EditorBaseEntity
{
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Attributes
	[Attribute("true", UIWidgets.Auto, "Are default modes created automatically?", category: "Editor Manager")]
	private bool m_bIsAutoModes;

	[Attribute("false", UIWidgets.Auto, "Is the editor launched by default?", category: "Editor Manager")]
	private bool m_bIsAutoInit;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Variables
	private bool m_bIsOpened;
	private bool m_bIsInTransition; //--- Opening or closing
	private EEditorCanOpen m_CanOpen = EEditorCanOpen.SCRIPT;
	private EEditorCanOpen m_CanClose = EEditorCanOpen.SCRIPT;
	private EEditorCanOpen m_CanOpenSum;
	private EEditorCanOpen m_CanCloseSum;

	private RplComponent m_RplComponent;
	private int m_iPlayerID;
	private bool m_bInit;
	private ref array<SCR_EditorModeEntity> m_Modes = new array<SCR_EditorModeEntity>;
	protected ref map<EEditorModeAccess, EEditorMode> m_ModesByAccess = new map<EEditorModeAccess, EEditorMode>();
	private EEditorMode m_PrevMode = -1;
	private EEditorMode m_CurrentMode = -1;
	private EEditorMode m_ProcessedMode = -1;
	private SCR_EditorModeEntity m_CurrentModeEntity;
	private bool m_bIsModeChangeRequested;
	private bool m_bIsLimited;
	protected SCR_NotificationsComponent m_NotificationsComponent;

	private EEditorEventOperation m_iEventOperation = EEditorEventOperation.NONE;
	private EEditorEvent m_iEvent = EEditorEvent.NONE;
	private int m_iEventComponent;
	private int m_iEventAttempt;
	private float m_iEventProgress;
	private float m_iEventProgressMax;
	private float m_fEventAsyncStart;
	private ref array<SCR_BaseEditorComponent> m_aEventComponentsActivate;
	private ref array<SCR_BaseEditorComponent> m_aEventComponentsDeactivate;
	private ref array<EEditorEvent> m_aEvents;
	private ref array<SCR_VONEntryRadio> editorRadioEntries;

	private ref ScriptInvoker Event_OnInit = new ScriptInvoker();
	private ref ScriptInvoker Event_OnRequest = new ScriptInvoker();
	private ref ScriptInvoker Event_OnOpened = new ScriptInvoker();
	private ref ScriptInvoker Event_OnCanOpen = new ScriptInvoker();
	private ref ScriptInvoker Event_OnCanClose = new ScriptInvoker();
	private ref ScriptInvoker Event_OnPreActivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnActivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnPostActivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnDeactivate = new ScriptInvoker();
	private ref ScriptInvoker Event_OnClosed = new ScriptInvoker();
	private ref ScriptInvoker Event_OnDebug = new ScriptInvoker();
	private ref ScriptInvoker Event_OnOpenedServer = new ScriptInvoker();
	private ref ScriptInvoker Event_OnOpenedServerCallback = new ScriptInvoker();
	private ref ScriptInvoker Event_OnClosedServer = new ScriptInvoker();
	private ref ScriptInvoker Event_OnClosedServerCallback = new ScriptInvoker();
	private ref ScriptInvoker Event_OnModeAdd = new ScriptInvoker();
	private ref ScriptInvoker Event_OnModeRemove = new ScriptInvoker();
	private ref ScriptInvoker Event_OnModeChangeRequest = new ScriptInvoker();
	private ref ScriptInvoker Event_OnModeChange = new ScriptInvoker();
	private ref ScriptInvoker Event_OnLimitedChange = new ScriptInvoker();
	private ref ScriptInvoker Event_OnAsyncLoad = new ScriptInvoker();
	private ref ScriptInvoker Event_OnCanEndGameChanged = new ScriptInvoker();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Opening

	//------------------------------------------------------------------------------------------------
	//! Check if the editor is currently opened.
	//! \return True if opened
	override bool IsOpened()
	{
		if (m_bIsInTransition)
			return !m_bIsOpened; //--- Don't return current state until async loading is finished
		else
			return m_bIsOpened;
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle the editor. If it's closed it will be opened, and vice versa.
	//! Nothing will happen if the desired action is disabled.
	void Toggle()
	{
		if (m_bIsOpened)
			Close();
		else
			Open();
	}

	//------------------------------------------------------------------------------------------------
	//! Open the editor.
	//! Nothing will happen if the editor is not allowed to be opened.
	//! \param showErrorNotification Set to false to prevent sending notification to player when the editor cannot be opened
	void Open(bool showErrorNotification = true)
	{
		if (m_bIsOpened || !IsAuthority()) return; //--- Exit when already opened or attempting to edit non-local editor
		if (!CanOpen())
		{
			if (showErrorNotification)
				SCR_NotificationsComponent.SendToPlayer(GetPlayerID(), ENotification.EDITOR_CANNOT_OPEN);

			return;
		}
		
		int playerId = GetPlayerID();
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		// Unscope when going into game master to avoid issues with ADS camera
		IEntity player = playerManager.GetPlayerControlledEntity(playerId);
		if (player)
		{
			CharacterControllerComponent characterCont = CharacterControllerComponent.Cast(player.FindComponent(CharacterControllerComponent));
			if (characterCont)
				characterCont.SetWeaponADS(false); 
		}

		// VON
		PlayerController playerController = playerManager.GetPlayerController(playerId);
		if (playerController)
		{
			SCR_VONController vonCont = SCR_VONController.Cast(playerController.FindComponent(SCR_VONController));
			SCR_VoNComponent editorVonComponent = SCR_VoNComponent.Cast(FindComponent(SCR_VoNComponent));
			
			// Connect the VON component used by the editor.
			editorVonComponent.ConnectEditorToVoNSystem(m_iPlayerID);
			
			// Tell SCR_VONController to use the SCR_VoNComponent attached to this editor manager.
			vonCont.SetVONComponent(editorVonComponent);
			
			// Give editor radio transceivers
			if (!vonCont.m_bIsEditorRadioAdded)
			{
				vonCont.m_bIsEditorRadioAdded = true;
				
				SCR_RadioComponent radioGadget = SCR_RadioComponent.Cast(FindComponent(SCR_RadioComponent));
				radioGadget.OnPostInit(this);
				BaseRadioComponent radioComp = radioGadget.GetRadioComponent();
				
				// This could be a good place to set the power of the radioComp. But this is not possible because, the SetPower() function needs
				// to check if the owner of the radio is a active game master for it to work. That is not the case at this time.
							
				int transceiversCount = radioComp.TransceiversCount();
				editorRadioEntries = new array<SCR_VONEntryRadio>();
				editorRadioEntries.Resize(transceiversCount);
				for (int i = 0; i < transceiversCount; i++)
				{
					BaseTransceiver transceiver = radioComp.GetTransceiver(i);
					EditorFactionTransceiver editorFactionTransceiver = EditorFactionTransceiver.Cast(transceiver);
					if (editorFactionTransceiver)
					{
						SCR_VONEntryRadio editorRadioEntry = new SCR_VONEntryRadio();
						editorRadioEntry.SetRadioEntry(transceiver, i + 1, radioGadget);	
						editorRadioEntry.SetChannelTextOverwrite("GM " + editorFactionTransceiver.GetEditorFaction());
						vonCont.AddEntry(editorRadioEntry);
						
						editorRadioEntries[i] = editorRadioEntry;
					}
					else
					{
						SCR_VONEntryRadio editorRadioEntry = new SCR_VONEntryRadio();
						editorRadioEntry.SetRadioEntry(transceiver, i + 1, radioGadget);	
						editorRadioEntry.SetChannelTextOverwrite("GM");
						editorRadioEntry.SetFrequencyTextOverwrite("     GM");
						vonCont.AddEntry(editorRadioEntry);
						
						editorRadioEntries[i] = editorRadioEntry;
					}
				}
			}			
		}
		
		StartEvents(EEditorEventOperation.REQUEST_OPEN);
		Rpc(ToggleServer, true);
	}

	//------------------------------------------------------------------------------------------------
	//! Close the editor.
	//! Nothing will happen if the editor is not allowed to be closed.
	//! \param showErrorNotification Set to false to prevent sending notification to player when the editor cannot be closed
	void Close(bool showErrorNotification = true)
	{
		if (m_CurrentModeEntity.GetPreventClose())
			return;

		if (!m_bIsOpened || !IsAuthority()) return; //--- Exit when already closed or attempting to edit non-local editor
		if (!CanClose())
		{
			if (showErrorNotification)
				SCR_NotificationsComponent.SendToPlayer(GetPlayerID(), ENotification.EDITOR_CANNOT_CLOSE);

			return;
		}
		
		// VON
		int playerId = GetPlayerID();
		PlayerManager playerManager = GetGame().GetPlayerManager();
		PlayerController playerController = playerManager.GetPlayerController(playerId);
		if (playerController)
		{
			SCR_VONController vonCont = SCR_VONController.Cast(playerController.FindComponent(SCR_VONController));
			SCR_VoNComponent editorVonComponent = SCR_VoNComponent.Cast(FindComponent(SCR_VoNComponent));
			
			// Disconnect the VON component used by editor.
			editorVonComponent.DisconnectEditorFromVoNSystem();
			
			IEntity player = playerManager.GetPlayerControlledEntity(playerId);
			if (player)
			{
				SCR_VoNComponent charVonComponent =  SCR_VoNComponent.Cast(player.FindComponent(SCR_VoNComponent));
				
				// Tell SCR_VONController to use the SCR_VoNComponent attached to the player character.
				vonCont.SetVONComponent(charVonComponent);
				
				// Remove editor radio transceivers
				if (editorRadioEntries)
				{
					foreach (SCR_VONEntryRadio entry : editorRadioEntries)
					{
						if (entry)
							vonCont.RemoveEntry(entry);
					}
					editorRadioEntries.Clear();
				}
				
				vonCont.m_bIsEditorRadioAdded = false;
			}
		}
		
		StartEvents(EEditorEventOperation.REQUEST_CLOSE);
		//Event_OnRequest.Invoke(false);
		Rpc(ToggleServer, false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ToggleServer(bool open)
	{
		if (m_bIsOpened == open || RplSession.Mode() == RplMode.Client) return;

		// Attempting to open when it's not allowed
		if (open && !CanOpen())
		{
			SCR_NotificationsComponent.SendToPlayer(GetPlayerID(), ENotification.EDITOR_CANNOT_OPEN);
			return;
		}
		// Attempting to close when it's not allowed
		if (!open && !CanClose())
		{
			SCR_NotificationsComponent.SendToPlayer(GetPlayerID(), ENotification.EDITOR_CANNOT_CLOSE);
			return;
		}

		m_bIsOpened = open;
		Rpc(ToggleOwner, m_bIsOpened);

		if (m_bIsOpened)
		{
			if (m_CurrentModeEntity)
				m_CurrentModeEntity.ActivateModeServer();

			Event_OnOpenedServer.Invoke();
		}
		else
		{
			if (m_CurrentModeEntity)
				m_CurrentModeEntity.DeactivateModeServer();

			Event_OnClosedServer.Invoke();
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ToggleOwner(bool open)
	{
		m_bIsOpened = open;
		if (m_bIsOpened)
			StartEvents(EEditorEventOperation.OPEN);
		else
			StartEvents(EEditorEventOperation.CLOSE);

		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_IS_OPENED, m_bIsOpened);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void ToggleOwnerServerCallback(bool open)
	{
		if (open)
			Event_OnOpenedServerCallback.Invoke();
		else
			Event_OnClosedServerCallback.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetEnumSum(typename enumType)
	{
		int enumCount = enumType.GetVariableCount();
		int val, sum;

		for (int i = 0; i < enumCount; i++)
		{
			if (enumType.GetVariableType(i) == int && enumType.GetVariableValue(null, i, val))
			{
				sum += val;
			}
		}
		return sum;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Access

	//------------------------------------------------------------------------------------------------
	//! Check if the editor can be toggled.
	//! \return True when it can
	bool CanToggle()
	{
		if (m_bIsOpened)
			return CanClose();
		else
			return CanOpen();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the editor can be opened.
	//! \return True when it can
	bool CanOpen()
	{
		//--- No modes available
		if (m_Modes.IsEmpty())
			return false;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode || gameMode.GetState() == SCR_EGameModeState.POSTGAME)
			return false;

		if (IsLimited())
			return m_CanOpen == m_CanOpenSum;
		else
			return m_CanOpen | EEditorCanOpen.ALIVE == m_CanOpenSum;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if specific opening type is enabled.
	//! \param accessType Access type
	//! \return True when it can
	bool CanOpen(EEditorCanOpen accessType)
	{
		return m_CanOpen & accessType;
	}

	//------------------------------------------------------------------------------------------------
	//! Set if the editor can be opened.
	//! \param canOpen true if it can be opened
	//! \param accessType Access type. Editor can be opened only if all access types are active.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void SetCanOpen(bool canOpen, EEditorCanOpen accessType)
	{
		if (canOpen == CanOpen(accessType) || RplSession.Mode() == RplMode.Client) return;

		if (canOpen)
			m_CanOpen = m_CanOpen | accessType;
		else
			m_CanOpen = m_CanOpen &~ accessType;

		Rpc(SetCanOpenOwner, m_CanOpen);

		//--- Opening disabled, close the editor if it's
		if (!CanOpen() && IsOpened()) ToggleServer(false);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SetCanOpenOwner(EEditorCanOpen canOpen)
	{
		m_CanOpen = canOpen;
		Event_OnCanOpen.Invoke(m_CanOpen);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_OPEN, CanOpen(EEditorCanOpen.SCRIPT));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCanOpenDebug(bool canOpen, EEditorCanOpen accessType = EEditorCanOpen.SCRIPT)
	{
		if (canOpen == CanOpen(accessType)) return;
		Rpc(SetCanOpen, canOpen, accessType);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the editor can be closed.
	//! \return True when it can
	bool CanClose()
	{
		return
			m_CanClose == m_CanCloseSum;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if specific closing type is enabled.
	//! \param accessType Access type
	//! \return True when it can
	bool CanClose(EEditorCanClose accessType)
	{
		return m_CanClose & accessType;
	}

	//------------------------------------------------------------------------------------------------
	//! Set if the editor can be closed.
	//! \param canOpen true if it can be closed
	//! \param accessType Access type. Editor can be closed only if all access types are active.
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	void SetCanClose(bool canClose, EEditorCanClose accessType)
	{
		if (canClose == CanClose(accessType) || RplSession.Mode() == RplMode.Client) return;

		if (canClose)
			m_CanClose = m_CanClose | accessType;
		else
			m_CanClose = m_CanClose &~ accessType;

		Rpc(SetCanCloseOwner, m_CanClose);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SetCanCloseOwner(EEditorCanClose canClose)
	{
		m_CanClose = canClose;
		Event_OnCanClose.Invoke(m_CanClose);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_CLOSE, CanClose(EEditorCanClose.SCRIPT));
	}

	//------------------------------------------------------------------------------------------------
	protected void SetCanCloseDebug(bool canClose, EEditorCanClose accessType = EEditorCanClose.SCRIPT)
	{
		if (canClose == CanClose(accessType)) return;
		Rpc(SetCanClose, canClose, accessType);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//---  Setters & Getters

	//------------------------------------------------------------------------------------------------
	//! Set if the editor to be opened right when the editor manager is created. Can be used only in Event_OnEditorManagerCreated handler of SCR_EditorManagerEntityEntity
	//! \param isAutoInit True to start the editor automatically
	void SetAutoInit(bool isAutoInit = false)
	{
		m_bIsAutoInit = isAutoInit;
	}

	//------------------------------------------------------------------------------------------------
	//! \return True if the editor should open upon game start.
	bool IsAutoInit()
	{
		return m_bIsAutoInit;
	}

	//------------------------------------------------------------------------------------------------
	//! Set if editor modes should be created automatically.
	//! By default it's true, turn it off if you plan to manage modes manually.
	//! \param isAutoModes True to automatically create default editor modes.
	void SetAutoModes(bool isAutoModes = false)
	{
		m_bIsAutoModes = isAutoModes;
	}

	//------------------------------------------------------------------------------------------------
	//! Get ID of the player to whom the editor manager belongs to
	//! \return ID
	int GetPlayerID()
	{
		return m_iPlayerID;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsAuthority()
	{
		return this == GetInstance() || RplSession.Mode() != RplMode.Client;
	}

	//------------------------------------------------------------------------------------------------
	bool IsOwner()
	{
		//--- Ignore when not initialized, e.g., when setting up the entity by a game mode before its ownership is transferred to client
		return m_bInit && m_RplComponent && m_RplComponent.IsOwner();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the editor is currently opening or closing.
	//! \return True if in transition
	bool IsInTransition()
	{
		return m_bIsInTransition;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the editor is limited, i.e., none of available modes give the user full Game Master powers.
	//! For example, photo mode is limited, and having only means the editor is limited.
	//! \return True when limited
	bool IsLimited()
	{
		return m_bIsLimited || DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_FORCE_LIMITED);
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the local editor instance is limited, i.e., none of available modes give the user full Game Master powers.
	//! For example, photo mode is limited, and having only means the editor is limited.
	//! \return True when limited
	static bool IsLimitedInstance()
	{
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		return !editorManager || editorManager.IsLimited();
	}
	override SCR_EditorManagerEntity GetManager()
	{
		return this;
	}

	//------------------------------------------------------------------------------------------------
	//! Show notification.
	//! Doesn't actually show a notification, it only triggers an event which other systems can react to.
	//! \param notificationID
	//! \param selfID
	//! \param targetID
	//! \param position
	void SendNotification(ENotification notificationID, int selfID = 0, int targetID = 0, vector position = vector.Zero)
	{
		//Send notification
		SCR_NotificationsComponent.SendLocal(notificationID, position, selfID, targetID);
	}

	/*
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SendNotificationRpc(EEditorNotification notificationType)
	{
		SendNotificationImpl(notificationType);
	}*/

	//------------------------------------------------------------------------------------------------
	protected void UpdateLimited()
	{
		//--- Check if the editor is in limited mode
		bool limitedPrev = m_bIsLimited;
		m_bIsLimited = true;
		foreach (SCR_EditorModeEntity mode : m_Modes)
		{
			if (mode && !mode.IsLimited())
			{
				m_bIsLimited = false;
				break;
			}
		}

		//--- Assign player role used by game code systems
		if (Replication.IsServer() && GetGame().GetGameMode())
		{
			if (m_bIsLimited)
				GetGame().GetPlayerManager().ClearPlayerRole(m_iPlayerID, EPlayerRole.GAME_MASTER);
			else
				GetGame().GetPlayerManager().GivePlayerRole(m_iPlayerID, EPlayerRole.GAME_MASTER);
		}

		if (m_bIsLimited != limitedPrev)
			Event_OnLimitedChange.Invoke(m_bIsLimited);
	}

	void EnableCameraNwkSimulation(bool enable)
	{
		NwkMovementComponent nwkComponent = NwkMovementComponent.Cast(FindComponent(NwkMovementComponent));
		if(nwkComponent)
		{
			nwkComponent.EnableSimulation(enable);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Events

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnInit()
	{
		return Event_OnInit;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnOpened()
	{
		return Event_OnOpened;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnPreActivate()
	{
		return Event_OnPreActivate;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnActivate()
	{
		return Event_OnActivate;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnPostActivate()
	{
		return Event_OnPostActivate;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnDeactivate()
	{
		return Event_OnDeactivate;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnClosed()
	{
		return Event_OnClosed;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnDebug()
	{
		return Event_OnDebug;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnOpenedServer()
	{
		return Event_OnOpenedServer;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnOpenedServerCallback()
	{
		return Event_OnOpenedServerCallback;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnClosedServer()
	{
		return Event_OnClosedServer;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnClosedServerCallback()
	{
		return Event_OnClosedServerCallback;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnActivateServer()
	{
		return Event_OnOpenedServer;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnDeactivateServer()
	{
		return Event_OnClosedServer;
	}

	//------------------------------------------------------------------------------------------------
	override ScriptInvoker GetOnRequest()
	{
		return Event_OnRequest;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCanOpen()
	{
		return Event_OnCanOpen;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCanClose()
	{
		return Event_OnCanClose;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnModeAdd()
	{
		return Event_OnModeAdd;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnModeRemove()
	{
		return Event_OnModeRemove;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnModeChangeRequest()
	{
		return Event_OnModeChangeRequest;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnModeChange()
	{
		return Event_OnModeChange;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnLimitedChange()
	{
		return Event_OnLimitedChange;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAsyncLoad()
	{
		return Event_OnAsyncLoad;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnCanEndGameChanged()
	{
		return Event_OnCanEndGameChanged;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//-- Static

	//------------------------------------------------------------------------------------------------
	//! Toggle the local instance of the editor. If it's closed it will be opened, and vice versa.
	//! Nothing will happen if the desired action is disabled.
	static bool ToggleInstance()
	{
		SCR_EditorManagerEntity instance = GetInstance();
		if (!instance) return false;
		instance.Toggle();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Open the local instance of the editor.
	//! Nothing will happen if the editor is not allowed to be opened.
	static bool OpenInstance()
	{
		SCR_EditorManagerEntity instance = GetInstance();
		if (!instance)
			return false;

		instance.Open();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Close the local instance of the editor.
	//! Nothing will happen if the editor is not allowed to be closed.
	static bool CloseInstance()
	{
		SCR_EditorManagerEntity instance = GetInstance();
		if (!instance)
			return false;

		instance.Close();
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the local editor is currently opened.
	//! \param includeLimited True includes limited open editor, pass false to check for open unlimited editor only
	//! \return True if opened (and matches includeLimited)
	static bool IsOpenedInstance(bool includeLimited = true)
	{
		SCR_EditorManagerEntity instance = GetInstance();
		return instance
			&& instance.IsOpened()
			&& (includeLimited || !instance.IsLimited());
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the local editor can be opened.
	//! \return True when it can
	static bool CanOpenInstance()
	{
		SCR_EditorManagerEntity instance = GetInstance();
		if (!instance)
			return false;

		return instance.CanOpen();
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the local editor can be closed.
	//! \return True when it can
	static bool CanCloseInstance()
	{
		SCR_EditorManagerEntity instance = GetInstance();
		if (!instance)
			return false;

		return instance.CanClose();
	}

	//------------------------------------------------------------------------------------------------
	//! Get the local instance of the editor manager
	//! \return Editor manager
	static SCR_EditorManagerEntity GetInstance()
	{
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return null;

		SCR_EditorManagerCore manager = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!manager)
			return null;

		return manager.GetEditorManager();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Editor modes
	//--- @name Editor Modes
	//--- Management of editor modes.

	//------------------------------------------------------------------------------------------------
	//! Create editor mode.
	//! Must be executed on server!
	//! No mode will be created when one of given type already exists.
	//! \param mode Mode type
	//! \param prefab SCR_EditorModeEntity prefab from which the mode entity will be spawned. When undefined, default will be used (recommended)
	//! \return Editor mode
	SCR_EditorModeEntity CreateEditorMode(EEditorMode mode, bool isInit, ResourceName prefab = "")
	{
		if (RplSession.Mode() == RplMode.Client)
			return null;

		RplComponent rplEditor = RplComponent.Cast(FindComponent(RplComponent));
		if (!rplEditor)
			return null;

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core)
			return null;

		if (FindModeEntity(mode))
		{
			Print(string.Format("Cannot create %1 mode manager, it already exists!", Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
			return null;
		}

		//--- Find default prefab
		bool autoSelect;
		if (prefab.IsEmpty())
		{
			array<SCR_EditorModePrefab> modePrefabs = {};
			core.GetBaseModePrefabs(modePrefabs, -1, true);
			foreach (SCR_EditorModePrefab basePrefab : modePrefabs)
			{
				if (mode == basePrefab.GetMode())
				{
					prefab = basePrefab.GetPrefab();
					autoSelect = SCR_Enum.HasFlag(basePrefab.GetFlags(), EEditorModeFlag.AUTO_SELECT);
					break;
				}
			}
		}

		//--- No prefab found, exit
		if (prefab.IsEmpty())
		{
			Print(string.Format("No prefab found for editor mode '%1'!", Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
			return null;
		}

		//--- Spawn mode entity
		EntitySpawnParams spawnParams = new EntitySpawnParams;
		spawnParams.Parent = this;

		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams));
		if (!modeEntity)
		{
			Print(string.Format("Prefab %1 for editor mode %2 is not SCR_EditorModeEntity!", prefab.GetPath(), Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
			delete modeEntity;
			return null;
		}

		if (!modeEntity.GetParent())
		{
			Print(string.Format("Prefab %1 for editor mode %2 is missing Hierarchy component!", prefab.GetPath(), Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
			delete modeEntity;
			return null;
		}

		RplComponent rplMode = RplComponent.Cast(modeEntity.FindComponent(RplComponent));
		if (!rplMode)
		{
			Print(string.Format("Prefab %1 for editor mode %2 is missing RplComponent!", prefab.GetPath(), Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
			delete modeEntity;
			return null;
		}

		modeEntity.InitServer(this); //--- Called before AddMode, so events invoked from there can already work with variables defined in init

		AddMode(modeEntity, isInit);

		Rpc(CreateEditorModeOwner, mode, Replication.FindId(modeEntity), isInit);
		SetCanOpen(true, EEditorCanOpen.MODES);

		//--- Set current mode in certain conditions
		if (
			(
				!m_CurrentModeEntity //--- There is no current mode
				|| (autoSelect && !IsOpened()) //--- Newly created mode has AUTO_SELECT flag and editor is not opened
			)
			&& !m_Modes.Contains(null) //--- No mode was deleted in this frame (would try to run MODE_DELETE and MODE_CHANGE operations at the same time, failing both)
		)
		{
			SetCurrentMode(mode);
		}

		return modeEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Add editor modes of given type(s).
	//! \param access Access layer to which modes are added. Editor will receive modes from all its access layers.
	//! \param modes Flags of added modes
	void AddEditorModes(EEditorModeAccess access, EEditorMode modes, bool isInit = false)
	{
		SetEditorModes(access, GetEditorModes() | modes, isInit);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes editor modes of given type(s).
	//! \param access Access layer to which modes are added. Editor will receive modes from all its access layers.
	//! \param modes Flags of removes modes
	void RemoveEditorModes(EEditorModeAccess access, EEditorMode modes)
	{
		SetEditorModes(access, GetEditorModes() & ~modes, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Set editor modes of given type(s), replacing all previous modes.
	//! \param access Access layer to which modes are added. Editor will receive modes from all its access layers.
	//! \param modes Flags of set modes
	void SetEditorModes(EEditorModeAccess access, EEditorMode modes, bool isInit = false)
	{
		EEditorMode preActiveModes = GetEditorModes();
		m_ModesByAccess.Set(access, modes);
		EEditorMode activeModes = GetEditorModes();

		//--- Remove
		array<EEditorMode> flags = {};
		for (int i, count = SCR_Enum.BitToIntArray(preActiveModes & ~activeModes, flags); i < count; i++)
		{
			RplComponent.DeleteRplEntity(FindModeEntity(flags[i]), false);
		}
		RepairEditorModes(isInit);
	}

	//------------------------------------------------------------------------------------------------
	//! Go thrugh current editor modes and make sure they all have a mode entity.
	void RepairEditorModes(bool isInit)
	{
		array<EEditorMode> flags = {};
		for (int i, count = SCR_Enum.BitToIntArray(GetEditorModes(), flags); i < count; i++)
		{
			if (!FindModeEntity(flags[i]))
				CreateEditorMode(flags[i], isInit);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return Editor modes from all access layers
	EEditorMode GetEditorModes()
	{
		EEditorMode modes;
		for (int i, count = m_ModesByAccess.Count(); i < count; i++)
		{
			modes |= m_ModesByAccess.GetElement(i);
		}
		return modes;
	}

	//------------------------------------------------------------------------------------------------
	//! Get the first enabled mode from the list.
	//! \return Editor mode
	EEditorMode GetDefaultMode()
	{
		foreach (SCR_EditorModeEntity modeEntity : m_Modes)
		{
			if (modeEntity && !modeEntity.IsDeleted())
				return modeEntity.GetModeType();
		}
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Set editor mode to the previously selected one.
	void RestorePreviousMode()
	{
		SetCurrentMode(m_PrevMode);
	}

	//------------------------------------------------------------------------------------------------
	//! Set current (un)limited mode.
	//! \param IsLimited True to select the first available limited mode, false to selected unlimited mode
	//! \return True if suitable mode was found
	bool SetCurrentMode(bool isLimited)
	{
		foreach (SCR_EditorModeEntity modeEntity : m_Modes)
		{
			if (modeEntity.IsLimited() == isLimited)
			{
				SetCurrentMode(modeEntity.GetModeType());
				return true;
			}
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Set current editor mode.
	//! \param mode New mode. Use -1 to choose first enabled mode from the list.
	void SetCurrentMode(EEditorMode mode)
	{
		//--- Check also if existing entity exists, so it can be reset after all modes were removed and a previosuly current mode was added again.
		if (m_CurrentMode == mode && m_CurrentModeEntity)
			return;

		m_bIsModeChangeRequested = true;
		Event_OnModeChangeRequest.Invoke(FindModeEntity(mode), m_CurrentModeEntity);
		Rpc(SetCurrentModeServer, mode);
	}

	//------------------------------------------------------------------------------------------------
	//! Get current editor mode.
	//! \return Editor mode
	EEditorMode GetCurrentMode()
	{
		if (m_bIsModeChangeRequested && m_CurrentMode == m_ProcessedMode)
			return m_PrevMode; //--- Don't return current mode until async loading is finished
		else
			return m_CurrentMode;
	}

	//------------------------------------------------------------------------------------------------
	//! Get current editor mode entity.
	//! \return Editor mode entity
	SCR_EditorModeEntity GetCurrentModeEntity()
	{
		return m_CurrentModeEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the editor has given mode.
	//! \return True when the mode is available
	bool HasMode(EEditorMode mode)
	{
		foreach (SCR_EditorModeEntity modeEntity : m_Modes)
		{
			if (modeEntity && modeEntity.GetModeType() == mode)
				return true;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Find editor mode entity for given mode.
	//! \return Editor mode entity (or null if no such mode exists)
	SCR_EditorModeEntity FindModeEntity(EEditorMode mode)
	{
		foreach (SCR_EditorModeEntity modeEntity : m_Modes)
		{
			if (modeEntity && modeEntity.GetModeType() == mode) return modeEntity;
		}
		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Get all mode entities registered in this manager.
	//! \param[out] modeEntities Array to be filled with mode entities
	//! \return Number of mode entities
	int GetModeEntities(out notnull array<SCR_EditorModeEntity> modeEntities)
	{
		modeEntities.Copy(m_Modes);
		return modeEntities.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get all modes registered in this manager.
	//! \param[out] modes Array to be filled with modes
	//! \return Number of modes
	int GetModes(out notnull array<EEditorMode> modes)
	{
		modes.Clear();
		foreach (SCR_EditorModeEntity editorMode : m_Modes)
		{
			modes.Insert(editorMode.GetModeType());
		}
		return modes.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get all modes registered in this manager as a bit mask.
	//! \return Modes flag
	EEditorMode GetModes()
	{
		EEditorMode modes;
		foreach (SCR_EditorModeEntity editorMode : m_Modes)
		{
			modes = modes | editorMode.GetModeType();
		}
		return modes;
	}

	//------------------------------------------------------------------------------------------------
	//! Check if the owner is waiting for mode change from server.
	//! \return True if mode change was requested
	bool IsModeChangeRequested()
	{
		return m_bIsModeChangeRequested;
	}

	//------------------------------------------------------------------------------------------------
	protected void AddMode(notnull SCR_EditorModeEntity modeEntity, bool isInit)
	{
		//--- Mode already registered, ignore
		if (m_Modes.Find(modeEntity) >= 0) return;

		int order = modeEntity.GetOrder();
		if (order < 0)
		{
			//--- When default order is used, place at the back
			m_Modes.Insert(modeEntity);
		}
		else
		{
			//--- Place according to custom order
			int index = 0;
			for (int c = m_Modes.Count() - 1; c >= 0; c--)
			{
				index = c;
				if (m_Modes[c] && order > m_Modes[c].GetOrder())
				{
					index++;
					break;
				}
			}
			m_Modes.InsertAt(modeEntity, index);
		}

		//Notification
		if (!isInit && Replication.IsServer())
		{
			int playerID = GetPlayerID();
			if (playerID > 0)
			{
				if (!modeEntity.SendNotificationLocalOnly())
					SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(playerID, modeEntity.GetOnAddNotification(), playerID);
				else if (SCR_PlayerController.GetLocalPlayerId() == playerID)
					SCR_NotificationsComponent.SendLocal(modeEntity.GetOnAddNotification(), playerID);
			}
		}


		UpdateLimited();
		Event_OnModeAdd.Invoke(modeEntity);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveMode(notnull SCR_EditorModeEntity modeEntity, bool OnDisconnnect)
	{
		if (!m_Modes.Contains(modeEntity))
			return;

		if (IsOwner())
		{
			m_ProcessedMode = modeEntity.GetModeType();
			StartEvents(EEditorEventOperation.MODE_DELETE);
			Rpc(RemoveModeServer, modeEntity.GetModeType());
		}

		m_Modes.RemoveItem(modeEntity);
		UpdateLimited();

		Event_OnModeRemove.Invoke(modeEntity);

		if (!OnDisconnnect)
		{
			int playerID = GetPlayerID();

			if (playerID > 0)
			{
				if (!modeEntity.SendNotificationLocalOnly())
					SCR_NotificationsComponent.SendToUnlimitedEditorPlayersAndPlayer(playerID, modeEntity.GetOnRemoveNotification(), playerID);
				else if (SCR_PlayerController.GetLocalPlayerId() == playerID)
					SCR_NotificationsComponent.SendLocal(modeEntity.GetOnRemoveNotification(), playerID);
			}

		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RemoveModeServer(EEditorMode mode)
	{
		if (!m_Modes.IsEmpty())
		{
			//--- Current mode was removed, switch to another one
			if (mode == m_CurrentMode)
				SetCurrentMode(-1);

			//--- Close when removing the mode changed access rules (e.g., remaining mode is limited)
			if (!CanOpen() && IsOpened())
				ToggleServer(false);
		}
		else
		{
			//--- Last mode was removed, close editor
			//if (Replication.IsRunning()) //--- Call only when the world is not shutting down
				SetCanOpen(false, EEditorCanOpen.MODES);
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void CreateEditorModeOwner(EEditorMode mode, int modeEntityId, bool isInit)
	{
		SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(Replication.FindItem(modeEntityId));
		if (!modeEntity) return;

		AddMode(modeEntity, isInit);
		modeEntity.InitOwner();

		m_ProcessedMode = mode;
		StartEvents(EEditorEventOperation.MODE_CREATE);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void SetCurrentModeServer(EEditorMode mode)
	{
		//--- Try to use the first enabled mode
		if (mode < 0)
			mode = GetDefaultMode();

		//--- Ignore when there is no mode change, or when all modes were removed (i.e., GetDefaultMode() didn't find a replacement)
		if ((m_CurrentMode == mode && m_CurrentModeEntity) || mode == -1)
			return;

		//--- Check if mode entity is enabled
		SCR_EditorModeEntity modeEntity = null;
		if (mode >= 0)
		{
			modeEntity = FindModeEntity(mode);
			if (!modeEntity)
			{
				Print(string.Format("SCR_EditorModeEntity for mode '%1' not found!", Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
				return;
			}
		}

		if (m_CurrentModeEntity && m_bIsOpened)
			m_CurrentModeEntity.DeactivateModeServer();

		Rpc(SetCurrentModeOwner, mode);
		m_CurrentMode = mode;
		m_CurrentModeEntity = modeEntity;

		if (m_CurrentModeEntity && m_bIsOpened)
			m_CurrentModeEntity.ActivateModeServer();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void SetCurrentModeOwner(EEditorMode mode)
	{
		//--- When closing the game, instance reference is already removed. Don't set any mode in such case.
		if (!SCR_EditorManagerEntity.GetInstance()) return;

		SCR_EditorModeEntity modeEntity = FindModeEntity(mode);
		if (!modeEntity && mode >= 0)
		{
			Print(string.Format("Editor mode '%1' not found!", Type().EnumToString(EEditorMode, mode)), LogLevel.ERROR);
			return;
		}
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_MODE, Math.Log2(mode));

		m_ProcessedMode = mode;
		StartEvents(EEditorEventOperation.MODE_CHANGE);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Authors logic (UGC)
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RequestAllAuthorsServer()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		set<SCR_EditableEntityAuthor> authors = core.GetAllAuthorsServer();

		foreach (SCR_EditableEntityAuthor author : authors)
		{
			Rpc(RegisterAuthorOwner, author);
		}

		Rpc(FinishAuthorsRequestOwner);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void FinishAuthorsRequestOwner()
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		core.OnAuthorRequestFinished();
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RegisterAuthorOwner(notnull SCR_EditableEntityAuthor author)
	{
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
			core.AddAuthorOnRequest(author);
	}

	//------------------------------------------------------------------------------------------------
	void RequestAllAuthors()
	{
		Rpc(RequestAllAuthorsServer);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Component events
	//------------------------------------------------------------------------------------------------
	protected void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE)
	{
		//--- New operation requested when another one is being processed - close the editor to prevent conflicts!
		//--- Happens for example when editor mode is deleted while being processed.
		if (m_iEventOperation != EEditorEventOperation.NONE)
		{
			Print(string.Format("Editor operation %1 was requested while %2 was still running! Editor closed to prevent conflicts!", typename.EnumToString(EEditorEventOperation, type), typename.EnumToString(EEditorEventOperation, m_iEventOperation)), LogLevel.WARNING);
			m_aEvents = {EEditorEvent.EXIT_OPERATION};
			ProcessEvent();
			ToggleOwner(false);
			return;
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ASYNC_LOAD_DEBUG))
		{
			string currentMode = typename.EnumToString(EEditorMode, m_CurrentMode);
			if (!m_CurrentModeEntity)
				currentMode += " (null)";
			Print(string.Format("StartEvents: %1 | current mode: %2 | processed mode: %3", typename.EnumToString(EEditorEventOperation, type), currentMode, typename.EnumToString(EEditorMode, m_ProcessedMode)), LogLevel.DEBUG);
		}

		m_iEventOperation = type;
		m_iEventProgress = 0;

		switch (type)
		{
			case EEditorEventOperation.INIT:
			{
				GetAllComponents(m_aEventComponentsActivate);
				m_aEvents = {
					EEditorEvent.INIT,
					EEditorEvent.EXIT_OPERATION
				};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.DELETE:
			{
				FindEditorComponents(m_aEventComponentsDeactivate);
				if (IsOpened())
				{
					if (m_ProcessedMode == m_CurrentMode)
					{
						//--- Currently opened mode
						m_aEvents = {
							EEditorEvent.DEACTIVATE,
							EEditorEvent.DEACTIVATE_ASYNC,
							EEditorEvent.POST_DEACTIVATE,
							EEditorEvent.CLOSE,
							EEditorEvent.DELETE,
							EEditorEvent.EXIT_OPERATION,
						};
					}
					else
					{
						//--- Inactive mode in opened editor
						m_aEvents = {
							EEditorEvent.CLOSE,
							EEditorEvent.DELETE,
							EEditorEvent.EXIT_OPERATION,
						};
					}
				}
				else
				{
					m_aEvents = {
						EEditorEvent.DELETE,
						EEditorEvent.EXIT_OPERATION,
					};
				}
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.REQUEST_OPEN:
			{
				GetAllComponents(m_aEventComponentsActivate);
				m_aEvents = {
					EEditorEvent.REQUEST_OPEN,
					EEditorEvent.EXIT_OPERATION
				};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.OPEN:
			{
				m_iEventProgressMax = 1;

				FindEditorComponents(m_aEventComponentsActivate);
				if (m_CurrentModeEntity) m_CurrentModeEntity.FindEditorComponents(m_aEventComponentsActivate);
				m_aEventComponentsDeactivate = {};
				m_bIsInTransition = true;

				m_aEvents = {
					EEditorEvent.OPEN_ALL,
					EEditorEvent.PRE_ACTIVATE,
					EEditorEvent.ACTIVATE,
					EEditorEvent.ACTIVATE_ASYNC,
					EEditorEvent.POST_ACTIVATE,
					EEditorEvent.EXIT_OPERATION,
				};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.REQUEST_CLOSE:
			{
				GetAllComponents(m_aEventComponentsActivate);
				m_aEvents = {
					EEditorEvent.REQUEST_CLOSE,
					EEditorEvent.EXIT_OPERATION
				};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.CLOSE:
			{
				m_iEventProgressMax = 1;

				FindEditorComponents(m_aEventComponentsDeactivate);
				if (m_CurrentModeEntity) m_CurrentModeEntity.FindEditorComponents(m_aEventComponentsDeactivate);
				m_aEventComponentsActivate = {};
				m_bIsInTransition = true;

				m_aEvents = {
					EEditorEvent.DEACTIVATE,
					EEditorEvent.DEACTIVATE_ASYNC,
					EEditorEvent.POST_DEACTIVATE,
					EEditorEvent.CLOSE,
					EEditorEvent.EXIT_OPERATION
				};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.MODE_CHANGE:
			{
				m_iEventProgressMax = 2;

				FindModeEntity(m_ProcessedMode).FindEditorComponents(m_aEventComponentsActivate);
				m_aEventComponentsDeactivate = {};
				if (m_CurrentModeEntity) m_CurrentModeEntity.FindEditorComponents(m_aEventComponentsDeactivate);

				if (IsOpened())
					m_aEvents = {
						EEditorEvent.PRE_ACTIVATE,
						EEditorEvent.DEACTIVATE,
						EEditorEvent.DEACTIVATE_ASYNC,
						EEditorEvent.UPDATE_MODE,
						EEditorEvent.ACTIVATE,
						EEditorEvent.ACTIVATE_ASYNC,
						EEditorEvent.POST_DEACTIVATE,
						EEditorEvent.POST_ACTIVATE,
						EEditorEvent.EXIT_OPERATION,
					};
				else
					m_aEvents = {
						EEditorEvent.UPDATE_MODE,
						EEditorEvent.EXIT_OPERATION,
					};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.MODE_CREATE:
			{
				FindModeEntity(m_ProcessedMode).FindEditorComponents(m_aEventComponentsActivate);
				if (IsOpened())
					m_aEvents = {
						EEditorEvent.INIT,
						EEditorEvent.OPEN,
						EEditorEvent.EXIT_OPERATION
					};
				else
					m_aEvents = {
						EEditorEvent.INIT,
						EEditorEvent.EXIT_OPERATION
					};
				ProcessEvent();
				break;
			}
			case EEditorEventOperation.MODE_DELETE:
			{
				FindModeEntity(m_ProcessedMode).FindEditorComponents(m_aEventComponentsDeactivate);
				if (IsOpened())
				{
					if (m_ProcessedMode == m_CurrentMode)
					{
						m_aEvents = {
							EEditorEvent.DEACTIVATE,
							EEditorEvent.DEACTIVATE_ASYNC,
							EEditorEvent.POST_DEACTIVATE,
							EEditorEvent.CLOSE,
							EEditorEvent.DELETE,
							EEditorEvent.EXIT_OPERATION,
						};
					}
					else
					{
						m_aEvents = {
							EEditorEvent.CLOSE,
							EEditorEvent.DELETE,
							EEditorEvent.EXIT_OPERATION,
						};
					}
				}
				else
				{
					m_aEvents = {
						EEditorEvent.DELETE,
						EEditorEvent.EXIT_OPERATION,
					};
				}
				ProcessEvent();
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! This would be so much nicer as a flow chart
	protected void ProcessEvent()
	{
		m_iEvent = m_aEvents[0];

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ASYNC_LOAD_DEBUG) && m_iEventComponent == 0 && m_iEventAttempt == 0)
		{
			Print(typename.EnumToString(EEditorEvent, m_iEvent), LogLevel.NORMAL);
		}

		switch (m_iEvent)
		{
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.INIT:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
					component.OnInitBase();
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.REQUEST_OPEN:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
					component.OnRequestBase(true);

				Event_OnRequest.Invoke(true);
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.REQUEST_CLOSE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
					if (component) component.OnRequestBase(false);

				Event_OnRequest.Invoke(false);
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.OPEN:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
				{
					if (component) component.OnOpenedBase();
				}
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.OPEN_ALL:
			{
				array<SCR_BaseEditorComponent> allComponents;
				GetAllComponents(allComponents);

				foreach (SCR_BaseEditorComponent component : allComponents)
				{
					if (component) component.OnOpenedBase();
				}
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.PRE_ACTIVATE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
				{
					if (component) component.OnPreActivateBase();
				}

				SCR_EditorModeEntity processedMode = FindModeEntity(m_ProcessedMode);
				if (processedMode)
					processedMode.PreActivateMode();

				Event_OnPreActivate.Invoke();
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.DEACTIVATE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsDeactivate)
				{
					if (component) component.OnDeactivateBase();
				}
				if (m_ProcessedMode == m_CurrentMode && m_CurrentModeEntity)
					m_CurrentModeEntity.DeactivateMode();

				Event_OnDeactivate.Invoke();
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.DEACTIVATE_ASYNC:
			{
				bool instantContinue;
				if (ProcessAsyncEvent(false, instantContinue))
				{
				}
				else if (instantContinue)
				{
					ProcessEvent();
					return;
				}
				else
				{
					return;
				}

				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.ACTIVATE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
				{
					if (component)
						component.OnActivateBase();
				}

				if (m_CurrentModeEntity)
					m_CurrentModeEntity.ActivateMode();

				Event_OnActivate.Invoke();
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.ACTIVATE_ASYNC:
			{
				bool instantContinue;
				if (ProcessAsyncEvent(true, instantContinue))
				{
				}
				else if (instantContinue)
				{
					ProcessEvent();
					return;
				}
				else
				{
					return;
				}

				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.POST_DEACTIVATE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsDeactivate)
				{
					if (component)
						component.OnPostDeactivateBase();
				}
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.POST_ACTIVATE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsActivate)
				{
					if (component) component.OnPostActivateBase();
				}

				if (m_CurrentModeEntity)
					m_CurrentModeEntity.PostActivateMode();

				Event_OnPostActivate.Invoke();
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.CLOSE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsDeactivate)
				{
					if (component)
						component.OnClosedBase();
				}
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.DELETE:
			{
				foreach (SCR_BaseEditorComponent component : m_aEventComponentsDeactivate)
				{
					if (component)
						component.OnDeleteBase();
				}
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.UPDATE_MODE:
			{
				m_PrevMode = m_CurrentMode;
				m_CurrentMode = m_ProcessedMode;
				m_CurrentModeEntity = FindModeEntity(m_CurrentMode);
				break;
			}
			//////////////////////////////////////////////////////////////////////////////////////////////
			case EEditorEvent.EXIT_OPERATION:
			{
				//--- Set before invokers are called, so using getters in them will not see transition as active anymore
				m_bIsInTransition = false;

				//--- Call invokers once all events are processed
				switch (m_iEventOperation)
				{
					case EEditorEventOperation.OPEN:
					{
						Event_OnAsyncLoad.Invoke(1);
						Event_OnOpened.Invoke();
						Rpc(ToggleOwnerServerCallback, true);
						break;
					}
					case EEditorEventOperation.CLOSE:
					{
						//--- Save stored user camera locations when leaving GM
						GetGame().UserSettingsChanged();
						GetGame().SaveUserSettings();
						Event_OnAsyncLoad.Invoke(1);
						Event_OnClosed.Invoke();
						Rpc(ToggleOwnerServerCallback, false);
						break;
					}
					case EEditorEventOperation.MODE_CHANGE:
					{
						m_bIsModeChangeRequested = false;
						if (IsOpened()) Event_OnAsyncLoad.Invoke(1);
						Event_OnModeChange.Invoke(m_CurrentModeEntity, FindModeEntity(m_PrevMode));
						break;
					}
				}

				//--- Clean up
				m_iEventOperation = EEditorEventOperation.NONE;
				m_aEventComponentsActivate = null;
				m_aEventComponentsDeactivate = null;
				m_iEvent = EEditorEvent.NONE;
				m_ProcessedMode = 0;
				return;
			}
		}

		//--- Next event
		m_aEvents.RemoveOrdered(0);
		if (!m_aEvents.IsEmpty())
			ProcessEvent();
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessAsyncEvent(bool toActivate, out bool instantContinue)
	{
		//--- Get components to be processed
		array<SCR_BaseEditorComponent> components;
		if (toActivate)
			components = m_aEventComponentsActivate;
		else
			components = m_aEventComponentsDeactivate;

		//--- All processed, clean up and exit
		if (m_iEventComponent >= components.Count())
		{
			m_iEventComponent = 0;
			m_iEventAttempt = 0;
			return true;
		}

		//--- Block input
		GetGame().GetInputManager().ActivateContext("DebugContext");

		//--- Start timer
		if (m_iEventAttempt == 0)
			m_fEventAsyncStart = GetGame().GetWorld().GetWorldTime();

		//--- Process component
		bool isProcessed;
		SCR_BaseEditorComponent component = components[m_iEventComponent];
		if (component)
		{
			if (toActivate)
				isProcessed = component.OnActivateAsyncBase(m_iEventAttempt);
			else
				isProcessed = component.OnDeactivateAsyncBase(m_iEventAttempt);

			//--- Editor mode to which the component belongs to is being deleted - execute instantly, no time for async
			instantContinue = IsDeleted() || !component.GetOwner() || component.GetOwner().IsDeleted();
		}
		else
		{
			//--- Component was meanwhile deleted, skip it
			isProcessed = true;
			instantContinue = true;
			m_iEventAttempt = 0;
		}

		if (isProcessed)
		{
			//--- Next component
			if (m_iEventAttempt == 0)
				instantContinue = true;

			m_iEventComponent++;
			m_iEventAttempt = 0;
			m_iEventProgress += 1 / components.Count() / m_iEventProgressMax;
			Event_OnAsyncLoad.Invoke(m_iEventProgress);

			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ASYNC_LOAD_DEBUG) && component)
				Print(string.Format("%1 (%2), %3 ms", Math.Round(m_iEventProgress * 100), component.Type(), GetGame().GetWorld().GetWorldTime() - m_fEventAsyncStart), LogLevel.VERBOSE);
		}
		else
		{
			//--- Next attempt
			m_iEventAttempt++;
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void GetAllComponents(out array<SCR_BaseEditorComponent> outComponents)
	{
		FindEditorComponents(outComponents);
		foreach (SCR_EditorModeEntity mode : m_Modes)
		{
			if (mode)
				mode.FindEditorComponents(outComponents);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Actions
	//------------------------------------------------------------------------------------------------
	protected void Action_EditorToggle(float value, EActionTrigger reason)
	{
		Toggle();
	}

	//------------------------------------------------------------------------------------------------
	protected void Action_GoToLatestNotification(float value, EActionTrigger reason)
	{
		if (!m_NotificationsComponent)
			m_NotificationsComponent = SCR_NotificationsComponent.GetInstance();

		if (IsOpened() && !IsLimited() && m_NotificationsComponent)
		{
			vector goToPosition;
			if (m_NotificationsComponent.GetLastNotificationLocation(goToPosition))
			{
				SCR_ManualCamera camera = SCR_CameraEditorComponent.GetCameraInstance();
				SCR_TeleportToCursorManualCameraComponent cursorManualCameraComponent = SCR_TeleportToCursorManualCameraComponent.Cast(camera.FindCameraComponent(SCR_TeleportToCursorManualCameraComponent));
				cursorManualCameraComponent.TeleportCamera(goToPosition);
			}

		}

	}

	//--- Debug

	//------------------------------------------------------------------------------------------------
	protected void ShowDebug()
	{
		array<string> debugTexts = new array<string>;

		debugTexts.Insert(string.Format("CanOpen: %1/%2", m_CanOpen, m_CanOpenSum));
		debugTexts.Insert(string.Format("CanClose: %1/%2", m_CanClose, m_CanCloseSum));

		string currentModeName = "N/A";
		if (m_CurrentModeEntity)
			currentModeName = typename.EnumToString(EEditorMode, m_CurrentModeEntity.GetModeType());

		debugTexts.Insert(string.Format("Current Mode: %1", currentModeName));
		debugTexts.Insert(string.Format("Available Modes: %1", m_Modes.Count()));
		debugTexts.Insert(string.Format("Limited: %1", m_bIsLimited));
		debugTexts.Insert(string.Format("Entity Author name: %1", GetEntityAuthor()));
		debugTexts.Insert(string.Format("Entity Author UID: %1", GetEntityAuthorUID()));
		GetOnDebug().Invoke(debugTexts);

		DbgUI.Begin("SCR_EditorManagerEntity", 0, 0);
		foreach (string text : debugTexts)
		{
			DbgUI.Text(text);
		}
		DbgUI.End();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_EditableEntityAuthor GetEntityAuthor()
	{
		SCR_EditableEntityComponent entity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
		if (!entity)
			return null;

		return entity.GetAuthor();
	}

	//------------------------------------------------------------------------------------------------
	protected int GetEntityAuthorUID()
	{
		SCR_EditableEntityComponent entity = SCR_BaseEditableEntityFilter.GetFirstEntity(EEditableEntityState.HOVER);
		if (!entity)
			return "";

		return entity.GetAuthorUID();
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessDebug()
	{
#ifdef ENABLE_DIAG
		bool isOpened = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_IS_OPENED);
		if (isOpened != IsOpened())
		{
			//--- Make sure the editor can be opened / closed
			if (isOpened)
			{
				SetCanOpenDebug(true, EEditorCanOpen.SCRIPT);
			}
			else
			{
				SetCanCloseDebug(true);
			}
			Toggle();
		}
		
		bool canOpen = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_OPEN);
		if (canOpen != CanOpen(EEditorCanOpen.SCRIPT))
		{
			SetCanOpenDebug(canOpen, EEditorCanOpen.SCRIPT);
		}
		
		bool canClose = DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_CLOSE);
		if (canClose != CanClose(EEditorCanOpen.SCRIPT))
		{
			SetCanCloseDebug(canClose);
		}

		if (m_CurrentModeEntity)
		{
			int debugMode = 1 << DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_MODE);
			if (m_CurrentMode != debugMode && FindModeEntity(debugMode))
			{
				SetCurrentMode(debugMode);
			}
		}

		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SHOW_DEBUG)) ShowDebug();
#endif
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Init

	//------------------------------------------------------------------------------------------------
	void InitServer(int playerID)
	{
		if (m_bInit)
			return;

		m_bInit = true;
		m_iPlayerID = playerID;

		InitComponents(true);
		Rpc(InitOwner, playerID);

		//--- Initialize modes
		IEntity child = GetChildren();
		while (child)
		{
			SCR_EditorModeEntity modeEntity = SCR_EditorModeEntity.Cast(child);
			if (modeEntity) modeEntity.InitServer(this);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void InitOwner(int playerID)
	{
		if (GetInstance())
			return; //--- Exit when already initialized

		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		if (!core || !core.SetEditorManager(this))
			return;

		m_bInit = true;
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (!m_RplComponent) Print("SCR_EditorManagerEntity is missing RplComponent component!", LogLevel.ERROR);

		m_iPlayerID = playerID;
		InitComponents(false);

		StartEvents(EEditorEventOperation.INIT); //--- Init on all editor components
		//Event_OnInit.Invoke();
		core.Event_OnEditorManagerInitOwner.Invoke(this); //--- External init

		SetEventMask(EntityEvent.FRAME);

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener("EditorToggle", EActionTrigger.PRESSED, Action_EditorToggle);
			inputManager.AddActionListener("EditorLastNotificationTeleport", EActionTrigger.DOWN, Action_GoToLatestNotification);
		}

		//--- Debug
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_IS_OPENED, "", "Is Opened", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_OPEN, "", "Can Open", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_CLOSE, "", "Can Close", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SHOW_DEBUG, "", "Show Debug", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_FORCE_LIMITED, "", "Force Limited", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_ASYNC_LOAD_DEBUG, "", "Log Async Load", "Editor");
		DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_EDITOR_SAVE_PHOTO_SCREENSHOT_SHOW, "", "Show Screenshot", "Editor");
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_NETWORK_DELAY, "", "Simulated Network Delay", "Editor", "0 1000 0 100");

		//--- Reset values, they're stored from the previous session
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_IS_OPENED, m_bIsOpened);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_OPEN, m_CanOpen);
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_CLOSE, m_CanClose);

		typename enumType = EEditorMode;
		int enumCount = enumType.GetVariableCount();
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_MODE, "", "Mode", "Editor", string.Format("0 %1 0 1", enumCount - 1));
		DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_MODE, Math.Max(m_CurrentMode, 0));
	}

	//------------------------------------------------------------------------------------------------
	void PostInitServer()
	{
		if (m_CurrentMode < 0)
		{
			SetCurrentModeServer(-1);
		}
	}

	//------------------------------------------------------------------------------------------------
	void AutoInit()
	{
		if (m_bIsAutoInit && !m_bIsLimited)
		{
			Open(false);
		}
		m_bIsAutoInit = false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	//--- Default functions

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
		{
			world.RegisterEntityToBeUpdatedWhileGameIsPaused(this);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice) //--- Active only when the entity is local (see InitOwner())
	{
		if (m_iEvent == EEditorEvent.NONE)
			ProcessDebug();
		else
			ProcessEvent();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EditorManagerEntity(IEntitySource src, IEntity parent)
	{
		m_CanOpenSum = GetEnumSum(EEditorCanOpen);
		m_CanCloseSum = GetEnumSum(EEditorCanClose);

		SetFlags(EntityFlags.NO_TREE | EntityFlags.NO_LINK);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_EditorManagerEntity()
	{
		while (!m_Modes.IsEmpty())
		{
			if (m_Modes[0])
				RemoveMode(m_Modes[0], true);
			else
				m_Modes.RemoveOrdered(0);
		}

		if (m_CurrentModeEntity)
			m_CurrentModeEntity.DeactivateModeServer();

		if (Replication.IsServer() && IsOpened())
			Event_OnClosedServer.Invoke();

		if (IsOwner())
			StartEvents(EEditorEventOperation.DELETE);

		if (GetInstance() == this)
		{
			//--- Local entity
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_IS_OPENED);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_OPEN);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_CAN_CLOSE);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_SHOW_DEBUG);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_MODE);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_ASYNC_LOAD_DEBUG);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_SAVE_PHOTO_SCREENSHOT_SHOW);
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_EDITOR_NETWORK_DELAY);
		}

		ChimeraWorld world = GetGame().GetWorld();
		if (world)
		{
			world.UnregisterEntityToBeUpdatedWhileGameIsPaused(this);
		}
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorMode, "m_Mode")]
class SCR_EditorModePrefab
{
	[Attribute(string.Format("%1", EEditorMode.EDIT), desc: "", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorMode))]
	private EEditorMode m_Mode;

	[Attribute("", UIWidgets.ResourceNamePicker, "Individual editor manager", "et")]
	private ResourceName m_Prefab;

	[Attribute("", desc: "", uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(EEditorModeFlag))]
	private EEditorModeFlag m_Flags;

	[Attribute()]
	protected ref SCR_EditorModeUIInfo m_ModeUIInfo;

	//------------------------------------------------------------------------------------------------
	EEditorMode GetMode()
	{
		return m_Mode;
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetPrefab()
	{
		return m_Prefab;
	}

	//------------------------------------------------------------------------------------------------
	EEditorModeFlag GetFlags()
	{
		return m_Flags;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EditorModeUIInfo GetInfo()
	{
		return m_ModeUIInfo;
	}
}
