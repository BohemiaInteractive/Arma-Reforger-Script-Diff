class SCR_TaskListDisplay : SCR_InfoDisplayExtended
{
	[Attribute("m_wTaskListEntryDescription")]
	protected string m_sTaskDescriptionWidgetName;

	protected bool m_bHasListener;
	protected SCR_MapEntity m_MapEntity;

	protected SCR_TaskManagerUIComponent m_TaskManagerUI;
	protected SCR_TaskListUIComponent m_TaskListComponent;

	protected const string ACTION_OPEN_TASK_LIST = "TasksOpen";
	protected const string CONTENT_TASK_LIST_MAP = "TaskListMapContext";
	protected const string CONTENT_TASK_LIST = "TaskListContext";
	
	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;

		m_TaskManagerUI = SCR_TaskManagerUIComponent.GetInstance();

		//## Need to listen for "activated" from input button due to gamepad support so its possible to exit from view with "B" press, but it also triggers action on chimera inputs
		m_TaskListComponent = SCR_TaskListUIComponent.Cast(m_wRoot.FindHandler(SCR_TaskListUIComponent));
		if (m_TaskListComponent)
			m_TaskListComponent.GetOnButtonTaskListHide().Insert(CloseTaskList);

		Widget taskDescriptionWidget = m_wRoot.FindAnyWidget(m_sTaskDescriptionWidgetName);
		if (taskDescriptionWidget)
			taskDescriptionWidget.SetVisible(false);

		GetGame().GetInputManager().AddActionListener(ACTION_OPEN_TASK_LIST, EActionTrigger.DOWN, OpenTaskList);
		m_bHasListener = true;
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (m_MapEntity)
			m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayStopDraw(IEntity owner)
	{
		if (m_TaskListComponent)
			m_TaskListComponent.GetOnButtonTaskListHide().Remove(CloseTaskList);

		if (m_MapEntity)
			m_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		
		GetGame().GetInputManager().RemoveActionListener(ACTION_OPEN_TASK_LIST, EActionTrigger.DOWN, OpenTaskList);
		m_bHasListener = false;
	}

	//------------------------------------------------------------------------------------------------
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		// Activate input Context
		if (m_bShown)
		{
			if (m_MapEntity && m_MapEntity.IsOpen())
				GetGame().GetInputManager().ActivateContext(CONTENT_TASK_LIST_MAP);
			else
				GetGame().GetInputManager().ActivateContext(CONTENT_TASK_LIST);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows task list on HUD
	protected void OpenTaskList()
	{
		if (m_bShown || m_MapEntity && m_MapEntity.IsOpen())
			return;

		Show(true);
		m_TaskListComponent.RefreshTaskList();
				
		if (m_TaskManagerUI)
		{
			m_TaskManagerUI.RegisterTaskList(m_TaskListComponent);
			m_TaskManagerUI.TaskHUDVisibilityChanged(true);
				
			if (!m_TaskManagerUI.GetSelectedTask())
				m_TaskListComponent.CollapseTask();
		}	

		m_TaskListComponent.FocusOnEntry();
		
		GetGame().GetInputManager().RemoveActionListener(ACTION_OPEN_TASK_LIST, EActionTrigger.DOWN, OpenTaskList);
		m_bHasListener = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Hides task list on HUD
	protected void CloseTaskList()
	{
		Show(false);
		if (m_TaskManagerUI)
			m_TaskManagerUI.TaskHUDVisibilityChanged(false);

		GetGame().GetCallqueue().Call(AddTaskListOpenListener);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration config)
	{
		Show(false);
		if (!m_bHasListener)
			AddTaskListOpenListener();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddTaskListOpenListener()
	{
		GetGame().GetInputManager().AddActionListener(ACTION_OPEN_TASK_LIST, EActionTrigger.DOWN, OpenTaskList);
		m_bHasListener = true;
	}
}
