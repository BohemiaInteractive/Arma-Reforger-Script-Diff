//! Editor Content browser State data that includes UI data for tabs, with specific condition for HQ
[BaseContainerProps(), SCR_BaseContainerCustomTitleUIInfo("m_BrowserStateUIInfo")]
class SCR_EditorContentBrowserSaveStateDataUIHQ : SCR_EditorContentBrowserSaveStateDataUI
{
	//------------------------------------------------------------------------------------------------
	override bool CanBeShown(IEntity provider, int playerId)
	{
		if (!provider)
			return false;

		return SCR_CampaignBuildingProviderComponent.CanBeUsedToEstablishBase(provider, playerId);
	}
}
