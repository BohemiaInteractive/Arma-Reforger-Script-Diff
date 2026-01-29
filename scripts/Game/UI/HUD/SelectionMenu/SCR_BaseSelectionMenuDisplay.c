//------------------------------------------------------------------------------------------------
[BaseContainerProps(configRoot: true)]
class SCR_BaseSelectionMenuDisplay
{
	//! Is the menu open, should this display be drawn?
	protected bool m_bIsOpen;
	
	//! Sets all parameters in display to default values
	//! DISCLAIMER: This will most likely override data you've had. Make sure you know what you're doing!
	void SetDefault();
	
	//! Used to pass in the radial menu content
	void SetContent(array<BaseSelectionMenuEntry> allEntries, array<BaseSelectionMenuEntry> disabledEntries);
	
	//! Used to set selected entry. Can be null as for no selection.
	void SetSelection(BaseSelectionMenuEntry selectedEntry, vector selectionInput, float selectionAngle, float minInputMagnitude);
	
	//! Generaly Used for setting selected entry
	void SetSelection(BaseSelectionMenuEntry selectedEntry);
	
	//! Used to set open state
	void SetOpen(IEntity owner, bool open) { m_bIsOpen = open; }
};

//------------------------------------------------------------------------------------------------
class SCR_RadialMenuIcons
{
	static const string RADIALMENU_ICON_EMPTY = "{74DDB9BCF5A40173}UI/Textures/RadialMenu/RadialMenuEmpty.edds";
	static const string RADIALMENU_ICON_DEFAULT_WEAPONSWITCHING = "{74753A21981BB84D}UI/Textures/RadialMenu/RadialMenuDefaultWeapon.edds";
	static const string RADIALMENU_ICON_DEFAULT_AIMESSAGE = "{164112D4DD73C39F}UI/Textures/RadialMenu/RadialMenuDefaultAIMessage.edds";
	
	//------------------------------------------------------------------------------------------------
	//! Returns provided icon path if its valid, or default (empty) icon otherwise
	protected static string GetSafeIconPath(string iconPath)
	{
		if (iconPath == string.Empty)
			return RADIALMENU_ICON_EMPTY;
		
		return iconPath;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Provided instance is expected to be of type BaseSelectionMenuEntry
	//! Returns path to icon that can be used as a fallback one
	static string GetDefaultIconPath(Class instance)
	{
		auto baseEntry = BaseSelectionMenuEntry.Cast(instance);
		if (!baseEntry)
			return RADIALMENU_ICON_EMPTY;
		
		auto aiMessageEntry = AIMessageSelectionMenuEntry.Cast(instance);
		if (aiMessageEntry)
		{
			return GetSafeIconPath(RADIALMENU_ICON_DEFAULT_AIMESSAGE);
		}
	
		return RADIALMENU_ICON_EMPTY;
	}
};