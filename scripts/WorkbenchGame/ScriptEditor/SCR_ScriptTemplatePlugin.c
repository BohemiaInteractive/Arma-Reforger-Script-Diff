#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Fill from Template", shortcut: "Ctrl+T", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xF1C9)]
class SCR_ScriptTemplatePlugin : WorkbenchPlugin
{
	/*
		Category: General
	*/

	[Attribute(defvalue: SCR_EScriptTemplateType.None.ToString(), desc: "Template type to be used", uiwidget: UIWidgets.ComboBox, enumType: SCR_EScriptTemplateType, category: "General")]
	protected SCR_EScriptTemplateType m_eClassType;

	[Attribute(desc: "Full classname - if left empty, file name will be used", category: "General")]
	protected string m_sClassName;

	[Attribute(desc: "Parent class name - if left empty, default for given script type will be used", category: "General")]
	protected string m_sParentName;

	/*
		Category: Advanced
	*/

	//--- Let's not expose this, could be overwhelming for the user
	// [Attribute("{51BB186E949A991B}Configs/Workbench/ScriptTemplatePlugin/ScriptTemplateConfig.conf", params: "conf class=SCR_ScriptTemplateConfig", category: "Advanced")]
	protected ResourceName m_sConfig = "{51BB186E949A991B}Configs/Workbench/ScriptTemplatePlugin/ScriptTemplateConfig.conf";

	//------------------------------------------------------------------------------------------------
	protected override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		string className;
		if (!scriptEditor.GetCurrentFile(className))
		{
			Print("Cannot insert script template, no file is opened!", LogLevel.WARNING);
			return;
		}

		if (!Workbench.ScriptDialog("Fill from Template", "Insert a template of given class type to selected line in currently opened file.", this) && Workbench.OpenModule(ScriptEditor))
			return;

		//--- Load config
		Resource configResource = Resource.Load(m_sConfig);
		if (!configResource.IsValid())
		{
			Debug.Error2(Type().ToString(), string.Format("Cannot load config '%1'!", m_sConfig));
			return;
		}

		SCR_ScriptTemplateConfig config = SCR_ScriptTemplateConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(configResource.GetResource().ToBaseContainer()));
		if (!config)
		{
			Debug.Error2(Type().ToString(), string.Format("Cannot load config '%1'!", m_sConfig));
			return;
		}

		if (!config.m_aEntries)
		{
			Debug.Error2(Type().ToString(), string.Format("Config '%1' has null entries!", m_sConfig));
			return;
		}

		//--- Get template
		string template, defaultParentName, suffix;
		foreach (SCR_ScriptTemplateConfigEntry entry : config.m_aEntries)
		{
			if (entry.m_eType == m_eClassType)
			{
				template = entry.m_sTemplate;
				defaultParentName = entry.m_sDefaultParentName;
				suffix = entry.m_sSuffix;
				break;
			}
		}

		//--- Get script class
		if (m_sClassName.IsEmpty())
			className = FilePath.StripExtension(FilePath.StripPath(className));
		else
			className = m_sClassName;

		if (!className.EndsWith(suffix))
			className += suffix;

		//--- Get parent class
		string parentName;
		if (m_sParentName.IsEmpty())
			parentName = defaultParentName;
		else
			parentName = m_sParentName;

		if (parentName)
			parentName = " : " + parentName;

		//--- Copy template into the script
		int lineNumber = scriptEditor.GetCurrentLine();
		scriptEditor.InsertLine(string.Format(template, className, parentName), lineNumber);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Fill", true)]
	protected bool ButtonConfirm()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected bool ButtonCancel()
	{
		return false;
	}
}

[BaseContainerProps(configRoot: true)]
class SCR_ScriptTemplateConfig
{
	[Attribute()]
	ref array<ref SCR_ScriptTemplateConfigEntry> m_aEntries;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_EScriptTemplateType, "m_eType")]
class SCR_ScriptTemplateConfigEntry
{
	[Attribute(uiwidget: UIWidgets.ComboBox, desc: "Template type", enumType: SCR_EScriptTemplateType)]
	SCR_EScriptTemplateType m_eType;

	[Attribute(uiwidget: UIWidgets.EditBoxMultiline, desc: "Template text that will be added to the file. Variables are:\n%1 - class name\n%2 - parent class name")]
	string m_sTemplate;

	[Attribute(desc: "When defined, this parent class name will be used when user does not set any in the plugin.")]
	string m_sDefaultParentName;

	[Attribute(desc: "Define the class' mandatory suffix (e.g xxxEntity, xxxPlugin, xxxTool)")]
	string m_sSuffix;
}

enum SCR_EScriptTemplateType
{
	None,
	Entity,
	Component,
	WidgetComponent,
	ScriptInvoker,
	ScriptedUserAction,
	UIMenu,
	ConfigRoot,
	WorkbenchPlugin,
	ScriptEditorPlugin,
	WorldEditorPlugin,
	WorldEditorTool,
}
#endif // WORKBENCH
