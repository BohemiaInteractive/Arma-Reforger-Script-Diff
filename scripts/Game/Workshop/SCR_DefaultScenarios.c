[BaseContainerProps(configRoot: true)]
class SCR_DefaultScenarios : Managed
{
	[Attribute("", UIWidgets.ResourceAssignArray, "", "conf")]
	ref array<ResourceName> m_aDefaultScenarios;
}