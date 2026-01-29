//#define TOOLBAR_DEBUG

//! @ingroup Editor_UI Editor_UI_Components

class SCR_BaseToolbarEditorUIComponent : SCR_DialogEditorUIComponent
{
	protected static const int EMPTY_Z_ORDER = int.MAX;
	
	[Attribute(params: "layout")]
	protected ResourceName m_ItemLayout;
	
	[Attribute("-1", desc: "When >= 0, it will hide parent when there are no items.\n0 means this widget will be hidden, 1 means its parent will, etc.")]
	protected int m_iHideParentIndexWhenEmpty;
	
	[Attribute(desc: "When defined, toolbar will be hidden when empty and widget with this name on the level defined by 'Hide Parent Index When Empty' will be shown instead.")]
	protected string m_sEmptyWidgetName;
	
	[Attribute()]
	protected string m_sItemsWidgetName;
	
	protected SCR_ExternalPaginationUIComponent m_Pagination;
	protected Widget m_ItemsWidget;
	protected Widget m_HideWidget;
	protected int m_iFirstShownIndex;
	protected int m_iFocusedIndex;
	protected int m_iItemsCount;
	protected int m_iPage;
	protected bool m_bIsUnderCursor;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsUnderCursor()
	{
		return m_bIsUnderCursor || !GetGame().GetInputManager().IsUsingMouseAndKeyboard();
	}

	//------------------------------------------------------------------------------------------------
	protected bool CreateItem(out Widget itemWidget, out SCR_BaseToolbarItemEditorUIComponent toolbarItem)
	{
		if (m_ItemsWidget)
		{
			itemWidget = GetGame().GetWorkspace().CreateWidgets(m_ItemLayout, m_ItemsWidget);
			if (!itemWidget)
				return false;
			
#ifdef TOOLBAR_DEBUG
			itemWidget.SetName(string.Format("%1_%2", itemWidget.GetName(), m_iItemsCount));
			TextWidget debugText = TextWidget.Cast(itemWidget.FindAnyWidget("Debug"));
			if (debugText)
			{
				debugText.SetVisible(true);
				debugText.SetText(m_iItemsCount.ToString());
			}
#endif

			toolbarItem = SCR_BaseToolbarItemEditorUIComponent.Cast(itemWidget.FindHandler(SCR_BaseToolbarItemEditorUIComponent));
			if (!toolbarItem) 
			{
				Print(string.Format("'%1' must contain SCR_BaseToolbarItemEditorUIComponent!", m_ItemLayout), LogLevel.WARNING);
				itemWidget.RemoveFromHierarchy();
				return false;
			}
			
			toolbarItem.InitToolbarItem(itemWidget);
			m_iItemsCount++;
			AllowFocus(itemWidget);
			
			return true;
		}
		else
		{
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeleteAllItems()
	{
		Widget deletedChild, child = m_ItemsWidget.GetChildren();
		while (child)
		{
			deletedChild = child;
			child = child.GetSibling(); //--- Get sibling before removing the child from hierarchy
			deletedChild.RemoveFromHierarchy();
		}
		m_iItemsCount = 0;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetToolbarVisible(bool show)
	{
		if (!m_HideWidget && m_iHideParentIndexWhenEmpty >= 0)
		{
			m_HideWidget = GetWidget();
			for (int i = 0; i < m_iHideParentIndexWhenEmpty; i++)
			{
				if (m_HideWidget)
					m_HideWidget = m_HideWidget.GetParent();
				else
					break;
			}
		}

		if (m_HideWidget)
		{
			m_HideWidget.SetVisible(show);
			
			if (m_sEmptyWidgetName)
			{
				Widget emptyWidget = m_HideWidget.GetParent().FindWidget(m_sEmptyWidgetName);
				if (emptyWidget)
					emptyWidget.SetVisible(!show);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
	}

	//------------------------------------------------------------------------------------------------
	protected void Refresh()
	{
		bool hasContent;
		if (m_Pagination)
		{
			hasContent = m_Pagination.RefreshPage();
		}
		else
		{
			DeleteAllItems();
			ShowEntries(m_ItemsWidget, 0, int.MAX);
			hasContent = m_ItemsWidget.GetChildren() != null;
		}
		
		SetToolbarVisible(hasContent);
	}

	//---- REFACTOR NOTE START: Call later used for fallback ui update
	
	//------------------------------------------------------------------------------------------------
	//!
	void MarkForRefresh()
	{
		GetGame().GetCallqueue().Remove(Refresh); //--- Remove the previous queued call, to guarantee there will be the only one
		GetGame().GetCallqueue().CallLater(Refresh, 1); //--- Call with a delay, in case whatever called this is not ready yet and the toolbar would reflect incorrect state
	}

	//---- REFACTOR NOTE END ----
	
	//------------------------------------------------------------------------------------------------
	protected void OnPageChanged(int page)
	{
		m_iPage = page;
	}

	//------------------------------------------------------------------------------------------------
	protected void CopyPage(SCR_DialogEditorUIComponent linkedComponent)
	{
		if (m_Pagination)
		{
			SCR_BaseToolbarEditorUIComponent toolbar = SCR_BaseToolbarEditorUIComponent.Cast(linkedComponent);
			//m_Pagination.SetPage(toolbar.m_Pagination.GetCurrentPage()); //--- Unreliable, pagination component can de-initialize before this one
			m_Pagination.SetPage(toolbar.m_iPage);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CanOpenDialog()
	{
		if (m_HideWidget)
			return m_HideWidget.IsVisible();
		else
			return super.CanOpenDialog();
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDialogOpened(SCR_DialogEditorUIComponent linkedComponent)
	{
		CopyPage(linkedComponent);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnDialogClosed(SCR_DialogEditorUIComponent linkedComponent)
	{
		CopyPage(linkedComponent);
	}

//	//------------------------------------------------------------------------------------------------
//	override void OnInputDeviceUserChanged(EInputDeviceType oldDevice, EInputDeviceType newDevice)
//	{
//		if (SCR_Global.IsChangedMouseAndKeyboard(oldDevice, newDevice))
//			return;
//
//		super.OnInputDeviceUserChanged(oldDevice, newDevice);
//		//if (!m_bIsInDialog) Refresh();
//	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		m_bIsUnderCursor = true;
		super.OnMouseEnter(w, x, y);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		m_bIsUnderCursor = false;
		super.OnMouseLeave(w, enterW, x, y);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		m_Pagination = SCR_ExternalPaginationUIComponent.Cast(w.FindHandler(SCR_ExternalPaginationUIComponent));
		if (m_Pagination)
		{
			m_Pagination.GetOnShowEntries().Insert(ShowEntries);
			m_Pagination.GetOnPageChanged().Insert(OnPageChanged);
		}
		
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ItemsWidget = w.FindAnyWidget(m_sItemsWidgetName);
		if (!m_ItemsWidget)
		{
			Print(string.Format("m_ItemsWidget '%1' not found!", m_sItemsWidgetName), LogLevel.WARNING);
			return;
		}
		
		SetToolbarVisible(false);
		
		DiagMenu.RegisterRange(SCR_DebugMenuID.DEBUGUI_EDITOR_GUI_TOOLBAR_FILL, "", "Duplicate Toolbar Items", "Editor GUI", "0, 24, 0, 1");		
	}
}
