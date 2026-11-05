#ifdef WORKBENCH


enum EBTBlenderPathStatus
{
	OK,
	NOT_CONFIGURED,	//!< No path has been set in the Blender Settings
	NOT_FOUND,		//!< A path is set but no file exists there
}

[WorkbenchPluginAttribute(name: "Blender Settings", wbModules: { "ResourceManager" }, awesomeFontCode: 0xf013)]
class EBTConfigPlugin : WorkbenchPlugin
{
	[Attribute("", uiwidget: UIWidgets.FileNamePicker, desc: "Absolute path to your blender.exe", params: "exe FileNameFormat=absolute", category: "Blender Executable")]
	string m_sPath = "";
	
	protected static EBTConfigPlugin s_Instance;
	
	//------------------------------------------------------------------------------------------------
	void EBTConfigPlugin()
	{
		if(!s_Instance)
			s_Instance = this;
	}
	
	//------------------------------------------------------------------------------------------------
	static EBTBlenderPathStatus GetBlenderPath(out string path)
	{
		path = string.Empty;

		if (!s_Instance || s_Instance.m_sPath.IsEmpty())
			return EBTBlenderPathStatus.NOT_CONFIGURED;

		if (!FileIO.FileExists(s_Instance.m_sPath))
			return EBTBlenderPathStatus.NOT_FOUND;

		path = s_Instance.m_sPath;
		return EBTBlenderPathStatus.OK;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	Resolves the Blender executable and reports in an error dialog why it cannot be used.
	Use this from user triggered actions; use GetBlenderPath() where no dialog may be shown.
	*/
	static bool GetBlenderPathOrWarn(out string path)
	{
		switch (GetBlenderPath(path))
		{
			case EBTBlenderPathStatus.OK:
				return true;

			case EBTBlenderPathStatus.NOT_CONFIGURED:
				Workbench.Dialog("Error", "Blender Executable has not been configured.\nSet it up in Plugins -> Settings -> Blender Settings");
				return false;

			case EBTBlenderPathStatus.NOT_FOUND:
				Workbench.Dialog("Error", "Blender executable could not be found.\nPlease configure it in Plugins -> Settings -> Blender Settings.");
				return false;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	static bool HasBlenderRegistered()
	{
		string path;
		return GetBlenderPath(path) != EBTBlenderPathStatus.NOT_CONFIGURED;
	}

	//------------------------------------------------------------------------------------------------
	static bool GetDefaultBlenderPath(out string path)
	{
		return GetBlenderPath(path) == EBTBlenderPathStatus.OK;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK")]
	protected void ButtonOK() {}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Blender Settings", "", this);
	}

}

#endif