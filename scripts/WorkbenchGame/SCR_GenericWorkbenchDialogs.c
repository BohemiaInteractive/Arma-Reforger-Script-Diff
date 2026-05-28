class SCR_OKWorkbenchDialog // Workbench.Dialog works too, only styling is different
{
	[ButtonAttribute("OK", true)]
	protected int ButtonOK()
	{
		return 1;
	}
}

[Obsolete("Use WorkbenchDialog_OKCancel instead")]
class SCR_OKCancelWorkbenchDialog
{
	[ButtonAttribute("OK", true)]
	protected int ButtonOK()
	{
		return 1;
	}

	[ButtonAttribute("Cancel")]
	protected int ButtonCancel()
	{
		return 0;
	}
}

class SCR_TextResultWorkbenchDialog
{
	[Attribute(uiwidget: UIWidgets.EditBoxMultiline, category: "Result")]
	protected string m_sResult;

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Copy to Clipboard", true)]
	protected int ButtonCopyToClipboard()
	{
		System.ExportToClipboard(m_sResult);
		return 1;
	}

	//------------------------------------------------------------------------------------------------
	[ButtonAttribute("Close")]
	protected int ButtonClose()
	{
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_TextResultWorkbenchDialog(string result)
	{
		m_sResult = result;
	}
}
