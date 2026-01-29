/*
Pooled scrollable list that can endless simulate scrolling with only small amount of widgets.
The component assumes two pages to be filled with entries, were Page 0 is by default on top of Page 1
These pages are shuffled every time the scrolling is about to reach the top/bottom of a page, thus resulting in continuous scrolling. 
Size widgets above and below the pages are used to fill space and simulate the presence of other entries.
Used with layout {611B71C627F5C61C}UI/layouts/WidgetLibraryExtended/ListView/WLib_PooledList.layout
*/

//------------------------------------------------------------------------------------------------
class SCR_PooledListComponent : SCR_ScriptedWidgetComponent
{
	// Entries
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Entry widget that list is filled with.")]
	protected ResourceName m_sEntry;

	// Animation effects attributes 
	[Attribute("1", UIWidgets.EditBox, desc: "Time for apearing animation")] 
	protected float m_fAnimationAppearTime;
	
	[Attribute("20", UIWidgets.EditBox, desc: "How many entries should be created for single page")] 
	protected int m_iPageEntriesCount;
	
	[Attribute("15", UIWidgets.EditBox, desc: "On which entry from border page should be changed")] 
	protected int m_iPageChangeOffset;
	
	// Count of all entries in list 
	protected int m_iAllEntriesCount;

	// Constant sizing 
	const float SIZE_UNMEASURED = -1;
	const int CHECK_ENTRY_SIZE_DELAY = 100;
	
	// Move offset when moving up in scroll layout 
	const float ENTRY_OFFSET_UP = 0.1;
	
	// Constant content z orders 
	const int ZORDER_OFFSET_TOP;
	const int ZORDER_OFFSET_BOTTOM = 3;
	
	// Constant idget names 
	const string WIDGET_PAGES = "VPages";
	const string WIDGET_PAGE0 = "VPage0";
	const string WIDGET_PAGE1 = "VPage1";
	
	const string WIDGET_SCROLL_LAYOUT =  "ScrollLayout";
	const string WIDGET_SIZE_OFFSET_TOP = "SizeOffsetTop";
	const string WIDGET_SIZE_OFFSET_BOTTOM = "SizeOffsetBottom";
	
	const string WIDGET_FOCUS_REST = "FocusRest";
	
	// Sizes 
	protected float m_fListPxHeight = SIZE_UNMEASURED;
 	protected float m_fPagePxHeight = SIZE_UNMEASURED;
	protected float m_fViewPxHeight = SIZE_UNMEASURED;
	protected float m_fEntryPxHeight = SIZE_UNMEASURED;
	
	// Content widgets 
	protected ScrollLayoutWidget m_wScroll;
	
	protected Widget m_wPagesWrap;
	protected Widget m_wPage0;
	protected Widget m_wPage1;

	protected Widget m_wPage0FirstEntry;
	protected Widget m_wPage0LastEntry;
	protected Widget m_wPage1FirstEntry;
	protected Widget m_wPage1LastEntry;
	
	protected Widget m_wLastFocused;

	protected ref array<Widget> m_aEntryWidgets = {};
	
	protected SizeLayoutWidget m_wSizeOffsetTop;
	protected SizeLayoutWidget m_wSizeOffsetBottom;
	
	// Pages switching 
	protected bool m_bPagesInverted;
	protected int m_iCurrentPage;

	// Scroll vars 
	protected float m_ScrollLastY;
	
	// Focus
	bool m_bIsListFocused;
	
	// Invokers 
	protected ref ScriptInvokerInt m_OnSetPage;
	
	//-------------------------------------
	// ScriptedWidgetComponent override
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// Accessing handlers and widgets 
		AccessingHandlers();	
		
		CreateEntriesWidgets();
		
