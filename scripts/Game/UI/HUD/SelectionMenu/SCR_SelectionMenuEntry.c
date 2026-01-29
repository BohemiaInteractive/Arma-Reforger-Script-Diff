/*!
Scripted selection menu entry based on UI info for storing data into SCR_SelectionMenu
*/

//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleUIInfo("Name")]
class SCR_SelectionMenuEntry : SCR_UIInfo
{
	[Attribute(desc: "Entry name used for searching and definition")]
	protected string m_sId;

	[Attribute("1")]
	protected bool m_bEnabled;
	
	[Attribute(desc: "Action name use as input shortcut for entry")]
	protected string m_sInputAction;

	[Attribute("", ".layout", desc: "By default menu has predefined layouts. Use only if specific layout for entry is needed")]
	protected ResourceName m_sCustomLayout;

	protected ref SCR_SelectionMenuEntryComponent m_EntryComponent;

	// Invokers
	protected ref ScriptInvoker<SCR_SelectionMenuEntry> m_OnPerform;
	protected ref ScriptInvoker<SCR_SelectionMenuEntry> m_OnPerformFail;
	protected ref ScriptInvoker<SCR_SelectionMenuEntry, ResourceName, string> m_OnIconChange;

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnPerform()
	{
		if (m_OnPerform)
			m_OnPerform.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPerform()
	{
		if (!m_OnPerform)
			m_OnPerform = new ScriptInvoker();

		return m_OnPerform;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnPerformFail()
	{
		if (m_OnPerformFail)
			m_OnPerformFail.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnPerformFail()
	{
		if (!m_OnPerformFail)
			m_OnPerformFail = new ScriptInvoker();

		return m_OnPerformFail;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeOnIconChange(ResourceName icon, string imageSetImage)
	{
		if (m_OnIconChange)
			m_OnIconChange.Invoke(this, icon, imageSetImage);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnIconChange()
	{
		if (!m_OnIconChange)
			m_OnIconChange = new ScriptInvoker();

		return m_OnIconChange;
	}

	//------------------------------------------------------------------------------------------------
	// Custom methods
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! Public method to make entry update itself
	void Update()
	{
	}

	//------------------------------------------------------------------------------------------------
	//! Public method to call entry perform
	//! Don't override! To define entry perform behavior override OnPerform() below
	void Perform()
	{
		// Check can be performed
		if (!m_bEnabled)
		{
			InvokeOnPerformFail();
			return;
		}

		OnPerform();
		InvokeOnPerform();
	}

	//------------------------------------------------------------------------------------------------
	//! Empty methods for deifinition of perform behavior
	protected void OnPerform()
	{
	}

	//------------------------------------------------------------------------------------------------
	void Enable(bool enable)
	{
		m_bEnabled = enable;
		
		if (m_EntryComponent)
			m_EntryComponent.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	// Callbacks
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	protected void OnEntryClick(SCR_SelectionMenuEntryComponent entryComponent)
	{
		Perform();
	}

	//------------------------------------------------------------------------------------------------
	// Get set
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SetId(string id)
	{
		m_sId = id;
	}

	//------------------------------------------------------------------------------------------------
	string GetId()
	{
		if (m_sId.IsEmpty())
			Print("Selection menu entry has enmpty id", LogLevel.WARNING);

		return m_sId;
	}

	//------------------------------------------------------------------------------------------------
	void SetName(string name)
	{
		Name = name;
	}

	//------------------------------------------------------------------------------------------------
	void SetDescription(string description)
	{
		Description = description;
	}

	//------------------------------------------------------------------------------------------------
	//! Set icon and invoke change
	void SetIcon(ResourceName iconPath, string imageSetName = "")
	{
		Icon = iconPath;
		IconSetName = imageSetName;

		InvokeOnIconChange(Icon, IconSetName);
	}

	//------------------------------------------------------------------------------------------------
	void SetIconFromDeafaultImageSet(string imageSetName = "")
	{
		Icon = UIConstants.ICONS_IMAGE_SET;
		IconSetName = imageSetName;

		InvokeOnIconChange(Icon, IconSetName);
	}

	//------------------------------------------------------------------------------------------------
	ResourceName GetCustomLayout()
	{
		return m_sCustomLayout;
	}

	//------------------------------------------------------------------------------------------------
	void SetCustomLayout(ResourceName layout)
	{
		m_sCustomLayout = layout;
	}

	//------------------------------------------------------------------------------------------------
	void SetEntryComponent(SCR_SelectionMenuEntryComponent entryComponent)
	{
		// Clear
		if (m_EntryComponent)
			m_EntryComponent.GetOnClick().Clear();


		m_EntryComponent = entryComponent;

		// Setup invokers
		if (m_EntryComponent)
			m_EntryComponent.GetOnClick().Insert(OnEntryClick);


		// Setup icons - TODO: probably can be moved to seperate component
		SCR_SelectionMenuEntryIconComponent iconEntry = SCR_SelectionMenuEntryIconComponent.Cast(entryComponent);
		if (iconEntry)
			iconEntry.SetImage(Icon, IconSetName);

		entryComponent.SetEntry(this);
	}

	//------------------------------------------------------------------------------------------------
	SCR_SelectionMenuEntryComponent GetEntryComponent()
	{
		return m_EntryComponent;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInputAction(string action)
	{
		m_sInputAction = action;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetInputAction()
	{
		return m_sInputAction;
	}

	//------------------------------------------------------------------------------------------------
	// Constructor
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	void SCR_SelectionMenuEntry()
	{
		m_bEnabled = true;
		
		if (Icon.IsEmpty())
			Icon = UIConstants.ICONS_IMAGE_SET;
	}
};
