[EnumLinear()]
enum SCR_ETaskTabType
{
	AVAILABLE,
	FINISHED
}

[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleEnum(SCR_ETaskTabType, "m_eTabType")]
class SCR_TaskTabStates
{
	[Attribute("", UIWidgets.ComboBox, enumType: SCR_ETaskTabType)]
	protected SCR_ETaskTabType m_eTabType;

	[Attribute("0", UIWidgets.Flags, "", enumType: SCR_ETaskState)]
	protected SCR_ETaskState m_eStates;

	//------------------------------------------------------------------------------------------------
	SCR_ETaskTabType GetTabType()
	{
		return m_eTabType;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_ETaskState GetTaskStates()
	{
		return m_eStates;
	}
}

class SCR_TaskListUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{B71975032150BAFB}UI/layouts/Task/TaskListEntry.layout", params: "layout")]
	protected ResourceName m_sTaskListEntryLayout;
	
	[Attribute("{A5326D834EB28E28}UI/layouts/Task/TaskListTabEntry.layout", params: "layout")]
	protected ResourceName m_sTaskTabEntryLayout;

	[Attribute("")]
	protected ref array<ref SCR_TaskTabStates> m_aTaskTabStates;
	
	protected SCR_ETaskTabType m_eSelectedTab = SCR_ETaskTabType.AVAILABLE;

	protected SCR_Task m_CurrentlySelectedTask;
	protected SCR_TaskListEntryDescriptionUIComponent m_TaskListDescriptionComponent;
	protected SCR_TaskManagerUIComponent m_TaskManager;

	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapCursorModule m_MapCursorModule;

