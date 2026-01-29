//------------------------------------------------------------------------------------------------
class SCR_ToolboxComponent : SCR_SelectionWidgetComponent
{
	protected Widget m_wButtonRow;
	
	[Attribute()]
	bool m_bAllowMultiselection;
	
	[Attribute("{D27D044A8145DC7C}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonTextUnderlined.layout", UIWidgets.ResourceNamePicker, "Layout element used", "layout")]
	ResourceName m_ElementLayout;
	
	[Attribute("4")]
	float m_fElementSpacing;

	[Attribute("false", UIWidgets.CheckBox, "On last item and pressing right arrow, it will go to the start of the list")]
	bool m_bCycleMode;

	ref array<SCR_ButtonBaseComponent> m_aSelectionElements = new array<SCR_ButtonBaseComponent>;
	SCR_ButtonBaseComponent m_FocusedElement;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wButtonRow = w.FindAnyWidget("ButtonRow");
		
		if (!m_wButtonRow || !m_ElementLayout)
			return;

		CreateWidgets();
		SetInitialState();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateWidgets()
	{
		
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Exposed an attribute, and then in the only place where it is used, changes it's value
		
		float padding = m_fElementSpacing * 0.5;
		
//---- REFACTOR NOTE END ----
		
		LayoutSlot.SetPadding(m_wButtonRow, -padding, -padding, -padding, -padding);
		foreach (string name : m_aElementNames)
		{
			Widget button = GetGame().GetWorkspace().CreateWidgets(m_ElementLayout, m_wButtonRow);
			LayoutSlot.SetPadding(button, padding, padding, padding, padding);
			LayoutSlot.SetSizeMode(button, LayoutSizeMode.Fill);
			LayoutSlot.SetVerticalAlign(button, LayoutHorizontalAlign.Stretch);
			LayoutSlot.SetHorizontalAlign(button, LayoutHorizontalAlign.Stretch);

			SCR_ButtonBaseComponent comp = SCR_ButtonBaseComponent.Cast(button.FindHandler(SCR_ButtonBaseComponent));
			if (!comp)
				continue;

			m_aSelectionElements.Insert(comp);
			SCR_ButtonTextComponent textComp = SCR_ButtonTextComponent.Cast(comp);
			if (textComp)
				textComp.SetText(name);
			comp.m_OnClicked.Insert(OnElementClicked);
			comp.m_OnToggled.Insert(OnElementChanged);
		}
		
		SetInitialState();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ClearWidgets()
	{
		foreach (SCR_ButtonBaseComponent element : m_aSelectionElements)
		{
			if (!element)
				continue;
			
			Widget w = element.GetRootWidget();
			if (w)
				w.RemoveFromHierarchy();
		}
		
		m_aSelectionElements.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		
		if (m_FocusedElement)
		{
			m_FocusedElement.ShowBorder(true);
		}
		else if (m_aSelectionElements.Count() > 0)
		{
			m_aSelectionElements[0].ShowBorder(true);
			m_FocusedElement = m_aSelectionElements[0];
		}
		
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnMenuLeft);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnMenuRight);
		
