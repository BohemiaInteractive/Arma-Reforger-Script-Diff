[BaseContainerProps(configRoot: true)]
class SCR_LoadingScreenHints : Managed
{
	[Attribute("", UIWidgets.ResourceAssignArray, "Loading screen hints", "conf")]
	protected ref array<string> m_aHints;
};