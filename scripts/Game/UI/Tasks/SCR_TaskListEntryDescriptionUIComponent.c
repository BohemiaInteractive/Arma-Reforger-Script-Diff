class SCR_TaskListEntryDescriptionUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{B71975032150BAFB}UI/layouts/Task/TaskListEntry.layout", params: "layout")]
	protected ResourceName m_sTaskListEntry;

	[Attribute("{B568B88051EFA3FA}UI/layouts/Task/AssignedPlayerLabel.layout", params: "layout")]
	protected ResourceName m_sAssignedPlayerEntry;

	[Attribute("{070989E0DF33DAD9}UI/layouts/Task/AssignedGroupLabel.layout", params: "layout")]
	protected ResourceName m_sAssignedGroupEntry;

	[Attribute("#AR-Tasks_TaskUnassign")]
	protected string m_sUnassignTaskText;

	[Attribute("#AR-Tasks_TaskAssign")]
	protected string m_sAssignTaskText;

	[Attribute("#AR-Tasks_TaskCancelled")]
	protected string m_sTaskCancelled;

	[Attribute("#AR-Tasks_TaskFailed")]
	protected string m_sTaskFailed;

	[Attribute("#AR-Tasks_TaskCompleted")]
	protected string m_sTaskCompleted;

	[Attribute(UIColors.GetColorAttribute(UIColors.WARNING_DISABLED))]
	protected ref Color m_TaskStateNegative;

	[Attribute(UIColors.GetColorAttribute(UIColors.CONFIRM))]
	protected ref Color m_TaskStatePositive;

	protected int m_iOffsetY = 4;
	protected bool m_bIsOverflowing;

	protected ref ScriptInvokerVoid m_OnButtonTaskAssign;
	protected ref ScriptInvokerVoid m_OnButtonShowOnMap;

	protected ref Color m_IconColor;
	protected ref Color m_OutlineColor;
	protected ref Color m_BackgroundColor;

	protected SCR_TaskListEntryUIComponent m_TaskComponent;
	protected SCR_TaskManagerUIComponent m_TaskManager;
	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_Task m_Task;

	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapCursorModule m_MapCursorModule;
	protected ref SCR_TaskListEntryDescriptionWidgets m_Widgets = new SCR_TaskListEntryDescriptionWidgets();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(m_wRoot);

		if (m_Widgets.m_ButtonAssignTaskComponent0)
			m_Widgets.m_ButtonAssignTaskComponent0.m_OnActivated.Insert(OnButtonAssign);

		if (m_Widgets.m_ButtonShowMapComponent0)
			m_Widgets.m_ButtonShowMapComponent0.m_OnActivated.Insert(OnButtonShowOnMap);

		m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
		m_TaskSystem = SCR_TaskSystem.GetInstance();

		if (m_TaskManager)
		{
			m_TaskManager.GetOnTaskColorsUpdated().Insert(SetTaskIconColors);
			m_TaskManager.GetOnTaskBackgroundUpdated().Insert(SetTaskBackground);
			m_TaskManager.GetOnTaskHUDVisible().Insert(OnTaskHUDVisible);
		}

		InitAssignGroupList(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_Widgets.m_ButtonAssignTaskComponent0)
			m_Widgets.m_ButtonAssignTaskComponent0.m_OnActivated.Remove(OnButtonAssign);

		if (m_Widgets.m_ButtonShowMapComponent0)
			m_Widgets.m_ButtonShowMapComponent0.m_OnActivated.Remove(OnButtonShowOnMap);

		if (m_TaskManager)
		{
			m_TaskManager.GetOnTaskColorsUpdated().Remove(SetTaskIconColors);
			m_TaskManager.GetOnTaskBackgroundUpdated().Remove(SetTaskBackground);
			m_TaskManager.GetOnTaskHUDVisible().Remove(OnTaskHUDVisible);
		}

		DeinitAssignGroupList(w);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitAssignGroupList(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		if (m_Widgets.m_TaskAssignGroupListComponent && m_Widgets.m_TaskAssignGroupListComponent.CanShow())
		{
			m_Widgets.m_TaskAssignGroupListComponent.GetOnListOpened().Insert(OnAssignGroupListOpened);
			m_Widgets.m_TaskAssignGroupListComponent.GetOnListClosed().Insert(OnAssignGroupListClosed);

			// hide assign button
			m_Widgets.m_wButtonAssignTask.SetVisible(false);
			m_Widgets.m_ButtonOpenAssignGroupListComponent0.m_OnActivated.Insert(OnButtonOpenAssignGroupListActivated);
			m_Widgets.m_wButtonOpenAssignGroupList.SetVisible(true);
		}
		else
		{
			m_Widgets.m_wButtonOpenAssignGroupList.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void DeinitAssignGroupList(Widget w)
	{
		if (m_Widgets.m_ButtonOpenAssignGroupListComponent0)
			m_Widgets.m_ButtonOpenAssignGroupListComponent0.m_OnActivated.Remove(OnButtonOpenAssignGroupListActivated);

		if (m_Widgets.m_TaskAssignGroupListComponent)
		{
			m_Widgets.m_TaskAssignGroupListComponent.GetOnListOpened().Remove(OnAssignGroupListOpened);
			m_Widgets.m_TaskAssignGroupListComponent.GetOnListClosed().Remove(OnAssignGroupListClosed);
		}
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
		super.OnMouseEnter(w, enterW, x, y);

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
	protected void OnButtonAssign()
	{
		m_TaskManager.AssignTask(m_Task);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnButtonShowOnMap()
	{
		if (m_OnButtonShowOnMap)
			m_OnButtonShowOnMap.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates entities assigned to task. If player is assigned, changes task background color.
	//! \param[in] task
	//! \param[in] newAssignee
	//! \param[in] requesterID
	protected void UpdateAssignees(notnull SCR_Task task, SCR_TaskExecutor newAssignee = null, int requesterID = 0)
	{
		if (m_Task != task)
			return;

		m_bIsOverflowing = false;
		m_Widgets.m_wAssignedCountIcon.SetColor(UIColors.NEUTRAL_INFORMATION);
		m_Widgets.m_wTaskIconBackground.SetColor(UIColors.NEUTRAL_INFORMATION);

		SCR_WidgetHelper.RemoveAllChildren(m_Widgets.m_wAssignedPlayersWrapper);

		array<ref SCR_TaskExecutor> assignees;
		m_TaskSystem.GetAssigneesForTask(m_Task, assignees);

		bool hasAssigned = assignees && !assignees.IsEmpty();
		m_Widgets.m_wAssignedPlayersCount.SetVisible(hasAssigned);
		m_Widgets.m_wAssignedPlayersWrapper.SetVisible(hasAssigned);

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		SCR_TaskExecutor group;
		
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (groupsManagerComponent)
		{
			const SCR_AIGroup aiGroup = groupsManagerComponent.GetPlayerGroup(playerID);
			if (aiGroup)
				group = SCR_TaskExecutor.FromGroup(aiGroup.GetGroupID());
		}

		if (!player && !group)
			return;

		if (m_Task.IsTaskAssignedTo(player) || m_Task.IsTaskAssignedTo(group))
			m_Widgets.m_ButtonAssignTaskComponent0.SetLabel(m_sUnassignTaskText);
		else
			m_Widgets.m_ButtonAssignTaskComponent0.SetLabel(m_sAssignTaskText);

		if (!hasAssigned)
			return;

		SCR_TaskExecutorPlayer assigneePlayer;
		SCR_TaskExecutorGroup assigneeGroup;
		foreach (SCR_TaskExecutor assignee : assignees)
		{
			assigneePlayer = SCR_TaskExecutorPlayer.Cast(assignee);
			if (assigneePlayer)
			{
				AddPlayerAssignee(assigneePlayer, player);
				continue;
			}

			assigneeGroup = SCR_TaskExecutorGroup.Cast(assignee);
			if (assigneeGroup)
			{
				const SCR_AIGroup aiGroup = groupsManagerComponent.FindGroup(assigneeGroup.GetGroupID());
				if (!aiGroup)
					continue;

				AddGroupAssignee(assigneeGroup, group, aiGroup);
				continue;
			}
		}

		m_Widgets.m_wAssignedCountText.SetText(m_Task.GetTaskAssigneePlayerCount().ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a player type assignee to the task
	//! \param[in] assignedPlayer to be added
	//! \param[in] player
	protected void AddPlayerAssignee(notnull SCR_TaskExecutorPlayer assignedPlayer, notnull SCR_TaskExecutor player)
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return;

		int playerID = assignedPlayer.GetPlayerID();
		if (assignedPlayer.IsClonedBy(player))
		{
			Faction targetFaction = factionManager.GetPlayerFaction(playerID);
			if (targetFaction)
				m_Widgets.m_wTaskIconBackground.SetColor(targetFaction.GetFactionColor());

			m_Widgets.m_wAssignedCountIcon.SetColor(UIColors.CONTRAST_COLOR);
		}

		if (playerID <= 0)
			return;

		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID);
		if (playerName == string.Empty)
			return;

		CreateAssignedPlayerEntry(playerName, playerID);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a group type assignee to the task
	//! \param[in] assignedGroup to be added
	//! \param[in] group
	//! \param[in] aiGroup
	protected void AddGroupAssignee(notnull SCR_TaskExecutorGroup assignedGroup, notnull SCR_TaskExecutor group, notnull SCR_AIGroup aiGroup)
	{
		Faction targetFaction = aiGroup.GetFaction();
		if (!targetFaction)
			return;

		if (assignedGroup.IsClonedBy(group))
		{
			m_Widgets.m_wTaskIconBackground.SetColor(targetFaction.GetFactionColor());

			m_Widgets.m_wAssignedCountIcon.SetColor(UIColors.CONTRAST_COLOR);
		}

		CreateAssignedGroupEntry(assignedGroup.GetGroupID(), targetFaction, aiGroup);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates label for player assigned to task
	//! \param[in] playerName
	//! \param[in] playerID
	protected void CreateAssignedPlayerEntry(string playerName, int playerID)
	{
		if (m_bIsOverflowing)
			return;

		Widget assignedLabel = GetGame().GetWorkspace().CreateWidgets(m_sAssignedPlayerEntry, m_Widgets.m_wAssignedPlayersWrapper);
		if (!assignedLabel)
			return;

		SCR_AssignedPlayerLabelUIComponent component = SCR_AssignedPlayerLabelUIComponent.Cast(assignedLabel.FindHandler(SCR_AssignedPlayerLabelUIComponent));
		if (!component)
			return;

		component.SetPlayerName(playerName);

		ImageWidget platformIcon = component.GetPlatformIconWidget();
		if (platformIcon)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageToKind(GetGame().GetPlayerManager().GetPlatformKind(playerID), platformIcon, showOnPC: true, showOnXbox: true);
		}

		GetGame().GetCallqueue().CallLater(CheckOverflow, 100, false, assignedLabel);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates group label for group assigned to task
	//! \param[in] groupID
	//! \param[in] groupFaction
	//! \param[in] aiGroup
	protected void CreateAssignedGroupEntry(int groupID, Faction groupFaction, SCR_AIGroup aiGroup)
	{
		if (m_bIsOverflowing)
			return;

		Widget assignedLabel = GetGame().GetWorkspace().CreateWidgets(m_sAssignedGroupEntry, m_Widgets.m_wAssignedPlayersWrapper);
		if (!assignedLabel)
			return;

		SCR_AssignedGroupLabelUIComponent component = SCR_AssignedGroupLabelUIComponent.Cast(assignedLabel.FindHandler(SCR_AssignedGroupLabelUIComponent));
		if (!component)
			return;

		component.LoadGroupName(aiGroup);
		component.LoadGroupFlag(aiGroup);

		GetGame().GetCallqueue().CallLater(CheckOverflow, 100, false, assignedLabel);
	}

	//------------------------------------------------------------------------------------------------
	protected void CheckOverflow(Widget w)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		m_Widgets.m_wAssignedPlayersWrapper.Update();
		float wrapperSizeX, wrapperSizeY;
		m_Widgets.m_wAssignedPlayersWrapper.GetScreenSize(wrapperSizeX, wrapperSizeY);

		float wrapperPosX, wrapperPosY;
		m_Widgets.m_wAssignedPlayersWrapper.GetScreenPos(wrapperPosX, wrapperPosY);

		float widgetSizeX, widgetSizeY;
		w.GetScreenSize(widgetSizeX, widgetSizeY);

		float widgetPosX, widgetPosY;
		w.GetScreenPos(widgetPosX, widgetPosY);

		widgetPosY = widgetPosY - workspace.DPIScale(m_iOffsetY);

		float scaledOffset = workspace.DPIScale(m_iOffsetY);

		if (widgetPosY + widgetSizeY + scaledOffset > wrapperPosY + wrapperSizeY)
		{
			w.SetVisible(false);
			m_bIsOverflowing = true;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Update buttons visibility based on task data. Set task visuals.
	//! \param[in] task
	void InitDescription(notnull SCR_Task task)
	{
		if (m_TaskComponent)
			m_TaskComponent.GetOnTaskVisualChanged().Remove(OnTaskVisualChanged);

		if (m_Task)
		{
			m_Task.GetOnTaskAssigneeAdded().Remove(UpdateAssignees);
			m_Task.GetOnTaskAssigneeRemoved().Remove(UpdateAssignees);
			m_Task.GetOnTaskUIVisibilityChanged().Remove(OnTaskUIVisibilityChanged);
			m_Task.GetOnTaskVisibilityChanged().Remove(OnTaskVisibilityChanged);
			m_Task.GetOnTaskOwnershipChanged().Remove(OnTaskOwnershipChanged);
		}

		m_Task = task;
		m_Task.GetOnTaskAssigneeAdded().Insert(UpdateAssignees);
		m_Task.GetOnTaskAssigneeRemoved().Insert(UpdateAssignees);
		m_Task.GetOnTaskUIVisibilityChanged().Insert(OnTaskUIVisibilityChanged);
		m_Task.GetOnTaskVisibilityChanged().Insert(OnTaskVisibilityChanged);
		m_Task.GetOnTaskOwnershipChanged().Insert(OnTaskOwnershipChanged);

		SCR_WidgetHelper.RemoveAllChildren(m_Widgets.m_wChildTasksWrapper);

		UpdateTask();

		// Reset the position of the description text slider
		m_Widgets.m_wDescriptionScrollLayout.SetSliderPos(0, 0);
		m_Widgets.m_wTasksScrollLayout.SetSliderPos(0, 0);
		m_Widgets.m_wChildTasks.SetVisible(false);

		SetFactionColors();
		UpdateAssignees(task);

		array<SCR_Task> childTasks = {};
		m_TaskSystem.GetChildTasksFor(m_Task, childTasks);

		SCR_ETaskState state = m_TaskSystem.GetTaskState(m_Task);
		HandleTaskStateVisuals(state);
	
		m_Widgets.m_wButtonShowMap.SetVisible(m_TaskSystem.IsTaskVisibleOnMap(m_Task));

		if ((state & (SCR_ETaskState.CREATED | SCR_ETaskState.ASSIGNED | SCR_ETaskState.PROGRESSED)) && (!childTasks || childTasks.IsEmpty()))
		{
			m_Widgets.m_wButtonAssignTask.SetVisible(true);

			int playerID = SCR_PlayerController.GetLocalPlayerId();
			SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
			if (player && m_TaskSystem.CanTaskBeAssignedTo(m_Task, player))
				m_Widgets.m_ButtonAssignTaskComponent0.SetEnabled(true);
			else
				m_Widgets.m_ButtonAssignTaskComponent0.SetEnabled(false);
		}
		else
		{
			m_Widgets.m_wButtonAssignTask.SetVisible(false);
			if (!childTasks || childTasks.IsEmpty())
				return;

			int playerID = SCR_PlayerController.GetLocalPlayerId();
			SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
			if (!player)
				return;

			foreach (SCR_Task subTask : childTasks)
			{
				if (!m_TaskSystem.IsTaskVisibleInTaskList(subTask))
					continue;

				if (m_TaskSystem.IsTaskVisibleFor(subTask, player))
				{
					CreateTaskEntry(subTask, m_Widgets.m_wChildTasksWrapper);
					m_Widgets.m_wChildTasks.SetVisible(true);
				}
			}
		}

		if (m_Widgets.m_TaskAssignGroupListComponent && m_Widgets.m_TaskAssignGroupListComponent.CanShow())
		{
			// hide assign button
			if (m_Widgets.m_wButtonAssignTask)
				m_Widgets.m_wButtonAssignTask.SetVisible(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates new entry for task.
	//! \param[in] task Task which will be initialized.
	//! \param[in] parent Widget, to which newly created entry will be parented.
	protected void CreateTaskEntry(SCR_Task task, Widget parent = null, bool isChild = false)
	{	
		if (!parent)
			parent = m_Widgets.m_wChildTasksWrapper;
		
		Widget taskWidget = GetGame().GetWorkspace().CreateWidgets(m_sTaskListEntry, parent);
		if (!taskWidget)
			return;

		SCR_TaskListEntryUIComponent taskComp = SCR_TaskListEntryUIComponent.Cast(taskWidget.FindHandler(SCR_TaskListEntryUIComponent));
		if (!taskComp)
			return;

		taskComp.InitTask(task, isChild);
		taskComp.GetOnTaskVisualChanged().Insert(OnTaskVisualChanged);

		array<SCR_Task> childTasks = {};
		m_TaskSystem.GetChildTasksFor(task, childTasks);
		if (!childTasks || childTasks.IsEmpty())
			return;

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return;

		Widget childWrapper = taskComp.GetChildWrapper();
		foreach (SCR_Task subTask : childTasks)
		{
			if (!m_TaskSystem.IsTaskVisibleInTaskList(subTask))
				continue;

			if (m_TaskSystem.IsTaskVisibleFor(subTask, player))
				CreateTaskEntry(subTask, childWrapper);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!	Invoked whenever task entry on list is selected. Triggers selection in manager.
	//! \param[in] task Selected task.
	protected void OnTaskEntrySelected(SCR_Task task)
	{
		m_TaskManager.SetSelectedTask(task);
	}

	//------------------------------------------------------------------------------------------------
	//!	Triggered task SCR_UIInfo is changed. Reinitializes task description with new data.
	//! \param[in] task Changed task.
	protected void OnTaskVisualChanged(SCR_Task task)
	{
		InitDescription(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visual data is changed
	protected void OnTaskUIVisibilityChanged(SCR_Task task, SCR_ETaskUIVisibility visible)
	{
		if (m_Task != task)
			return;
		
		m_Widgets.m_wButtonShowMap.SetVisible(visible != SCR_ETaskUIVisibility.LIST_ONLY);
		
		SetVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visibility is changed
	protected void OnTaskVisibilityChanged(SCR_Task task, SCR_ETaskVisibility visible)
	{
		if (m_Task != task)
			return;

		SetVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task ownership is changed
	protected void OnTaskOwnershipChanged(SCR_Task task, SCR_ETaskOwnership ownership)
	{
		if (m_Task != task)
			return;

		SetVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when open assign group list button is activated by action
	protected void OnButtonOpenAssignGroupListActivated(SCR_InputButtonComponent button, string action)
	{
		if (!m_Widgets.m_TaskAssignGroupListComponent || !m_Widgets.m_TaskAssignGroupListComponent.CanShow())
			return;

		if (!m_Widgets.m_TaskAssignGroupListComponent.IsListOpened())
			m_Widgets.m_TaskAssignGroupListComponent.OpenList();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when assign group list is opened
	protected void OnAssignGroupListOpened()
	{
		if (m_Widgets.m_wButtonOpenAssignGroupList)
			m_Widgets.m_wButtonOpenAssignGroupList.SetVisible(false);

		if (m_Widgets.m_wButtonShowMap)
			m_Widgets.m_wButtonShowMap.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when assign group list is closed
	protected void OnAssignGroupListClosed()
	{
		if (m_Widgets.m_wButtonOpenAssignGroupList)
			m_Widgets.m_wButtonOpenAssignGroupList.SetVisible(true);

		if (m_Widgets.m_wButtonShowMap)
			m_Widgets.m_wButtonShowMap.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskHUDVisible(bool visible)
	{
		// task description is updated if it is opened together with task hud
		if (m_Task && visible && IsVisible())
		{
			UpdateTask();
			UpdateAssignees(m_Task);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! \return true if assing group list is opened
	bool IsAssignGroupListOpened()
	{
		return m_Widgets.m_TaskAssignGroupListComponent && m_Widgets.m_TaskAssignGroupListComponent.IsListOpened();
	}

	//------------------------------------------------------------------------------------------------
	//! Set task icon and title to the ones set in tasks' SCR_TaskUIInfo.
	void UpdateTask()
	{
		m_Widgets.m_TitleFrameComponent.AnimationStop();
		
		SCR_TaskUIInfo info = m_Task.GetTaskUIInfo();
		if (!info)
			return;
		
		if (m_Task.GetAuthorID() > 0)
		{
			m_Widgets.m_wAuthorFrame.SetVisible(true);
			
			string playerName = GetGame().GetPlayerManager().GetPlayerName(m_Task.GetAuthorID());
			m_Widgets.m_TaskAuthorComponent.SetPlayerName(playerName);
			
			ImageWidget platformIcon = m_Widgets.m_TaskAuthorComponent.GetPlatformIconWidget();
			if (platformIcon)
			{
				platformIcon.SetVisible(false);
				PlatformKind platformKind = GetGame().GetPlayerManager().GetPlatformKind(m_Task.GetAuthorID());
				SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
				if (playerController)
					playerController.SetPlatformImageToKind(platformKind, platformIcon, showOnPC: true, showOnXbox: true);
			}
		}
		else
			m_Widgets.m_wAuthorFrame.SetVisible(false);
		
		info.SetNameTo(m_Widgets.m_wTaskTitle);
		info.SetDescriptionTo(m_Widgets.m_wDescription);
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(m_Task);
		if (extendedTask)
			SetChildTaskText(extendedTask.GetCustomSubtaskDescription());

		m_Widgets.m_wAssignedCountText.SetText(m_Task.GetTaskAssigneePlayerCount().ToString());

		bool iconSet = info.SetIconTo(m_Widgets.m_wTaskIconSymbol);
		if (!iconSet)
		{
			ResourceName imageset = m_TaskManager.GetDefaultImageset();
			if (!imageset)
				return;

			string iconName = m_TaskManager.GetDefaultIcon();
			if (!iconName)
				return;

			m_Widgets.m_wTaskIconSymbol.LoadImageFromSet(0, imageset, iconName);
		}
		
		float frameX, frameY, textX, textY;
		m_Widgets.m_wTitleFrame.GetScreenSize(frameX, frameY);
		m_Widgets.m_wTaskTitle.GetScreenSize(textX, textY);
		
		if (textX > frameX)		
			m_Widgets.m_TitleFrameComponent.AnimationStart();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates task visuals to color of players faction
	void SetFactionColors()
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (!faction)
			return;

		m_Widgets.m_wTaskIconBackground.SetColor(faction.GetFactionColor());

		Color color = faction.GetOutlineFactionColor();
		if (!color)
			return;

		m_IconColor = color;
		m_OutlineColor = color;
		m_BackgroundColor = UIColors.NEUTRAL_INFORMATION;

		UpdateTaskIconColors(UIColors.NEUTRAL_INFORMATION, color, color);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task icon colors
	//! \param[in] Color of background
	//! \param[in] Color of icon
	//! \param[in] Color of outline
	void UpdateTaskIconColors(Color backgroundColor, Color iconColor, Color outlineColor)
	{
		if (!backgroundColor)
			backgroundColor = m_BackgroundColor;

		if (!iconColor)
			iconColor = m_IconColor;

		if (!outlineColor)
			outlineColor = m_OutlineColor;

		m_Widgets.m_wTaskIconBackground.SetColor(backgroundColor);
		m_Widgets.m_wTaskIconSymbol.SetColor(iconColor);
		m_Widgets.m_wTaskIconOutline.SetColor(outlineColor);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task icon colors
	//! \param[in] Color of background
	//! \param[in] Color of icon
	//! \param[in] Color of outline
	void SetTaskIconColors(Color backgroundColor = null, Color iconColor = null, Color outlineColor = null)
	{
		if (backgroundColor)
			m_BackgroundColor = backgroundColor;

		if (iconColor)
			m_IconColor = iconColor;

		if (outlineColor)
			m_OutlineColor = outlineColor;
	}

	//------------------------------------------------------------------------------------------------
	//! Sets new background of task.
	//! \param[in] imageset with the background.
	//! \param[in] name in imageset.
	void SetTaskBackground(ResourceName imageset, string name)
	{
		if (imageset.IsEmpty() || name.IsEmpty())
			return;

		m_Widgets.m_wTaskIconBackground.LoadImageFromSet(0, imageset, name);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task visuals based on new state of task.
	//! \param[in] state New state of task.
	protected void HandleTaskStateVisuals(SCR_ETaskState state)
	{
		m_Widgets.m_wTaskStateWrapper.SetVisible(true);
		m_Widgets.m_wOverlayTaskStates.SetVisible(true);
		m_Widgets.m_wIconOverlay.SetOpacity(0.5);
		m_Widgets.m_wAssignedPlayersWrapper.SetVisible(false);
		m_Widgets.m_wButtonAssignTask.SetVisible(false);
		m_Widgets.m_wButtonShowMap.SetVisible(false);
		m_Widgets.m_wIconOverlayCancelled.SetVisible(false);
		m_Widgets.m_wStateIconOverlayCancelled.SetVisible(false);
		m_Widgets.m_wIconOverlayFinished.SetVisible(false);
		m_Widgets.m_wStateIconOverlayFinished.SetVisible(false);
		m_Widgets.m_wIconOverlayFailed.SetVisible(false);
		m_Widgets.m_wStateIconOverlayFailed.SetVisible(false);

		switch (state)
		{
			case SCR_ETaskState.COMPLETED:
			{
				m_Widgets.m_wIconOverlayFinished.SetVisible(true);
				m_Widgets.m_wStateIconOverlayFinished.SetVisible(true);
				m_Widgets.m_wTaskStateText.SetText(m_sTaskCompleted);
				m_Widgets.m_wTaskStateText.SetColor(m_TaskStatePositive);
				break;
			}

			case SCR_ETaskState.FAILED:
			{
				m_Widgets.m_wIconOverlayFailed.SetVisible(true);
				m_Widgets.m_wStateIconOverlayFailed.SetVisible(true);
				m_Widgets.m_wTaskStateText.SetText(m_sTaskFailed);
				m_Widgets.m_wTaskStateText.SetColor(m_TaskStateNegative);
				break;
			}

			case SCR_ETaskState.CANCELLED:
			{
				m_Widgets.m_wIconOverlayCancelled.SetVisible(true);
				m_Widgets.m_wStateIconOverlayCancelled.SetVisible(true);
				m_Widgets.m_wTaskStateText.SetText(m_sTaskCancelled);
				m_Widgets.m_wTaskStateText.SetColor(m_TaskStateNegative);
				break;
			}

			default:
			{
				m_Widgets.m_wOverlayTaskStates.SetVisible(false);
				m_Widgets.m_wIconOverlay.SetOpacity(1);
				m_Widgets.m_wTaskStateWrapper.SetVisible(false);
				m_Widgets.m_wAssignedPlayersWrapper.SetVisible(true);
				m_Widgets.m_wButtonAssignTask.SetVisible(true);
				m_Widgets.m_wButtonShowMap.SetVisible(true);
				break;
			}
		}

		if (m_Widgets.m_TaskAssignGroupListComponent && m_Widgets.m_TaskAssignGroupListComponent.CanShow())
		{
			// hide assign button
			if (m_Widgets.m_wButtonAssignTask)
				m_Widgets.m_wButtonAssignTask.SetVisible(false);

			if (m_Widgets.m_wButtonShowMap)
				m_Widgets.m_wButtonShowMap.SetVisible(!m_Widgets.m_TaskAssignGroupListComponent.IsListOpened());
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetChildTaskText(string text)
	{
		if (!text)
			return;

		m_Widgets.m_wChildTaskText.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets visibility of entry
	//! \param[in] show New visibility
	void SetVisibility(bool show)
	{
		m_wRoot.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	SCR_Task GetCurrentTask()
	{
		if (m_Task)
			return m_Task;

		return null;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnButtonShowOnMap()
	{
		if (!m_OnButtonShowOnMap)
			m_OnButtonShowOnMap = new ScriptInvokerVoid();

		return m_OnButtonShowOnMap;
	}
}
