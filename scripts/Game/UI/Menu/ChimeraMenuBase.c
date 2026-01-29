//------------------------------------------------------------------------------------------------
//! Menu presets
enum ChimeraMenuPreset : ScriptMenuPresetEnum
{
	MainMenu,
	PlayMenu,
	EditorSelectionMenu,
	CareerMenu,
	PauseMenu,
	LoginDialog,
	LoginDialogConsole,
	ServerBrowserMenu,
	ErrorDialog,
	MapMenu,
	WorldEditorIngameMenu,
	EditorMenu,
	EditorModesDialog,
	EditorActionListDialog,
	EditorEntityListDialog,
	EditorPlayerListDialog,
	EditorFactionListDialog,
	EditorPlacingMenuDialog,
	EditorAttributesDialog,
	EditorBrowserDialog,
	EditorSaveDialog,
	EditorLoadDialog,
	FeedbackDialog,
	Inventory20Menu,
	ReportItemDialog,
	ContentBrowser,
	ContentBrowserDetailsMenu,
	ContentBrowserDetailsMenuSave,
	RespawnSuperMenu,
	WelcomeScreenMenu,
	DebriefingScreenMenu,
	SettingsSuperMenu,
	PlayerListMenu,
	FieldManualDialog,
	WidgetLibraryMenu,
	TaskDetail,
	EndgameScreen,
	ScenarioMenu,
	ProfileSuperMenu,
	NewsDialog,
	GalleryDialog,
	ConfigurableDialog,
	LoadingOverlay,
	CreditsMenu,
	CreditsLicensesMenu,
	PrivacyPolicyMenu,
	WelcomeDialog,
	GroupSettingsDialog,
	CareerProfileMenu,
	CampaignBuildingPlacingMenuDialog,
	GroupMenu,
	GroupFlagDialog,
	RoleSelectionDialog,
	GamepadRemovalDialog,
	TutorialFastTravel,
	CreateGroupSettingsDialog,
	CommandPostMapMenu,
	VolunteerForGroupLeaderDialog,
	CampaignMenu
};

//------------------------------------------------------------------------------------------------
//! Constant variables used in various menus.
// #define MPTEST // TODO: remove

//------------------------------------------------------------------------------------------------
class ChimeraMenuBase : MenuBase
{

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// The base scripted menu class should not care about edit boxes
	
	// Editbox type check
	protected const string INPUT_CONTEXT_EDIT = "MenuTextEditContext";
	protected bool m_bTextEditActive = false;
	ref ScriptInvoker m_OnTextEditContextChange = new ScriptInvoker;

//---- REFACTOR NOTE END ----

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Components not ticking is an important architecture decision and should be respected as such or changed at the engine level: this is a hacky workaround. Also, all invokers for other important events are missing
	
	ref ScriptInvoker m_OnUpdate = new ScriptInvoker; // (float tDelta)

//---- REFACTOR NOTE END ----

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Did people get confused and thought it was another menu xD?
	
	protected static ChimeraMenuBase m_ThisMenu;

	//------------------------------------------------------------------------------------------------
	static ChimeraMenuBase CurrentChimeraMenu()
	{
		return m_ThisMenu;
	}
	
//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuInit()
	{
		m_ThisMenu = this;
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);

		// Check edit box context active
		bool ctxActive = GetGame().GetInputManager().IsContextActive(INPUT_CONTEXT_EDIT);
		m_OnTextEditContextChange.Invoke(ctxActive);

		// Invoke OnUpdate
		m_OnUpdate.Invoke(tDelta);
	}
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Wouldn't it make more sense for a scripted MenuManager to provide the generic events now handled by SCR_MenuHelper?
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpened();
		SCR_MenuHelper.OnMenuOpen(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpened()
	{
		super.OnMenuOpened();
		SCR_MenuHelper.OnMenuOpened(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		SCR_MenuHelper.OnMenuClose(this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		SCR_MenuHelper.OnMenuFocusGained(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		SCR_MenuHelper.OnMenuFocusLost(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		SCR_MenuHelper.OnMenuShow(this);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuHide()
	{
		super.OnMenuHide();
		SCR_MenuHelper.OnMenuHide(this);
	}

//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	//! Returns parent menu of a widget
	static ChimeraMenuBase GetOwnerMenu(Widget w)
	{
		auto game = GetGame();

		if (!game)
			return null;

		auto menuManager = game.GetMenuManager();

		if (!menuManager)
			return null;

		MenuBase menuBase = menuManager.GetOwnerMenu(w);

		if (!menuBase)
			return null;

		ChimeraMenuBase m = ChimeraMenuBase.Cast(menuBase);

		if (!m)
			return null;

		return m;
	}
};
