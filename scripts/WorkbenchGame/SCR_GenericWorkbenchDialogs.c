class SCR_OKWorkbenchDialog // Workbench.Dialog works too, only styling is different
{
	[ButtonAttribute("OK", true)]
	protected int BtnOK()
	{
		return 1;
	}
}

class SCR_OKCancelWorkbenchDialog
{
	[ButtonAttribute("OK", true)]
	protected int BtnOK()
	{
		return 1;
	}

	[ButtonAttribute("Cancel")]
	protected int BtnCancel()
	{
		return 0;
	}
}
