class SCR_HUDElement
{
	protected string m_sParentWidgetName;
	protected Widget m_wSlotWidget;
	protected SCR_HUDLayout m_ParentLayout;

	//------------------------------------------------------------------------------------------------
	string GetParentWidgetName()
	{
		return m_sParentWidgetName;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetWidget()
	{
		return m_wSlotWidget;
	}

	//------------------------------------------------------------------------------------------------
	void SetParentWidgetName(string parentWidgetName)
	{
		m_sParentWidgetName = parentWidgetName;
	}

	//------------------------------------------------------------------------------------------------
	void SetWidget(notnull Widget widget)
	{
		m_wSlotWidget = widget;
	}

	//------------------------------------------------------------------------------------------------
	SCR_HUDLayout GetParentLayout()
	{
		return m_ParentLayout;
	}

	//------------------------------------------------------------------------------------------------
	void SetParentLayout(notnull SCR_HUDLayout parentLayout)
	{
		m_ParentLayout = parentLayout;
	}
}
