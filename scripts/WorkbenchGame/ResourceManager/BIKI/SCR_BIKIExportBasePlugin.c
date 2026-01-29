#ifdef WORKBENCH
class SCR_BIKIExportBasePlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		const string error = "The Run method has not been overridden";
		Print(error, LogLevel.ERROR);
		Workbench.Dialog("BIKI Export Base", error);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowResult(string result)
	{
		Workbench.ScriptDialog(
			"Result to copy/paste",
			"Copy/paste the text below - be sure to expand the field or use Ctrl+A to get all the content.",
			new SCR_BIKIExportPlugin_Result(result));
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("OK", true)]
	protected int ButtonOK()
	{
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}

class SCR_BIKIExportPlugin_Result
{
	[Attribute(uiwidget: UIWidgets.EditBoxMultiline, category: "Result")]
	protected string m_sResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close", true)]
	protected bool ButtonClose()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_BIKIExportPlugin_Result(string result)
	{
		m_sResult = result;
	}
}
#endif // WORKBENCH
