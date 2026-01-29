//#define DISABLE_HUD
//#define DEBUG_INFO_DISPLAY_EXT

enum EShowGUI // TODO: SCR_
{
	IN_ADS = 1,
	IN_1ST_PERSON = 2,
	IN_3RD_PERSON = 4,
	IN_PAUSE_MENU = 8,
	WITHOUT_ENTITY = 16,
	WHILE_UNCONSCIOUS = 32,
	IN_EDITOR = 64
}

//------------------------------------------------------------------------------------------------
class SCR_InfoDisplayExtended : SCR_InfoDisplay
{
	[Attribute("1", UIWidgets.CheckBox, "Toggles ON/OFF info display.")]
	protected bool m_bIsEnabled;
	
	[Attribute("39", UIWidgets.Flags, "Defines when the GUI element is allowed to show.", "", ParamEnumArray.FromEnum(EShowGUI) )]
	private EShowGUI m_eShow;	
	
	[Attribute("", desc: "If Display can be disabled in settings, define the correct variable defined in: 'SCR_InterfaceSettings'")]
	protected string m_sInterfaceSettingName;
	
	protected SCR_PlayerController m_PlayerController;
	protected SCR_CharacterControllerComponent m_CharacterController;
	protected SCR_CharacterCameraHandlerComponent m_CameraHandler;
	protected MenuManager m_MenuManager;
	protected EventHandlerManagerComponent m_EventHandlerManager;
	protected SCR_EditorManagerEntity m_EditorManager;
	
	protected bool m_bAttachedToPlayerController;
	
	protected bool m_bInThirdPerson;
	protected bool m_bInADS;
	protected bool m_bIsUnconscious;
	protected bool m_bInPauseMenu;
	protected bool m_bInEditor;
	protected bool m_bCanShow;							// Global GUI visibility flag, is not meant to be fiddled with outside this class
	protected bool m_bIsEnabledInSettings = true;
	protected bool m_bShowInAllCameras = true;
		
