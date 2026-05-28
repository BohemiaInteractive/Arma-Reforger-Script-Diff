[BaseContainerProps(configRoot: true)]
class SCR_TasksTabConfig
{
	[Attribute(SCR_ETaskTabType.AVAILABLE.ToString(), UIWidgets.ComboBox, "Default tab", enumType: SCR_ETaskTabType)]
	protected SCR_ETaskTabType m_eDefaultTab;

	[Attribute("", UIWidgets.ComboBox, enumType: SCR_ETaskTabType)]
	protected ref array<SCR_ETaskTabType> m_aTaskTabs;

	//------------------------------------------------------------------------------------------------
	SCR_ETaskTabType GetDefaultTab()
	{
		return m_eDefaultTab;
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_ETaskTabType> GetTaskTabs()
	{
		return m_aTaskTabs;
	}
}
