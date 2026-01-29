class SCR_SpinBoxComponent : SCR_SelectionWidgetComponent
{
	protected static const ref Color COLOR_HINT_SELECTED = UIColors.CONTRAST_COLOR;
	protected static const ref Color COLOR_HINT_DESELECTED = UIColors.WHITE_HOVERED;

	protected SCR_PagingButtonComponent m_ButtonLeft;
	protected SCR_PagingButtonComponent m_ButtonRight;
	protected TextWidget m_wText;
	protected Widget m_wContent;
	protected Widget m_wCountBar;
	protected ref array<Widget> m_aHintElements = new array<Widget>();
	
	[Attribute("false", UIWidgets.CheckBox, "use light grey arrows instead of big yellow ones")]
	protected bool m_bUseLightArrows;

	[Attribute("false", UIWidgets.CheckBox, "On last item and pressing right arrow, it will go to the start of the list")]
	protected bool m_bCycleMode;

	[Attribute("true", UIWidgets.CheckBox, "Show bar of available elements and which one is selected.")]
	protected bool m_bShowHints;

	[Attribute("24", UIWidgets.EditBox, "Width of a hint element")]
	protected float m_fHintElementWidth;

	[Attribute("4", UIWidgets.EditBox, "Height of a hint element")]
	protected float m_fHintElementHeight;

	[Attribute("1", UIWidgets.EditBox, "How much wider should be the selected hint element")]
	protected float m_fHintSelectedWidthMultiplier;

	[Attribute("true", UIWidgets.CheckBox, "Should hints fill the available space")]
	protected bool m_fHintFillMode;

	[Attribute("4", UIWidgets.EditBox, "How large gaps between hints should there be")]
	protected float m_fHintSpacing;

	[Attribute("", UIWidgets.EditBox, "")]
	protected ResourceName m_sHintElementTexture;

	[Attribute("", UIWidgets.EditBox, "")]
	protected ResourceName m_sHintElementImage;
	
	[Attribute("HintBarElement", UIWidgets.EditBox, "Name for generated Hint bar widgets")]
	protected string m_sHintBarElementName;
	
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Old untyped invokers
	
	protected ref ScriptInvoker m_OnLeftArrowClick;
	protected ref ScriptInvoker m_OnRightArrowClick;

//---- REFACTOR NOTE END ----
	
	protected bool m_bHasActionListeners;
	protected bool m_bAllowSwitchingWithoutFocus;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wText = TextWidget.Cast(w.FindAnyWidget("SelectionText"));
		m_wContent = w.FindAnyWidget("Overlay");
		m_wCountBar = w.FindAnyWidget("HintBar");

		// Buttons
		Widget left = w.FindAnyWidget("ButtonLeft");
		Widget right = w.FindAnyWidget("ButtonRight");

		if (left)
		{
			m_ButtonLeft = SCR_PagingButtonComponent.Cast(left.FindHandler(SCR_PagingButtonComponent));
			if (m_ButtonLeft)
				m_ButtonLeft.m_OnActivated.Insert(OnLeftArrowClick);
		}

		if (right)
		{
			m_ButtonRight = SCR_PagingButtonComponent.Cast(right.FindHandler(SCR_PagingButtonComponent));
			if (m_ButtonRight)
				m_ButtonRight.m_OnActivated.Insert(OnRightArrowClick);
		}

		if (m_wText)
		{
			if (m_aElementNames && m_iSelectedItem > -1 && m_iSelectedItem < m_aElementNames.Count())
				m_wText.SetText(m_aElementNames[m_iSelectedItem]);
			else
				m_wText.SetText(string.Empty);
		}

		SetInitialState(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);

		AddActionListeners();

		UpdateHintBar(m_iSelectedItem, -1);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);

		if (!m_bAllowSwitchingWithoutFocus)
			RemoveActionListeners();

		UpdateHintBar(m_iSelectedItem, -1);
		return false;
	}

	//TODO: revise this logic: there should be no reason to reset current item every time a new one is added or removed	
	//------------------------------------------------------------------------------------------------
	override int AddItem(string item, bool last = false, Managed data = null)
	{
		int i = super.AddItem(item, last, data);

		SetInitialState(last);
		return i;
	}

	//------------------------------------------------------------------------------------------------
	override void RemoveItem(int item, bool last = false)
	{
		super.RemoveItem(item, last);
		SetInitialState(last);
	}

	//------------------------------------------------------------------------------------------------
	override void ClearAll()
	{
		super.ClearAll();
		SetInitialState(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool SetCurrentItem(int i, bool playSound = false, bool animate = false)
	{
		return SetCurrentItem_Internal(i, playSound, animate, true);
	}

	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	protected bool SetCurrentItem_Internal(int i, bool playSound, bool animate, bool invokeOnChanged)
	{
		int lastIndex = m_iSelectedItem;
		if (!super.SetCurrentItem(i, playSound, animate))
			return false;

		if (m_wText)
			m_wText.SetText(m_aElementNames[i]);

		if (m_bShowHints)
			UpdateHintBar(i, lastIndex);

		EnableArrows(i, animate);
		
		if (invokeOnChanged)
			m_OnChanged.Invoke(this, m_iSelectedItem);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateHintBar()
	{
		// Delete any old elements
		foreach (Widget w : m_aHintElements)
		{
			w.RemoveFromHierarchy();
		}
		m_aHintElements.Clear();

		m_wCountBar.SetVisible(true);
		
		for (int i = 0, len = m_aElementNames.Count(); i < len; i++)
		{
			Widget w = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.STRETCH | WidgetFlags.BLEND | WidgetFlags.INHERIT_CLIPPING, Color.FromInt(Color.WHITE), 0, m_wCountBar);
			ImageWidget img = ImageWidget.Cast(w);
			if (!img)
				break;

			SetTexture(img, m_sHintElementTexture, m_sHintElementImage);
			// Force size only when sizes are more than 0 - automatic sizing
			if (m_fHintElementWidth >= 0 && m_fHintElementHeight >= 0)
				img.SetSize(m_fHintElementWidth, m_fHintElementHeight);

			img.SetColor(COLOR_HINT_DESELECTED);
			HorizontalLayoutSlot.SetPadding(img, m_fHintSpacing * 0.5, 0, m_fHintSpacing * 0.5, 0);
			HorizontalLayoutSlot.SetHorizontalAlign(img, LayoutHorizontalAlign.Stretch);

			// Use fill instead
			if (m_fHintFillMode)
				HorizontalLayoutSlot.SetSizeMode(img, LayoutSizeMode.Fill);

			m_aHintElements.Insert(img);
			img.SetName(m_sHintBarElementName + i);
		}
		
		UpdateHintBar(m_iSelectedItem, -1);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateHintBar(int currentIndex, int oldIndex)
	{
		if (!m_bShowHints)
			return;

		bool focused = GetGame().GetWorkspace().GetFocusedWidget() == m_wRoot;

		int count = m_aHintElements.Count();
		if (oldIndex > -1 && oldIndex < count)
		{
			AnimateWidget.Color(m_aHintElements[oldIndex], COLOR_HINT_DESELECTED, m_fAnimationRate);
			if (m_fHintSelectedWidthMultiplier != 1)
				AnimateWidget.LayoutFill(m_aHintElements[oldIndex], 1, m_fAnimationRate);
		}

		if (currentIndex > -1 && currentIndex < count)
		{
			AnimateWidget.Color(m_aHintElements[currentIndex], COLOR_HINT_SELECTED, m_fAnimationRate);
			if (m_fHintSelectedWidthMultiplier != 1)
				AnimateWidget.LayoutFill(m_aHintElements[currentIndex], m_fHintSelectedWidthMultiplier, m_fAnimationRate);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetInitialState(bool invokeOnChanged = true)
	{
		int realIndex = m_iSelectedItem;
		if (realIndex < 0 || realIndex >= m_aElementNames.Count())
			realIndex = 0;

		m_iSelectedItem = int.MIN;
		SetCurrentItem_Internal(realIndex, false, false, invokeOnChanged);

		if (m_bShowHints && m_aElementNames && m_wCountBar)
			CreateHintBar();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Based on cycle mode set which arrow should be enabled at given selection
	protected void EnableArrows(int selected, bool animate)
	{
		if (!m_ButtonLeft || !m_ButtonRight)
			return;
		
		if (m_bCycleMode)
		{
			bool enabled = m_aElementNames.Count() > 1;
			m_ButtonLeft.SetEnabled(enabled, animate);
			m_ButtonRight.SetEnabled(enabled, animate);
		}
		else
		{
			m_ButtonLeft.SetEnabled(selected != 0, animate);
			m_ButtonRight.SetEnabled(selected != (m_aElementNames.Count() - 1), animate);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLeftArrowClick()
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);

		if (m_iSelectedItem <= 0)
		{
			if (m_bCycleMode)
				SetCurrentItem(m_aElementNames.Count() - 1, true, true);
		}
		else
		{
			SetCurrentItem(m_iSelectedItem - 1, true, true);
		}

		if (m_OnLeftArrowClick)
			m_OnLeftArrowClick.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRightArrowClick()
	{
		GetGame().GetWorkspace().SetFocusedWidget(m_wRoot);

		if (m_iSelectedItem >= (m_aElementNames.Count() - 1))
		{
			if (m_bCycleMode)
				SetCurrentItem(0, true, true);
		}
		else
		{
			SetCurrentItem(m_iSelectedItem + 1, true, true);
		}

		if (m_OnRightArrowClick)
			m_OnRightArrowClick.Invoke();
	}

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Directly calling OnClick, even if it's public, feels rather hacky, as it is triggered by the C++ widget classes, and here we do not even provide proper data in params
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuLeft()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_wRoot && !m_bAllowSwitchingWithoutFocus)
			return;

		if (m_ButtonLeft && m_ButtonLeft.IsEnabled())
			m_ButtonLeft.OnClick(m_ButtonLeft.m_wRoot, 0, 0, 0); // TODO: Replace with other function, which accepts more params (turn of anims and sounds separately)
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuRight()
	{
		if (GetGame().GetWorkspace().GetFocusedWidget() != m_wRoot && !m_bAllowSwitchingWithoutFocus)
			return;

		if (m_ButtonRight && m_ButtonRight.IsEnabled())
			m_ButtonRight.OnClick(m_ButtonRight.m_wRoot, 0, 0, 0); // TODO: Replace with other function, which accepts more params (turn of anims and sounds separately)
	}
	
//---- REFACTOR NOTE END ----

	// --- Public ---
	//------------------------------------------------------------------------------------------------
	void AddActionListeners()
	{
		if (m_bHasActionListeners)
			return;
		
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnMenuLeft);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnMenuRight);
		
		m_bHasActionListeners = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void RemoveActionListeners()
	{
		if (!m_bHasActionListeners)
			return;
		
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_LEFT, EActionTrigger.DOWN, OnMenuLeft);
		GetGame().GetInputManager().RemoveActionListener(UIConstants.MENU_ACTION_RIGHT, EActionTrigger.DOWN, OnMenuRight);
		
		m_bHasActionListeners = false;
	}
	
	//------------------------------------------------------------------------------------------------
	// Determines if left/right page switch action listeners should be removed on focus lost
	void SetKeepActionListeners(bool keep)
	{
		m_bAllowSwitchingWithoutFocus = keep;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCycleMode(bool cycle)
	{
		m_bCycleMode = cycle;
		EnableArrows(m_iSelectedItem, false);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnLeftArrowClick()
	{
		if (!m_OnLeftArrowClick)
			m_OnLeftArrowClick = new ScriptInvoker();

		return m_OnLeftArrowClick;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnRightArrowClick()
	{
		if (!m_OnRightArrowClick)
			m_OnRightArrowClick = new ScriptInvoker();

		return m_OnRightArrowClick;
	}

	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_SpinBoxComponent GetSpinBoxComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		return SCR_SpinBoxComponent.Cast(SCR_ScriptedWidgetComponent.GetComponent(SCR_SpinBoxComponent, name, parent, searchAllChildren));
	}
}
