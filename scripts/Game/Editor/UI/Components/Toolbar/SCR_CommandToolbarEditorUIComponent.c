//! @ingroup Editor_UI Editor_UI_Components

class SCR_CommandToolbarEditorUIComponent : SCR_BaseToolbarEditorUIComponent
{
	[Attribute("", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorActionGroup))]
	protected ref array<EEditorActionGroup> m_ActionGroups;
	
	[Attribute(params: "layout")]
	protected ResourceName m_SeparatorLayout;
	
	protected SCR_CommandActionsEditorComponent m_EditorActionsComponent;
	protected SCR_BaseEditableEntityFilter m_Filter;
	protected ref array<ref SCR_EditorActionData> m_aActionData = {};
	protected ref array<ref SCR_BaseEditorAction> m_aShortcuts = {};
	protected ref map<Widget, SCR_BaseEditorAction> m_Actions = new map<Widget, SCR_BaseEditorAction>();
	
	bool m_queuedRefresh = false;
	//------------------------------------------------------------------------------------------------
	protected void CreateItem(SCR_EditorActionData actionData, int shortcutIndex)
	{
		Widget itemWidget;
		SCR_BaseToolbarItemEditorUIComponent item;
		if (!CreateItem(itemWidget, item))
			return;
		
		SCR_BaseEditorAction action = actionData.GetAction();
		if (!action)
			return;

		SCR_ActionToolbarItemEditorUIComponent actionItem = SCR_ActionToolbarItemEditorUIComponent.Cast(item);
		if (actionItem)
			actionItem.SetAction(action, itemWidget);
		
		//--- Set shortcut reference - shown in GUI, but otherwise has no effect
		action.SetShortcutRef(string.Format("EditorQuickCommand%1", shortcutIndex + 1));
		m_aShortcuts.InsertAt(action, shortcutIndex);
		
		m_Actions.Insert(itemWidget, action);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorQuickCommand(int index)
	{
		if (m_aShortcuts.Count() >= index)
			m_EditorActionsComponent.StartPlacing(m_aShortcuts[index - 1]);
	}
	protected void OnEditorQuickCommand1() { OnEditorQuickCommand(1); }
	protected void OnEditorQuickCommand2() { OnEditorQuickCommand(2); }
	protected void OnEditorQuickCommand3() { OnEditorQuickCommand(3); }
	protected void OnEditorQuickCommand4() { OnEditorQuickCommand(4); }
	protected void OnEditorQuickCommand5() { OnEditorQuickCommand(5); }
	protected void OnEditorQuickCommand6() { OnEditorQuickCommand(6); }
	protected void OnEditorQuickCommand7() { OnEditorQuickCommand(7); }
	protected void OnEditorQuickCommand8() { OnEditorQuickCommand(8); }
	protected void OnEditorQuickCommand9() { OnEditorQuickCommand(9); }
	protected void OnEditorQuickCommand10() { OnEditorQuickCommand(10); }
	//protected void OnEditorQuickCommand11() { OnEditorQuickCommand(11); }
	//protected void OnEditorQuickCommand12() { OnEditorQuickCommand(12); }
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard())
			return;
		
		int shortcutIndex;
		indexEnd = Math.Min(indexEnd, m_aActionData.Count());		
		for (int i = indexStart; i < indexEnd; i++)
		{
			if (m_aActionData[i])
			{
				CreateItem(m_aActionData[i], shortcutIndex);
				shortcutIndex++;
			}
			else if (i > indexStart && i < indexEnd - 1)
			{
				//--- Separator (not as first or last item)
				GetGame().GetWorkspace().CreateWidgets(m_SeparatorLayout, m_ItemsWidget);
			}
		}
	}

	//Many actions can request a refresh for the GM toolbar, but we should only refresh it once per frame.
	protected void QueueRefresh()
	{
		if(m_queuedRefresh)
			return;
		
		GetGame().GetCallqueue().CallLater(Refresh);
		m_queuedRefresh = true;
	}
	//------------------------------------------------------------------------------------------------
	override protected void Refresh()
	{
		int actionFlags;
		m_EditorActionsComponent.ValidateSelection(false);
		int count = m_EditorActionsComponent.GetAndEvaluateActions(vector.Zero, m_aActionData, actionFlags);
		m_aShortcuts.Clear();
		
		//--- Filter by action group
		for (int i = count - 1; i >= 0; i--)
		{
			if (!m_ActionGroups.Contains(m_aActionData[i].GetAction().GetActionGroup()))
			{
				m_aActionData.Remove(i);
				count--;
			}
		}
		
		//--- Add separators
		if (m_SeparatorLayout)
		{
			EEditorActionGroup group, prevGroup;
			for (int i = count - 1; i >= 0; i--)
			{
				group = m_aActionData[i].GetAction().GetActionGroup();
				if (group != prevGroup)
				{
					m_aActionData.InsertAt(null, i + 1);
					prevGroup = group;
					if (i != count - 1)
						count++;
				}
			}
		}
	
#ifdef WORKBENCH
		int debugCount = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_GUI_TOOLBAR_FILL);
		if (count > 0 && debugCount > 0)
		{
			for (int i; i < debugCount; i++)
			{
				if (m_EditorActionsComponent.ActionCanBeShown(m_aActionData[i].GetAction(), vector.Zero, actionFlags))
					m_aActionData.Insert(m_aActionData[i % count]);
			}
			count += debugCount;
		}
