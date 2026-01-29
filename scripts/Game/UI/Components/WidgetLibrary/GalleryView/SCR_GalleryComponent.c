class SCR_GalleryComponent : ScriptedWidgetComponent
{
	[Attribute("0")]
	protected int m_iInitialItemCount;
	
	protected int m_iSelectedItem;
	
	[Attribute("3")]
	protected int m_iCountShownItems;
	
	[Attribute("true", desc: "Should movement switch between whole pages, or just individual items?")]
	protected bool m_bMovePerPage;
	
	[Attribute("10")]
	protected float m_fSpacing;
		
	[Attribute("{02155A85F2DC521F}UI/layouts/Menus/PlayMenu/PlayMenuTile.layout", UIWidgets.ResourceNamePicker, "", "layout")]
	protected ResourceName m_sItemLayout;
	
	[Attribute("MouseWheelUp")]
	protected string m_sActionLeft;

	[Attribute("MouseWheelDown")]
	protected string m_sActionRight;
	
	[Attribute("true")]
	protected bool m_bShowPagingHints;

	[Attribute("false")]
	protected bool m_bCycleMode;

	[Attribute("0.2")]
	protected float m_fAnimationTime;
	
	[Attribute("true")]
	protected bool m_bShowNavigationArrows;
	
	[Attribute("Hint")]
	protected string m_sHintName;
	
	[Attribute("Content")]
	protected string m_sContentName;

	protected ref array<Widget> m_aWidgets = {};
	protected SCR_SelectionHintComponent m_Hint;
	protected SCR_PagingButtonComponent m_PagingLeft;
	protected SCR_PagingButtonComponent m_PagingRight;
	protected Widget m_wContentRoot;
	protected Widget m_wRoot;
	protected float m_fAnimationRate = SCR_WLibComponentBase.START_ANIMATION_RATE;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wContentRoot = w.FindAnyWidget(m_sContentName);
		m_wRoot = w;
		
		// Convert time to animation speed
		GetGame().GetCallqueue().CallLater(SetAnimationRate, SCR_WLibComponentBase.START_ANIMATION_PERIOD);
		
		if (m_iInitialItemCount > 0)
		{
			CreateWidgets(m_iInitialItemCount);
			SetupHints(m_iInitialItemCount, false);
			SetFocusedItem(m_iSelectedItem);
		}
		
		// Setup controls
		m_PagingLeft = SCR_PagingButtonComponent.GetPagingButtonComponent("PagingLeft", w);
		if (m_PagingLeft)
		{
			m_PagingLeft.m_OnClicked.Insert(OnNavigationLeft);
			m_PagingLeft.GetRootWidget().SetVisible(m_bShowNavigationArrows);
		}
		
		m_PagingRight = SCR_PagingButtonComponent.GetPagingButtonComponent("PagingRight", w);
		if (m_PagingRight)
		{
			m_PagingRight.m_OnClicked.Insert(OnNavigationRight);
			m_PagingRight.GetRootWidget().SetVisible(m_bShowNavigationArrows);
		}
		
		UpdatePagingButtons();
		
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnPreviousItem);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnNextItem);
		GetGame().GetInputManager().AddActionListener(m_sActionLeft, EActionTrigger.DOWN, OnCustomLeft);
		GetGame().GetInputManager().AddActionListener(m_sActionRight, EActionTrigger.DOWN, OnCustomRight);

		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		ShowPagingButtons();
	}
	
	
	// TODO: This method should be used everywhere, but is bugged!!!
	//------------------------------------------------------------------------------------------------
	void ShowPagingButtons(bool animate = true)
	{
		bool show = GetGame().GetInputManager().IsUsingMouseAndKeyboard() && m_aWidgets.Count() > m_iCountShownItems;
		
		if (m_PagingLeft)
			m_PagingLeft.SetVisible(show, animate);
		
		if (m_PagingRight)
			m_PagingRight.SetVisible(show, animate);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePagingButtons()
	{
		if (!m_PagingLeft || !m_PagingRight)
			return;
		
		ShowPagingButtons();

		// In cycle mode, if paging buttons are shown, they are always enabled
		if (m_bCycleMode)
		{
			m_PagingLeft.SetEnabled(true);
			m_PagingRight.SetEnabled(true);
			return;
		}

		int count = m_aWidgets.Count();
		int currentItem = m_iSelectedItem;
		if (m_bMovePerPage && m_iCountShownItems > 0)
		{
			count = Math.Ceil(count / m_iCountShownItems);
			currentItem = Math.Floor(m_iSelectedItem / m_iCountShownItems);
		}
		
		m_PagingLeft.SetEnabled(currentItem > 0);
		m_PagingRight.SetEnabled(currentItem < count - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupHints(int count, bool animate = true)
	{
		if (!m_Hint)
		{
			Widget hint = m_wRoot.FindAnyWidget(m_sHintName);
			if (hint)
				m_Hint = SCR_SelectionHintComponent.Cast(hint.FindHandler(SCR_SelectionHintComponent));
		}

		if (!m_Hint)
			return;
		
		int hintsCount = count;
		int currentItem = m_iSelectedItem;
		if (m_bMovePerPage && m_iCountShownItems > 0)
		{
			hintsCount = Math.Ceil(count / m_iCountShownItems);
			currentItem = Math.Ceil(m_iSelectedItem / m_iCountShownItems);
		}

		m_Hint.SetItemCount(hintsCount, animate);
		m_Hint.SetCurrentItem(currentItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateWidgets(int count)
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
		
		for (int i = 0; i < count; i++)
		{
			CreateWidget();
		}
		
		m_iSelectedItem = 0;
		ShowTiles(0);
	}

	//------------------------------------------------------------------------------------------------
	protected Widget CreateWidget()
	{
		if (!m_wContentRoot)
			return null;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sItemLayout, m_wContentRoot);
		if (!w)
			return null;
		
		SetupWidget(w);
		return w;
	}
	
	//------------------------------------------------------------------------------------------------
	private void SetupWidget(Widget w)
	{
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
		{
			Print("[SCR_GalleryComponent.SetupWidget] the gallery element root has to be a button," 
			+ "otherwise interactivity will not work", LogLevel.WARNING);
		}
		
		// Apply spacing
		float l, t, r, b;
		AlignableSlot.GetPadding(w, l, t, r, b);
		AlignableSlot.SetPadding(w, m_fSpacing * 0.5, t, m_fSpacing * 0.5, b);
		UpdatePagingButtons();

		m_aWidgets.Insert(w);
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


		int index = m_iSelectedItem;
		if (m_bMovePerPage && m_iCountShownItems > 0)
			index = Math.Floor(m_iSelectedItem / m_iCountShownItems);

		m_Hint.SetCurrentItem(index);
		UpdatePagingButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCustomLeft()
	{
		if (!m_wRoot.IsEnabledInHierarchy() || !m_wRoot.IsVisibleInHierarchy())
			return;
		
		int selectedItem;
		if (m_bMovePerPage)
			selectedItem = Math.Max(m_iSelectedItem - m_iCountShownItems, 0);
		else
			selectedItem = m_iSelectedItem - 1;

		SetFocusedItem(selectedItem);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnCustomRight()
	{
		if (!m_wRoot.IsEnabledInHierarchy() || !m_wRoot.IsVisibleInHierarchy())
			return;
		
		int selectedItem;
		if (m_bMovePerPage)
			selectedItem = Math.Min(m_iSelectedItem + m_iCountShownItems, m_aWidgets.Count() - 1);
		else
			selectedItem = m_iSelectedItem + 1;
				
		SetFocusedItem(selectedItem);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNavigationLeft()
	{
		if (!m_wRoot.IsEnabledInHierarchy() || !m_wRoot.IsVisibleInHierarchy())
			return;

		SetFocusedItem(m_iSelectedItem - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnNavigationRight()
	{
		if (!m_wRoot.IsEnabledInHierarchy() || !m_wRoot.IsVisibleInHierarchy())
			return;
		
		SetFocusedItem(m_iSelectedItem + 1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNextItem()
	{
		if (!m_wRoot.IsEnabledInHierarchy() || !m_wRoot.IsVisibleInHierarchy())
			return;
		
		int nextItem = m_iSelectedItem + 1;
		if (nextItem >= m_aWidgets.Count())
			return;
		
		if (!m_aWidgets[nextItem].IsEnabled())
			SetFocusedItem(nextItem);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPreviousItem()
	{
		if (!m_wRoot.IsEnabledInHierarchy() || !m_wRoot.IsVisibleInHierarchy())
			return;
		
		int previousItem = m_iSelectedItem - 1;
		if (previousItem < 0)
			return;
		
		if (!m_aWidgets[previousItem].IsEnabled())
			SetFocusedItem(previousItem);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFocusedItem(int index, bool force = false)
	{
		index = Math.ClampInt(index, 0, m_aWidgets.Count() - 1);
		if ((!force && m_iSelectedItem == index) || index < 0)
			return;

		bool moveRight;
		if (m_iSelectedItem < index)
			moveRight = true;
		
		m_iSelectedItem = index;
		//if (m_Hint)
			//m_Hint.SetCurrentItem(m_iSelectedItem);

		if (!m_aWidgets[m_iSelectedItem].IsEnabled())
		{
			int firstShownTile = m_iSelectedItem;
			if (m_bMovePerPage && m_iCountShownItems > 0)
			{
				firstShownTile = Math.Floor(firstShownTile / m_iCountShownItems) * m_iCountShownItems;
			}
			else
			{
				if (moveRight)
					firstShownTile = m_iSelectedItem - m_iCountShownItems + 1;
			}
			
			ShowTiles(firstShownTile);
		}
		
		GetGame().GetWorkspace().SetFocusedWidget(m_aWidgets[m_iSelectedItem]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowTiles(int startingIndex, bool animate = true)
	{
		foreach (int i, Widget w : m_aWidgets)
		{
			bool setVisible = i >= startingIndex && i < (startingIndex + m_iCountShownItems);
			w.SetEnabled(setVisible);
			w.SetVisible(setVisible);
		}
		
		// TODO: Implement any animation for the widget gallery
	}
	
	/*
	//------------------------------------------------------------------------------------------------
	protected void ExpandWidget(Widget widget, bool expand, bool animate = true)
	{
		if (!widget)
			return;

		if (animate && m_fAnimationRate < 1000)
		{
			LayoutSlot.SetFillWeight(widget, !expand);
			AnimateWidget.LayoutFill(widget, expand, m_fAnimationRate);
		}
		else
		{
			LayoutSlot.SetFillWeight(widget, !expand);
		}
	}
	*/

	// Public API
	//------------------------------------------------------------------------------------------------
	int GetWidgets(out array<Widget> widgets)
	{
		if (widgets)
			widgets = m_aWidgets;

		return m_aWidgets.Count();
	}

	//------------------------------------------------------------------------------------------------
	Widget AddItem()
	{
		Widget w = CreateWidget();
		ShowTiles(m_iSelectedItem);
		SetupHints(m_aWidgets.Count());
		return w;
	}
	
	//------------------------------------------------------------------------------------------------
	int AddItem(Widget widget)
	{
		if (!widget)
			return -1;
		
		m_wContentRoot.AddChild(widget);
		SetupWidget(widget);
		ShowTiles(m_iSelectedItem);

		int count = m_aWidgets.Count();
		SetupHints(count);
		return count;
	}

	//------------------------------------------------------------------------------------------------
	void ClearAll()
	{
		CreateWidgets(0);
		SetupHints(0);
	}

	//------------------------------------------------------------------------------------------------
	void RemoveItem(int item)
	{
		if (item < 0 || item >= m_aWidgets.Count())
			return;
		
		Widget w = m_aWidgets[item];
		w.RemoveFromHierarchy();
		m_aWidgets.Remove(item);
		
		SetupHints(m_aWidgets.Count());
	}

	//------------------------------------------------------------------------------------------------
	void SetCurrentItem(int i, bool animate = false)
	{
		SetFocusedItem(i);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_GalleryComponent GetGalleryComponent(string name, Widget parent, bool searchAllChildren = true)
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
			
		SCR_GalleryComponent comp = SCR_GalleryComponent.Cast(w.FindHandler(SCR_GalleryComponent));
		return comp;
	}
};