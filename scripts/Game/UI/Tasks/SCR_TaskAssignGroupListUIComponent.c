class SCR_TaskAssignGroupListUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{4178951F3767082E}UI/layouts/Task/TaskAssignGroupListEntry.layout", params: "layout")]
	protected ResourceName m_sTaskAssignSquadEntryLayout;

	protected ref map<int, SCR_TaskAssignGroupListEntryUIComponent> m_mEntries; //!<groupID, SCR_TaskAssignGroupListEntryUIComponent>
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_Task m_Task;
	protected SCR_TaskManagerUIComponent m_TaskManagerUI;
	protected SCR_GroupTaskManagerComponent m_GroupTaskManager;
	protected bool m_bIsListOpened;

	protected ref ScriptInvokerVoid m_OnListOpened;
	protected ref ScriptInvokerVoid m_OnListClosed;

	protected ref SCR_TaskAssignGroupListWidgets m_Widgets = new SCR_TaskAssignGroupListWidgets();

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnListOpened()
	{
		if (!m_OnListOpened)
			m_OnListOpened = new ScriptInvokerVoid();

		return m_OnListOpened;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerVoid GetOnListClosed()
	{
		if (!m_OnListClosed)
			m_OnListClosed = new ScriptInvokerVoid();

		return m_OnListClosed;
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterEvents()
	{
		// register only once
		if (m_TaskSystem)
			return;

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem)
			return;

		SCR_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
		SCR_Task.GetOnTaskAssigneeRemoved().Insert(OnTaskAssigneeRemoved);

		m_mEntries = new map<int, SCR_TaskAssignGroupListEntryUIComponent>();
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		m_GroupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();

		m_TaskManagerUI = SCR_TaskManagerUIComponent.GetInstance();
		if (m_TaskManagerUI)
		{
			m_TaskManagerUI.GetOnTaskSelected().Insert(OnTaskSelected);
			m_Task = m_TaskManagerUI.GetSelectedTask();
		}

		m_Widgets.m_ConfirmGroupAssignmentButtonComponent.m_OnActivated.Insert(OnConfirmGroupAssignmentButtonActivated);
		m_Widgets.m_CloseAssingGroupListComponent.m_OnActivated.Insert(OnCloseAssignGroupListButtonActivated);

		RefreshEntryList();

		if (m_GroupsManager)
		{
			SCR_AIGroup.GetOnGroupRoleChanged().Insert(OnGroupRoleChanged);
			SCR_AIGroup.GetOnPlayerAdded().Insert(OnGroupPlayerAdded);
			SCR_AIGroup.GetOnPlayerRemoved().Insert(OnGroupPlayerRemoved);

			m_GroupsManager.GetOnPlayableGroupCreated().Insert(OnPlayableGroupCreated);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);

		if (SCR_Global.IsEditMode())
			return;

		m_wRoot.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
		SCR_Task.GetOnTaskAssigneeRemoved().Remove(OnTaskAssigneeRemoved);

		if (m_TaskManagerUI)
			m_TaskManagerUI.GetOnTaskSelected().Remove(OnTaskSelected);

		if (m_Widgets.m_ConfirmGroupAssignmentButtonComponent)
			m_Widgets.m_ConfirmGroupAssignmentButtonComponent.m_OnActivated.Remove(OnConfirmGroupAssignmentButtonActivated);

		if (m_Widgets.m_CloseAssingGroupListComponent)
			m_Widgets.m_CloseAssingGroupListComponent.m_OnActivated.Remove(OnCloseAssignGroupListButtonActivated);

		if (m_GroupsManager)
		{
			SCR_AIGroup.GetOnGroupRoleChanged().Remove(OnGroupRoleChanged);
			SCR_AIGroup.GetOnPlayerAdded().Remove(OnGroupPlayerAdded);
			SCR_AIGroup.GetOnPlayerRemoved().Remove(OnGroupPlayerRemoved);

			m_GroupsManager.GetOnPlayableGroupCreated().Remove(OnPlayableGroupCreated);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Open list
	void OpenList()
	{
		RegisterEvents();

		m_bIsListOpened = true;
		m_wRoot.SetVisible(true);

		RefreshEntryList();

		if (m_OnListOpened)
			m_OnListOpened.Invoke();

		if (!m_mEntries)
			return;

		if (!m_mEntries.IsEmpty())
		{
			// focus to first element
			SCR_TaskAssignGroupListEntryUIComponent entry = m_mEntries.GetElement(0);
			if (!entry)
				return;

			entry.SetFocus();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Close list
	void CloseList()
	{
		if (!m_bIsListOpened)
			return;

		m_bIsListOpened = false;
		m_wRoot.SetVisible(false);

		// set focus
		if (m_TaskManagerUI && m_TaskManagerUI.GetTaskListComponent())
			m_TaskManagerUI.GetTaskListComponent().FocusOnEntry(m_Task);

		if (m_OnListClosed)
			m_OnListClosed.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if list is opened
	bool IsListOpened()
	{
		return m_bIsListOpened;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool CanShow()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity || !mapEntity.IsOpen())
			return false;

		MapConfiguration config = mapEntity.GetMapConfig();
		if (!config)
			return false;

		SCR_MapTaskListUI taskListUI;
		foreach (SCR_MapUIBaseComponent mapUIComponent : config.Components)
		{
			taskListUI = SCR_MapTaskListUI.Cast(mapUIComponent);
			if (taskListUI && taskListUI.CanShowAssignGroupList())
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (!task)
			return;

		SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(executor);
		if (!groupExecutor)
			return;

		SCR_TaskAssignGroupListEntryUIComponent entry;
		if (!m_mEntries.Find(groupExecutor.GetGroupID(), entry))
			return;

		// don't change the toggle if user has already changed the toggle
		if (!entry.IsAssignmentChangedByPlayer())
			entry.SetAssignmentToggle(task == m_Task);

		entry.UpdateTaskIcon(m_TaskSystem.GetTaskAssignedTo(groupExecutor));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskAssigneeRemoved(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		if (!task)
			return;

		SCR_TaskExecutorGroup groupExecutor = SCR_TaskExecutorGroup.Cast(executor);
		if (!groupExecutor)
			return;

		SCR_TaskAssignGroupListEntryUIComponent entry;
		if (!m_mEntries.Find(groupExecutor.GetGroupID(), entry))
			return;

		// don't change the toggle if user has already changed the toggle
		if (!entry.IsAssignmentChangedByPlayer())
			entry.SetAssignmentToggle(false);

		entry.UpdateTaskIcon(m_TaskSystem.GetTaskAssignedTo(groupExecutor));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupRoleChanged(int groupId, SCR_EGroupRole groupRole)
	{
		RefreshEntryList();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupPlayerAdded(SCR_AIGroup group, int playerID)
	{
		RefreshEntryList();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupPlayerRemoved(SCR_AIGroup group, int playerID)
	{
		TryRemoveGroupFromList(group);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayableGroupCreated(SCR_AIGroup group)
	{
		RefreshEntryList();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayableGroupRemoved(SCR_AIGroup group)
	{
		RefreshEntryList();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfirmGroupAssignmentButtonActivated(SCR_InputButtonComponent button, string action)
	{
		if (!m_GroupsManager)
			return;

		PlayerController localPlayerController = GetGame().GetPlayerController();
		if (!localPlayerController)
			return;

		SCR_TaskSystemNetworkComponent networkComponent = SCR_TaskSystemNetworkComponent.Cast(localPlayerController.FindComponent(SCR_TaskSystemNetworkComponent));
		if (!networkComponent)
			return;

		SCR_TaskExecutorGroup groupExecutor;
		SCR_Task assignedTask;

		foreach (int groupID, SCR_TaskAssignGroupListEntryUIComponent entry : m_mEntries)
		{
			// if player didn't change assignment
			if (!entry.IsAssignmentChangedByPlayer())
				continue;

			groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(groupID));
			assignedTask = m_TaskSystem.GetTaskAssignedTo(groupExecutor);

			if (m_Task != assignedTask && entry.GetAssignmentToggle())
				networkComponent.AssignTask(m_Task, groupExecutor, force: true, localPlayerController.GetPlayerId());
			else if (m_Task == assignedTask && !entry.GetAssignmentToggle())
				networkComponent.UnassignTask(m_Task, groupExecutor, localPlayerController.GetPlayerId());
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCloseAssignGroupListButtonActivated(SCR_InputButtonComponent button, string action)
	{
		ClearList();
		CloseList();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskSelected(SCR_Task selectedTask)
	{
		ClearList();
		CloseList();
		m_Task = selectedTask;
	}

	//------------------------------------------------------------------------------------------------
	protected bool TryRemoveGroupFromList(notnull SCR_AIGroup group)
	{
		SCR_TaskAssignGroupListEntryUIComponent entry;
		if (m_mEntries.Find(group.GetGroupID(), entry) && group.GetPlayerCount() == 0)
		{
			entry.GetRootWidget().RemoveFromHierarchy();
			m_mEntries.Remove(group.GetGroupID());

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Refresh and update list
	void RefreshEntryList()
	{
		if (!m_Task || !m_GroupTaskManager)
			return;

		if (m_Task.GetTaskState() & (SCR_ETaskState.COMPLETED | SCR_ETaskState.FAILED | SCR_ETaskState.CANCELLED))
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());
		if (!faction)
			return;

		array<SCR_AIGroup> playableGroups = m_GroupTaskManager.GetPlayableGroupsSortedByRelevance(m_Task, faction);
		if (!playableGroups)
			return;

		SCR_TaskExecutorGroup groupExecutor;
		SCR_Task assignedTask;
		SCR_TaskAssignGroupListEntryUIComponent entry;

		foreach (SCR_AIGroup group : playableGroups)
		{
			if (group.GetPlayerCount() == 0)
			{
				TryRemoveGroupFromList(group);
				continue;
			}

			entry = null;
			groupExecutor = SCR_TaskExecutorGroup.Cast(SCR_TaskExecutorGroup.FromGroup(group.GetGroupID()));
			if (m_TaskSystem.CanTaskBeAssignedTo(m_Task, groupExecutor))
			{
				// create entry if is not
				if (!m_mEntries.Find(group.GetGroupID(), entry))
					entry = CreateEntry(group);
			}
			else
			{
				// remove entry if is exist
				TryRemoveGroupFromList(group);
				continue;
			}

			if (!entry)
				continue;

			entry.UpdateEntry();

			assignedTask = m_TaskSystem.GetTaskAssignedTo(groupExecutor);
			if (assignedTask)
			{
				entry.UpdateTaskIcon(assignedTask);
				entry.SetAssignmentToggle(assignedTask == m_Task);
			}
			else
			{
				entry.UpdateTaskIcon(null);
				entry.SetAssignmentToggle(false);
			}
		}

		UpdateConfirmAssignmentButton();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateConfirmAssignmentButton()
	{
		bool isEnabled = true;
		if (!m_mEntries || m_mEntries.IsEmpty())
			isEnabled = false;

		m_Widgets.m_ConfirmGroupAssignmentButtonComponent.SetEnabled(isEnabled);
		m_Widgets.m_wNoRelevantGroupText.SetVisible(!isEnabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearList()
	{
		if (m_mEntries)
			m_mEntries.Clear();

		SCR_WidgetHelper.RemoveAllChildren(m_Widgets.m_wGroupEntries);

		UpdateConfirmAssignmentButton();
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_TaskAssignGroupListEntryUIComponent CreateEntry(notnull SCR_AIGroup group)
	{
		Widget entryWidget = GetGame().GetWorkspace().CreateWidgets(m_sTaskAssignSquadEntryLayout, m_Widgets.m_wGroupEntries);
		if (!entryWidget)
			return null;

		SCR_TaskAssignGroupListEntryUIComponent entryComp = SCR_TaskAssignGroupListEntryUIComponent.Cast(entryWidget.FindHandler(SCR_TaskAssignGroupListEntryUIComponent));
		if (!entryComp)
			return null;

		m_mEntries.Insert(group.GetGroupID(), entryComp);

		entryComp.InitEntry(group);

		return entryComp;
	}
}