#endif
		
		if (m_Pagination)
			m_Pagination.SetEntryCount(count);
		
		super.Refresh();
		
		m_queuedRefresh = false;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	//! \param[in] entitiesInsert
	//! \param[in] entitiesRemove
	protected void OnFilterChange(EEditableEntityState state, set<SCR_EditableEntityComponent> entitiesInsert, set<SCR_EditableEntityComponent> entitiesRemove)
	{
		QueueRefresh();
	}

	//------------------------------------------------------------------------------------------------
	override bool IsUnique()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (!isGamepad)
		{
			m_Filter.GetOnChanged().Insert(OnFilterChange);
			QueueRefresh();
		}
		else
		{
			m_Filter.GetOnChanged().Remove(OnFilterChange);
			DeleteAllItems();
		}
		
		super.OnInputDeviceIsGamepad(isGamepad);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button != 0)
			return false;
		
		SCR_BaseEditorAction action;
		if (m_Actions.Find(w, action))
		{
			m_EditorActionsComponent.StartPlacing(action);
		}
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_EditorActionsComponent = SCR_CommandActionsEditorComponent.Cast(SCR_CommandActionsEditorComponent.GetInstance(SCR_CommandActionsEditorComponent, true));
		if (!m_EditorActionsComponent)
			return;
		
		m_Filter = SCR_BaseEditableEntityFilter.GetInstance(EEditableEntityState.COMMANDED, true); //--- ToDo: Don't hardcode
		if (!m_Filter)
			return;
		
		//--- Assign quick commands (cannot be parametrized, the function would not know which input action called it)
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.AddActionListener("EditorQuickCommand1", EActionTrigger.DOWN, OnEditorQuickCommand1);
		inputManager.AddActionListener("EditorQuickCommand2", EActionTrigger.DOWN, OnEditorQuickCommand2);
		inputManager.AddActionListener("EditorQuickCommand3", EActionTrigger.DOWN, OnEditorQuickCommand3);
		inputManager.AddActionListener("EditorQuickCommand4", EActionTrigger.DOWN, OnEditorQuickCommand4);
		inputManager.AddActionListener("EditorQuickCommand5", EActionTrigger.DOWN, OnEditorQuickCommand5);
		inputManager.AddActionListener("EditorQuickCommand6", EActionTrigger.DOWN, OnEditorQuickCommand6);
		inputManager.AddActionListener("EditorQuickCommand7", EActionTrigger.DOWN, OnEditorQuickCommand7);
		inputManager.AddActionListener("EditorQuickCommand8", EActionTrigger.DOWN, OnEditorQuickCommand8);
		inputManager.AddActionListener("EditorQuickCommand9", EActionTrigger.DOWN, OnEditorQuickCommand9);
		inputManager.AddActionListener("EditorQuickCommand10", EActionTrigger.DOWN, OnEditorQuickCommand10);
		//inputManager.AddActionListener("EditorQuickCommand11", EActionTrigger.DOWN, OnEditorQuickCommand11);
		//inputManager.AddActionListener("EditorQuickCommand12", EActionTrigger.DOWN, OnEditorQuickCommand12);
		
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());

		super.HandlerAttachedScripted(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_Filter)
			m_Filter.GetOnChanged().Remove(OnFilterChange);
		
		super.HandlerDeattached(w);
	}
}
