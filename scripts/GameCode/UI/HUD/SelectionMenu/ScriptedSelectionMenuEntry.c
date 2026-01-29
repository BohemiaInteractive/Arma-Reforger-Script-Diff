class ScriptedSelectionMenuEntry : BaseScriptedSelectionMenuEntry
{
	const string LAYOUT_ENTRY_DEFAULT = "{121C45A1F59DC1AF}UI/layouts/Common/RadialMenu/RadialEntryElement.layout";
	
	// Entry widget
	protected ResourceName m_EntryLayout = LAYOUT_ENTRY_DEFAULT;
	protected ref Widget m_wEntry;
	protected string m_sName;
	
	//! Callback for when this entry is supposed to be performed
	override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		#ifdef ENABLE_DIAG
		if (SCR_BaseSelectionMenu.IsRadialMenuLoggingEnabled())
		{
			Print("Radial menu " + sourceMenu + " performed: " + this + "! User: " + user);
		}
		#endif
	}

	//! Can this entry be shown?
	override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}

	//! Can this entry be performed?
	override bool CanBePerformedScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}
	
	//! If overridden and true is returned, outName is returned when BaseSelectionMenuEntry.GetEntryName is called.
	//! If not overridden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetEntryNameScript(out string outName)
	{
		outName = m_sName;
		return true;
	}
	
	//! If overridden and true is returned, outDescription is returned when BaseSelectionMenuEntry.GetEntryDescription is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetEntryDescriptionScript(out string outDescription)
	{
		return false;
	}
	
	//! If overridden and true is returned, outIconPath is returned when BaseSelectionMenuEntry.GetEntryIconPath is called.
	//! If not overriden or false is returned the default value from UIInfo is taken (or empty string if no UI info exists)
	override bool GetEntryIconPathScript(out string outIconPath)
	{
		return false;
	}
	
	//! Can be overridden to return desired UIInfo in GetUIInfo method
	override UIInfo GetUIInfoScript()
	{
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Base visuals update function for specific entry update 
	void UpdateVisuals() {}
	
	
	
	//! API for entry widget and component 
	
	//------------------------------------------------------------------------------------------------
	ResourceName GetEntryLayout() { return m_EntryLayout; }
	
	//------------------------------------------------------------------------------------------------
	void SetEntryLayout(ResourceName entryLayout) { m_EntryLayout = entryLayout; }
	
	//------------------------------------------------------------------------------------------------
	Widget GetEntryWidget() { return m_wEntry; }
	
	//------------------------------------------------------------------------------------------------
	void SetEntryWidget(Widget root) { m_wEntry = root; }
	
	//------------------------------------------------------------------------------------------------
	string GetName() { return m_sName; }
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name) { m_sName = name; }
};