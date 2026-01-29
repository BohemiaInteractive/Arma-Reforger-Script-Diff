class SCR_MapTaskListUI : SCR_MapUIBaseComponent
{	
	[Attribute("exclamationCircle", desc: "Map task list imageset quad name")]
	protected string m_sTaskListToolMenuIconName;
	
	[Attribute("{ACCF501DD69CAF7B}UI/layouts/Tasks/TaskListWrapper.layout")]
	protected ResourceName m_sTaskListLayout;

	[Attribute("MapTaskList", desc: "Map task list root widget name")]
	protected string m_sMapTaskListFrame;
	
	[Attribute("m_wTaskList", desc: "Task list widget name")]
	protected string m_sTaskList;
	
	[Attribute("8", desc: "Offset substracted from total widget size.", params: "0 inf")]
	protected int m_iOffset;

	[Attribute("0", desc: "Show assign group list")]
	protected bool m_bShowAssignGroupList;

	[Attribute("1", desc: "Show all task types")]
	protected bool m_bShowAllTaskTypenames;

	[Attribute("", UIWidgets.Auto, "Show only task typenames, which is in the list, only if m_bShowAllTaskType is false")]
	protected ref array<string> m_aAllowedTaskTypenames;

	protected const string ICON_NAME = "faction";
	protected const float TASK_LIST_HUD_OFFSET = -0.25;
	protected const string CONTEXT_ASSIGN_GROUP_LIST = "TaskAssignGroupListContext";
	protected Widget m_wTaskList;
	protected Widget m_wTaskListFrame;
	protected SCR_TaskManagerUIComponent m_TaskUIManager;
	protected SCR_MapToolMenuUI m_ToolMenu;
	protected SCR_MapToolEntry m_ToolMenuEntry;

	protected SCR_TaskListUIComponent m_TaskComponent;

	protected bool m_bOpened;
	protected bool m_bMapContextAllowed;
	
	protected const string ACTION_OPEN_TASK_LIST = "TasksOpen";
	
	//------------------------------------------------------------------------------------------------
	//! Recreate task list on map open.
	//! \param[in] config of the map.
	override protected void OnMapOpen(MapConfiguration config)
	{
		m_wTaskListFrame = m_RootWidget.FindAnyWidget(m_sMapTaskListFrame);
		if (!m_wTaskListFrame)
			return;
		
		CreateTaskList(m_wTaskListFrame);
		
		GetGame().GetCallqueue().Call(UpdateTaskListHeight);
		GetGame().GetInputManager().AddActionListener(ACTION_OPEN_TASK_LIST, EActionTrigger.DOWN, ToggleTaskListListener);
		ToggleTaskList(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Destroy task list on map close.
	//! \param[in] config of the map.
	override protected void OnMapClose(MapConfiguration config)
	{	
		SCR_MapCursorModule cursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
		if (cursorModule)
			cursorModule.SetJournalVisibility(false);
		
		GetGame().GetInputManager().RemoveActionListener(ACTION_OPEN_TASK_LIST, EActionTrigger.DOWN, ToggleTaskListListener);
		if (!m_wTaskListFrame)
			return;
		
		array<ref Widget> tasks = {};
		SCR_WidgetHelper.GetAllChildren(m_wTaskListFrame, tasks);
		
		foreach (Widget w : tasks)
		{
			w.RemoveFromHierarchy();
		}
		
		m_ToolMenuEntry.SetActive(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Register task list button to the tools menu.
	override void Init()
	{
		m_TaskUIManager = SCR_TaskManagerUIComponent.GetInstance();
		if (!m_TaskUIManager)
		{
			SetActive(false); // deactivate component
			return;
		}

		m_ToolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (m_ToolMenu)
		{
			m_ToolMenuEntry = m_ToolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, ICON_NAME, 2, m_bIsExclusive);
			m_ToolMenuEntry.m_OnClick.Insert(OnMapTaskListClicked);
			m_ToolMenuEntry.GetOnDisableMapUIInvoker().Insert(DisableMapUIComponent);
			m_ToolMenuEntry.SetEnabled(true);
		}
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);

		//If there is a OverlayWidget like on the DeployMenu we use that instead of the default one
		m_wTaskListFrame = m_RootWidget.FindAnyWidget(m_sMapTaskListFrame);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Create task list 
	//! \param[in] parent of the task list.
	protected void CreateTaskList(Widget parent = null)
	{	
		Widget widget;
		if (parent)
		{
			widget = GetGame().GetWorkspace().CreateWidgets(m_sTaskListLayout, parent);
		}
		else
		{
			widget = GetGame().GetWorkspace().CreateWidgets(m_sTaskListLayout);
			FrameSlot.SetAlignment(widget, 0, TASK_LIST_HUD_OFFSET);
		}
		
		if (!widget)
			return;
		
		m_wTaskList = widget.FindAnyWidget(m_sTaskList);
		if (!m_wTaskList)
			return;
		
		m_TaskComponent = SCR_TaskListUIComponent.Cast(m_wTaskList.FindHandler(SCR_TaskListUIComponent));
		if (!m_TaskComponent)
			return;
		
		if (!m_bShowAllTaskTypenames && m_TaskUIManager)
		{
			m_TaskComponent.AddAllowedTaskTypenames(m_aAllowedTaskTypenames);
		}

		m_TaskComponent.RefreshTaskList();
		m_TaskComponent.GetOnButtonTaskListHide().Insert(OnButtonTaskListHide);
		
		m_TaskComponent.SetTaskListVisibility(false);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_HUD_TASK_MENU_OPEN);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (!m_TaskComponent || !m_TaskComponent.GetTaskDescription())
			return;

		if (m_TaskComponent.GetTaskDescription().IsAssignGroupListOpened())
			GetGame().GetInputManager().ActivateContext(CONTEXT_ASSIGN_GROUP_LIST);
	}

	//------------------------------------------------------------------------------------------------
	//! Update task list height to match tools menu height.
	protected void UpdateTaskListHeight()
	{
		Widget toolMenuBackground = m_ToolMenu.GetBackgroundWidget();
		if (!toolMenuBackground)
			return;
		
		toolMenuBackground.Update();
		
		float sizeW, sizeY;
		toolMenuBackground.GetScreenSize(sizeW, sizeY);
		
		sizeY = GetGame().GetWorkspace().DPIUnscale(sizeY) - m_iOffset;
		
		m_TaskComponent.SetTaskListHeight(sizeY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Toggle task list visibility.
	protected void ToggleTaskListListener()
	{
		ToggleTaskList(!m_bOpened);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] val
	void SetMapContextAllowed(bool val)
	{
		m_bMapContextAllowed = val;
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle task list visibility.
	//! \param[in] visibility new visibility of task list
	void ToggleTaskList(bool visibility)
	{	
		m_wTaskListFrame.SetVisible(visibility);
		if (m_TaskComponent)
		{
			m_TaskComponent.SetTaskListVisibility(visibility);
			m_TaskComponent.FocusOnEntry();
		}
		
		m_bOpened = visibility;
		
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(visibility);
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered on task list button clicked on tools menu. Toggles off visuals of other buttons.
	//! \param[in] comp button component
	protected void OnMapTaskListClicked(notnull SCR_ButtonBaseComponent comp)
	{	
		if (!m_TaskComponent)
			return;
		
		ToggleTaskList(!m_TaskComponent.GetTaskListVisibility());
		
		array<ref SCR_MapToolEntry> menuEntries = {};
		menuEntries = m_ToolMenu.GetMenuEntries();
		if (!menuEntries || menuEntries.IsEmpty())
			return;
		
		Widget mapTaskListFrame;
		foreach (SCR_MapToolEntry toolEntry : menuEntries)
		{
			if (toolEntry.GetImageSet() != m_sTaskListToolMenuIconName)
				continue;
			
			toolEntry.SetActive(false);
				
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonTaskListHide()
	{
		DisableMapUIComponent();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		if (isGamepad && m_bOpened)
		{
			m_bMapContextAllowed = false;
			return;
		}

		m_bMapContextAllowed = true;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		if (m_ToolMenuEntry)
			m_ToolMenuEntry.GetOnDisableMapUIInvoker().Remove(DisableMapUIComponent);

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsTaskListVisible()
	{	
		if (m_wTaskList)
			return m_wTaskList.IsVisible();
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableMapUIComponent()
	{
		ToggleTaskList(false);
	}

	//------------------------------------------------------------------------------------------------
	//! return true if can show assign group list
	bool CanShowAssignGroupList()
	{
		return m_bShowAssignGroupList;
	}
}
