//#define TOOLBAR_DEBUG
[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditorActionType, "m_ActionType")]
class SCR_ActionsToolbarItemEditorUIComponent
{
	[Attribute(SCR_Enum.GetDefault(EEditorActionType.ACTION), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditorActionType))]
	EEditorActionType m_ActionType;
	
	[Attribute(params: "layout")]
	ResourceName m_Layout;
}

//! @ingroup Editor_UI Editor_UI_Components

class SCR_ActionsToolbarEditorUIComponent : SCR_BaseToolbarEditorUIComponent
{
	[Attribute()]
	protected ref array<ref SCR_ActionsToolbarItemEditorUIComponent> m_aItemLayouts;
	
	[Attribute(params: "layout")]
	protected ResourceName m_SeparatorLayout;
	
	protected SCR_ToolbarActionsEditorComponent m_EditorActionsComponent;
	protected ref array<ref SCR_EditorActionData> m_aActionData = {};
	protected SCR_BaseEditorAction m_RepeatAction;
	protected int m_iActionFlags;
	protected ref map<Widget, SCR_BaseEditorAction> m_Actions = new map<Widget, SCR_BaseEditorAction>();
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateItem(SCR_EditorActionData actionData)
	{
		SCR_BaseEditorAction action = actionData.GetAction();
		if (!action)
			return null;
		
		int itemLayoutCount = m_aItemLayouts.Count();
		if (itemLayoutCount == 0)
			return null;
		
		//--- Use first configured layout as default
		m_ItemLayout = m_aItemLayouts[0].m_Layout;
		
		//--- Find actual layout
		for (int i; i < itemLayoutCount; i++)
		{
			if (action.GetActionType() == m_aItemLayouts[i].m_ActionType)
			{
				m_ItemLayout = m_aItemLayouts[i].m_Layout;
				break;
			}
		}
		
		//--- Create layout
		Widget itemWidget;
		SCR_BaseToolbarItemEditorUIComponent item;
		if (!CreateItem(itemWidget, item))
			return null;

		SCR_ActionToolbarItemEditorUIComponent actionItem = SCR_ActionToolbarItemEditorUIComponent.Cast(item);
		if (actionItem)
			actionItem.SetAction(action, itemWidget);
		
		itemWidget.SetName(action.Type().ToString());
		m_Actions.Insert(itemWidget, action);
		return itemWidget;
	}

	//------------------------------------------------------------------------------------------------
	override protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		indexEnd = Math.Min(indexEnd, m_aActionData.Count());
		for (int i = indexStart; i < indexEnd; i++)
		{
			if (m_aActionData[i])
			{
				//--- Action
				if (m_EditorActionsComponent.ActionCanBeShown(m_aActionData[i].GetAction(), vector.Zero, m_iActionFlags))
				{
					Widget itemWidget = CreateItem(m_aActionData[i]);
					if (itemWidget)
						itemWidget.SetEnabled(m_EditorActionsComponent.ActionCanBePerformed(m_aActionData[i].GetAction(), vector.Zero, m_iActionFlags));
				}
			}
			else if (i > indexStart && i < indexEnd - 1)
			{
				//--- Separator (not as first or last item)
				GetGame().GetWorkspace().CreateWidgets(m_SeparatorLayout, m_ItemsWidget);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void Refresh()
	{
		int count = m_EditorActionsComponent.GetAndEvaluateActions(vector.Zero, m_aActionData, m_iActionFlags);
		
		//--- Add separators
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

		#ifdef WORKBENCH
				int debugCount = DiagMenu.GetValue(SCR_DebugMenuID.DEBUGUI_EDITOR_GUI_TOOLBAR_FILL);
				if (count > 0 && debugCount > 0)
				{
					for (int i; i < debugCount; i++)
					{
						m_aActionData.Insert(m_aActionData[i % count]);
					}
					count += debugCount;
				}
		#endif

		if (m_Pagination)
			m_Pagination.SetEntryCount(count);
		
		super.Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Called when nightmode enabled changed to make sure the nightmode action is hidden if global night mode is enabled
	protected void OnGlobalNightModeEnabledChanged(bool enabled)
	{
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnRepeat()
	{
		if (m_RepeatAction)
			m_EditorActionsComponent.ActionPerformInstantly(m_RepeatAction);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);
		
		if (button != 0)
			return false;
		
		SCR_BaseEditorAction action;
		if (m_Actions.Find(w, action))
		{
			m_EditorActionsComponent.ActionPerformInstantly(action);
			
			SCR_ActionsToolbarEditorUIComponent linkedComponent = SCR_ActionsToolbarEditorUIComponent.Cast(m_LinkedComponent);
			if (linkedComponent)
				linkedComponent.m_RepeatAction = action;
		}
		
		if (m_bIsInDialog)
		{
			EditorMenuBase menu = EditorMenuBase.Cast(GetMenu());
			if (menu)
				menu.CloseSelf();
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_EditorActionsComponent = SCR_ToolbarActionsEditorComponent.Cast(SCR_ToolbarActionsEditorComponent.GetInstance(SCR_ToolbarActionsEditorComponent, true));
		if (!m_EditorActionsComponent)
			return;
		
		//--- Initialize all actions
		array<SCR_BaseEditorAction> actions = {};
		SCR_EditorToolbarAction toolbarAction;
		for (int i = 0, count = m_EditorActionsComponent.GetActions(actions); i < count; i++)
		{
			toolbarAction = SCR_EditorToolbarAction.Cast(actions[i]);
			if (toolbarAction)
				toolbarAction.OnInit(this);
		}
		
		//--- ToDo: Don't hardcode, but allow each action to set its refresh event
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (placingManager)
			placingManager.GetOnSelectedPrefabChange().Insert(Refresh);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnCanEndGameChanged().Insert(Refresh);
			editorManager.GetOnLimitedChange().Insert(Refresh);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnGameModeEnd().Insert(Refresh);
			
			SCR_NightModeGameModeComponent nightModeComponent = SCR_NightModeGameModeComponent.Cast(gameMode.FindComponent(SCR_NightModeGameModeComponent));
			if (nightModeComponent)
				nightModeComponent.GetOnGlobalNightModeEnabledChanged().Insert(OnGlobalNightModeEnabledChanged);
		}
			
		super.HandlerAttachedScripted(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		//--- Terminate all actions
		if (m_EditorActionsComponent)
		{
			array<SCR_BaseEditorAction> actions = {};
			SCR_EditorToolbarAction toolbarAction;
			for (int i = 0, count = m_EditorActionsComponent.GetActions(actions); i < count; i++)
			{
				toolbarAction = SCR_EditorToolbarAction.Cast(actions[i]);
				if (toolbarAction)
					toolbarAction.OnExit(this);
			}
		}
		
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, false, true));
		if (placingManager)
			placingManager.GetOnSelectedPrefabChange().Remove(Refresh);
		
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager)
		{
			editorManager.GetOnCanEndGameChanged().Remove(Refresh);
			editorManager.GetOnLimitedChange().Remove(Refresh);
		}
			
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnGameModeEnd().Remove(Refresh);
			
			SCR_NightModeGameModeComponent nightModeComponent = SCR_NightModeGameModeComponent.Cast(gameMode.FindComponent(SCR_NightModeGameModeComponent));
			if (nightModeComponent)
				nightModeComponent.GetOnGlobalNightModeEnabledChanged().Remove(OnGlobalNightModeEnabledChanged);
		}
	}
}
