//------------------------------------------------------------------------------------------------
/*!
Scripted base for selection menu used in HUD
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenu
{
	protected const int OPEN_DELAY = 200;

	// Attributes
	[Attribute()]
	protected ref SCR_SelectionMenuInputs m_Inputs;

	[Attribute()]
	protected string m_sOpenSound;

	[Attribute()]
	protected string m_sCloseSound;

	[Attribute()]
	protected string m_sSelectionSound;

	[Attribute()]
	protected string m_sPerformSound;

	[Attribute()]
	protected string m_sEnterCategorySound;

	[Attribute()]
	protected string m_sLeaveCategorySound;

	// Variables
	protected SCR_SelectionMenuDisplay m_Display;
	
	protected ref array<ref SCR_SelectionMenuEntry> m_aRootEntries = {}; // 1st layer of entries
	protected ref array<ref SCR_SelectionMenuEntry> m_aEntries = {}; // Current layer entries
	protected SCR_SelectionMenuEntry m_SelectedEntry;
	protected int m_iSelectedEntryId;

	protected ref SCR_SelectionMenuControllerInputs m_ControllerInputs;
	protected bool m_bOpened;
	protected bool m_bEntryPerformed;
	protected bool m_bUsingAlternativeToggle;
	protected bool m_bClosingMenu;
	protected bool m_bOpenedForTime;

	// Categories and multi layering
	protected ref array<SCR_SelectionMenuCategoryEntry> m_aSelectedCategories = {};

	// Events
	protected ref ScriptInvoker<SCR_SelectionMenu> m_OnBeforeOpen;
	protected ref ScriptInvoker<SCR_SelectionMenu> m_OnOpen;
	protected ref ScriptInvoker<SCR_SelectionMenu> m_OnClose;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry, int> m_OnSelect;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry> m_OnPerform;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuCategoryEntry, int> m_OnOpenCategory;

	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry> m_OnAddEntry;
	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuEntry> m_OnRemoveEntry;
	protected ref ScriptInvoker<SCR_SelectionMenu, array<ref SCR_SelectionMenuEntry>> m_OnUpdateEntries;

	protected ref ScriptInvoker<SCR_SelectionMenu, SCR_SelectionMenuControllerInputs> m_OnControllerChanged;
	protected ref ScriptInvoker<SCR_SelectionMenu, SelectionMenuFailReason> m_OnOpenFailed;

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnBeforeOpen()
	{
		if (m_OnBeforeOpen)
			m_OnBeforeOpen.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnBeforeOpen()
	{
		if (!m_OnBeforeOpen)
			m_OnBeforeOpen = new ScriptInvoker();

		return m_OnBeforeOpen;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeOnOpen()
	{
		if (m_OnOpen)
			m_OnOpen.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnOpen()
	{
		if (!m_OnOpen)
			m_OnOpen = new ScriptInvoker();

		return m_OnOpen;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnClose()
	{
		if (m_OnClose)
			m_OnClose.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClose()
	{
		if (!m_OnClose)
			m_OnClose = new ScriptInvoker();

		return m_OnClose;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnSelect(SCR_SelectionMenuEntry entry, int id)
	{
		if (m_OnSelect)
			m_OnSelect.Invoke(this, entry, id);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnSelect()
	{
		if (!m_OnSelect)
			m_OnSelect = new ScriptInvoker();

		return m_OnSelect;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnOpenFailed()
	{
		if (!m_OnOpenFailed)
			m_OnOpenFailed = new ScriptInvoker();

		return m_OnOpenFailed;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnPerform(SCR_SelectionMenuEntry entry)
	{
		if (m_OnPerform)
			m_OnPerform.Invoke(this, entry);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPerform()
	{
		if (!m_OnPerform)
			m_OnPerform = new ScriptInvoker();

		return m_OnPerform;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnOpenCategory(SCR_SelectionMenuCategoryEntry entry, int level)
	{
		if (m_OnOpenCategory)
			m_OnOpenCategory.Invoke(this, entry, level);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnOpenCategory()
	{
		if (!m_OnOpenCategory)
			m_OnOpenCategory = new ScriptInvoker();

		return m_OnOpenCategory;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnAddEntry(SCR_SelectionMenuEntry entry)
	{
		if (m_OnAddEntry)
			m_OnAddEntry.Invoke(this, entry);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnAddEntry()
	{
		if (!m_OnAddEntry)
			m_OnAddEntry = new ScriptInvoker();

		return m_OnAddEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnRemoveEntry(SCR_SelectionMenuEntry entry)
	{
		if (m_OnRemoveEntry)
			m_OnRemoveEntry.Invoke(this, entry);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnRemoveEntry()
	{
		if (!m_OnRemoveEntry)
			m_OnRemoveEntry = new ScriptInvoker();

		return m_OnRemoveEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnUpdateEntries(array<ref SCR_SelectionMenuEntry> entries)
	{
		if (m_OnUpdateEntries)
			m_OnUpdateEntries.Invoke(this, entries);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnUpdateEntries()
	{
		if (!m_OnUpdateEntries)
			m_OnUpdateEntries = new ScriptInvoker();

		return m_OnUpdateEntries;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnControllerChanged(SCR_SelectionMenuControllerInputs inputs)
	{
		if (m_OnControllerChanged)
			m_OnControllerChanged.Invoke(this, inputs);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnControllerChanged()
	{
		if (!m_OnControllerChanged)
			m_OnControllerChanged = new ScriptInvoker();

		return m_OnControllerChanged;
	}

	//------------------------------------------------------------------------------------------------
	// Custom methods
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void Open()
	{
		InvokeOnBeforeOpen();
				
		// Prevent opening emtpy menu
		if (m_ControllerInputs.m_bPreventEmptyMenuOpen && m_aEntries.IsEmpty())
		{
			if (m_OnOpenFailed)
				m_OnOpenFailed.Invoke(this, SCR_ESelectionMenuFailReason.MENU_EMPTY);
			return;
		}
		
		if (m_bOpened)
		{
			if (m_OnOpenFailed)
				m_OnOpenFailed.Invoke(this, SCR_ESelectionMenuFailReason.MENU_ALREADY_OPEN);
			return;
		}
			
		m_bOpened = true;
		m_bEntryPerformed = false;
		m_bClosingMenu = false;
		m_bUsingAlternativeToggle = false;

		// Call later to set opened for time to allow closing after reaching the delay
		m_bOpenedForTime = false;
		
		if (!m_ControllerInputs.m_sToggleActionAlternative.IsEmpty())
			GetGame().GetCallqueue().CallLater(AllowClosing, OPEN_DELAY);
		else
			AllowClosing(); // Allow closing immidiatelly when toggle is not set

		// Start on root
		if (m_ControllerInputs.m_bOpenInRoot)
			OpenInRoot();

		OnOpen();

		InvokeOnOpen();
		PlaySound(m_sOpenSound);
		
		// Call update in next frame to be sure item preview is prepared 
		GetGame().GetCallqueue().CallLater(UpdateEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! Allow closing after some time in order to have alternative toggle action with double press
	protected void AllowClosing()
	{
		// Set true to state that time for allow closing has already passed
		m_bOpenedForTime = true;

		// Close menu if closing was requested before it was possible
		if (m_bClosingMenu && !m_bUsingAlternativeToggle)
		{
			Close();
			return;
		}

		m_bClosingMenu = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Empty method called on open ready for override
	protected void OnOpen(){}

	//------------------------------------------------------------------------------------------------
	//! Callback when close is requested
	void Close()
	{
		m_bClosingMenu = true;

		if (!m_bOpenedForTime)
			return;

		// Perform
		if (m_ControllerInputs && m_ControllerInputs.m_bPerformOnClose && m_SelectedEntry && !m_bEntryPerformed)
			PerformEntry(m_SelectedEntry);

		m_bOpened = false;
		m_bUsingAlternativeToggle = false;
		OnClose();

		InvokeOnClose();

		if (!m_bEntryPerformed || !m_SelectedEntry)
			PlaySound(m_sCloseSound);
	}

	//------------------------------------------------------------------------------------------------
	//! Empty method called on close ready for override
	protected void OnClose(){}

	//------------------------------------------------------------------------------------------------
	void Update(float timeSlice)
	{
		// Context
		if (m_bOpened)
			GetGame().GetInputManager().ActivateContext(m_Inputs.m_sContext);

		if (m_ControllerInputs)
			GetGame().GetInputManager().ActivateContext(m_ControllerInputs.m_sControllerContext);

		OnUpdate(timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	//! Empty method called on update ready for override
	protected void OnUpdate(float timeSlice){}

	//------------------------------------------------------------------------------------------------
	void Init()
	{
		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string sound)
	{
		if (!sound.IsEmpty())
			SCR_UISoundEntity.SoundEvent(sound);
	}

	//------------------------------------------------------------------------------------------------
	// Entries handling
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Generic method for custom entry selection based on used interface
	protected void SelectEntry() {}

	//------------------------------------------------------------------------------------------------
	void PerformEntry(notnull SCR_SelectionMenuEntry entry)
	{
		if (!entry.IsEnabled())
			return;

		// Is category entry
		SCR_SelectionMenuCategoryEntry category = SCR_SelectionMenuCategoryEntry.Cast(entry);

		entry.Perform();

		// Generic entry
		if (!category)
		{
			m_bEntryPerformed = true;
			
			if (m_ControllerInputs.m_bCloseOnPerform)
				Close();
		}

		// Category
		if (category)
		{
			if (!m_bClosingMenu)
				OpenCategoryEntry(category);
			// Perform default action on closing

			return;
		}

		PlaySound(m_sPerformSound);

		InvokeOnPerform(entry);
		
		if (!category && !m_bClosingMenu)
			m_bEntryPerformed = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Specific peform action for category to change menu content
	protected void OpenCategoryEntry(notnull SCR_SelectionMenuCategoryEntry category)
	{
		//AddEntries(category.GetEntries(), true);

		m_aSelectedCategories.Insert(category);

		// Repopulate entries
		m_aEntries.Clear();

		for (int i = 0, count = category.GetEntries().Count(); i < count; i++)
		{
			m_aEntries.Insert(category.GetEntries()[i]);
		}

		// Update
		InvokeOnOpenCategory(category, m_aSelectedCategories.Count());
		InvokeOnUpdateEntries(category.GetEntries());

		PlaySound(m_sEnterCategorySound);
	}

	//------------------------------------------------------------------------------------------------
	protected void LeaveCategory()
	{
		SCR_SelectionMenuCategoryEntry category = CurrentCategory();
		if (!category)
			return;

		m_aSelectedCategories.RemoveItem(category);

		category = CurrentCategory();
		m_aEntries.Clear();

		
		// Repopulate entries with previous entry
		if (category)
		{
			array<ref SCR_SelectionMenuEntry> catEntries = category.GetEntries();
			
			// Previous category
			for (int i = 0, count = catEntries.Count(); i < count; i++)
			{
				m_aEntries.Insert(catEntries[i]);
			}
		}
		else
		{
			// Root
			for (int i = 0, count = m_aRootEntries.Count(); i < count; i++)
			{
				m_aEntries.Insert(m_aRootEntries[i]);
			}
		}

		InvokeOnOpenCategory(category, m_aSelectedCategories.Count());
		InvokeOnUpdateEntries(m_aEntries);

		PlaySound(m_sLeaveCategorySound);
	}

	//------------------------------------------------------------------------------------------------
	//! Clear open categories and use entries from root
	protected void OpenInRoot()
	{
		m_aSelectedCategories.Clear();
		m_aEntries.Clear();

		for (int i = 0, count = m_aRootEntries.Count(); i < count; i++)
		{
			m_aEntries.Insert(m_aRootEntries[i]);
		}

		InvokeOnOpenCategory(null, 0);
		InvokeOnUpdateEntries(m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! Add new entry into menu root
	//! Entries update is notified only if menu is opened in root level
	void AddEntry(SCR_SelectionMenuEntry entry = null)
	{
		if (!entry)
		{
			SCR_SelectionMenuEntry newEntry = new SCR_SelectionMenuEntry();
			m_aRootEntries.Insert(newEntry);
			return;
		}

		m_aRootEntries.Insert(entry);

		// Update entries
		if (!CurrentCategory())
		{
			m_aEntries.Insert(entry);
			InvokeOnAddEntry(entry);
			InvokeOnUpdateEntries(m_aEntries);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Add empty or custom category entry
	void AddCategoryEntry(SCR_SelectionMenuCategoryEntry category = null)
	{
		SCR_SelectionMenuEntry entry = category;

		if (!entry)
			entry = new SCR_SelectionMenuCategoryEntry();

		AddEntry(entry);
	}

	//------------------------------------------------------------------------------------------------
	//! Add multiple entries in array
	//! Replace true will clear menu and use given entries
	void AddEntries(notnull array<ref SCR_SelectionMenuEntry> entries, bool replace = false)
	{
		// Clear
		if (replace)
		{
			m_aRootEntries.Clear();

			if (!CurrentCategory())
				m_aEntries.Clear();
		}

		// Add
		for (int i = 0, count = entries.Count(); i < count; i++)
		{
			AddEntry(entries[i]);
		}

		if (!CurrentCategory())
			InvokeOnUpdateEntries(m_aEntries);

		UpdateEntries();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoke data update for all entries
	void UpdateEntries()
	{
		InvokeOnUpdateEntries(m_aEntries);
		
		foreach (SCR_SelectionMenuEntry entry : m_aEntries)
		{
			if (entry)
				entry.Update();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Invoke data update for selected entries
	void UpdateSelectedEntries(notnull array<ref SCR_SelectionMenuEntry> entries)
	{
		InvokeOnUpdateEntries(entries);
		
		foreach (SCR_SelectionMenuEntry entry : entries)
		{
			if (entry)
				entry.Update();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Remove selected entry and invoke data update
	void RemoveEntry(notnull SCR_SelectionMenuEntry entry)
	{
		m_aRootEntries.RemoveItem(entry);

		// Update entries
		if (!CurrentCategory())
		{
			m_aEntries.RemoveItem(entry);
			InvokeOnUpdateEntries(m_aEntries);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Clear all entries and invoke data update
	void ClearEntries()
	{
		m_aRootEntries.Clear();

		// Update entries
		if (!CurrentCategory())
		{
			m_aEntries.Clear();
			InvokeOnUpdateEntries(m_aEntries);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return current opened category entry
	//! Null means menu is in root
	SCR_SelectionMenuCategoryEntry CurrentCategory()
	{
		if (m_aSelectedCategories.IsEmpty())
			return null;

		return m_aSelectedCategories[m_aSelectedCategories.Count() - 1];
	}

	//------------------------------------------------------------------------------------------------
	//! Find and setup display used for menu 
	void SetMenuDisplay(SCR_SelectionMenuDisplay display = null)
	{
		m_Display = display;
		
		if (display)
			display.SetupMenu(this);
	}
	
	//------------------------------------------------------------------------------------------------
	// Inputs
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Add all action listeners for basic menu control
	protected void AddActionListeners()
	{
		if (m_ControllerInputs.m_bCloseOnReleaseOpen)
			GetGame().GetInputManager().AddActionListener(m_ControllerInputs.m_sOpenAction, EActionTrigger.UP, OnOpenInputRelease);

		if (!m_ControllerInputs.m_sToggleActionAlternative.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_ControllerInputs.m_sToggleActionAlternative, EActionTrigger.DOWN, OnAlternativeToggleInput);

		if (!m_Inputs.m_sPerformAction.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_Inputs.m_sPerformAction, EActionTrigger.DOWN, OnPerformInput);

		if (!m_Inputs.m_sBackAction.IsEmpty())
			GetGame().GetInputManager().AddActionListener(m_Inputs.m_sBackAction, EActionTrigger.DOWN, OnBackInput);
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all action listeners for basic menu control
	protected void RemoveActionListeners()
	{
		if (!m_ControllerInputs || !m_Inputs)
			return;
		
		GetGame().GetInputManager().RemoveActionListener(m_ControllerInputs.m_sOpenAction, EActionTrigger.UP, OnOpenInputRelease);
		GetGame().GetInputManager().RemoveActionListener(m_ControllerInputs.m_sToggleActionAlternative, EActionTrigger.DOWN, OnAlternativeToggleInput);
		GetGame().GetInputManager().RemoveActionListener(m_Inputs.m_sPerformAction, EActionTrigger.DOWN, OnPerformInput);
		GetGame().GetInputManager().RemoveActionListener(m_Inputs.m_sBackAction, EActionTrigger.DOWN, OnBackInput);
	}

	//------------------------------------------------------------------------------------------------
	//! On open input handle menu closing
	protected void OnOpenInputRelease(float value, EActionTrigger reason)
	{
		if (m_bUsingAlternativeToggle)
			return;

		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! On select input handle entry selection and moving into layers
	protected void OnPerformInput()
	{
		if (m_SelectedEntry)
			PerformEntry(m_SelectedEntry);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBackInput()
	{
		// Prevent entry performing
		m_SelectedEntry = null;

		// Should close on top level
		if (CurrentCategory())
		{
			LeaveCategory();
		}
		else
		{
			// Close on top level
			Close();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAlternativeToggleInput()
	{
		m_bUsingAlternativeToggle = true;
	}

	//------------------------------------------------------------------------------------------------
	// API
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	bool IsOpened()
	{
		return m_bOpened;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set controller entity and controls
	void SetController(IEntity owner, SCR_SelectionMenuControllerInputs controls)
	{
		if (controls)
			controls.m_Owner = owner;

		if (m_ControllerInputs != controls)
			InvokeOnControllerChanged(controls);
		
		m_ControllerInputs = controls;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuControllerInputs GetControllerInputs()
	{
		return m_ControllerInputs;
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuEntry GetSelectionEntry()
	{
		return m_SelectedEntry;
	}

	//------------------------------------------------------------------------------------------------
	int GetSelectedEntryId()
	{
		return m_iSelectedEntryId;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetEntryPerformed()
	{
		return m_bEntryPerformed;
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref SCR_SelectionMenuEntry> GetEntries()
	{
		array<ref SCR_SelectionMenuEntry> entries = {};

		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}

		return entries;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetEntryCount()
	{
		if (!m_aEntries)
			return 0;

		return m_aEntries.Count();
	}
	
		//------------------------------------------------------------------------------------------------
	bool HasDisplay()
	{
		return m_Display != null;
	}

	//------------------------------------------------------------------------------------------------
	// Debug
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void DebugPrint(string method, string msg)
	{
		Print(string.Format("[SCR_SelectionMenu] - %1() - '%2'", method, msg), LogLevel.DEBUG);
	}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable selection menu inputs for separating menu controls actions
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenuInputs
{
	[Attribute("", desc: "Input context used for menu controls")]
	string m_sContext;

	[Attribute("", desc: "Action used for closing menu or moving back between the layers")]
	string m_sBackAction;

	[Attribute("", desc: "Input action used for performing selected entry")]
	string m_sPerformAction;

	//------------------------------------------------------------------------------------------------
	//! Initialization method for setting up inputs at runtime
	void Init() {}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable menu controls for controlling entity
This controls are defined at controlling entity - e.g. Player, GM entity, Map, etc.
E.g. For Quick slots menu and Editor actions menu will be different keys to open
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenuControllerInputs
{
	IEntity m_Owner;

	[Attribute("", desc: "Input context used for extra menu controls from controller entity")]
	string m_sControllerContext;

	[Attribute("", desc: "Action for opening (and closing) radial menu")]
	string m_sOpenAction;

	[Attribute("", desc: "Alternative action for always toggling menu")]
	string m_sToggleActionAlternative;

	[Attribute("1", desc: "If this field is checked - Menu is closed if open input is released. Otherwise separate close input has to be used")]
	bool m_bCloseOnReleaseOpen;

	[Attribute("1", desc: "If this field is checked - Perform entry(call entry action) when closing menu and having selected entry")]
	bool m_bPerformOnClose;

	[Attribute("0", desc: "If this field is checked - Close the menu after performing the entry (selecting enabled entry)")]
	bool m_bCloseOnPerform;

	[Attribute("1", desc: "Checked - on opening menu is always filled with root entries. Otherwise menu will stay in last category.")]
	bool m_bOpenInRoot;
	
	[Attribute("0", desc: "If checked, menu won't open if there are no entries in the menu")]
	bool m_bPreventEmptyMenuOpen;
	
	[Attribute("1", UIWidgets.Slider, desc: "Unchecked will prevent opening of radial menu while character is unconcious.")]
	bool m_bShowWhileUnconcious;

	//------------------------------------------------------------------------------------------------
	void SCR_SelectionMenuOpening(string openAction = "")
	{
		if (!openAction.IsEmpty())
			m_sOpenAction = openAction;
	}

	//------------------------------------------------------------------------------------------------
	//! Return true if given entity is controlling current menu
	bool IsControllingMenu(IEntity controller)
	{
		return controller == m_Owner;
	}
};

//------------------------------------------------------------------------------------------------
/*!
Configurable selection menu entries list
*/
[BaseContainerProps(configRoot: true)]
class SCR_SelectionMenuData
{
	[Attribute()]
	protected ref array<ref SCR_SelectionMenuEntry> m_aEntries;

	//------------------------------------------------------------------------------------------------
	array<ref SCR_SelectionMenuEntry> GetEntries()
	{
		array<ref SCR_SelectionMenuEntry> entries = {};

		for (int i = 0, count = m_aEntries.Count(); i < count; i++)
		{
			entries.Insert(m_aEntries[i]);
		}

		return entries;
	}
};

//------------------------------------------------------------------------------------------------
//! Custom seletion menu preview class for indentification
//! Goal is to specify fov and setup icon size
class SCR_SelectionMenuPreviewAttributes : BaseItemAttributeData
{
	[Attribute("10")]
	float m_fCustomFov;

	[Attribute("-1", desc: "Adjust how final render preview size should be big by current icon size multiplicaiton")]
	float m_fIconSizeXMultiplier;
	
	[Attribute(desc: "If true colorize item shadow in Radial Menu to medical color")]
	bool m_bShowMedicalColor;
};

//! Enum of reason why the menu did not open
enum SCR_ESelectionMenuFailReason
{
	UNKNOWN,
	MENU_EMPTY,
	MENU_ALREADY_OPEN
}
