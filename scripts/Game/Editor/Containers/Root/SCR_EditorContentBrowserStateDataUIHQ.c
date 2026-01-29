//! Editor Content browser State data that includes UI data for tabs, with specific condition for HQ
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_BrowserStateUIInfo")]
class SCR_EditorContentBrowserSaveStateDataUIHQ : SCR_EditorContentBrowserSaveStateDataUI
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		// The construction of command posts in allowed in every game mode
		return true;
	}
}
