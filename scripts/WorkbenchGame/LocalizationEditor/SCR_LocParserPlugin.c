#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Localise Selected Files", shortcut: "Ctrl+Alt+L", wbModules: { "ResourceManager" }, awesomeFontCode: 0xF1AB)]
class SCR_LocParserPlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "{D3EAEB413B6F0AA3}Configs/Workbench/LocParser/Default.conf")]
	protected ResourceName m_sConfigPath;

	[Attribute(defvalue: "", params: "st")]
	protected ResourceName m_sStringTableOverride;

	[Attribute(defvalue: "")]
	protected string m_sPrefixOverride;

	protected bool m_bLogOnly;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		string header = "Localise strings in selected files." +
			"\nNot all strings will be affected, e.g., texture paths will be omitted." +
			"\nOpen the config to review processing rules." +
			"\n\nPress [Log] to find and log all strings which can be localized." +
			"\n\nPress [Localise] to add the strings to the string table." +
			"\nWARNING: THIS ACTION WILL MODIFY FILES!";

		if (Workbench.ScriptDialog("Localise Selected", header, this))
			LocParserManager.Run(m_sConfigPath, m_bLogOnly, m_sStringTableOverride, m_sPrefixOverride);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Log", focused: true)]
	protected void ButtonLog()
	{
		m_bLogOnly = true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Localise")]
	protected void ButtonLocalise()
	{
		m_bLogOnly = false;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}
#endif // WORKBENCH
