[BaseContainerProps(configRoot: true)]
class SCR_GMMenuConfiguration : Managed
{
	[Attribute("", UIWidgets.ResourceAssignArray, "GM scenarios headers", "conf")]
	protected ref array<ResourceName> m_aGameMasterScenarios;
	
	//------------------------------------------------------------------------------------------------
	array<ResourceName> GetScenarios()
	{
		return m_aGameMasterScenarios;
	}
}