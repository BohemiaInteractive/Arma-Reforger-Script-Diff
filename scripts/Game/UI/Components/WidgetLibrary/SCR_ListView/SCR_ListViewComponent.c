/*
Scrollable list view with static count of entries.
*/

//------------------------------------------------------------------------------------------------
class SCR_ListViewComponent : ScriptedWidgetComponent
{
	// Constant widget names 
	const string WIDGET_VERTICAL_LIST = "VList";
	const string WIDGET_SIZE_SCROLL_OFFSET = "SizeScrollOffset";
	const string WIDGET_FOCUS_REST = "BtnFocusRest";
	
	// Entries
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Entry widget that list is filled with.")]
	protected ResourceName m_sEntry;
	
	[Attribute("MouseWheel", UIWidgets.EditBox, desc: "Reference for scrolling action")] 
	protected string m_sScrollAction;
	
	// Entries properties
	[Attribute("0", UIWidgets.CheckBox, desc: "True will automatically set entry limit value by list size and entry size")] 
	protected bool m_bAutomaticEntriesLimit;
	
	[Attribute("10", UIWidgets.EditBox, desc: "Number of item that should be crated to the list, that player is able to see in list")] 
	protected int m_iEntriesLimit;

	[Attribute("4", UIWidgets.EditBox, desc: "Space between each entry")] 
	protected int m_iEntriesBottomPadding;
	
	[Attribute("1", UIWidgets.EditBox, desc: "How many entries are move in one scroll")] 
	protected float m_fScrollMove;
	
	[Attribute("1", UIWidgets.EditBox, desc: "Time for apearing animation")] 
	protected float m_fAnimationAppearTime;
	
	// widgets 
	Widget m_wRoot;
	protected Widget m_wVerticalList;
	protected SizeLayoutWidget m_wSizeScrollOffset;
	protected Widget m_wFocusRest;
	
	// Server entries data 
	protected ref array<Widget> m_aEntryWidgets = new array<Widget>;
	
	// Hadling conponents 
	protected SCR_ScrollBarComponent m_Scrollbar;
	
	// Scroll properties 
	protected float m_fScrollPosition = 0; // Whole number = entry id on top, decimal point = offset between whole step 
	protected float m_iLastScrollPosition = 0;
	protected float m_iEntriesCount = 0;
	protected float m_fEntryWidgetHeight; 
	
	protected int m_iFocusedEntryId = 0;
	
	protected bool m_bIscrollActive;
	protected bool m_bAnimateListAppearing;
	
	protected bool m_bCreateList = false;
	
	// Animations properties 
	
	//-------------------------------------
	// ScriptedWidgetComponent override
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Accessing handlers and widgets 
		m_wRoot = w;
		AccessingHandlers();	
		
		CreateEntriesWidgets();
		