		if (m_bAllowMultiselection)
			GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, OnMenuSelect);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		
		if (m_FocusedElement)
			m_FocusedElement.ShowBorder(false);
		
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnMenuLeft);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnMenuRight);
		
		if (m_bAllowMultiselection)
			GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_SELECT, EActionTrigger.DOWN, OnMenuSelect);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetInitialState()
	{
		m_FocusedElement = GetFirstSelectedIndex();
		int realIndex = m_iSelectedItem;
		m_iSelectedItem = -int.MAX;
		SetCurrentItem(realIndex, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuSelect()
	{
		if (m_FocusedElement)
			m_FocusedElement.SetToggled(!m_FocusedElement.IsToggled());
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ButtonBaseComponent GetFirstSelectedIndex()
	{
		foreach (int i, SCR_ButtonBaseComponent element : m_aSelectionElements)
		{
			if (element.IsToggled())
				return element;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnElementClicked(SCR_ButtonBaseComponent comp)
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);
		int i = m_aSelectionElements.Find(comp);
		
		if (m_bAllowMultiselection)
			SetFocusedItem(comp);
		else
		{
			if (i == m_iSelectedItem)
			{
				comp.SetToggled(!comp.IsToggled(), false, false);
				m_iSelectedItem = i;
			}
			else
			{
				SetCurrentItem(i, true, false);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnElementChanged(SCR_ButtonBaseComponent comp, bool state)
	{
		// Do not invoke on false state when not having a multiselection
		if (!m_bAllowMultiselection && !state)
			return;
		
		int i = m_aSelectionElements.Find(comp);
		if (m_bAllowMultiselection)
			m_OnChanged.Invoke(this, i, state);
		else
			m_OnChanged.Invoke(this, i);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetCurrentItem(int i, bool playSound = false, bool animate = false)
	{
		int oldIndex = m_iSelectedItem;
		int itemsCount = m_aSelectionElements.Count();
		if (!super.SetCurrentItem(i, true, true))
			return false;
		
		if (!m_bAllowMultiselection && oldIndex >= 0 && oldIndex < itemsCount)
		{
			m_aSelectionElements[oldIndex].SetToggled(false, playSound, animate);
		}
		
		if (i >= 0 && i < itemsCount)
		{
			m_aSelectionElements[i].SetToggled(true, playSound, animate);
			//m_OnChanged.Invoke(this, m_aSelectionElements[i].GetRootWidget(), i);
			SetFocusedItem(m_aSelectionElements[i]);
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		if (IsChildWidget(w, WidgetManager.GetWidgetUnderCursor()))
			super.OnMouseEnter(w, x, y);
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuLeft()
	{
		int i = GetNextValidItem(true);
		if (i < 0)
			return;
		
		if (m_bAllowMultiselection)
			SetFocusedItem(i);
		else
			SetCurrentItem(i, true, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuRight()
	{
		int i = GetNextValidItem(false);
		if (i < 0)
			return;
		
		if (m_bAllowMultiselection)
			SetFocusedItem(i);
		else
			SetCurrentItem(i, true, true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetNextValidItem(bool toLeft)
	{
		if (m_aSelectionElements.IsEmpty())
			return -1;

		SCR_ButtonBaseComponent selectedElement;
		int lastIndex = m_aSelectionElements.Count() - 1;

		int i = m_aSelectionElements.Find(m_FocusedElement);
		int foundItem = -1;
		int nextItem = 1;
		if (toLeft)
			nextItem = -1;
		
		while (foundItem < 0)
		{
			i += nextItem;
			if (i < 0)
			{
				if (m_bCycleMode)
					i = lastIndex;
				else
					return -1;
			
			}
			else if (i > lastIndex)
			{
				if (m_bCycleMode)
					i = 0;
				else
					return -1;
			}
			
			if (m_FocusedElement == m_aSelectionElements[i])
				return -1; // Went through all elements, found nothing

			if (m_aSelectionElements[i].IsEnabled())
				foundItem = i;
		}
		return foundItem;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetItems(notnull array<SCR_ButtonBaseComponent> elements)
	{
		elements.Clear();
		
		foreach (SCR_ButtonBaseComponent element: m_aSelectionElements)
			elements.Insert(element);
		
		return elements.Count();
	}
	
	SCR_ButtonBaseComponent GetItem(int index)
	{
		if (index < 0 || index >= m_aSelectionElements.Count())
			return null;
		
		return 	m_aSelectionElements[index];
	}
	
	//------------------------------------------------------------------------------------------------
	override int AddItem(string item, bool last = false, Managed data = null)
	{
		int i = super.AddItem(item, last, data);
		ClearWidgets();
		CreateWidgets();
		return i;
	}
	
	//------------------------------------------------------------------------------------------------
	override void ClearAll()
	{
		super.ClearAll();
		ClearWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	override void RemoveItem(int item, bool last = false)
	{
		super.RemoveItem(item, last);
		ClearWidgets();
		CreateWidgets();
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsItemSelected(int index)
	{
		if (index < 0 || index >= m_aSelectionElements.Count())
			return false;
		
		return m_aSelectionElements[index].IsToggled();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetItemSelected(int index, bool select, bool animate = true)
	{
		if (index < 0 || index >= m_aSelectionElements.Count())
			return;
		
		m_iSelectedItem = index;
		m_aSelectionElements[index].SetToggled(select, animate, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocusedItem(int index, bool animate = true)
	{
		if (index < 0 || index >= m_aSelectionElements.Count())
			return;
		
		SCR_ButtonBaseComponent element = m_aSelectionElements[index];
		if (element == m_FocusedElement)
			return;
		
		bool focused = GetGame().GetWorkspace().GetFocusedWidget() == m_wRoot;

		if (m_FocusedElement && focused)
			m_FocusedElement.ShowBorder(false, animate);
		
		m_FocusedElement = element;
		
		if (focused)
			m_FocusedElement.ShowBorder(true, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocusedItem(SCR_ButtonBaseComponent element, bool animate = true)
	{
		if (!element || element == m_FocusedElement)
			return;
		
		bool focused = GetGame().GetWorkspace().GetFocusedWidget() == m_wRoot;
		
		if (m_FocusedElement && focused)
			m_FocusedElement.ShowBorder(false, animate);
		
		m_FocusedElement = element;
		
		if (focused)
			m_FocusedElement.ShowBorder(true, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	int GetFocusedItem()
	{
		return m_aSelectionElements.Find(m_FocusedElement);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_ToolboxComponent GetToolboxComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_ToolboxComponent.Cast(
			SCR_WLibComponentBase.GetComponent(SCR_ToolboxComponent, name, parent, searchAllChildren)
		);
		return comp;
	}
};