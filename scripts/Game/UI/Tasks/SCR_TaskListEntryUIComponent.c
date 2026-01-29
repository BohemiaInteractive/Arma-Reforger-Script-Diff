void TaskWidgetSelected(SCR_Task task);
typedef func TaskWidgetSelected;
typedef ScriptInvokerBase<TaskWidgetSelected> SCR_TaskWidgetSelectedInvoker;

void TaskVisualsUpdated(notnull SCR_Task task);
typedef func TaskVisualsUpdated;
typedef ScriptInvokerBase<TaskVisualsUpdated> SCR_TaskVisualsChangedInvoker;

class SCR_TaskListEntryUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION))]
	protected ref Color m_UnassignedTaskColor;

	[Attribute(UIColors.GetColorAttribute(Color.FromSRGBA(43, 2, 2, 255)))]
	protected ref Color m_IconColor;

	[Attribute(UIColors.GetColorAttribute(Color.FromSRGBA(43, 2, 2, 255)))]
	protected ref Color m_OutlineColor;

	[Attribute(UIColors.GetColorAttribute(Color.FromSRGBA(255, 255, 255, 255)))]
	protected ref Color m_BackgroundColor;

	protected ref Color m_FactionColor;
	protected SCR_Task m_Task;
	protected SCR_TaskManagerUIComponent m_TaskManager;

	protected ref SCR_TaskWidgetSelectedInvoker m_OnTaskSelected;
	protected ref SCR_TaskVisualsChangedInvoker m_OnTaskVisualUpdated;
	protected ref SCR_TaskListEntryWidgets m_Widgets = new SCR_TaskListEntryWidgets();

	protected const int MAX_CHILD_COUNT = 9;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);
		if (m_Widgets.m_ButtonComponent)
		{	
			m_Widgets.m_ButtonComponent.m_OnClicked.Insert(OnTaskEntryClicked);
			m_Widgets.m_ButtonComponent.m_OnMouseEnter.Insert(OnHoveredStart);
			m_Widgets.m_ButtonComponent.m_OnMouseLeave.Insert(OnHoveredEnd);
		}
		
		if (m_Widgets.m_ButtonFoldComponent)
			m_Widgets.m_ButtonFoldComponent.m_OnClicked.Insert(OnWrapperFolded);

		m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
		if (!m_TaskManager)
			return;
		
		m_TaskManager.GetOnTaskColorsUpdated().Insert(SetTaskIconColors);
		m_TaskManager.GetOnTaskBackgroundUpdated().Insert(SetTaskBackground);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_Task)
		{
			m_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
			m_Task.GetOnTaskAssigneeRemoved().Remove(OnTaskAssigneeAdded);
			m_Task.GetOnDisplayDataChanged().Remove(OnDisplayDataChanged);
			m_Task.GetOnTaskStateChanged().Remove(OnTaskStateChanged);
			m_Task.GetOnTaskUIVisibilityChanged().Remove(OnTaskUIVisibilityChanged);
			m_Task.GetOnTaskVisibilityChanged().Remove(OnTaskVisibilityChanged);
			m_Task.GetOnTaskOwnershipChanged().Remove(OnTaskOwnershipChanged);
			m_Task.GetOnOwnerFactionAdded().Remove(OnTaskFactionChanged);
			m_Task.GetOnOwnerFactionRemoved().Remove(OnTaskFactionChanged);
			m_Task.GetOnOwnerGroupAdded().Remove(OnTaskGroupChanged);
			m_Task.GetOnOwnerGroupRemoved().Remove(OnTaskGroupChanged);
		}

		if (m_Widgets.m_ButtonComponent)
		{
			m_Widgets.m_ButtonComponent.m_OnClicked.Remove(OnTaskEntryClicked);
			m_Widgets.m_ButtonComponent.m_OnMouseEnter.Remove(OnHoveredStart);
			m_Widgets.m_ButtonComponent.m_OnMouseLeave.Remove(OnHoveredEnd);
		}

		if (m_Widgets.m_ButtonFoldComponent)
			m_Widgets.m_ButtonFoldComponent.m_OnClicked.Remove(OnWrapperFolded);

		if (!m_TaskManager)
			return;

		m_TaskManager.GetOnTaskColorsUpdated().Remove(SetTaskIconColors);
		m_TaskManager.GetOnTaskBackgroundUpdated().Remove(SetTaskBackground);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoveredStart(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		if (CheckTitleOverflow())
			m_Widgets.m_TitleFrameComponent.AnimationStart();

		GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_wButton);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoveredEnd(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		m_Widgets.m_TitleFrameComponent.AnimationStop();
		m_Widgets.m_TitleFrameComponent.ResetPosition();
	}

	//------------------------------------------------------------------------------------------------
	//! \return True when content doesn't fit parent widget.
	protected bool CheckTitleOverflow()
	{
		float frameX, frameY;
		m_Widgets.m_wTitleFrame.GetScreenSize(frameX, frameY);

		float titleX, titleY;
		m_Widgets.m_wTaskTitle.GetScreenSize(titleX, titleY);

		return frameX < titleX;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Number of IEntities assigned to task.
	protected int GetAssignees()
	{
		if (!m_Task)
			return 0;
	
		return m_Task.GetTaskAssigneePlayerCount();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes task and sets its visuals
	//! \param[in] task Initialized task
	void InitTask(notnull SCR_Task task, bool hasChild = false)
	{
		m_Task = task;
		m_BackgroundColor = UIColors.NEUTRAL_INFORMATION;

		// Update task icon and title
		UpdateTask();

		// Set assignees visuals
		SetAssigneeCount(GetAssignees());

		m_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
		m_Task.GetOnTaskAssigneeRemoved().Insert(OnTaskAssigneeAdded);
		m_Task.GetOnTaskStateChanged().Insert(OnTaskStateChanged);
		m_Task.GetOnDisplayDataChanged().Insert(OnDisplayDataChanged);
		m_Task.GetOnTaskUIVisibilityChanged().Insert(OnTaskUIVisibilityChanged);
		m_Task.GetOnTaskVisibilityChanged().Insert(OnTaskVisibilityChanged);
		m_Task.GetOnTaskOwnershipChanged().Insert(OnTaskOwnershipChanged);
		m_Task.GetOnOwnerFactionAdded().Insert(OnTaskFactionChanged);
		m_Task.GetOnOwnerFactionRemoved().Insert(OnTaskFactionChanged);
		m_Task.GetOnOwnerGroupAdded().Insert(OnTaskGroupChanged);
		m_Task.GetOnOwnerGroupRemoved().Insert(OnTaskGroupChanged);
		
		SCR_ExtendedTask extendedTask = SCR_ExtendedTask.Cast(m_Task);
		if (extendedTask)
			extendedTask.GetOnProgressChanged().Insert(OnProgressChanged);

		if (hasChild)
		{
			m_Widgets.m_wSizeFoldButton.SetVisible(true);
			SetChildWrapperFolded(null, false);
		}

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		// Handle progression bar
		if (taskSystem.CanProgressBeShownForTask(m_Task))
		{
			float progress = taskSystem.GetTaskProgress(m_Task);
			m_Widgets.m_wProgressBar.SetVisible(true);
			SetProgressValue(progress);
		}

		//## Don't show task if not visible for player
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor executor = SCR_TaskExecutor.FromPlayerID(playerID);
		if (executor)
			m_wRoot.SetVisible(taskSystem.IsTaskVisibleFor(task, executor));

		SetIconFactionColor();

		if (!executor)
			return;

		HandleTaskState(m_Task.GetTaskState(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the icon color to the faction color if local player has the task assigned or local player is faction commander and task has any assignees
	void SetIconFactionColor()
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (!faction)
			return;

		Color factionColor = faction.GetFactionColor();
		if (factionColor)
			m_FactionColor = factionColor;

		Color color = faction.GetOutlineFactionColor();
		if (color)
		{
			m_IconColor = color;
			m_OutlineColor = color;
		}

		SCR_TaskExecutor executor = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!executor)
			return;

		if (m_FactionColor && (m_Task.IsTaskAssignedTo(executor) || faction.IsPlayerCommander(playerID) && m_Task.GetTaskAssigneeCount() > 0 ))
			UpdateTaskIconColors(m_FactionColor);
		else
			UpdateTaskIconColors();
	}

	//------------------------------------------------------------------------------------------------
	//! Toggles child tasks fold
	void OnWrapperFolded()
	{
		SetChildWrapperFolded();
	}

	//------------------------------------------------------------------------------------------------
	//! Toggles child tasks fold
	void SetChildWrapperFolded(SCR_ModularButtonComponent comp = null, bool toggled = -1)
	{
		if (toggled == -1)
			toggled = !m_Widgets.m_wChildTaskWrapper.IsVisible();

		m_Widgets.m_wChildTaskWrapper.SetVisible(toggled);

		if (toggled)
			AnimateWidget.Rotation(m_Widgets.m_wIconFold, 180, 5);
		else
			AnimateWidget.Rotation(m_Widgets.m_wIconFold, 0, 5);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task button is clicked.
	protected void OnTaskEntryClicked()
	{
		if (!m_Task || !m_TaskManager)
			return;
		
		if (m_TaskManager.GetSelectedTask() == m_Task)
		{
			SetEntrySelected(false);
			m_TaskManager.SetSelectedTask(null);
		}
		else
		{
			m_TaskManager.SetSelectedTask(m_Task);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when IEntity is assigned to task.
	//! If player is assignee, sets color of task background to currently controlled character faction color.
	//! \param[in] task
	//! \param[in] assignee IEntity being assigned to task.
	//! \param[in] requesterID
	protected void OnTaskAssigneeAdded(notnull SCR_Task task, SCR_TaskExecutor assignee, int requesterID)
	{
		if (m_Task != task)
			return;

		SetAssigneeCount(GetAssignees());
		if (!m_BackgroundColor)
			m_BackgroundColor = UIColors.NEUTRAL_INFORMATION;

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (m_Task.IsTaskAssignedTo(player))
		{
			//## Assign task if it's not assigned already
			if (m_TaskManager && !m_TaskManager.GetAssigedTask())
				m_TaskManager.AssignTask(task);

			if (!faction)
				return;

			Color color = faction.GetFactionColor();
			if (!color)
				return;

			m_Widgets.m_wTaskIconBackground.SetColor(color);
		}
		else if (faction && faction.IsPlayerCommander(playerID) && m_Task.GetTaskAssigneeCount() > 0)
		{
			Color color = faction.GetFactionColor();
			if (!color)
				return;

			UpdateTaskIconColors(color);
		}
		else
		{
			if (m_TaskManager && m_Task == m_TaskManager.GetSelectedTask())
			{
				UpdateTaskIconColors(UIColors.CONTRAST_COLOR);
				return;
			}

			m_Widgets.m_wTaskIconBackground.SetColor(m_BackgroundColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked on task state change
	//! \param[in] state New state of task.
	protected void OnTaskStateChanged(SCR_Task task, SCR_ETaskState state)
	{
		SetAssigneeCount(GetAssignees());

		HandleTaskState(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task visuals based on new state of task.
	//! \param[in] state New state of task.
	//! \param[in] forceRefresh of task list.
	protected void HandleTaskState(SCR_ETaskState state, bool forceRefresh = true)
	{
		m_Widgets.m_wOverlayTaskStates.SetVisible(true);
		switch (state)
		{
			case SCR_ETaskState.COMPLETED:
			{
				m_Widgets.m_wIconOverlay.SetOpacity(0.5);
				m_Widgets.m_wIconOverlayFinished.SetVisible(true);
				break;
			}

			case SCR_ETaskState.FAILED:
			{
				m_Widgets.m_wIconOverlay.SetOpacity(0.5);
				m_Widgets.m_wIconOverlayFailed.SetVisible(true);
				break;
			}

			case SCR_ETaskState.CANCELLED:
			{
				m_Widgets.m_wIconOverlay.SetOpacity(0.5);
				m_Widgets.m_wIconOverlayCancelled.SetVisible(true);
				break;
			}

			default:
			{
				forceRefresh = false;
				m_Widgets.m_wIconOverlay.SetOpacity(1);
				m_Widgets.m_wOverlayTaskStates.SetVisible(false);
				break;
			}
		}

		if (forceRefresh && m_TaskManager)
			m_TaskManager.RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task progression visuals based on new progress.
	//! \param[in] change Difference between old and new progress.
	//! \param[in] newProgress New value of task progress.
	protected void OnProgressChanged(float change, float newProgress)
	{
		SetProgressValue(newProgress);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visual data is changed
	protected void OnDisplayDataChanged()
	{
		CheckTaskVisibility();
		UpdateTask();
		SetIconFactionColor();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visual data is changed
	protected void OnTaskUIVisibilityChanged(SCR_Task task, SCR_ETaskUIVisibility visible)
	{
		if (m_Task != task)
			return;

		if (m_TaskManager)
			m_TaskManager.RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visibility is changed
	protected void OnTaskVisibilityChanged(SCR_Task task, SCR_ETaskVisibility visible)
	{
		if (m_Task != task)
			return;

		if (m_TaskManager)
			m_TaskManager.RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task ownership is changed
	protected void OnTaskOwnershipChanged(SCR_Task task, SCR_ETaskOwnership ownership)
	{
		if (m_Task != task)
			return;

		if (m_TaskManager)
			m_TaskManager.RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task faction is changed
	protected void OnTaskFactionChanged(SCR_Task task, FactionKey key)
	{
		if (m_Task != task)
			return;

		if (m_TaskManager)
			m_TaskManager.RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task group is changed
	protected void OnTaskGroupChanged(SCR_Task task, int id)
	{
		if (m_Task != task)
			return;

		if (m_TaskManager)
			m_TaskManager.RefreshTaskList();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void CheckTaskVisibility()
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return;

		SetVisibility(taskSystem.IsTaskVisibleFor(m_Task, player));
	}

	//------------------------------------------------------------------------------------------------
	//! Set task icon and title to the ones set in tasks' SCR_TaskUIInfo.
	void UpdateTask()
	{
		SCR_TaskUIInfo info = m_Task.GetTaskUIInfo();
		if (!info)
			return;

		info.SetNameTo(m_Widgets.m_wTaskTitle);
		info.SetIconTo(m_Widgets.m_wTaskIconSymbol);

		m_Widgets.m_wAssigneesAmountText.SetText(m_Task.GetTaskAssigneePlayerCount().ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Sets color of task title.
	//! \param[in] color new color of text.
	void SetTaskTextColor(notnull Color color)
	{
		m_Widgets.m_wTaskTitle.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets task icon colors
	//! \param[in] Color of background
	//! \param[in] Color of icon
	//! \param[in] Color of outline
	void UpdateTaskIconColors(Color backgroundColor = null, Color iconColor = null, Color outlineColor = null)
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
	//! Sets entry as selected.
	//! \param[in] selected New selection state.
	void SetEntrySelected(bool selected)
	{
		if (m_Widgets.m_ButtonComponent)
			m_Widgets.m_ButtonComponent.SetToggled(selected);
		
		if (selected)
		{
			UpdateTaskIconColors(UIColors.CONTRAST_COLOR);
			return;
		}
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));

		if (player && (m_Task.IsTaskAssignedTo(player) || (faction && faction.IsPlayerCommander(playerID) && m_Task.GetTaskAssigneeCount() > 0)))
		{
			UpdateTaskIconColors(m_FactionColor);
			return;
		}

		UpdateTaskIconColors();
	}

	//------------------------------------------------------------------------------------------------
	//! Sets icon to icon from provided imageset.
	//! \param[in] imageset
	//! \param[in] iconName Name of icon in imageset
	void SetIcon(ResourceName imageset, string iconName)
	{
		m_Widgets.m_wTaskIconSymbol.LoadImageFromSet(0, imageset, iconName);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets icon to icon from provided texture.
	//! \param[in] texture New icon texture
	void SetIcon(ResourceName texture)
	{
		m_Widgets.m_wTaskIconSymbol.LoadImageTexture(0, texture);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets visibility of task progression bar.
	//! \param[in] show Visibility of progress bar.
	void ShowProgressBar(bool show)
	{
		m_Widgets.m_wProgressBar.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets progression of task progression bar. If progress is -1, gets progress from task itself.
	//! \param[in] progress Value of progression to set on progress bar component.
	void SetProgressValue(float progress = -1)
	{
		m_Widgets.m_ProgressBarComponent.SetValue(progress);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets progression of task progression bar. If progress is -1, gets progress from task itself.
	//! \param[in] show Visibility of assignee count icon
	void ShowAssigneeCount(bool show)
	{
		m_Widgets.m_wAssigneesLayout.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets number of assignees assigned to the task
	//! \param[in] assigneCount Number of assignees
	void SetAssigneeCount(int assigneesCount)
	{
		m_Widgets.m_wAssigneesAmountText.SetText(assigneesCount.ToString());

		ShowAssigneeCount(assigneesCount > 0);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] count
	void SetChildCount(int count)
	{
		string displayCount;
		if (count > MAX_CHILD_COUNT)
			displayCount = UIConstants.FormatUnitShortPlus(MAX_CHILD_COUNT);
		else
			displayCount = count.ToString();

		m_Widgets.m_wChildTasksCountText.SetText(displayCount);
		m_Widgets.m_wChildTasksCount.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets visibility of entry
	//! \param[in] show New visibility
	void SetVisibility(bool show)
	{
		m_wRoot.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	//! \return Wrapper for child tasks
	Widget GetChildWrapper()
	{
		return m_Widgets.m_wChildTaskWrapper;
	}

	//------------------------------------------------------------------------------------------------
	//! \return entry main button
	Widget GetEntryButton()
	{
		return m_Widgets.m_wButton;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskVisualsChangedInvoker GetOnTaskVisualChanged()
	{
		if (!m_OnTaskVisualUpdated)
			m_OnTaskVisualUpdated = new SCR_TaskVisualsChangedInvoker();

		return m_OnTaskVisualUpdated;
	}
}