	protected ref map<SCR_Task, SCR_TaskListEntryUIComponent> m_mTasksMap;
	protected ref ScriptInvokerVoid m_OnButtonTaskListHide;
	protected ref SCR_TaskListWidgets m_Widgets = new SCR_TaskListWidgets();
	protected ref array<string> m_aAllowedTaskTypenames = {};

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(m_wRoot);

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem)
			return;

		m_mTasksMap = new map<SCR_Task, SCR_TaskListEntryUIComponent>();

		m_Widgets.m_wTaskListEntryDescription.SetVisible(false);
		m_TaskListDescriptionComponent = SCR_TaskListEntryDescriptionUIComponent.Cast(m_Widgets.m_wTaskListEntryDescription.FindHandler(SCR_TaskListEntryDescriptionUIComponent));
		
		if (m_TaskListDescriptionComponent)
			m_TaskListDescriptionComponent.GetOnButtonShowOnMap().Insert(ShowTaskOnMap);

		if (m_Widgets.m_HideTasksButtonComponent)
			m_Widgets.m_HideTasksButtonComponent.m_OnActivated.Insert(OnHideButton);

		m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
		if (m_TaskManager)
		{
			m_TaskManager.RegisterTaskList(this);
			m_TaskManager.GetOnTaskSelected().Insert(OnTaskSelected);
			InititializeTaskTabs();
		}

		m_TaskSystem.GetOnTaskAdded().Insert(OnTaskAdded);
		m_TaskSystem.GetOnTaskRemoved().Insert(OnTaskRemoved);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_TaskListDescriptionComponent)
			m_TaskListDescriptionComponent.GetOnButtonShowOnMap().Remove(ShowTaskOnMap);

		if (m_Widgets.m_HideTasksButtonComponent)
			m_Widgets.m_HideTasksButtonComponent.m_OnActivated.Remove(OnHideButton);

		if (m_TaskManager)
		{
			m_TaskManager.RegisterTaskList(null);
			m_TaskManager.GetOnTaskSelected().Remove(OnTaskSelected);
		}

		if (m_TaskSystem)
		{
			m_TaskSystem.GetOnTaskAdded().Remove(OnTaskAdded);
			m_TaskSystem.GetOnTaskRemoved().Remove(OnTaskRemoved);
		}
	}

	//------------------------------------------------------------------------------------------------
	void FocusOnEntry(SCR_Task task = null)
	{
		Widget widget;
		if (!task && m_eSelectedTab == SCR_ETaskTabType.AVAILABLE && !m_mTasksMap.IsEmpty() && m_mTasksMap.GetElement(0))
			widget = m_mTasksMap.GetElement(0).GetEntryButton();

		if (!widget)
		{
			SCR_TaskListEntryUIComponent comp = m_mTasksMap.Get(task);
			if (comp)
				widget = comp.GetEntryButton();
		}

		if (widget)
		{
			GetGame().GetWorkspace().SetFocusedWidget(widget);
		}
		else
		{
			Widget w = m_Widgets.m_wTabsWrapper.GetChildren();
			if (w)
				GetGame().GetWorkspace().SetFocusedWidget(w);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTabClicked(notnull SCR_ScriptedWidgetComponent comp)
	{
		SCR_TaskListTabEntryUIComponent tabComp = SCR_TaskListTabEntryUIComponent.Cast(comp);
		if (!tabComp)
			return;
		
		SCR_ETaskTabType type = tabComp.GetTabType();
		if (type == m_eSelectedTab)
			return;
		
		m_eSelectedTab = type;
		
		RefreshTaskList();

		Widget widget = tabComp.GetRootWidget();

		array<ref Widget> tabsArray = {};
		SCR_WidgetHelper.GetAllChildren(m_Widgets.m_wTabsWrapper, tabsArray);
		SCR_TaskListTabEntryUIComponent component;
		foreach (Widget w : tabsArray)
		{
			component = SCR_TaskListTabEntryUIComponent.Cast(w.FindHandler(SCR_TaskListTabEntryUIComponent));
			if (!component)
				return;

			if (w == widget)
				component.ChangeTabColor(UIColors.CONTRAST_COLOR);
			else
				component.ChangeTabColor(GUIColors.DEFAULT_GLOW);
		}

		m_Widgets.m_wTasksListScroll.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);

		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();

		if (!m_MapEntity)
			return false;

		if (!m_MapCursorModule)
			m_MapCursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));

		if (m_MapCursorModule)
			m_MapCursorModule.SetJournalVisibility(true);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);
		if (enterW && enterW.GetTypeName() == "ButtonWidget")
			return false;

		if (!m_MapEntity)
			m_MapEntity = SCR_MapEntity.GetMapInstance();

		if (!m_MapEntity)
			return false;

		if (!m_MapCursorModule)
			m_MapCursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));

		if (m_MapCursorModule)
			m_MapCursorModule.SetJournalVisibility(false);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Request refresh of task list on task faction change
	//! \param[in] task which faction was changed
	//! \param[in] factionKey of adjusted faction
	void RequestRefreshFactionChange(SCR_Task task, FactionKey factionKey)
	{
		RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//! Get a list of all available to display tasks. If entry for task is already created, toggles its visibility, otherwise creates a new entry.
	void RefreshTaskList()
	{
		SCR_ExtendedTask extendedTask;
		foreach (SCR_Task oldTask, SCR_TaskListEntryUIComponent oldComponent : m_mTasksMap)
		{
			if (!oldComponent)
				continue;

			extendedTask = SCR_ExtendedTask.Cast(oldTask);
			if (extendedTask)
			{
				extendedTask.GetOnChildTaskAdded().Remove(OnChildTaskChanged);
				extendedTask.GetOnChildTaskRemoved().Remove(OnChildTaskChanged);
			}
			oldComponent.GetOnTaskVisualChanged().Remove(OnTaskVisualChanged);
			oldComponent.GetRootWidget().RemoveFromHierarchy();
		}

		m_mTasksMap.Clear();

		//## Select states to check based on opened tab
		Widget parent;
		if (m_eSelectedTab == SCR_ETaskTabType.FINISHED)
			parent = m_Widgets.m_wTasksWrapper;
		
		SCR_ETaskState stateToCheck = GetStateToCheck();
		array<SCR_Task> allTasks = {};
		m_TaskSystem.GetTasksByState(allTasks, stateToCheck);
		if (allTasks.IsEmpty())
		{
			m_CurrentlySelectedTask = null;
			return;
		}
		
		int playerID = GetGame().GetPlayerController().GetPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		foreach (SCR_Task task : allTasks)
		{
			if (!m_TaskSystem.IsTaskVisibleInTaskList(task))
				continue;

			if (m_TaskSystem.IsTaskVisibleFor(task, player))
				CreateTaskEntry(task, parent, stateToCheck);

			extendedTask = SCR_ExtendedTask.Cast(task);
			if (extendedTask)
			{
				extendedTask.GetOnChildTaskAdded().Insert(OnChildTaskChanged);
				extendedTask.GetOnChildTaskRemoved().Insert(OnChildTaskChanged);
			}
		}

		foreach (SCR_Task task : allTasks)
		{
			if (!m_TaskSystem.IsTaskVisibleInTaskList(task))
				continue;

			if (m_TaskSystem.IsTaskVisibleFor(task, player))
				ReparentTask(task);
		}

		if (m_TaskListDescriptionComponent && m_eSelectedTab != SCR_ETaskTabType.FINISHED)
		{
			SCR_Task descriptionTask = m_TaskListDescriptionComponent.GetCurrentTask();
			if (descriptionTask && (!m_TaskSystem.IsTaskVisibleFor(descriptionTask, SCR_TaskExecutor.FromPlayerID(playerID)) || (descriptionTask.GetTaskState() & (SCR_ETaskState.COMPLETED | SCR_ETaskState.FAILED | SCR_ETaskState.CANCELLED))))
				CollapseTask();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected SCR_ETaskState GetStateToCheck()
	{
		foreach (SCR_TaskTabStates tab : m_aTaskTabStates)
		{
			if (tab.GetTabType() != m_eSelectedTab)
				continue;
			
			return tab.GetTaskStates();
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InititializeTaskTabs()
	{	
		Widget w;
		SCR_TaskListTabEntryUIComponent comp;
		foreach (SCR_ETaskTabType tab : m_TaskManager.GetTaskTabs())
		{
			w = GetGame().GetWorkspace().CreateWidgets(m_sTaskTabEntryLayout, m_Widgets.m_wTabsWrapper);
			if (!w)
				return;

			comp = SCR_TaskListTabEntryUIComponent.Cast(w.FindHandler(SCR_TaskListTabEntryUIComponent));
			if (!comp)
				return;
			
			comp.Initialize(tab);
			
			if (m_eSelectedTab == tab)
				comp.ChangeTabColor(UIColors.CONTRAST_COLOR);
			
			comp.m_OnClick.Insert(OnTabClicked);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates new entry for task
	//! \param[in] task Task which will be initialized.
	//! \param[in] parent Widget, to which newly created entry will be parented.
	protected void CreateTaskEntry(notnull SCR_Task task, Widget parent = null, SCR_ETaskTabType tabType = SCR_ETaskTabType.AVAILABLE)
	{
		// if m_aAllowedTaskTypenames array is set, it will be showed only tasks from the array
		if (!m_aAllowedTaskTypenames.IsEmpty() && !m_aAllowedTaskTypenames.Contains(task.Type().ToString()))
			return;

		if (!parent)
			parent = m_Widgets.m_wTasksWrapper;

		Widget taskWidget = GetGame().GetWorkspace().CreateWidgets(m_sTaskListEntryLayout, parent);
		if (!taskWidget)
			return;

		SCR_TaskListEntryUIComponent taskComp = SCR_TaskListEntryUIComponent.Cast(taskWidget.FindHandler(SCR_TaskListEntryUIComponent));
		if (!taskComp)
			return;

		taskComp.GetOnTaskVisualChanged().Insert(OnTaskVisualChanged);
		m_mTasksMap.Insert(task, taskComp);

		array<SCR_Task> childTasks = {};
		m_TaskSystem.GetChildTasksFor(task, childTasks);

		bool isChildVisible;
		if (childTasks)
		{
			foreach (SCR_Task child : childTasks)
			{
				if (m_TaskSystem.IsTaskVisibleInTaskList(child))
					isChildVisible = true;
			}
		}

		bool hasChildTasks = childTasks && isChildVisible && !childTasks.IsEmpty() && tabType & SCR_ETaskTabType.FINISHED;
		taskComp.InitTask(task, hasChildTasks);
		if (!hasChildTasks)
			return;

		taskComp.SetChildCount(childTasks.Count());
	}

	//------------------------------------------------------------------------------------------------
	//! Reparents task to keep hierachy.
	//! \param[in] task to be evaulated
	protected void ReparentTask(SCR_Task task)
	{
		SCR_Task parentTask;
		Widget parentWrapper;
		m_TaskSystem.GetParentTasksFor(task, parentTask);
		if (parentTask)
		{
			SCR_TaskListEntryUIComponent parentComp = m_mTasksMap.Get(parentTask);
			if (!parentComp)
				return;
			
			parentWrapper = parentComp.GetChildWrapper();
			if (!parentWrapper)
				return;
		}
		
		if (!parentWrapper)
			parentWrapper = m_Widgets.m_wTasksWrapper;
		
		if (!parentWrapper)
			return;
		
		SCR_TaskListEntryUIComponent taskComp = m_mTasksMap.Get(task);
		if (!taskComp)
			return;
		
		Widget childWidget = taskComp.GetRootWidget();
		if (!childWidget)
			return;
		
		parentWrapper.AddChild(childWidget);
	}

	//------------------------------------------------------------------------------------------------
	//!	Invoked whenever task entry on list is selected. Triggers selection in manager.
	//! \param[in] task Selected task.
	protected void UnfoldParent(notnull SCR_Task task)
	{
		SCR_TaskListEntryUIComponent parentUIComp = m_mTasksMap.Get(task);
		if (parentUIComp)
			parentUIComp.SetChildWrapperFolded(null, true);

		SCR_Task parentTask;
		m_TaskSystem.GetParentTasksFor(task, parentTask);
		if (parentTask)
			UnfoldParent(parentTask);
	}

	//------------------------------------------------------------------------------------------------
	//!	Invoked whenever task is selected in manager. Controls display of task description.
	//! \param[in] task Selected task.
	protected void OnTaskSelected(notnull SCR_Task task)
	{
		// Collapse already selected task
		CollapseTask();

		SCR_TaskListEntryUIComponent taskUIComp = m_mTasksMap.Get(task);
		if (!taskUIComp)
		{
			m_CurrentlySelectedTask = null;
			return;
		}

		if (m_CurrentlySelectedTask == task)
		{
			m_CurrentlySelectedTask = null;
			return;
		}

		UnfoldParent(task);
		ExpandTask(task, taskUIComp);
	}

	//------------------------------------------------------------------------------------------------
	//!	Invoked whenever new task is added to the system. Creates entry for it.
	//! \param[in] task Added task.
	protected void OnTaskAdded(SCR_Task task)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask)
		{
			SCR_Task parentTask = extendedTask.GetParentTask();
			if (!parentTask)
			{
				CreateTaskEntry(task);
				return;
			}

			SCR_TaskListEntryUIComponent component = m_mTasksMap.Get(parentTask);
			Widget widget = component.GetRootWidget();

			CreateTaskEntry(task, widget);
		}
		else
		{
			CreateTaskEntry(task);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!	Invoked whenever task is removed from the system. Removes entry from task list.
	//! \param[in] task Removed task.
	protected void OnTaskRemoved(notnull SCR_Task task)
	{
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(task);
		if (extendedTask)
		{
			extendedTask.GetOnChildTaskAdded().Insert(OnChildTaskChanged);
			extendedTask.GetOnChildTaskRemoved().Insert(OnChildTaskChanged);
		}

		SCR_TaskListEntryUIComponent taskComp;
		if (!m_mTasksMap.Find(task, taskComp))
			return;

		if (taskComp)
		{
			taskComp.GetOnTaskVisualChanged().Remove(OnTaskVisualChanged);
			taskComp.GetRootWidget().RemoveFromHierarchy();
			
			if (m_CurrentlySelectedTask == task)
				m_CurrentlySelectedTask = null;
		}

		m_mTasksMap.Remove(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Triggered on hide tasks button clicked, hides task list.
	protected void OnHideButton()
	{
		if (!IsVisible())
			return;
		
		if (m_OnButtonTaskListHide)
			m_OnButtonTaskListHide.Invoke();

		SetTaskListVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	//!	Triggered task SCR_UIInfo is changed. Reinitializes task description with new data.
	//! \param[in] task Changed task.
	protected void OnTaskVisualChanged(SCR_Task task)
	{
		m_TaskListDescriptionComponent.InitDescription(task);
	}

	//------------------------------------------------------------------------------------------------
	//!	Triggered when task is added as a child
	//! \param[in] task Changed task.
	protected void OnChildTaskChanged(SCR_Task task)
	{
		ReparentTask(task);
	}

	//------------------------------------------------------------------------------------------------
	//!	Shows task description and initializes it with task data.
	//! \param[in] task Expanded task.
	//! \param[in] taskUIComp Expanded task UI component.
	protected void ExpandTask(notnull SCR_Task task, notnull SCR_TaskListEntryUIComponent taskUIComp)
	{
		// Show Description of task
		if (!m_TaskListDescriptionComponent)
			return;

		m_CurrentlySelectedTask = task;

		m_TaskListDescriptionComponent.InitDescription(task);

		m_Widgets.m_wTaskListEntryDescription.SetVisible(true);
		taskUIComp.SetEntrySelected(true);
	}

	//------------------------------------------------------------------------------------------------
	//!	Hides task description.
	//! \param[in] taskUIComp UI component of collapsed task.
	void CollapseTask()
	{
		m_Widgets.m_wTaskListEntryDescription.SetVisible(false);

		if (!m_CurrentlySelectedTask)
			return;
		
		SCR_TaskListEntryUIComponent component = m_mTasksMap.Get(m_CurrentlySelectedTask);
		if (component)
			component.SetEntrySelected(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Opens map and moves camera to task location.
	protected void ShowTaskOnMap()
	{
		m_TaskManager.ShowTaskOnMap();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets height of task list.
	//! \param[in] size New height of task list.
	void SetTaskListHeight(float size)
	{
		m_Widgets.m_wSizeLayout.SetHeightOverride(size);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets visibility of task list.
	//! \param[in] size New visibility of task list.
	void SetTaskListVisibility(bool show = false)
	{
		m_wRoot.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Visibility of task list
	bool GetTaskListVisibility()
	{
		return IsVisible();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] visibility of task description.
	void SetTaskDescriptionVisiblity(bool visibility)
	{
		if (visibility)
		{
			SCR_Task task = m_TaskManager.GetSelectedTask();
			if (task)
			{
				SCR_TaskListEntryUIComponent taskComp = m_mTasksMap.Get(task);
				if (taskComp)
					ExpandTask(task, taskComp);
			}
		}
		else
		{
			CollapseTask();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return visibility of task description.
	bool GetTaskDescriptionVisiblity()
	{
		if (m_Widgets.m_wTaskListEntryDescription)
			return m_Widgets.m_wTaskListEntryDescription.IsVisible();

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! \return task description ui component
	SCR_TaskListEntryDescriptionUIComponent GetTaskDescription()
	{
		if (m_TaskListDescriptionComponent)
			return m_TaskListDescriptionComponent;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnButtonTaskListHide()
	{
		if (!m_OnButtonTaskListHide)
			m_OnButtonTaskListHide = new ScriptInvokerVoid();

		return m_OnButtonTaskListHide;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds allowed task typenames and only those will be shown
	//! \param[in] allowedTaskTypenames
	void AddAllowedTaskTypenames(notnull array<string> allowedTaskTypenames)
	{
		foreach (string taskTypename : allowedTaskTypenames)
		{
			if (!m_aAllowedTaskTypenames.Contains(taskTypename))
				m_aAllowedTaskTypenames.Insert(taskTypename);
		}
	}
}
