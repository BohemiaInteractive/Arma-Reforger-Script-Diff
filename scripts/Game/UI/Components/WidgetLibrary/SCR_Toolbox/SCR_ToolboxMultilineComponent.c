class SCR_ToolboxMultilineComponent : SCR_ToolboxComponent
{
	[Attribute("4")]
	protected int m_iMaxItemsInRow;
	
	[Attribute("36")]
	float m_fElementHeight;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_iMaxItemsInRow = Math.Max(m_iMaxItemsInRow, 1);
	}

	//------------------------------------------------------------------------------------------------
	override protected void CreateWidgets()
	{
		float padding = m_fElementSpacing * 0.5;
		LayoutSlot.SetPadding(m_wButtonRow, -padding, -padding, -padding, -padding);
		foreach (int i, string name : m_aElementNames)
		{
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_ElementLayout, m_wButtonRow);

			int row = Math.Floor(i / m_iMaxItemsInRow);
			int column = i % m_iMaxItemsInRow;
			UniformGridSlot.SetRow(button, row);
			UniformGridSlot.SetColumn(button, column);

			SCR_ButtonBaseComponent comp = SCR_ButtonBaseComponent.Cast(button.FindHandler(SCR_ButtonBaseComponent));
			if (!comp)
				continue;

			m_aSelectionElements.Insert(comp);
			
			SCR_ButtonTextComponent textComp = SCR_ButtonTextComponent.Cast(comp);
			if (textComp)
				textComp.SetText(name);
			
			comp.m_OnClicked.Insert(OnElementClicked);
			comp.m_OnToggled.Insert(OnElementChanged);
			
			SizeLayoutWidget size = SizeLayoutWidget.Cast(comp.GetRootWidget().GetChildren());
			if (!size)
				return;
			
			size.EnableHeightOverride(m_fElementHeight > 0);
			size.SetHeightOverride(m_fElementHeight);
		}
		
		SetInitialState();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RemoveWidgets()
	{
		foreach (SCR_ButtonBaseComponent comp : m_aSelectionElements)
		{
			comp.GetRootWidget().RemoveFromHierarchy();
		}
		m_aSelectionElements.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxItemsInRow(int maxItems)
	{
		if (m_iMaxItemsInRow == maxItems)
			return;
		
		m_iMaxItemsInRow = maxItems;
		m_iMaxItemsInRow = Math.Max(m_iMaxItemsInRow, 1);
		RemoveWidgets();
		CreateWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaxItemsInRow()
	{
		return m_iMaxItemsInRow;
	}
	
	//------------------------------------------------------------------------------------------------
	SizeLayoutWidget FindSizeWidget(Widget root)
	{
		Widget child = root.GetChildren();
		while (child)
		{
			SizeLayoutWidget size = SizeLayoutWidget.Cast(child);
			if (size)
				return size;
			child = child.GetChildren();
		}
		return null;
	}
};