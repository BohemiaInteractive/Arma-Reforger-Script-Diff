#ifdef WORKBENCH
[WorkbenchPluginAttribute(
	name: "Autotest Tool",
	description: "",
	awesomeFontCode: 0xF188,
	wbModules: { "WorldEditor" }
)]
class SCR_AutotestToolPlugin : WorkbenchPlugin
{
	
	[Attribute("", UIWidgets.FileNamePicker, desc: "Game exe path", params: "exe FileNameFormat=absolute")]
	protected string m_sExePath;
	
	override void Configure()
	{
		Workbench.ScriptDialog("Autotest Tool Configuration", "", this);
	}
	
	string GetExecutablePath()
	{
		return m_sExePath;
	}
}