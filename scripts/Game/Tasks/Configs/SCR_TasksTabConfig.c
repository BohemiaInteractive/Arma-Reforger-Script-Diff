[BaseContainerProps(configRoot: true)]
class SCR_TasksTabConfig
{
	[Attribute("", UIWidgets.ComboBox, enumType: SCR_ETaskTabType)]
	protected ref array<SCR_ETaskTabType> m_aTaskTabs;

	//------------------------------------------------------------------------------------------------
	array<SCR_ETaskTabType> GetTaskTabs()
	{
		return m_aTaskTabs;
	}
}
