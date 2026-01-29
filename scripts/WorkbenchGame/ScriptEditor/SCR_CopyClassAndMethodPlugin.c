#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Copy Class and Method Name", description: "Copy class and method name based on the current line.\nWhen it is inside of a method, the format is 'MyClass.MyMethod()', otherwise the result is just 'MyClass'", shortcut: "Ctrl+Shift+C", wbModules: { "ScriptEditor" }, awesomeFontCode: 0xF0C5)]
class SCR_CopyClassAndMethodPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);

		string className, methodName;
		if (!GetCursorClassAndMethodNames(scriptEditor, className, methodName))
		{
			Print("Nothing copied to clipboard, the current line is not inside of a class.", LogLevel.DEBUG);
			return;
		}

		string result;
		if (methodName) // !IsEmpty
			result = string.Format("%1.%2()", className, methodName);
		else
			result = className;

		System.ExportToClipboard(result);
		Print(string.Format("Copied to clipboard: \"%1\"", result), LogLevel.DEBUG);
	}

	//------------------------------------------------------------------------------------------------
	//! Get the current class name / class + method name where the cursor is
	//! \param[in] scriptEditor a ScriptEditor instance
	//! \param[out] className
	//! \param[out] methodName is emptied if not in method
	//! \return true on success, false otherwise
	static bool GetCursorClassAndMethodNames(notnull ScriptEditor scriptEditor, out string className, out string methodName)
	{
		string line;
		array<string> lines = {};
		for (int lineId, lineCount = scriptEditor.GetCurrentLine() + 1; lineId < lineCount; ++lineId)
		{
			scriptEditor.GetLineText(line, lineId);
			lines.Insert(line);
		}

		return GetLineClassAndMethodNames(lines, scriptEditor.GetCurrentLine(), className, methodName);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] lines
	//! \param[in] lineId
	//! \param[in] cursorLineId
	//! \param[out] className
	//! \param[out] methodName
	//! \return
	static bool GetLineClassAndMethodNames(notnull array<string> lines, int cursorLineId, out string className, out string methodName)
	{
		const string bracketOpen = "{";
		const string bracketClose = "}";
		const string commentOpen = "/" + "*";
		const string commentClose = "*" + "/";

		//--- Get scope hierarchy
		array<int> scopes = {};
		bool isComment;
		int linesCount = cursorLineId;

		foreach (int lineId, string line : lines)
		{
			if (lineId == linesCount)
				break;

			line.Replace(SCR_StringHelper.SPACE, string.Empty);
			line.Replace(SCR_StringHelper.TAB, string.Empty);

			if (!isComment)
			{
				if (line.StartsWith(bracketOpen))
				{
					scopes.Insert(lineId - 1);
				}
				else if (line.StartsWith(bracketClose))
				{
					if (!scopes.IsEmpty())
						scopes.Resize(scopes.Count() - 1);
				}
				else if (line.StartsWith(commentOpen))
				{
					isComment = true;
				}
			}
			else if (line.StartsWith(commentClose))
			{
				isComment = false;
			}
		}

		if (scopes.IsEmpty())
			scopes.Insert(cursorLineId);

		string line = lines[scopes[0]];

		array<string> lineArray = {};
		line.Split(SCR_StringHelper.SPACE, lineArray, false);
		lineArray.RemoveItemOrdered("modded");
		lineArray.RemoveItemOrdered("sealed");
		if (lineArray.Count() < 2)
			return false;

		className = lineArray[1];
		className.Replace(":", string.Empty);
		className.TrimInPlace();

		if (scopes.Count() == 1)
			scopes.Insert(cursorLineId);

		if (!lines.IsIndexValid(scopes[1]))
		{
			methodName = string.Empty;
			return true;
		}

		if (lines[scopes[1]].Trim().StartsWith(bracketOpen))
			scopes[1] = scopes[1] - 1;

		// methodName = lines[scopes[1]];
		lines[scopes[1]].Split("(", lineArray, false);
		if (lineArray.Count() >= 2)
		{
			methodName = lineArray[0];
			methodName.Split(SCR_StringHelper.SPACE, lineArray, false);
			if (lineArray.Count() > 1)
				methodName = lineArray[lineArray.Count() - 1];
			else
				methodName = string.Empty;
		}

		return true;
	}
}
#endif // WORKBENCH
