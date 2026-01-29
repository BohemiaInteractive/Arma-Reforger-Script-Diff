[BaseContainerProps(configRoot: true)]
class SCR_CampaignMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.ResourceAssignArray, "CampaignMenu scenarios headers", "conf")]
	ref array<ResourceName> m_aCampaignMenuScenarios;
}