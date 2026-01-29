#ifdef WORKBENCH
//! This plugin generates a scripted class with variables for widgets and code to find widgets by their name.
//! It also generates code to find all widget components.
[WorkbenchPluginAttribute(name: PLUGIN_NAME, wbModules: { "ResourceManager" }, awesomeFontCode: 0xF0DB)]
class SCR_GenerateLayoutClassPlugin : WorkbenchPlugin
{
	protected static const string PLUGIN_VERSION = "0.5.1"; // plugin version, printed in the generated file
	protected static const string PLUGIN_NAME = "Generate Class from Layout";
	protected static const string DIALOG_CAPTION = PLUGIN_NAME + " [v" + PLUGIN_VERSION + "]";
	protected static const string INTRO_TEXT = "This plugin autogenerates widget-binding scripts for .layout files.\n\n";

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		_print("Run()");

		BaseContainer widgetSource = Workbench.GetModule(ResourceManager).GetContainer();
		if (!widgetSource) // bail if we are not in layout editor
		{
			Workbench.Dialog(DIALOG_CAPTION, INTRO_TEXT + "You need to open a .layout file first!");
			return;
		}

		BaseContainer exportRule = SCR_WidgetExportRuleRoot.FindInWidgetSource(widgetSource);
		if (!exportRule) // bail if SCR_WidgetExportRuleRoot was not found
		{
			Workbench.Dialog(DIALOG_CAPTION, INTRO_TEXT + "You need to attach a SCR_WidgetExportRuleRoot component to root widget of your layout!");
			return;
		}

		string layoutPath = widgetSource.GetName();
		string scriptClassName = GenerateScriptClassName(layoutPath, exportRule);
		string destinationPath = ResolveDestinationPath(scriptClassName + ".c");
		string dialogText = INTRO_TEXT +
			"The generator only exports widgets and their components if the widget name starts with 'm_'.\n" +
			"Attach SCR_WidgetExportRule component to a widget to alter its export rules.\n\n" +
			"- - - - - - - - - - - - - - - - - - - - \n\n" +
			string.Format("Generated script class name:\n%1\n\n", scriptClassName) +
			string.Format("Destination file path:\n%1\n\n", destinationPath) +
			string.Format("WARNING: The destination file will be overwritten if it already exists! (file exists: %1)", FileIO.FileExists(destinationPath).ToString());

		if (!Workbench.ScriptDialog(DIALOG_CAPTION, dialogText, this))
			return;

