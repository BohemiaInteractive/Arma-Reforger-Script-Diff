/*
#ifdef WORKBENCH
[WorkbenchPluginAttribute(name: "Move Line Up", shortcut: "Alt+Up", wbModules: { "ScriptEditor" }, category: "Move Line", awesomeFontCode: 0xF062)]
class SCR_MoveLineUpPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		int currentLineIndex = scriptEditor.GetCurrentLine();
		if (currentLineIndex == 0)
			return;

		int aboveLineIndex = currentLineIndex - 1;

		string aboveLine;
		string currentLine;

		scriptEditor.GetLineText(aboveLine, aboveLineIndex);
		scriptEditor.GetLineText(currentLine, currentLineIndex);

		SCR_MoveLineDownPluginHelper.IndentChange(aboveLine, currentLine);

		scriptEditor.SetLineText(currentLine, aboveLineIndex);
		scriptEditor.SetLineText(aboveLine, currentLineIndex);
	}
}

[WorkbenchPluginAttribute(name: "Move Line Down", shortcut: "Alt+Down", wbModules: { "ScriptEditor" }, category: "Move Line", awesomeFontCode: 0xF063)]
class SCR_MoveLineDownPlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		ScriptEditor scriptEditor = Workbench.GetModule(ScriptEditor);
		int currentLineIndex = scriptEditor.GetCurrentLine();
		if (currentLineIndex == scriptEditor.GetLinesCount() - 1)
			return;

		int belowLineIndex = currentLineIndex + 1;

		string currentLine;
		string belowLine;

		scriptEditor.GetLineText(currentLine, currentLineIndex);
		scriptEditor.GetLineText(belowLine, belowLineIndex);

		SCR_MoveLineDownPluginHelper.IndentChange(currentLine, belowLine);

		scriptEditor.SetLineText(currentLine, belowLineIndex);
		scriptEditor.SetLineText(belowLine, currentLineIndex);
	}
}

class SCR_MoveLineDownPluginHelper
{
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in,out] lineAbove
	//! \param[in,out] lineBelow
	static void IndentChange(inout string lineAbove, inout string lineBelow)
	{
//		int tabsDiffAbove; // "above" and "below" are -before- change
//		int tabsDiffBelow;

		string trimmedLine = lineAbove.Trim();

		if (trimmedLine == "{" || trimmedLine.StartsWith("{ /" + "/") || trimmedLine.StartsWith("{\t/" + "/")) // think of arrays { 0, 0, 0 }
			lineBelow = lineBelow.Substring(1, lineBelow.Length() - 1);
		else
		if (trimmedLine.StartsWith("}") && lineBelow != "\t" && lineBelow.StartsWith("\t"))
			lineBelow = "\t" + lineBelow;

		trimmedLine = lineBelow.Trim();

		if (trimmedLine == "{" || trimmedLine.StartsWith("{ /" + "/") || trimmedLine.StartsWith("{\t/" + "/")) // think of arrays { 0, 0, 0 }
			lineAbove = "\t" + lineAbove;
		else
		if (trimmedLine.StartsWith("}") && lineBelow != "\t" && lineAbove.StartsWith("\t"))
			lineAbove = lineAbove.Substring(1, lineAbove.Length() - 1);
	}
}
#endif // WORKBENCH
*/