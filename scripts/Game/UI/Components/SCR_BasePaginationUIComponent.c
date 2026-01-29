class SCR_BasePaginationUIComponent: MenuRootSubComponent
{	
	[Attribute("1", desc: "Number of rows in content area.")]
	protected int m_iRows;
	
	[Attribute("1", desc: "Number of columns in content area.")]
	protected int m_iColumns;
	
	[Attribute("-1", desc: "Minimum mumber of rows in content area.\n When -1, value from 'Rows' will be used.")]
	protected int m_iMinRows;
	
	[Attribute("-1", desc: "Minimum number of rows in content area.\n When -1, value from 'Columns' will be used.")]
	protected int m_iMinColumns;
	
	[Attribute(desc: "Name of content area widget.")]
	protected string m_sContentName;
	
	[Attribute(desc: "Name of a widget which, when activated, will move to the previous page. This button is shown when cursor is focused on the area.")]
	protected string m_sButtonPrevName;
	
	[Attribute(desc: "Name of a widget which, when activated, will move to the next page. This button is shown when cursor is focused on the area.")]
	protected string m_sButtonNextName;
	
	[Attribute("PrevButton_NoScroll", desc: "Name of a widget which, when activated, will move to the previous page. This button is shown when cursor is not focused on the area.")]
	protected string m_sButtonPrevName_NoScrolling;
	
	[Attribute("NextButton_NoScroll", desc: "Name of a widget which, when activated, will move to the next page. This button is shown when cursor is not focused on the area.")]
	protected string m_sButtonNextName_NoScrolling;
	
	[Attribute(desc: "Name of a widget which, when focused, will move to the previous page.\nused to auto-paging when focusing outside of content area in given direction.")]
	protected string m_sFocusPrevName;
	
	[Attribute(desc: "Name of a widget which, when focused, will move to the next page.\nused to auto-paging when focusing outside of content area in given direction.")]
	protected string m_sFocusNextName;
	
	[Attribute(desc: "Name of the widget which will show page numbers.")]
	protected string m_sPageIndexVisualName;
	
	[Attribute("#AR-Editor_ContentBrowser_PageIndex_Text", desc: "Format of page number text.")]
	protected string m_sPageIndexVisualText;
	
	[Attribute(desc: "When enabled, button shortcuts will be accepted only when cursor is over content widget.\nWhen disabled, cursor can be anywhere inside area of the widget this component is attached to.\nDoes not affect gamepad controls.")]
	protected bool m_bMustHoverOverContent;
	
	[Attribute(desc: "If content widget is UniformGridSlot, empty grid tiles will be filled with these layouts.\nIf there are more items than defined here, the last layout in the list will be used.\n\nWhen undefined, no filling will be performed.", uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ref array<ResourceName> m_aEmptyItemLayouts;
	
	[Attribute(desc: "When enabled, going next from the last page will cycle back to the first page, and vice versa.")]
	protected bool m_bLoop;
	
	[Attribute("1", desc: "When true it will hide the arrows instead of disabling them if the button is disabled.")]
	protected bool m_bHideArrowsIfDisabled;
	
	[Attribute("0", desc: "When true always ignore gamepad input. Set this true if the widget is always on the screen and there is a seperate dialog version of it for the gamepad. Setting this true will make sure the next and prev focus buttons will have 'No Focus' enabled")]
	protected bool m_bIgnoreGamePadInput;
	
	[Attribute("1", desc: "If true it will play the next and prev page audio on page change")]
	protected bool m_bPlayAudioOnPageChange;
	
	[Attribute("1", desc: "If true it will show empty entries")]
	protected bool m_bShowEmptyEntries;
	
	[Attribute(SCR_SoundEvent.TURN_PAGE, UIWidgets.EditBox)]
	protected string m_sOnNextPageSfx;
	
	[Attribute(SCR_SoundEvent.TURN_PAGE, UIWidgets.EditBox)]
	protected string m_sOnPrevPageSfx;
	
	[Attribute("", desc: "When true it will disable the arrows if number of pages is one.")]
	protected bool m_bDisableArrowsSinglePage;
	
	protected int m_iCurrentPage;
	protected bool m_bUnderCursor;
	protected Widget m_ContentWidget, m_ButtonPrevWidget, m_ButtonNextWidget, m_FocusPrevWidget, m_FocusNextWidget, m_ButtonPrevNoScrollWidget, m_ButtonNextNoScrollWidget;
	protected TextWidget m_PageNumberWidget;
	protected ref ScriptInvoker m_OnPageChanged = new ScriptInvoker();
	protected int m_FocusedIndex[2];
	
	//------------------------------------------------------------------------------------------------
	//! Disable page change button if flag is true and number of pages is 1
	protected void DisableArrowsSinglePage()
	{
		if (!m_bDisableArrowsSinglePage || !m_ButtonPrevWidget || !m_ButtonNextWidget || m_iRows == 0 || m_iColumns == 0)
			return;
		
		int countEntries = GetEntryCount();
		int totalPages = Math.Ceil(countEntries / (m_iRows * m_iColumns));
		if (totalPages > 1)
			return;
		
		m_ButtonPrevWidget.SetVisible(false);
		m_ButtonNextWidget.SetVisible(false);
	}
	
	//--- To be overridden by inherited classes
	/*!
	Function called when the page is refreshed.
	To be overridden by inherited classes.
	\param contentWidget Content widget
	\param indexStart Index of the first displayed item
	\param indexEnd Index of the last displayed item
	*/
	protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd);
	/*!
	Return total number of entries.
	To be overridden by inherited classes.
	\return Number of entries
	*/
	int GetEntryCount();
	
	//--- Public functions
	/*!
	\return Number of rows
	*/
	int GetRows()
	{
		return m_iRows;
	}
	/*!
	\return Number of Columns
	*/
	int GetColumns()
	{
		return m_iColumns;
	}
	/*!
	\return Currently selected page
	*/
	int GetCurrentPage()
	{
		return m_iCurrentPage;
	}
	/*!
	\param w Queried widget
	\return Index of entry represented by given widget
	*/
	int GetEntryIndex(Widget w)
	{
		int row = UniformGridSlot.GetRow(w);
		int column = UniformGridSlot.GetColumn(w);		
		return (m_iCurrentPage * m_iRows * m_iColumns) + (row * m_iColumns + column);
	}
	/*!
	\return Event called when current page changes.
	*/
	ScriptInvoker GetOnPageChanged()
	{
		return m_OnPageChanged;
	}
	
	/*!
	Set current page.
	\param page Page number
	\return True if the page has any content
	*/
	sealed bool SetPage(int page, bool isRefresh = false)
	{
		if (!m_ContentWidget)
			return false;
		
		//--- Get currently focused widget
		int focusedRow = m_FocusedIndex[0];
		int focusedColumn = m_FocusedIndex[1];
		
		//--- Set new page
		int pageCapacity = m_iRows * m_iColumns;
		int countEntries = GetEntryCount();
		int totalPages = Math.Ceil(countEntries / (m_iRows * m_iColumns));
		if (totalPages > 0 && m_bLoop)
			page = Math.Repeat(page, totalPages);
		
		int indexStart = page * pageCapacity;
		int indexEnd = Math.Min(indexStart + pageCapacity, countEntries);
		if (indexStart < 0 || (countEntries > 0 && indexStart >= countEntries))
			return false;
		
		if (m_bPlayAudioOnPageChange && !isRefresh && m_iCurrentPage != page)
		{
			if (page > m_iCurrentPage)
				SCR_UISoundEntity.SoundEvent(m_sOnNextPageSfx, true);
			else 
				SCR_UISoundEntity.SoundEvent(m_sOnPrevPageSfx, true);
		}
		
		m_iCurrentPage = page;
		
		//--- Delete existing entries
		while (m_ContentWidget.GetChildren())
		{
			m_ContentWidget.GetChildren().RemoveFromHierarchy();
		}
		
		//--- Add new entries
		ShowEntries(m_ContentWidget, indexStart, indexEnd);
		bool hasContent = m_ContentWidget.GetChildren() != null;
		
		//--- Position entries in the grid
		Widget focusedButton;
		if (m_ContentWidget.GetTypeID() == WidgetType.UniformGridLayoutWidgetTypeID)
		{
			int row, column;
			Widget child = m_ContentWidget.GetChildren();
			while (child)
			{
				UniformGridSlot.SetRow(child, row);
				UniformGridSlot.SetColumn(child, column);
			
				//--- Set navigation
				if (!m_bIgnoreGamePadInput)
				{
					if (column == 0 && m_sFocusPrevName)
						child.SetNavigation(WidgetNavigationDirection.LEFT, WidgetNavigationRuleType.EXPLICIT, m_sFocusPrevName);
	
					
					if (column == m_iColumns - 1 && m_sFocusNextName)
						child.SetNavigation(WidgetNavigationDirection.RIGHT, WidgetNavigationRuleType.EXPLICIT, m_sFocusNextName);
				}
				
				//--- Find focused widget
				if (row == focusedRow && column == focusedColumn)
					focusedButton = SCR_WidgetTools.FindWidgetInChildren(child, WidgetType.ButtonWidgetTypeID);
				
				IterateIndex(row, column);
				child = child.GetSibling();
			}
			
			if (m_bShowEmptyEntries)
			{
				//--- Fill the rest with empty layout
				int minRows = m_iRows;
				if (page == 0 && m_iMinRows >= 0)
					minRows = Math.Max(m_iMinRows, row);
				
				int minColumns = m_iColumns;
				if (page == 0 && m_iMinColumns >= 0)
					minColumns = Math.Max(m_iMinColumns, row);
				
				Widget emptyWidget;
				int emptyItemsCount = m_aEmptyItemLayouts.Count() - 1;
				while (row < minRows && column < minColumns)
				{
					if (m_aEmptyItemLayouts.IsEmpty())
					{
						emptyWidget = GetGame().GetWorkspace().CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.IGNORE_CURSOR | WidgetFlags.NOFOCUS, new Color(0, 0, 0, 0), 0, m_ContentWidget);
					}
					else
					{
						int index = Math.Min(row * m_iColumns + column, emptyItemsCount);
						emptyWidget = GetGame().GetWorkspace().CreateWidgets(m_aEmptyItemLayouts[index], m_ContentWidget);
					}
					
					UniformGridSlot.SetRow(emptyWidget, row);
					UniformGridSlot.SetColumn(emptyWidget, column);
					
					IterateIndex(row, column);
				}
			}
		}
		
		//--- No focus preserved, choose the first item
		if (!focusedButton)
			focusedButton = m_ContentWidget.GetChildren();
		
		//--- Set focus
		if (focusedButton)
			GetGame().GetCallqueue().Call(FocusWidget, focusedButton);
		
		/*
		//--- Create dummy widget in bottom right cell to stretch the grid
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (workspace)
		{
			Widget dummy = workspace.CreateWidget(WidgetType.ImageWidgetTypeID, WidgetFlags.VISIBLE | WidgetFlags.DISABLED, new Color(0, 0, 0, 0), 0, m_ContentWidget);
			UniformGridSlot.SetRow(dummy, m_iRows - 1);
			UniformGridSlot.SetColumn(dummy, m_iColumns - 1);
		}*/
		
		//--- Update page number
		if (m_PageNumberWidget)
		{ 
			if (countEntries == 0)
			{
				m_PageNumberWidget.SetVisible(false);
			}
			else
			{
				m_PageNumberWidget.SetTextFormat(m_sPageIndexVisualText, m_iCurrentPage + 1, totalPages);
				m_PageNumberWidget.SetVisible(true);
			}
		}
		
		//--- Set prev/next button visibility
		bool canLoop = m_bLoop && totalPages > 1;
		
		EnablePageButton(m_ButtonPrevWidget, canLoop || page > 0);
		EnablePageButton(m_ButtonPrevNoScrollWidget, canLoop || page > 0);
		
		EnablePageButton(m_ButtonNextWidget, canLoop || page < totalPages - 1);
		EnablePageButton(m_ButtonNextNoScrollWidget, canLoop || page < totalPages - 1);
		
		m_OnPageChanged.Invoke(page);
		
		return hasContent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Separated focus function for later call
	protected void FocusWidget(Widget w)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
	}
	
	/*!
	Refresh the current page.
	\return True if the page has any content
	*/
	sealed bool RefreshPage()
	{
		return SetPage(m_iCurrentPage, true);
	}
	
	protected void IterateIndex(out int row, out int column)
	{
		column++;
		if (column >= m_iColumns)
		{
			column = 0;
			row++;
		}
	}
	
	protected void EnablePageButton(Widget w, bool enable)
	{
		if (!w)
			return;
		
		if (m_bHideArrowsIfDisabled)
		{
			w.SetOpacity(enable);
			return;
		}
		
		SCR_ButtonBaseComponent buttonComponent = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		
		if (buttonComponent)
			buttonComponent.SetEnabled(enable);
		else 	
			w.SetEnabled(enable);
	}
	
	protected void OnButtonPrev()
	{
		if (m_bIgnoreGamePadInput && !GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		if (m_bUnderCursor && GetMenu().IsFocused())		
			SetPage(m_iCurrentPage - 1);
			
	}
	
	protected void OnButtonNext()
	{
		if (m_bIgnoreGamePadInput && !GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		if (m_bUnderCursor && GetMenu().IsFocused())
			SetPage(m_iCurrentPage + 1);
	}
	
	protected void SetUnderCursor(bool newUnderCursor)
	{
		m_bUnderCursor = newUnderCursor;
		
		if (m_ButtonPrevNoScrollWidget && m_ButtonNextNoScrollWidget)
		{
			m_ButtonPrevWidget.SetVisible(m_bUnderCursor);
			m_ButtonPrevNoScrollWidget.SetVisible(!m_bUnderCursor);
		
			m_ButtonNextWidget.SetVisible(m_bUnderCursor);
			m_ButtonNextNoScrollWidget.SetVisible(!m_bUnderCursor);
		}
	}
	
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		//--- Always considered "under cursor" with gamepad
		SetUnderCursor(isGamepad);
	}
	
	override protected bool IsUnique()
	{
		return false;
	}
	
	override bool OnFocus(Widget w, int x, int y)
	{
		if (w == m_FocusPrevWidget)
		{
			m_FocusedIndex[1] = m_iColumns - 1; //--- Select rightmost item to preserve continuity
			OnButtonPrev();
		}
		else if (w == m_FocusNextWidget)
		{
			m_FocusedIndex[1] = 0; //--- Select leftmost item to preserve continuity
			OnButtonNext();
		}
		else
		{
			//--- Remember coordinates of selected item, so it can be restored when current page changes
			while (w)
			{
				if (w.GetParent() == m_ContentWidget)
				{
					m_FocusedIndex[0] = UniformGridSlot.GetRow(w);
					m_FocusedIndex[1] = UniformGridSlot.GetColumn(w);
					break;
				}
				w = w.GetParent();
			}
		}
		
		return false;
	}
	
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_bMustHoverOverContent)
		{
			//--- Hovering over prev/next buttons always counts
			if (w == m_ButtonPrevWidget || w == m_ButtonNextWidget)// || (w == m_ButtonPrevNoScrollWidget && w != null)  || (w == m_ButtonNextNoScrollWidget && w != null))
			{
				SetUnderCursor(true);
				return false;
			}
			
			//--- Considered under cursor only when hovering over content area
			while (w)
			{
				if (w == m_ContentWidget)
				{
					SetUnderCursor(true);
					break;
				}
				w = w.GetParent();
			}
		}
		else
		{
			SetUnderCursor(true);
		}
		return false;
	}
	
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		SetUnderCursor(false);
		return false;
	}
	
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_ContentWidget = w.FindAnyWidget(m_sContentName);
			
		m_PageNumberWidget = TextWidget.Cast(w.FindAnyWidget(m_sPageIndexVisualName));
		
		m_ButtonPrevWidget = w.FindAnyWidget(m_sButtonPrevName);
		m_ButtonNextWidget = w.FindAnyWidget(m_sButtonNextName);
		
		m_ButtonPrevNoScrollWidget = w.FindAnyWidget(m_sButtonPrevName_NoScrolling);
		m_ButtonNextNoScrollWidget = w.FindAnyWidget(m_sButtonNextName_NoScrolling);
		
		m_FocusPrevWidget = w.FindAnyWidget(m_sFocusPrevName);
		m_FocusNextWidget = w.FindAnyWidget(m_sFocusNextName);
		
		if (m_bIgnoreGamePadInput)
		{
			if (m_FocusPrevWidget)
				m_FocusPrevWidget.SetFlags(WidgetFlags.NOFOCUS);
			if (m_FocusNextWidget)
				m_FocusNextWidget.SetFlags(WidgetFlags.NOFOCUS); 
		
			if (m_ButtonPrevWidget)
				m_ButtonPrevWidget.SetFlags(WidgetFlags.NOFOCUS);
			if (m_ButtonNextWidget)
				m_ButtonNextWidget.SetFlags(WidgetFlags.NOFOCUS); 
			
			if (m_ButtonPrevNoScrollWidget)
				m_ButtonPrevNoScrollWidget.SetFlags(WidgetFlags.NOFOCUS);
			if (m_ButtonNextNoScrollWidget)
				m_ButtonNextNoScrollWidget.SetFlags(WidgetFlags.NOFOCUS);
		}
		
		SCR_InputButtonComponent prevButtonComponent = SCR_InputButtonComponent.Cast(m_ButtonPrevWidget.FindHandler(SCR_InputButtonComponent));
		if (prevButtonComponent)
			prevButtonComponent.m_OnActivated.Insert(OnButtonPrev);
		else
			ButtonActionComponent.GetOnAction(m_ButtonPrevWidget, true).Insert(OnButtonPrev);
		
		SCR_InputButtonComponent nextButtonComponent = SCR_InputButtonComponent.Cast(m_ButtonNextWidget.FindHandler(SCR_InputButtonComponent));
		if (nextButtonComponent)
			nextButtonComponent.m_OnActivated.Insert(OnButtonNext);
		else
			ButtonActionComponent.GetOnAction(m_ButtonNextWidget, true).Insert(OnButtonNext);
		
		if (m_ButtonPrevNoScrollWidget)
		{
			prevButtonComponent = SCR_InputButtonComponent.Cast(m_ButtonPrevNoScrollWidget.FindHandler(SCR_InputButtonComponent));
			if (prevButtonComponent)
				prevButtonComponent.m_OnActivated.Insert(OnButtonPrev);
			else
				ButtonActionComponent.GetOnAction(m_ButtonPrevNoScrollWidget, true).Insert(OnButtonPrev);
		}
		
		if (m_ButtonNextNoScrollWidget)
		{
			nextButtonComponent = SCR_InputButtonComponent.Cast(m_ButtonNextNoScrollWidget.FindHandler(SCR_InputButtonComponent));
			if (nextButtonComponent)
				nextButtonComponent.m_OnActivated.Insert(OnButtonNext);
			else
				ButtonActionComponent.GetOnAction(m_ButtonNextNoScrollWidget, true).Insert(OnButtonNext);
		}
		
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		SetPage(m_iCurrentPage, true);
		
		DisableArrowsSinglePage(); // Only runs if the flag is set to true
	}
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
};