		// Setup inputs invokers 
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_DOWN, EActionTrigger.DOWN, OnMenuDown);
		GetGame().GetInputManager().AddActionListener(UIConstants.MENU_ACTION_UP, EActionTrigger.DOWN, OnMenuUp);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnUpdate(Widget w)
	{	
		super.OnUpdate(w);
		
		if (m_bCreateList)
		{
			PostCheck();
			//UpdateEntries();

			//m_bCreateList = false;
		}
			
		return false;
	}

	//-------------------------------------
	// public functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Fill entries with data 
	void UpdateEntries(bool animated = false)
	{
		m_bAnimateListAppearing = animated;
		
		// Fill list with data 
		foreach (Widget w : m_aEntryWidgets)
		{
			if (w)
				FillEntry(w);
		}
		
		// Hide scrollbar if list is not overfloating
		float scrollsCount = ScrollableEntriesCount();
		SetScrollbarVisible(scrollsCount != -1);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetScrollbarVisible(bool visible)
	{
		if (m_Scrollbar)
			m_Scrollbar.GetRootWidget().SetVisible(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find first available entry and focus it 
	void FocusFirstAvailableEntry()
	{
		// Get and check widget 
		Widget availableEntry = FirstAvailableEntry();
		if (!availableEntry)
			return;
		
		// Focus 
		GetGame().GetWorkspace().SetFocusedWidget(null);
		GetGame().GetWorkspace().SetFocusedWidget(availableEntry);
	}
	
	//-------------------------------------
	// protected functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Get reference to all needed component for handling 
	protected void AccessingHandlers() 
	{
		// Get Widgets 
		m_wVerticalList = m_wRoot.FindWidget(WIDGET_VERTICAL_LIST);
		m_wSizeScrollOffset = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SIZE_SCROLL_OFFSET));
		m_wFocusRest = m_wRoot.FindAnyWidget(WIDGET_FOCUS_REST);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create server entries widget 
	protected void CreateEntriesWidgets()
	{
		if(!m_wRoot || m_sEntry.IsEmpty())
			return;
		
		// Create first entry
		CreateEntry();
		//m_bCreateList = true;
		///CheckEntrySize();
		
		// Allow create rest of entries 
		
		//GetGame().GetCallqueue().CallLater(PostCheck, 500);
		
		// Auto calculate limit 
		/*if (m_bAutomaticEntriesLimit)
			m_iEntriesLimit = CalculateLimit();*/
		
		// Create server entry widgets 		
		for (int i = 1; i <  m_iEntriesLimit; i++)
		{
			CreateEntry();
		}
		
		// Focus on first entry 
		if(m_aEntryWidgets.Count() > 0)
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntryWidgets[0]);
	}	
	
	//------------------------------------------------------------------------------------------------
	protected void PostCheck()
	{
		CheckEntrySize();
		
		// Check size 
		if (m_fEntryWidgetHeight == m_iEntriesBottomPadding)
			return;
		
		int limitPrev = m_iEntriesLimit;
		
		// Auto calculate limit 
		if (m_bAutomaticEntriesLimit)
			m_iEntriesLimit = CalculateLimit() + 1;
		
		// Remove widgets over limit		
		/*for (int i = limitPrev - 1; i > m_iEntriesLimit; i--)
		{
			m_aEntryWidgets[i].RemoveFromHierarchy();
			m_aEntryWidgets.RemoveItem(m_aEntryWidgets[i]);
		}*/
		
		for (int i = 0; i < m_iEntriesLimit; i++)
		{
			CreateEntry();
		}
		
		m_bCreateList = false;
		
		// Focus on first entry 
		if(m_aEntryWidgets.Count() > 0)
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntryWidgets[0]);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateEntry()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (!m_sEntry || !m_wVerticalList || !workspace)
			return;
		
		Widget widget = workspace.CreateWidgets(m_sEntry, m_wVerticalList);
		if (!widget)
			return;
		
		// Setup widget entry behavior 
		SetupEntryBehavior(widget);
		
		// Save in widget list  
		m_aEntryWidgets.Insert(widget);
		
		// Set padding 
		VerticalLayoutSlot.SetPadding(widget, 0, 0, 0, m_iEntriesBottomPadding);
		
		// Visible in editor 
		widget.SetVisible(!GetGame().InPlayMode()); 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Calulate entry limit number from list size and entry size 
	protected int CalculateLimit()
	{
		float wrapHeight, x;
		m_wVerticalList.GetScreenSize(x, wrapHeight);
		int entryH = m_fEntryWidgetHeight + m_iEntriesBottomPadding;
		
		int res = Math.Ceil(wrapHeight / entryH);
		return res;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get fraction of whole entry in percents from positon in list view in percent based on scroll postion 
	//! EntryFractionFromViewPos(1, ) = how much of entry is missing from bottom view 
	protected float EntryFractionFromViewPos(float viewPos, float scrollPos)
	{
		// Wrap 
		float wrapHeight, x;
		m_wVerticalList.GetScreenSize(x, wrapHeight);
		
		// Entry 
		int entryH = m_fEntryWidgetHeight + m_iEntriesBottomPadding;
		
		float entriesLimit = wrapHeight / entryH;
		
		float entryFraction = (entriesLimit - Math.Floor(entriesLimit));
		entriesLimit -= m_iEntriesBottomPadding*2 / m_fEntryWidgetHeight / entriesLimit; 
		//float scrollFracion = scrollPos
		
		return entryFraction;
	}
	
	const int ENTRY_FIRST = 0;
	
	//------------------------------------------------------------------------------------------------
	//! Get size of widget entry used in list 
	protected void CheckEntrySize()
	{
		if (m_iEntriesLimit > ENTRY_FIRST)
		{
			float x;
			m_aEntryWidgets[ENTRY_FIRST].GetScreenSize(x, m_fEntryWidgetHeight);
			m_fEntryWidgetHeight += m_iEntriesBottomPadding;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Empty functions for setting up widget entry beavhior actions, callbacks, etc.
	//! Override this to assign specific beavhior for each entry 
	protected void SetupEntryBehavior(Widget entry) { }
	
	//------------------------------------------------------------------------------------------------
	//! Empty function for filling entries with data 
	//! Override this to fill entries with data 
	protected void FillEntry(Widget w) {}
	
	//------------------------------------------------------------------------------------------------
	//! Modify offset size to fake scrolling by scroll position
	protected void ScrollOffsetMove(float scrollPos)
	{
		// Check first entry height if = 0 
		if (m_fEntryWidgetHeight == 0)
			CheckEntrySize();
		
		// Get offset from scroll position
		float scrollsCount = ScrollableEntriesCount();
		
		// Prevent scroll if there are no entries to scroll 
		if (scrollsCount == -1)
			return;
		
		scrollPos *= scrollsCount; 
		float offset = scrollPos - Math.Floor(scrollPos);
		offset = offset * m_fEntryWidgetHeight;
		
		// Move list if offset is overflown
		if (m_iLastScrollPosition != Math.Floor(scrollPos))
		{	
			m_iLastScrollPosition = Math.Floor(scrollPos);
			UpdateEntries();
		}
		
		// Move list offset
		m_wSizeScrollOffset.SetHeightOverride(-offset);
		
		// Save scroll position
		m_fScrollPosition = scrollPos;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Scroll list based on input action 
	protected void ScrollWheelMove()
	{
		if (!m_bIscrollActive)
			return;
		
		// Check scroll input value
		float scrollDir = GetGame().GetInputManager().GetActionValue(m_sScrollAction);
		scrollDir = Math.Clamp(scrollDir, -1, 1);

		// Scroll if any move
		if (scrollDir != 0)
		{
			ScrollList(m_fScrollMove, -scrollDir);
			
			StayOnLastEntry();
		}
		
		// Repeat scrolling 
		GetGame().GetCallqueue().CallLater(ScrollWheelMove, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Kepp focus on last entry on scroll
	protected void StayOnLastEntry()
	{
		int focusedIdInList = m_iFocusedEntryId - Math.Floor(m_fScrollPosition);
		
		if (focusedIdInList > -1 && focusedIdInList < m_aEntryWidgets.Count())
			ChangeFocusWithoutAnimation(m_aEntryWidgets[focusedIdInList]);
		else
			GetGame().GetWorkspace().SetFocusedWidget(m_wFocusRest);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ScrollList(float step, float input)
	{
		float scrollsCount = ScrollableEntriesCount();
		float move = 0;
		
		if (m_bIscrollActive)
		{
			move = ScrollMove(step, input);
			ScrollOffsetMove(move);
			
			// Update scroll position 
			m_fScrollPosition = move * scrollsCount;
		}
		
		if (m_Scrollbar)
			m_Scrollbar.MoveHandlerPos(move);
	}
	
	//------------------------------------------------------------------------------------------------
	protected float ScrollMove(float step, float inputMove)
	{
		float scrollsCount = ScrollableEntriesCount();
		
		float pos = step / scrollsCount * inputMove;
		
		// Move scrollbar 
		pos += m_fScrollPosition / scrollsCount; 
		pos = Math.Clamp(pos, 0, 1);
		
		return pos;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ChangeFocusWithoutAnimation(Widget w)
	{
		// Unfocus previous 
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		
		if (focused)
		{
			SCR_ButtonBaseComponent prevBtn = SCR_ButtonBaseComponent.Cast(focused.FindHandler(SCR_ButtonBaseComponent));
			if (prevBtn)
				prevBtn.ShowBorder(false, false);
		}
		
		// Focus next
		GetGame().GetWorkspace().SetFocusedWidget(w);
		SCR_ButtonBaseComponent nextBtn = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (nextBtn)
		{
			nextBtn.ShowBorder(true, false);
			AnimateWidget.StopAnimation(nextBtn.m_wBorder, WidgetAnimationOpacity);
			nextBtn.m_wBorder.SetOpacity(1);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup opacity animation 
	protected void AnimateEntryOpacity(Widget w, int delay, float animTime, float opacityEnd, float opacityStart = -1)
	{
		if (opacityStart != -1)
			w.SetOpacity(opacityStart);
		
		GetGame().GetCallqueue().CallLater(OpacityAnimation, delay, false, w, animTime, opacityEnd);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OpacityAnimation(Widget w, int time, float opacityEnd) 
	{
		AnimateWidget.Opacity(w, opacityEnd, time);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clear widgets from server list 
	protected void ClearServerList()
	{
		// Remove widgets 
		foreach (Widget entry : m_aEntryWidgets)
		{
			entry.RemoveFromHierarchy();
		}
		
		// Clear list 
		m_aEntryWidgets.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Scrolling with up/down buttons outside of list view 
	//! Focus is focus rest widget 
	/*protected void SteppingOutside(int next)
	{
		int nextClampped = m_iFocusedEntryId - Math.Floor(m_fScrollPosition) + next; 
		
		// Settping down to next hidden 
		if (nextClampped > m_iEntriesLimit)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntryWidgets[m_iEntriesLimit - 1]);
		}
		
		// Stepping up to next hidden 
 		if (nextClampped < 1)
		{
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntryWidgets[0]);
		}
	}*/
	
	//------------------------------------------------------------------------------------------------
	//! Check stepping over limit down
	protected void OnMenuDown()
	{
		// Check room coount boundary 
		if (m_iFocusedEntryId >= m_iEntriesCount - 1)
			return;
		
		// Move focus
		m_iFocusedEntryId++;
		int nextClampped = m_iFocusedEntryId - Math.Ceil(m_fScrollPosition); 
		int limit = m_iEntriesLimit - 1;
		

		// Settping down to next hidden 
		if (nextClampped >= limit)
		{
			float scrollStep = EntryFractionFromViewPos(1, 0);
			Print("scrollStep: " + scrollStep);
			ScrollList(scrollStep, 1);
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntryWidgets[limit - 1]);
		}
	}
	
	//------------------------------------------------------------------------------------------------//------------------------------------------------------------------------------------------------
	//! Check stepping over limit up
	protected void OnMenuUp()
	{
		// Check 0
		if (m_iFocusedEntryId < 1)
			return;
		
		// Move focus 
		m_iFocusedEntryId--;
		int nextClampped = m_iFocusedEntryId - Math.Floor(m_fScrollPosition) - 1; 
		
		// Settping down to next hidden 
		if (nextClampped < 1)
		{
			ScrollList(1, -1);
			GetGame().GetWorkspace().SetFocusedWidget(m_aEntryWidgets[0]);
		}
	}
	
	//-------------------------------------
	// Get & Set
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void SetScrollbar(SCR_ScrollBarComponent scrollbar) 
	{ 
		// Check and assign 
		if (!scrollbar)
			return;
		
		m_Scrollbar = scrollbar;
		
		UpdateScrollbar();
		
		// Invoker
		m_Scrollbar.m_OnScroll.Insert(ScrollOffsetMove);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Activate scrolling with input action 
	void ActivateScrolling(bool activate)
	{
		m_bIscrollActive = activate;
		
		// Start scrolling update
		if (m_bIscrollActive)
			ScrollWheelMove();
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateScrollbar()
	{
		if (!m_Scrollbar || m_iEntriesCount < 1)
			return;
		
		float fillAmount = m_iEntriesLimit / m_iEntriesCount;
		m_Scrollbar.SetupHandlerFill(fillAmount);
	}
	
	//------------------------------------------------------------------------------------------------
	array<Widget> GetEntryWidgets() { return m_aEntryWidgets; }
	
	//------------------------------------------------------------------------------------------------\
	//! Return how many entries can be scrolled 
	float ScrollableEntriesCount() 
	{ 
		float dif = m_iEntriesCount - m_iEntriesLimit;
		
		// No entries check 
		if (dif <= 0)
			return -1;
		
		return dif;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return first that is visible and enabled
	Widget FirstAvailableEntry()
	{
		foreach (Widget entry : m_aEntryWidgets)
		{
			if (entry.IsVisible() && entry.IsEnabled())
				return entry;
		}
		
		return null;
	}
};