	//------------------------------------------------------------------------------------------------		
	// DEBUG: Used to output debug prints only for specific class
	//------------------------------------------------------------------------------------------------		
	#ifdef DEBUG_INFO_DISPLAY_EXT
	//------------------------------------------------------------------------------------------------
	void _printClass(string str)
	{
		if (this.Type() != SCR_DeathScreenEffect)
			return;
		
		Print(str, LogLevel.DEBUG);
	}
	#endif

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool isEnabled)
	{
		m_bIsEnabled = isEnabled;
	}
	
	//------------------------------------------------------------------------------------------------		
	// Interface methods for the extended InfoDisplay class.
	//------------------------------------------------------------------------------------------------		
	protected bool DisplayStartDrawInit(IEntity owner)
	{
		return true;
	}	

	//------------------------------------------------------------------------------------------------
	protected void DisplayStartDraw(IEntity owner);

	//------------------------------------------------------------------------------------------------
	protected void DisplayStopDraw(IEntity owner);

	//------------------------------------------------------------------------------------------------
	protected void DisplayInit(IEntity owner);

	//------------------------------------------------------------------------------------------------
	protected void DisplayUpdate(IEntity owner, float timeSlice);

	//------------------------------------------------------------------------------------------------
	protected void DisplayControlledEntityChanged(IEntity from, IEntity to);

	//------------------------------------------------------------------------------------------------
	protected void DisplayConsciousnessChanged(bool conscious, bool init = false);

	//------------------------------------------------------------------------------------------------
	//! Called when GUI is temporarily suspended due to visibility flags; e.g. GM entered and GUI marked as not to show in GM
	protected void DisplayOnSuspended()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [DisplayOnSuspended]", this));
		#endif		
	}

	//------------------------------------------------------------------------------------------------
	// Called when the visibility flags no longer suspend the GUI; e.g. GM left and GUI marked as not to show in GM -> GUI can show again
	// Doesn't mean the GUI is visible, it is just not hidded due to visibility flags; use m_bShown to check the visibility
	protected void DisplayOnResumed()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [DisplayOnResumed]", this));
		#endif		
	}		

	//------------------------------------------------------------------------------------------------
	protected void InitializeInterfaceSettings()
	{
		if (!m_HUDManager)
			return;
		
		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(m_HUDManager.GetInterfaceSettingsClass());
		if (!interfaceSettings)
			return;
		
		bool state;
		interfaceSettings.Get(m_sInterfaceSettingName, state);
		m_bIsEnabledInSettings = state;
		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnSettingsChanged);		
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSettingsChanged()
	{		
		if (!m_HUDManager)
			return;
		
		BaseContainer interfaceSettings = GetGame().GetGameUserSettings().GetModule(m_HUDManager.GetInterfaceSettingsClass());
		if (!interfaceSettings)
			return;
		
		bool state;
		if (!interfaceSettings.Get(m_sInterfaceSettingName, state))
			return;

		m_bIsEnabledInSettings = state;
		
		UpdateVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	// InfoDisplay events blocked for overriding.
	// The interface methods above should be used instead.		
	//------------------------------------------------------------------------------------------------
	private override event void OnStartDraw(IEntity owner)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnStartDraw] owner: %2", this, owner));
		#endif
		
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif
		
		if (!m_bIsEnabled)
			return;

		m_MenuManager = GetGame().GetMenuManager();
		
		if (!m_MenuManager)
		{
			m_bIsEnabled = false;
			return;
		}
	
		m_HUDManager = SCR_HUDManagerComponent.GetHUDManager();
		
		if (!m_HUDManager)
		{
			m_bIsEnabled = false;
			return;
		}		
		
		// Terminate if UI is not enabled for any of the camera modes (1PV, 3PV, ADS) && requires a controlled entity
		if (m_eShow & (EShowGUI.IN_ADS | EShowGUI.IN_1ST_PERSON | EShowGUI.IN_3RD_PERSON | EShowGUI.WITHOUT_ENTITY) == 0)
		{
			m_bIsEnabled = false;
			return;
		}		
		
		m_bShowInAllCameras = (m_eShow & (EShowGUI.IN_ADS | EShowGUI.IN_1ST_PERSON | EShowGUI.IN_3RD_PERSON)) == (EShowGUI.IN_ADS | EShowGUI.IN_1ST_PERSON | EShowGUI.IN_3RD_PERSON);
			
		m_PlayerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());

		if (!m_PlayerController)
		{
			m_bIsEnabled = false;
			return;
		}

		// Init Editor Open/Close handling
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		
		// If Editor doesn't exist yet, we need to add invoker and postpone the initialization of Editor Open/Close invokers
		if (!editorManager)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			core.Event_OnEditorManagerInitOwner.Insert(OnEditorInit);		
		}
		else
		{
			OnEditorInit(editorManager);
		}

		// Init PauseMenu handling
		if ((m_eShow & EShowGUI.IN_PAUSE_MENU) == 0)
		{
			PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpen);
			
			PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClose);
		}
		
		// Init monitor of controlled entity and setup initial GUI visibility
		if (m_bAttachedToPlayerController)
		{
			m_PlayerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		}
		// Init the 'OnControlledEntityChanged' with currently controlled entity
		OnControlledEntityChanged(null, m_PlayerController.GetControlledEntity());		
				
		m_bIsEnabled = DisplayStartDrawInit(owner);
		
		if (!m_bIsEnabled)
			return;
		
		super.OnStartDraw(owner);
		
		if (!m_wRoot)
		{
			m_bIsEnabled = false;
			return;
		}
		
		if (m_sInterfaceSettingName != string.Empty)
			InitializeInterfaceSettings();
		
		// Call the interface method, to allow setup and customization of newly created InfoDisplayExtended class
		DisplayStartDraw(owner);

		// Call the interface method, to allow init of custom 'EntityChanged' code
		DisplayControlledEntityChanged(null, m_PlayerController.GetControlledEntity());
		
		// Call the interface method, with 'init = true' to flag this is not a standard state change, but initialization
		DisplayConsciousnessChanged(!m_bIsUnconscious, true);
		
		// Init visibility of newly created InfoDisplay
		UpdateVisibility();
	}

	//------------------------------------------------------------------------------------------------
	private void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnControlledEntityChanged] %2 -> %3", this, from, to));
		#endif
		
		if (from)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(from);
			if (character && m_CharacterController)
				m_CharacterController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);
		}
		
		// Update camera handler + init 1st/3rd person monitoring, if visibility is changing between 1st/3rd person cameras
		if (m_CameraHandler)
			m_CameraHandler.GetThirdPersonSwitchInvoker().Remove(UpdateVisibility);
		
		if (to)
			m_CameraHandler = SCR_CharacterCameraHandlerComponent.Cast(to.FindComponent(SCR_CharacterCameraHandlerComponent));
		else
			m_CameraHandler = null;
		
		if (m_CameraHandler && !m_bShowInAllCameras)
			m_CameraHandler.GetThirdPersonSwitchInvoker().Insert(UpdateVisibility);
		
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnADSChanged", this, OnADSSwitched);	
		
		IEntity eventHandlerOwner;
		if (m_bAttachedToPlayerController)
			eventHandlerOwner = to;
		else
			eventHandlerOwner = m_OwnerEntity;
		
		if (eventHandlerOwner)
			m_EventHandlerManager = EventHandlerManagerComponent.Cast(eventHandlerOwner.FindComponent(EventHandlerManagerComponent));
		else
			m_EventHandlerManager = null;
		
		if (m_EventHandlerManager)
			m_EventHandlerManager.RegisterScriptHandler("OnADSChanged", this, OnADSSwitched);	
		
		m_bIsUnconscious = false;
		
		// Update character controller
		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (character)
		{
			m_CharacterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (m_CharacterController)
			{
				m_CharacterController.m_OnLifeStateChanged.Insert(OnLifeStateChanged);
				m_bIsUnconscious = m_CharacterController.GetLifeState() == ECharacterLifeState.INCAPACITATED;
			}
		}
		
		// Init the state flags
		m_bInADS = false;
		m_bInThirdPerson = m_CameraHandler && m_CameraHandler.Is3rdPersonView();
		
		// Stop if InfoDisplay is not created yet - happens on the 1st 'init' run only
		if (!m_wRoot)
			return;
			
		// Call the interface method
		DisplayControlledEntityChanged(from, to);
		
		// Call the interface method, with 'init = true' to flag this is not a standard state change, but initialization when entity changed
		DisplayConsciousnessChanged(!m_bIsUnconscious, true);
		
		// Update visibility after entity changed
		UpdateVisibility();		
	}	
	
	//------------------------------------------------------------------------------------------------
	private void OnADSSwitched(BaseWeaponComponent weapon, bool inADS)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnADSSwitched] inADS: %2", this, inADS));
		#endif
		
		m_bInADS = inADS;
		
		UpdateVisibility();
	}

	//------------------------------------------------------------------------------------------------
	private void OnLifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnLifeStateChanged] lifeState: %2", this, SCR_Enum.GetEnumName(ECharacterLifeState, newLifeState)));
		#endif
		
		m_bIsUnconscious = newLifeState == ECharacterLifeState.INCAPACITATED;
		
		DisplayConsciousnessChanged(!m_bIsUnconscious);
		
		UpdateVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnPauseMenuOpen()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnPauseMenuOpen]", this));
		#endif
		
		m_bInPauseMenu = true;
		
		UpdateVisibility();
	}	

	//------------------------------------------------------------------------------------------------
	private void OnPauseMenuClose()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnPauseMenuClose]", this));
		#endif
		
		m_bInPauseMenu = false;
		
		UpdateVisibility();
	}	

	//------------------------------------------------------------------------------------------------
	private void OnEditorInit(SCR_EditorManagerEntity editorManager)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnEditorInit] editorManager: %2", this, editorManager));
		#endif
		
		if (!editorManager)
			return;
		
		m_EditorManager = editorManager;
		m_bInEditor = editorManager.IsOpened();
		
		// Init Editor handling
		if ((m_eShow & EShowGUI.IN_EDITOR) == 0)
		{
			m_EditorManager.GetOnOpened().Insert(OnEditorOpen);
			
			m_EditorManager.GetOnClosed().Insert(OnEditorClose);
		}			
	}	
	
	//------------------------------------------------------------------------------------------------
	private void OnEditorOpen()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnEditorOpen]", this));
		#endif
		
		m_bInEditor = true;
		
		UpdateVisibility();
	}	

	//------------------------------------------------------------------------------------------------
	private void OnEditorClose()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnEditorClose]", this));
		#endif
		
		m_bInEditor = false;
		
		UpdateVisibility();
	}	

	//------------------------------------------------------------------------------------------------
	private void UpdateVisibility()
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [UpdateVisibility] m_bInEditor: %2 | m_bInPauseMenu: %3 | m_bIsUnconscious: %4", this, m_bInEditor, m_bInPauseMenu, m_bIsUnconscious));
		#endif

		bool menuPassed = !m_bInPauseMenu || (m_bInPauseMenu && m_eShow & EShowGUI.IN_PAUSE_MENU);
		bool editorPassed = !m_bInEditor || (m_bInEditor && m_eShow & EShowGUI.IN_EDITOR);
		bool statePassed = !m_bIsUnconscious || (m_bIsUnconscious && m_eShow & EShowGUI.WHILE_UNCONSCIOUS);
		bool cameraPassed;
		
		if (m_CameraHandler)		
		{
			m_bInThirdPerson = !m_bInADS && m_CameraHandler.Is3rdPersonView();
			
			cameraPassed = (m_bInADS && m_eShow & EShowGUI.IN_ADS) || (m_bInThirdPerson && m_eShow & EShowGUI.IN_3RD_PERSON) || (!m_bInThirdPerson && !m_bInADS && m_eShow & EShowGUI.IN_1ST_PERSON);
		}
		else
		{
			cameraPassed = m_eShow & EShowGUI.WITHOUT_ENTITY;
		}		
		
		bool updateNeeded = (cameraPassed && statePassed && menuPassed && editorPassed && m_bIsEnabledInSettings) != m_bCanShow;
		
		m_bCanShow = cameraPassed && statePassed && menuPassed && editorPassed && m_bIsEnabledInSettings;
		
		if (!updateNeeded)
			return;
		
		Show(m_bShown);
		
		if (m_bCanShow)
			DisplayOnResumed();
		else
			DisplayOnSuspended();
	}

	//------------------------------------------------------------------------------------------------
	override void Show(bool show, float speed = UIConstants.FADE_RATE_INSTANT, EAnimationCurve curve = EAnimationCurve.LINEAR)
	{
		// Make hiding GUI cuz failing show-conditions always instant as we want to prevent visual artifacts
		if (!m_bCanShow)
			speed = UIConstants.FADE_RATE_INSTANT;

		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [Show] m_bShown && m_bCanShow: %2 | m_bShown: %3 | m_bCanShow: %4", this, show && m_bCanShow, show, m_bCanShow));
		#endif		
				
		super.Show(show && m_bCanShow, speed, curve);
		
		m_bShown = show;			// Re-store the 'shown' flag, that get overridden inside the Show() method, so the info is not lost
	}
	
	//------------------------------------------------------------------------------------------------
	private override event void OnStopDraw(IEntity owner)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnStopDraw] owner: %2", this, owner));
		#endif
		
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif
	
		if (!m_bIsEnabled)
			return;
		
		if (m_PlayerController)
			m_PlayerController.m_OnControlledEntityChanged.Remove(OnControlledEntityChanged);	
			
		if (m_CharacterController)
			m_CharacterController.m_OnLifeStateChanged.Remove(OnLifeStateChanged);

		PauseMenuUI.m_OnPauseMenuOpened.Remove(OnPauseMenuOpen);
		PauseMenuUI.m_OnPauseMenuClosed.Remove(OnPauseMenuClose);

		if(m_EditorManager)
		{
			m_EditorManager.GetOnOpened().Remove(OnEditorOpen);
			m_EditorManager.GetOnClosed().Remove(OnEditorClose);		
		}
		
		if(m_CameraHandler)
			m_CameraHandler.GetThirdPersonSwitchInvoker().Remove(UpdateVisibility);	
		
		if (m_EventHandlerManager)
			m_EventHandlerManager.RemoveScriptHandler("OnADSChanged", this, OnADSSwitched);
		
		super.OnStopDraw(owner);
		
		DisplayStopDraw(owner);		
		
		GetGame().OnUserSettingsChangedInvoker().Remove(OnSettingsChanged);
	}	
	
	//------------------------------------------------------------------------------------------------
	private override event void OnInit(IEntity owner)
	{
		#ifdef DEBUG_INFO_DISPLAY_EXT
		_printClass(string.Format("%1 [OnInit] owner: %2", this, owner));
		#endif
		
		#ifdef DISABLE_HUD
		m_bIsEnabled = false;	
		#endif

		if (!m_bIsEnabled)
			return;

		m_bAttachedToPlayerController = PlayerController.Cast(owner);
		
		super.OnInit(owner);
		
		DisplayInit(owner);
	}
	
	//------------------------------------------------------------------------------------------------
	private override event void UpdateValues(IEntity owner, float timeSlice)
	{
		if (!m_bIsEnabled || !m_bCanShow)
			return;
		
		super.UpdateValues(owner, timeSlice);		
		
		DisplayUpdate(owner, timeSlice);
	}	
}
