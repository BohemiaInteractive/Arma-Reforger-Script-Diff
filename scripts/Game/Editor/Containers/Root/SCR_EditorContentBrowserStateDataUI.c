//! Editor Content browser State data that includes UI data for tabs
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_BrowserStateUIInfo")]
class SCR_EditorContentBrowserSaveStateDataUI : SCR_EditorContentBrowserSaveStateData
{
	[Attribute(desc: "UI info for browser state")]
	protected ref SCR_UIInfo m_BrowserStateUIInfo;
		
	/*!
	To be overridden in inherited classes. Adds a custom condition that allows to show the tab only with given conditions.
	*/
	bool CanBeShown()
	{
		return true;
	}
	
	/*!
	Get UI Info
	\return Ui Info
	*/
	SCR_UIInfo GetUIInfo()
	{
		return m_BrowserStateUIInfo;
	}
	
	/*!
	Get tab info for displaying in editor content browser
	\param[out] name Name to display
	\param[out] filterAmount amount of filters active
	\param[out] icon icon to display (not used)
	\param activeLabels List of all active labels
	\return Saved label count
	*/
	void GetTabInfo(out string name, out string filterAmount, out ResourceName icon, array<EEditableEntityLabel> activeLabels = null)
	{
		name = m_BrowserStateUIInfo.GetName();
		icon = m_BrowserStateUIInfo.GetIconPath();
		filterAmount = "0";
		
		array<EEditableEntityLabel> activeLabelsToCheck = new array<EEditableEntityLabel>();
		
		if (activeLabels)
			activeLabelsToCheck.Copy(activeLabels);
		else if (!m_iSavedLabels.IsEmpty())
			activeLabelsToCheck.Copy(m_iSavedLabels);
		
		if (activeLabelsToCheck.IsEmpty())
			return;
		
		filterAmount = activeLabelsToCheck.Count().ToString();
	}
};