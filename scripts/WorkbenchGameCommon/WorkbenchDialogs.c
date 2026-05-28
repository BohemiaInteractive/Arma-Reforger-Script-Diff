class WorkbenchDialog_AbortRetryIgnore
{
	//! 0 = abort, 1 = retry, 2 = ignore
	int m_iResult;

	[ButtonAttribute("Abort")]
	protected int ButtonAbort()
	{
		m_iResult = 0;
		return m_iResult;
	}

	[ButtonAttribute("Retry", true)]
	protected int ButtonRetry()
	{
		m_iResult = 1;
		return m_iResult;
	}

	[ButtonAttribute("Ignore")]
	protected int ButtonIgnore()
	{
		m_iResult = 2;
		return m_iResult;
	}
}

class WorkbenchDialog_OKCancel
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
