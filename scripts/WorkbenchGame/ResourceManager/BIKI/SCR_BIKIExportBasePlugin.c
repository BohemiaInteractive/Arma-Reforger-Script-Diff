#ifdef WORKBENCH
class SCR_BIKIExportBasePlugin : WorkbenchPlugin
{
	//------------------------------------------------------------------------------------------------
	override void Run()
	{
		SCR_WorkbenchHelper.PrintDialog("The Run method has not been overridden", "BIKI Export Base", LogLevel.ERROR);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowResult(string result)
	{
		Workbench.ScriptDialog(
			"BIKI-formatted result",
			"Use the \"Copy to Clipboard\" button to grab the entire result."
			+ "\nIf you want to manually select and copy parts of the result, EXPAND the field below then copy/paste its full content;"
			+ " copying from the inline field may truncate big data.",
			new SCR_TextResultWorkbenchDialog(result));
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
#endif // WORKBENCH