		SetupOffsets(0);
	}
	
	//-------------------------------------
	// Input actions 
	//-------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Call this when up action is activated 
	protected void OnActionUp()
	{
		if (!m_bIsListFocused)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (m_wLastFocused)
			workspace.SetFocusedWidget(m_wLastFocused);

		if (m_fPagePxHeight == 0)
			return;

		float invisibleEntries = EntriesOutOfView();
		if (invisibleEntries == 0)
			return;
		
		// Check current page 
		float scrollPageRatio = (m_iPageEntriesCount + ENTRY_OFFSET_UP) / invisibleEntries;
		int currentPage = Math.Floor(m_ScrollLastY / scrollPageRatio);
		
		if (m_iCurrentPage == currentPage)
			return;
		
		// Is focus on first entry of one of page 
		Widget focus = workspace.GetFocusedWidget();
		
		if (focus != m_wPage0FirstEntry && focus != m_wPage1FirstEntry)
			return;
		
		// Switch pages 
		SetCurrentPage(m_iCurrentPage - 1);
		
		if (focus == m_wPage0FirstEntry)
			workspace.SetFocusedWidget(m_wPage1LastEntry);
		else if (focus == m_wPage1FirstEntry)
			workspace.SetFocusedWidget(m_wPage0LastEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this when down action is activated 
	protected void OnActionDown()
	{
		if (!m_bIsListFocused)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (m_wLastFocused)
			workspace.SetFocusedWidget(m_wLastFocused);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call this position of scroll is changed 
	protected void OnScroll(float scrollY)
	{
		if (m_fPagePxHeight == 0)
			return;
		
		// Check if page is changed
		int currentPage = CurrentPageFromScrollPos(scrollY);

		if (m_iCurrentPage != currentPage)
			SetCurrentPage(currentPage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return current page from scroll position
	protected int CurrentPageFromScrollPos(float scrollY)
	{
		// Zero checks 
		if (m_iPageEntriesCount == 0)
			return 0;
		
		int entriesOutOfView = EntriesOutOfView();
		if (entriesOutOfView == 0)
			return 0;
		
		// Ratio between entries in single page and entries out of view
		float scrollPageRatio = m_iPageEntriesCount / entriesOutOfView;
		
		// Check if page is changed
		float pos = scrollY / scrollPageRatio;
		
		// Moving up
		pos = pos - (m_iPageChangeOffset / m_iPageEntriesCount);
		
		//TODO: find out why moving down gets stuck
		
		int currentPage = Math.Floor(pos);
		if (currentPage < 0)
			return 0;
		
		return currentPage;
	}

	//------------------------------------------------------------------------------------------------
	//! How many entries are of view - how many entries can be scrolled 
	protected float EntriesOutOfView()
	{
		if (m_fPagePxHeight == 0)
			return 0;
		
		// What percent of whole list can bee seen 
		float pageViewRatio = m_fViewPxHeight / m_fPagePxHeight; 
		float outOfView = m_iAllEntriesCount - m_iPageEntriesCount * pageViewRatio;

		if (outOfView < 0)
			return 0;

		return outOfView;
	}

	//-------------------------------------
	// public functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Fill entries with data 
	void UpdateEntries(bool animated = false)
	{
		// Fill list with data 
		foreach (Widget w : m_aEntryWidgets)
		{
			if (w)
				FillEntry(w);
		}
				
		SetupOffsets(m_iCurrentPage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update positions of pages and offsets 
	void UpdateScroll()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		// Get scroll positions 
		float scrollX, scrollY = 0;
		m_wScroll.GetSliderPos(scrollX, scrollY);

		// Check if current focus is in list 
		Widget focused = FocusedWidgetFromEntryList();
		
		if (focused)
			m_wLastFocused = focused;
		
		// Is list foucused - is current focus on list fallback widget or list entry?
		Widget workspaceFocused = workspace.GetFocusedWidget();
		m_bIsListFocused = (workspaceFocused == m_wLastFocused);
		// Check scroll change 
		if (m_ScrollLastY == scrollY)
			return;
		
		OnScroll(scrollY);
		m_ScrollLastY = scrollY;
		
		// Check sizes 
		if (m_fEntryPxHeight == SIZE_UNMEASURED || m_fEntryPxHeight == 0 && m_iAllEntriesCount != 0)
		{
			CheckEntrySize();
			MoveToTop();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set which page should be currently displayed
	protected void SetCurrentPage(int page)
	{
		SetCurrentPageBase(page);
		UpdateEntries();
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Base function for updating current page
	protected void SetCurrentPageBase(int page)
	{
		m_iCurrentPage = page;

		// Check if pages are inverted to simulate scrolling 
		bool pagesInverted = (m_iCurrentPage % 2 != 0);
		
		// Don't switch on last page
		int lastPage = Math.Floor(m_iAllEntriesCount / m_iPageEntriesCount);
		
		if (m_iCurrentPage >= lastPage - 1)
			pagesInverted = false;
		
		SwitchPages(pagesInverted, m_fPagePxHeight, m_iCurrentPage);
		m_OnSetPage.Invoke(page);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Change order of list to fake endless scrolling 
	protected void SwitchPages(bool invert, float size, int page)
	{
		// Check entries count 
		if (m_iAllEntriesCount == 0)
			return;
		
		// Check vertical list widgets 
		if (!m_wPage0 || !m_wPage1)
			return;
		
		// Save invert 
		m_bPagesInverted = invert;

		// Check end of scrolling 
		int maxPages = Math.Floor(m_iAllEntriesCount / m_iPageEntriesCount);
		if (page > maxPages - 1)
			return;

		SetupOffsets(page);
		
		// Inverted order
		m_wPage0.SetZOrder(invert);
		m_wPage1.SetZOrder(!invert);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set sizes of top and bottom offsets 
	protected void SetupOffsets(int page)
	{
		int overflowEntryCount = m_iAllEntriesCount - m_iPageEntriesCount*2;
		float offset = m_fEntryPxHeight * overflowEntryCount - m_fPagePxHeight * page;
		
		// testing calculations 
		float pageSizeBaseOnEntry = m_fEntryPxHeight * m_iPageEntriesCount;

		m_wSizeOffsetTop.SetHeightOverride(m_fPagePxHeight * page);
		m_wSizeOffsetBottom.SetHeightOverride(offset);
		
		m_wSizeOffsetTop.SetZOrder(ZORDER_OFFSET_TOP);
		m_wSizeOffsetBottom.SetZOrder(ZORDER_OFFSET_BOTTOM);
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
	
	//------------------------------------------------------------------------------------------------
	//! Move list scroll to top
	void MoveToTop()
	{
		SetCurrentPage(0);
		OnScroll(0);
		
		m_bPagesInverted = false;
		SwitchPages(m_bPagesInverted, 0, 0);
		
		m_ScrollLastY = 0;
		m_wScroll.SetSliderPos(0, 0);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clip scroll bar to hide
	void ShowScrollbar(bool show)
	{
		if (show)
		{
			m_wRoot.ClearFlags(WidgetFlags.CLIPCHILDREN);
			m_wRoot.SetFlags(WidgetFlags.INHERIT_CLIPPING);
		}
		else
		{
			m_wRoot.SetFlags(WidgetFlags.CLIPCHILDREN);
			m_wRoot.ClearFlags(WidgetFlags.INHERIT_CLIPPING);
		}
	}
	
	//-------------------------------------
	// protected functions 
	//-------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Get reference to all needed component for handling 
	protected void AccessingHandlers() 
	{
		// Base scroll layout 
		m_wScroll = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SCROLL_LAYOUT));
		
		// Find pages
		m_wPagesWrap = m_wRoot.FindAnyWidget(WIDGET_PAGES);
		m_wPage0 = m_wRoot.FindAnyWidget(WIDGET_PAGE0);
		m_wPage1 = m_wRoot.FindAnyWidget(WIDGET_PAGE1);
		
		// Find size offsets 
		m_wSizeOffsetTop = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SIZE_OFFSET_TOP));
		m_wSizeOffsetBottom = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget(WIDGET_SIZE_OFFSET_BOTTOM));
	}

	//------------------------------------------------------------------------------------------------
	//! Create server entries widget 
	protected void CreateEntriesWidgets()
	{
		if(!m_wRoot || m_sEntry.IsEmpty())
			return; 
		
		CreateEntriesWidgetsInPage(m_wPage0, m_iPageEntriesCount, m_wPage0FirstEntry, m_wPage0LastEntry);
		CreateEntriesWidgetsInPage(m_wPage1, m_iPageEntriesCount, m_wPage1FirstEntry, m_wPage1LastEntry);
	}	
	
	//------------------------------------------------------------------------------------------------
	//! Create entries to selected page and assing first and last entries 
	protected void CreateEntriesWidgetsInPage(Widget parent, int count, out Widget firstEntry, out Widget lastEntry)
	{
		for (int i = 0; i < count; i++)
		{
			Widget entry = CreateEntry(parent);
			
			// Assing first entry of page
			if (i == 0)
				firstEntry = entry;
			
			// Assing last entry of page
			if (i == count - 1)
				lastEntry = entry;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create entry to given page and setup it's behavior  
	protected Widget CreateEntry(Widget parent)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		if (!m_sEntry || !parent || !workspace)
			return null;
		
		Widget widget = workspace.CreateWidgets(m_sEntry, parent);
		if (!widget)
			return null;
		
		// Setup widget entry behavior 
		SetupEntryBehavior(widget);
		
		// Save in widget list  
		m_aEntryWidgets.Insert(widget);
		
		return widget;
	}

	protected bool m_bIsMeasured = false;
	
	//------------------------------------------------------------------------------------------------
	//! Get size of widget entry used in list 
	protected void CheckEntrySize()
	{
		if (m_aEntryWidgets.IsEmpty())
			return;
		
		// Check widgets 
		if (!m_bIsMeasured)
		{
			ShowScrollbar(false);
			
			// Entry height in px - with bottom padding 
			float x;
			m_aEntryWidgets[0].GetScreenSize(x, m_fEntryPxHeight);
	
			// Page height in px
			m_wPage0.GetScreenSize(x, m_fPagePxHeight);
	
			// Height of view int px - how much can user see in px
			m_wRoot.GetScreenSize(x, m_fViewPxHeight);
			
			// Check measures
			if (m_fEntryPxHeight != 0)
			{
				m_bIsMeasured = true;
				ShowScrollbar(true);
				
				SetupOffsets(m_iCurrentPage);
			}
		}
		
		// Whole height size in px - cut last bottom padding 
		m_fListPxHeight = m_fEntryPxHeight * m_iAllEntriesCount;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Empty functions for setting up widget entry beavhior actions, callbacks, etc.
	Override this to assign specific beavhior for each entry 
	*/
	protected void SetupEntryBehavior(Widget entry) { }
	
	//------------------------------------------------------------------------------------------------
	/*!
	Empty function for filling entries with data 
	Override this to fill entries with data
	*/
	protected void FillEntry(Widget w) {}
	
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
	//! Display right entries with right position by scroll
	protected void ShowEntries(int dataCount)
	{
		// Show entries 
		for (int i = 0; i < m_aEntryWidgets.Count(); i++)
		{
			bool show = i < dataCount;
			m_aEntryWidgets[i].SetVisible(show);	
		}
		
		// Check if offsets needed
		int overflow = dataCount - m_iPageEntriesCount*2;
		bool addOffsets = overflow > 0;
		
		// Set offstes visible if needed
		m_wSizeOffsetTop.SetVisible(addOffsets);
		m_wSizeOffsetBottom.SetVisible(addOffsets);
		
		if (!addOffsets)
			return; 
	}
	
	//------------------------------------------------------------------------------------------------
	//! Return if and which entry widget is actually focus 
	protected Widget FocusedWidgetFromEntryList()
	{
		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		
		int focusedEntryId = m_aEntryWidgets.Find(focused);
		
		if (focusedEntryId == -1)
			return null;
		
		return focused;
	}
	
	//-------------------------------------
	// Get & Set
	//-------------------------------------
	//------------------------------------------------------------------------------------------------
	int GetPageEntriesCount()
	{
		return m_iPageEntriesCount;
	}
	
	//------------------------------------------------------------------------------------------------
	array<Widget> GetEntryWidgets()
	{ 
		return m_aEntryWidgets;
	}

	//------------------------------------------------------------------------------------------------
	// Setup entries defaults and total count 
	void SetDataEntries(int entriesCount)
	{
		// Set data 
		m_iAllEntriesCount = entriesCount;
		
		// Show entries 
		ShowEntries(entriesCount);
		
		// Check sizes 
		GetGame().GetCallqueue().CallLater(CheckEntrySize, CHECK_ENTRY_SIZE_DELAY);
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

	//------------------------------------------------------------------------------------------------
	void SetIsListFocused(bool focused) 
	{
		m_bIsListFocused = focused; 
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsListFocused() 
	{ 
		return m_bIsListFocused;
	}

	//------------------------------------------------------------------------------------------------
	Widget GetScrollWidget()
	{
		return m_wScroll;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnSetPage()
	{
		if (!m_OnSetPage)
			m_OnSetPage = new ScriptInvokerInt();

		return m_OnSetPage;
	}
}