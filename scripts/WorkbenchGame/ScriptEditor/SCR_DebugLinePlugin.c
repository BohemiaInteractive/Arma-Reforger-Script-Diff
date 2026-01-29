#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Debug Line Shortcut", description: "Insert a debug line, as formatted in its options", shortcut: "Ctrl+Shift+D", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xF1CD)]
class SCR_DebugLinePlugin : WorkbenchPlugin
{
	[Attribute(defvalue: "[%1.%2] debug line (%4 L%5)", desc: "Text to be printed in the debug line"
		+ "\n- %1 = class name (as string, hardcoded)"
		+ "\n- %2 = method name (as string, hardcoded)"
		+ "\n- %3 = file name (as preprocess command)"
		+ "\n- %4 = file's full path (as preprocessor command)"
		+ "\n- %5 = debug line number (as preprocessor command)", uiwidget: UIWidgets.EditBoxMultiline, category: "Format")]
	protected string m_sFormat;

	[Attribute(defvalue: LogLevel.WARNING.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Log level of the debug line",
		enums: GetLogLevelEnumOptions(),
		category: "Log Level"
	)]
	protected LogLevel m_eLogLevel;

	[Attribute(defvalue: "0", desc: "Make debug line use PrintFormat to output custom variables", category: "Format")]
	protected bool m_bIncludeParams;

	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);

		string fileFormat;
		if (!scriptEditor.GetCurrentFile(fileFormat))
		{
			Print("Cannot insert debug line, no file is opened!", LogLevel.WARNING);
			return;
		}

		string lineText;
		scriptEditor.GetLineText(lineText);

		int tabIndex = lineText.LastIndexOf(SCR_StringHelper.TAB);
		if (lineText.EndsWith("{"))
			tabIndex++;

		string tabPrefix;
		for (int i; i <= tabIndex; ++i)
		{
			tabPrefix += SCR_StringHelper.TAB;
		}

		int nextLine = scriptEditor.GetCurrentLine() + 1;
		string debugLine = tabPrefix + GetDebugLine(scriptEditor);
		scriptEditor.InsertLine(debugLine, nextLine);
	}

	//------------------------------------------------------------------------------------------------
	//! Get a formatted debug line
	//! \param[in] scriptEditor
	//! \return the debug output ("Print(" + formatted text + ", " + logLevel + ");"
	protected string GetDebugLine(notnull ScriptEditor scriptEditor)
	{
		string debugLine = m_sFormat;
		debugLine.TrimInPlace();
		if (!debugLine)
		{
			if (m_bIncludeParams)
				return "PrintFormat(\"%1\", this);";
			else
				return "Print(\"test\");";
		}

		string className, methodName;

		if (debugLine.Contains("%1"))
		{
			if (!className)
				SCR_CopyClassAndMethodPlugin.GetCursorClassAndMethodNames(scriptEditor, className, methodName);

			debugLine.Replace("%1", className);
		}

		if (debugLine.Contains("%2"))
		{
			if (!className) // methodName can be empty
				SCR_CopyClassAndMethodPlugin.GetCursorClassAndMethodNames(scriptEditor, className, methodName);

			debugLine.Replace("%2", methodName);
		}

		if (debugLine.Contains("%3"))
			debugLine.Replace("%3", "\" + FilePath.StripPath(__FILE__) + \"");

		if (debugLine.Contains("%4"))
			debugLine.Replace("%4", "\" + __FILE__ + \"");

		if (debugLine.Contains("%5"))
			debugLine.Replace("%5", "\" + __LINE__ + \"");

		if (m_bIncludeParams)
		{
			debugLine = "PrintFormat(\"" + debugLine + "\", this";
			if (m_eLogLevel > -1 && m_eLogLevel != int.MAX)
				debugLine += ", level: LogLevel." + typename.EnumToString(LogLevel, m_eLogLevel);
		}
		else
		{
			debugLine = "Print(\"" + debugLine + "\"";
			if (m_eLogLevel > -1 && m_eLogLevel != int.MAX)
				debugLine += ", LogLevel." + typename.EnumToString(LogLevel, m_eLogLevel);
		}

		debugLine += ");";

		return debugLine;
	}

	//------------------------------------------------------------------------------------------------
	//! \return LogLevel ParamEnumArray, with a leading "no logLevel" value
	protected static ParamEnumArray GetLogLevelEnumOptions()
	{
		typename e = LogLevel;
		ParamEnumArray result = { new ParamEnum("No LogLevel (temporary Print)", "-1") };

		int val;
		string firstLetter;
		string displayName;

		for (int i, count = e.GetVariableCount(); i < count; ++i)
		{
			if (e.GetVariableType(i) == int && e.GetVariableValue(null, i, val))
			{
				displayName = e.GetVariableName(i);
				firstLetter = displayName[0];
				firstLetter.ToUpper(); // not needed as they are all uppercase
				displayName.ToLower();
				displayName = firstLetter + displayName.Substring(1, displayName.Length() - 1);
				result.Insert(new ParamEnum(displayName, val.ToString()));
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override void Configure()
	{
		Workbench.ScriptDialog("Configure the 'Insert Debug Line' plugin", "", this);
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected bool ButtonClose()
	{
		return true;
	}
}
#endif // WORKBENCH
