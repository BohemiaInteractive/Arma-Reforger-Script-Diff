class SCR_PlayMenuComponent : ScriptedWidgetComponent
{
	[Attribute("3")]
	protected int m_iColumns;
	[Attribute("1")]
	protected int m_iRows;

	protected int m_iItems;
	protected int m_iSelectedItem;
	
	[Attribute("2")]
	protected int m_iPaddingHorizontal;
	[Attribute("2")]
	protected int m_iPaddingVertical;

	[Attribute("{4FFBD0D5E9A5ED50}UI/layouts/Menus/PlayMenu/PlayMenuTile_Recent.layout", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_sItemLayout;
	
	[Attribute("0.2")]
	protected float m_fAnimationTime;
	
	protected ref array<Widget> m_aWidgets = {};
	protected GridLayoutWidget m_wContentRoot;
	protected Widget m_wRoot;
	protected float m_fAnimationRate = SCR_WLibComponentBase.START_ANIMATION_RATE;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
		m_wContentRoot = GridLayoutWidget.Cast(w.FindAnyWidget("Content"));

		if (!m_wContentRoot)
			return;
		
		m_iItems = m_iColumns * m_iRows;
	
		if (m_iItems == 0)
			return;
				
		// Convert time to animation speed
		GetGame().GetCallqueue().CallLater(SetAnimationRate, SCR_WLibComponentBase.START_ANIMATION_PERIOD);
		
		CreateWidgets(m_iColumns, m_iRows);
		
		m_iSelectedItem = 0;
		SetFocusedItem(m_iSelectedItem);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateWidgets(int columns, int rows)
	{
		if (!m_wContentRoot)
			return;
		
		// Delete old widgets
		foreach (Widget w: m_aWidgets)
		{
			if (w)
				w.RemoveFromHierarchy();
		}
		m_aWidgets.Clear();

		// Create new grid tile widgets
	    for (int r = 0; r < rows; r++)
	    {		
		    for (int c = 0; c < columns; c++)
		    {
				CreateWidget(c, r);
		    }
		}		
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateWidget(int column, int row)
	{
		if (!m_wContentRoot)
			return null;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sItemLayout, m_wContentRoot);
		if (!w)
			return null;

		GridSlot.SetColumn(w, column);
		GridSlot.SetRow(w, row);
		m_wContentRoot.SetRowFillWeight(row, 1);
		m_wContentRoot.SetColumnFillWeight(column, 1);

		// Listen on focus event
		SCR_EventHandlerComponent comp = SCR_EventHandlerComponent.Cast(w.FindHandler(SCR_EventHandlerComponent));
		if (!comp)
		{
			comp = new SCR_EventHandlerComponent();
			w.AddHandler(comp);
		}
		comp.GetOnFocus().Insert(OnItemFocused);
		
		// Check if root is actually a button
		ButtonWidget button = ButtonWidget.Cast(w);

		if (!button)
			Print("Gallery element root has to be a button, otherwise interactivity will not work", LogLevel.WARNING);
		
		// Apply spacing
		int left, top, right, bottom;
		
		if (column > 0) 
			left = m_iPaddingHorizontal;
		
		if (column < m_iColumns - 1)
			right = m_iPaddingHorizontal;
		
		if (row > 0)
			top = m_iPaddingVertical;
		
		if (row < m_iRows - 1)
			bottom = m_iPaddingVertical;
		
		AlignableSlot.SetPadding(w, left, top, right, bottom);

		m_aWidgets.Insert(w);		
		
		return w;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetAnimationRate()
	{
		if (m_fAnimationTime <= 0)
			m_fAnimationRate = 1000;
		else
			m_fAnimationRate = 1 / m_fAnimationTime;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnItemFocused(Widget w)
	{
		m_iSelectedItem = m_aWidgets.Find(w);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocusedItem(int index)
	{
		if (!m_aWidgets.IsIndexValid(index))
			return;
		
		Widget w = m_aWidgets[index];
		
		if (!w.IsEnabled())
			return;
		
		GetGame().GetWorkspace().SetFocusedWidget(w);
		m_iSelectedItem = index;
	}
	
	//------------------------------------------------------------------------------------------------
	array<Widget> GetWidgets()
	{
		return m_aWidgets;
	}	

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_PlayMenuComponent GetComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		if (!parent || name == string.Empty)
			return null;
		
		Widget w;
		if (searchAllChildren)
			w = parent.FindAnyWidget(name);
		else
			w = parent.FindWidget(name);
		
		if (!w)
			return null;
			
		return SCR_PlayMenuComponent.Cast(w.FindHandler(SCR_PlayMenuComponent));
	}
};