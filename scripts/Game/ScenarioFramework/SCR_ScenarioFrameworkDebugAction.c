[BaseContainerProps(), BaseContainerCustomTitleField("m_sDebugActionName")]
class SCR_ScenarioFrameworkDebugAction
{
	[Attribute(defvalue: "Default value", desc: "Debug Action Name")]
	string m_sDebugActionName;
	
	[Attribute(desc: "List of Actions to perform for debug.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aDebugActions;
}