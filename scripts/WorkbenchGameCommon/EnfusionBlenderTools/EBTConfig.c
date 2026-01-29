#ifdef WORKBENCH


[WorkbenchPluginAttribute(name: "EBT: Config", wbModules: { "ResourceManager" }, awesomeFontCode: 0xf013)]
class EBTConfigPlugin : WorkbenchPlugin
{
	
	[Attribute("0", UIWidgets.ComboBox, "", "", enums: GetRegisteredBlenderExecutables(), category: "Paths")]
	int m_iDefaultPath;
	
	[Attribute("", UIWidgets.Auto, "Blender Executables", category: "Paths")]
	protected ref array<ref BlenderPathContainer> m_aBlenderPaths;
	
	[Attribute("True", UIWidgets.CheckBox, "Check this option to print output to clipboard.", category : "Development")]
	protected bool m_bCopyToClipboard = false;
	
	protected bool shouldReload = true;
	
	protected static EBTConfigPlugin s_Instance;

	protected static const ref ParamEnum NO_EXECUTABLE_ENUM = new ParamEnum("None", "0");
	
	protected static ref ParamEnumArray s_RegisteredBlenderExecutables = {NO_EXECUTABLE_ENUM};
	
	static ref ParamEnumArray GetRegisteredBlenderExecutables()
	{
		return s_RegisteredBlenderExecutables;
	}
	
	void EBTConfigPlugin()
	{
		if(!s_Instance)
			s_Instance = this;
	}
	
	static bool CopyToClipboard()
	{
		return s_Instance.m_bCopyToClipboard;
	}
	
	static void UpdateRegisteredBlenderExecutables()
	{
		s_RegisteredBlenderExecutables.Clear();

		int len = s_Instance.m_aBlenderPaths.Count();

		if (len == 0)
		{
			s_RegisteredBlenderExecutables.Insert(NO_EXECUTABLE_ENUM);
			return;
		}

		for (int i = 0; i < len; i++)
		{
			string path = s_Instance.m_aBlenderPaths[i].m_sPath;
			if (path != string.Empty)
			{
				string title;
				if (BlenderPathContainer.GetTitleFromPath(path, title))
					s_RegisteredBlenderExecutables.Insert(new ParamEnum(title, i.ToString()));
			}
		}

		if (s_RegisteredBlenderExecutables.Count() == 0)
		{
			s_RegisteredBlenderExecutables.Insert(NO_EXECUTABLE_ENUM);
		}
	}
	
	static bool HasBlenderRegistered()
	{
		UpdateRegisteredBlenderExecutables();

		
		if (s_RegisteredBlenderExecutables[0].m_Key == "None")
		{
			Print("No blender executable set! Please set one in EBT Config!", LogLevel.WARNING);
			return false;
		}
		
		return true;
	}
	
	// it returns bool in case user in any way makes path to executable invalid in the meantime from its registration
	// and caller doesn't check for HasBlenderRegistered
	static bool GetBlenderPathFromEnumIndex(int index, out string path)
	{	
		ref array<ref BlenderPathContainer> pathContainers = s_Instance.m_aBlenderPaths;
		
		if (!FileIO.FileExists(pathContainers[index].m_sPath))
		{
			Print(pathContainers[index].m_sPath + " does not exist! Please select a valid path in EBT Config.", LogLevel.WARNING);
			return false;
		}
		
		path = pathContainers[index].m_sPath;
		return true;
	}
	
	static bool GetDefaultBlenderPath(out string path)
	{
		return GetBlenderPathFromEnumIndex(s_Instance.m_iDefaultPath, path);
	}
	
	[ButtonAttribute("OK")]
	protected void ButtonOK() {}
	
	[ButtonAttribute("Update Menu")]
	protected void ButtonUpdateDropdownMenu()
	{
		shouldReload = true;
		return;
	}
	
	override void Configure()
	{
		while(shouldReload)
		{
			EBTConfigPlugin.HasBlenderRegistered();
			shouldReload = false;
			Workbench.ScriptDialog("EBT Config", "", this);
		}
		shouldReload = true;
	}
	
	static void Update()
	{
		Print("updated");
	}
}

class BaseContainerCustomTitleBlenderVersion : BaseContainerCustomTitle
{
	protected string m_sPropertyName;

	//------------------------------------------------------------------------------------------------
	void BaseContainerCustomTitleBlenderVersion(string propertyName)
	{
		m_sPropertyName = propertyName;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{	
		string path;
		if (!source.Get(m_sPropertyName, path))
			return false;

		BlenderPathContainer.GetTitleFromPath(path, title);
		EBTConfigPlugin.HasBlenderRegistered();
		
		return true;
	}
}

[BaseContainerProps(), BaseContainerCustomTitleBlenderVersion("m_sPath")]
class BlenderPathContainer
{
	[Attribute("", uiwidget: UIWidgets.FileNamePicker, desc: "Absolute path to your blender.exe", params: "exe FileNameFormat=absolute", category: "Path")]
	string m_sPath;
	

	//------------------------------------------------------------------------------------------------
	static bool GetTitleFromPath(string path, out string title)
	{		
		if (!path)
		{
			title = "Unassigned";
			return false;
		}
		
		if (!FileIO.FileExists(path))
		{
			title = "Non Existing";
			return false;
		}
			
		array<string> splitString = {};
		path.Split("\\", splitString, true);
		
		if (splitString.Count() < 2)
		{
			title = "Invalid Path";
	
			return false;
		}
		
		title = splitString[splitString.Count() - 2];
		
		return true;
	}
}


#endif