		Generate(widgetSource, exportRule, scriptClassName, destinationPath);
	}

	//------------------------------------------------------------------------------------------------
	//! Generates the class name for the generated class
	//! \param[in] widgetSource
	//! \param[in] exportRule
	//! \param[in] scriptClassName
	//! \param[in] destinationPath
	protected void Generate(notnull BaseContainer widgetSource, notnull BaseContainer exportRule, string scriptClassName, string destinationPath)
	{
		// build array of all elements we are going to generate code for
		array<BaseContainer> widgets = {};
		array<ref array<BaseContainer>> paths = {};
		BuildWidgetArray(widgetSource, {}, widgets, paths);

		// generate the output class name
		string layoutPath = widgetSource.GetName();
		_print(string.Format("Layout path: %1", layoutPath));
		_print(string.Format("Script class name: %1", scriptClassName));

		// strings with generated sections
		string variablesDeclaration;	// Type m_wMyWidget;
		string variablesBinding;		// m_wMyWidget = Type.Cast(root.Find...)

		// generate the code
		_print("Iterating widgets...");
		bool generateFullWidgetPath = SCR_WidgetExportRuleRoot.GetGenerateFullWidgetPath(exportRule);
		array<BaseContainer> pathToThisWidget;
		array<BaseContainer> components;
		foreach (int widgetId, BaseContainer thisWidget : widgets)
		{
			pathToThisWidget = paths[widgetId];

			string wName = GetWidgetName(thisWidget);

			// issue a warning if widget name is not correct
			if (!IsWidgetExportRequired(thisWidget, pathToThisWidget))
				continue;

			if (!ValidateWidget(thisWidget))
				continue;

			components = ResolveWidgetComponentsForExport(thisWidget);
			int componentsCount = components.Count();

			if (componentsCount > 0 && !variablesDeclaration.IsEmpty())
			{
				variablesDeclaration += "\n";
				variablesBinding += "\n";
			}

			string widgetVariableName = ResolveWidgetVariableName(thisWidget);
			if (!widgetVariableName.StartsWith("m_w"))
			{
				if (widgetVariableName.StartsWith("m_"))
					widgetVariableName = SCR_StringHelper.InsertAt(widgetVariableName, "w", 2);
				else
					widgetVariableName = "m_w" + widgetVariableName;
			}

			string pathToThisWidgetStr = GetStringPathToWidget(pathToThisWidget);

			_print(string.Format("Exporting widget: %1, %2, %3", wName, widgetVariableName, pathToThisWidgetStr));

			// declare variable for this widget
			string wClassName = thisWidget.GetClassName();
			wClassName = wClassName.Substring(0, wClassName.Length() - 5); // remove "Class" from the end
			variablesDeclaration += string.Format("\t%1 %2;\n", wClassName, widgetVariableName);

			// perform variable binding for this widget
			if (generateFullWidgetPath)
				variablesBinding += string.Format("\t\t%1 = %2.Cast(root.FindWidget(\"%3\"));\n", widgetVariableName, wClassName, pathToThisWidgetStr);
			else
				variablesBinding += string.Format("\t\t%1 = %2.Cast(root.FindAnyWidget(\"%3\"));\n", widgetVariableName, wClassName, GetWidgetName(thisWidget));

			if (componentsCount < 1) // no components in this one, next
				continue;

			string noMWPrefixWidgetVariableName = SCR_StringHelper.ReplaceTimes(widgetVariableName, "m_w", "m_", 1);

			// declare variables for widget components
			foreach (int i, BaseContainer comp : components)
			{
				string compClassName = comp.GetClassName();
				string compVarName;

				if (componentsCount == 1)
					compVarName = string.Format("%1Component", noMWPrefixWidgetVariableName);
				else
					compVarName = string.Format("%1Component%2", noMWPrefixWidgetVariableName, i);

				variablesDeclaration += string.Format("\t%1 %2;\n", compClassName, compVarName);

				// variable binding for component
				variablesBinding += string.Format("\t\t%1 = %2.Cast(%3.FindHandler(%4));\n", compVarName, compClassName, widgetVariableName, compClassName);
			}

			variablesDeclaration += "\n";
			variablesBinding += "\n";
		}

		// generate whole code
		string gc =
			string.Format("/" + "/ Autogenerated by the Generate Class from Layout plugin v%1\n", PLUGIN_VERSION) +
			string.Format("/" + "/ Layout file: %1\n", layoutPath) +
			string.Format("class %1\n{\n", scriptClassName) + // class declaration and opening
			string.Format("\tprotected static const ResourceName LAYOUT = \"%1\";\n\n", widgetSource.GetResourceName()) + // constant with layout path
			variablesDeclaration + "\n" +			// class variablesBinding
			"\t//------------------------------------------------------------------------------------------------\n" +
			"\tbool Init(notnull Widget root)\n\t{\n" +		// variable binding
			variablesBinding + "\n" +
			"\t\treturn true;\n\t}\n\n" +
			"\t//------------------------------------------------------------------------------------------------\n" +
			"\tResourceName GetLayout()\n\t{\n\t\t" + "return LAYOUT;\n\t}\n" +
			"}\n";									// close class

		// save everything to file
		string fileOutPath = destinationPath;

		_print(string.Format("Exporting to file: %1", fileOutPath));
		FileHandle fileHandle = FileIO.OpenFile(fileOutPath, FileMode.WRITE);
		if (!fileHandle)
		{
			_print("Error opening file!");
			return;
		}

		fileHandle.Write(gc);
		fileHandle.Close();
		_print("Export finished!");

		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		if (scriptEditor)
			scriptEditor.SetOpenedResource(fileOutPath);
	}

	// ---------------- Misc functions ---------------

	//------------------------------------------------------------------------------------------------
	//! Checks if conditions for widget export are satisfied
	protected bool IsWidgetExportRequired(notnull BaseContainer ws, notnull array<BaseContainer> path)
	{
		// check if any widgets in the path explicitly disables export
		// ignore the last widget (this widget), even if its child widgets are not exported,
		// it still can be exported

		BaseContainer rule;
		foreach (BaseContainer pathWs : path)
		{
			rule = SCR_WidgetExportRule.FindInWidgetSource(pathWs);
			if (rule)
			{
				bool exportChildren = SCR_WidgetExportRule.GetExportChildWidgets(rule);
				if (!exportChildren)
					return false;
			}
		}

		// check widget properties

		rule = SCR_WidgetExportRule.FindInWidgetSource(ws);
		if (rule)	// widget name matches pattern, no rule provided - default behaviour
			return SCR_WidgetExportRule.GetExportThisWidget(rule);

		return GetWidgetName(ws).StartsWith("m_");
	}

	//------------------------------------------------------------------------------------------------
	//! Resolves variable name for widget
	protected string ResolveWidgetVariableName(BaseContainer ws)
	{
		BaseContainer rule = SCR_WidgetExportRule.FindInWidgetSource(ws);
		string wName = GetWidgetName(ws);

		if (!rule)
			return wName; // no rule, default behaviour - same as widget name

		string wNameFromRule = SCR_WidgetExportRule.GetWidgetVariableName(rule);
		if (wNameFromRule.IsEmpty())
			return wName;

		return wNameFromRule;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs various validations related to exporting of this widget
	//! \return widget export validity
	protected bool ValidateWidget(BaseContainer ws)
	{
		string wName = GetWidgetName(ws);
		if (wName.Contains(" "))
		{
			_print(string.Format("Widget name contains space: %1", wName), LogLevel.ERROR);
			return false;
		}

		BaseContainer rule = SCR_WidgetExportRule.FindInWidgetSource(ws);
		if (!rule)
			return true;

		string wNameFromRule = SCR_WidgetExportRule.GetWidgetVariableName(rule);
		if (!wNameFromRule.IsEmpty() && wNameFromRule.Contains(" "))
		{
			_print(string.Format("Widget name in the export rule contains space: %1, widget: %2", wNameFromRule, wName), LogLevel.ERROR);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Generates an array of components which will be exported for this widget
	protected array<BaseContainer> ResolveWidgetComponentsForExport(WidgetSource ws)
	{
		BaseContainerList components = ws.GetObjectArray("components");
		array<BaseContainer> outArray = {};
		BaseContainer comp;
		typename compTypename;
		for (int i, count = components.Count(); i < count; i++)
		{
			comp = components.Get(i);
			compTypename = comp.GetClassName().ToType();

			// we do not want to export this one obviously
			if (!compTypename || (!compTypename.IsInherited(SCR_WidgetExportRule) && !compTypename.IsInherited(SCR_WidgetExportRuleRoot)))
				outArray.Insert(comp);
		}

		return outArray;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the script classname based on prefix, path file and suffix (see SCR_WidgetExportRuleRoot.GetClassPrefixAndSuffix)
	protected string GenerateScriptClassName(string path, notnull BaseContainer exportRule)
	{
		int slashId = path.LastIndexOf("/");
		int dotId = path.LastIndexOf(".");
		int cutSize = dotId - slashId - 1;
		string fileNameNoPathNoExt = path.Substring(slashId + 1, cutSize);

		string prefix, suffix;
		SCR_WidgetExportRuleRoot.GetClassPrefixAndSuffix(exportRule, prefix, suffix);

		return string.Format("%1%2%3", prefix, fileNameNoPathNoExt, suffix);
	}

	//------------------------------------------------------------------------------------------------
	//! Converts a widget tree into a widget array
	//! \param[in] ws
	//! \param[in] pathToThis
	//! \param[out] outArray - each element will contain a WidgetSource
	//! \param[out] outArrayPaths - each element will contain a path to this widget, including this widget
	protected static void BuildWidgetArray(
		notnull BaseContainer ws,
		notnull array<BaseContainer> pathToThis,
		notnull out array<BaseContainer> outArray,
		notnull out array<ref array<BaseContainer>> outArrayPaths)
	{
		array<BaseContainer> fullPathToThis = {};
		fullPathToThis.Copy(pathToThis);
		fullPathToThis.Insert(ws); // path to this widget also includes this widget

		outArrayPaths.Insert(fullPathToThis);

		outArray.Insert(ws);

		for (int e = 0, count = ws.GetNumChildren(); e < count; e++)
		{
			BuildWidgetArray(ws.GetChild(e), fullPathToThis, outArray, outArrayPaths);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns path to widget separated with dots
	//! First widget is omitted
	//! \param[in] path array of base containers
	//! \return path compliant with Widget.FindWidget() or "_error_" if path is empty
	protected static string GetStringPathToWidget(notnull array<BaseContainer> path)
	{
		int c = path.Count();
		if (c < 2)
			return "_error_";

		string pathStr;
		for (int i = 1; i < c - 1; i++)
		{
			pathStr = pathStr + GetWidgetName(path[i]) + ".";
		}

		return pathStr + GetWidgetName(path[c - 1]); // last widget name without a dot
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] ws
	//! \return widget's Name property, or empty string in case of no Name property
	protected static string GetWidgetName(BaseContainer ws)
	{
		string result;
		ws.Get("Name", result);
		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Resolves the target directory where the file must be saved
	protected string ResolveDestinationPath(string fileOutName)
	{
		// first, look up the parameters in the root widget
		BaseContainer widgetSource = Workbench.GetModule(ResourceManager).GetContainer();

		string destinationPath;
		string scriptAddon;

		BaseContainer rule = SCR_WidgetExportRuleRoot.FindInWidgetSource(widgetSource);
		if (rule)
		{
			destinationPath = SCR_WidgetExportRuleRoot.GetDestinationPath(rule);
			scriptAddon = SCR_WidgetExportRuleRoot.GetScriptAddon(rule);
		}

		int bracketIndex = destinationPath.LastIndexOf("}");
		string fileOutPath = destinationPath.Substring(bracketIndex + 1, destinationPath.Length() - bracketIndex - 1);
		if (!fileOutPath.EndsWith("/"))
			fileOutPath = fileOutPath + "/";

		fileOutPath = fileOutPath + fileOutName;
		fileOutPath = scriptAddon + fileOutPath;

		return fileOutPath;
	}

	//------------------------------------------------------------------------------------------------
	//! Prefixes Print() calls with "[SCR_GenerateLayoutClassPlugin] "
	protected static void _print(string str, LogLevel logLevel = LogLevel.NORMAL)
	{
		PrintFormat("[GenerateLayoutClassPlugin] %1", str, level: logLevel);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Generate", true)]
	protected bool ButtonOK()
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
#endif // WORKBENCH
