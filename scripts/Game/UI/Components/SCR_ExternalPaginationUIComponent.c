class SCR_ExternalPaginationUIComponent: SCR_BasePaginationUIComponent
{
	protected int m_iEntryCount;
	protected ref ScriptInvoker m_OnShowEntries = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	void SetPageIndex(int index)
	{
		m_iCurrentPage = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntryCount(int count)
	{
		m_iEntryCount = count;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnShowEntries()
	{
		return m_OnShowEntries;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		m_OnShowEntries.Invoke(contentWidget, indexStart, indexEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected int GetEntryCount()
	{
		return m_iEntryCount;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
	}
};