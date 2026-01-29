class SCR_TaskMapUIComponent : SCR_MapUIElement
{
	protected ref Color m_IconColor;
	protected ref Color m_OutlineColor;
	protected ref Color m_BackgroundColor;
	protected ref Color m_FactionColor;

	protected SCR_Task m_Task;
	protected SCR_TaskSystem m_TaskSystem;

	protected bool m_bEnableContext;
	protected bool m_bSelected;
	protected bool m_bDisplayAssigned = 1;

	protected SCR_TaskManagerUIComponent m_TaskManager;
	protected ref ScriptInvokerVoid m_OnTaskIconClick;
	protected ref SCR_TaskMapWidgets m_Widgets = new SCR_TaskMapWidgets();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);

		if (m_Widgets.m_TaskIconButtonComponent)
		{
			m_Widgets.m_TaskIconButtonComponent.m_OnMouseEnter.Insert(OnHoveredIconStart);
			m_Widgets.m_TaskIconButtonComponent.m_OnMouseLeave.Insert(OnHoveredEnd);
			m_Widgets.m_TaskIconButtonComponent.m_OnClicked.Insert(OnTaskIconClicked);
		}

		if (m_Widgets.m_TaskTitleButtonComponent)
		{
			m_Widgets.m_TaskTitleButtonComponent.m_OnClicked.Insert(OnTaskTitleClicked);
			m_Widgets.m_TaskTitleButtonComponent.m_OnMouseLeave.Insert(OnHoveredTitleEnd);
			m_Widgets.m_TaskTitleButtonComponent.m_OnMouseEnter.Insert(OnHoveredTitleStart);
		}

		m_eIconType = SCR_EIconType.TASK;

		if (m_Widgets.m_wTaskTitleButton)
			m_Widgets.m_wTaskTitleButton.SetVisible(false);

		m_TaskManager = SCR_TaskManagerUIComponent.GetInstance();
		if (m_TaskManager)
		{
			m_TaskManager.GetOnTaskColorsUpdated().Insert(SetTaskIconColors);
			m_TaskManager.GetOnTaskBackgroundUpdated().Insert(SetTaskBackground);
			m_bDisplayAssigned = m_TaskManager.GetDisplayAssigneesOnMap();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		if (m_Widgets.m_TaskIconButtonComponent)
		{
			m_Widgets.m_TaskIconButtonComponent.m_OnMouseEnter.Remove(OnHoveredIconStart);
			m_Widgets.m_TaskIconButtonComponent.m_OnMouseLeave.Remove(OnHoveredEnd);
			m_Widgets.m_TaskIconButtonComponent.m_OnClicked.Remove(OnTaskIconClicked);
		}

		if (m_Widgets.m_TaskTitleButtonComponent)
		{
			m_Widgets.m_TaskTitleButtonComponent.m_OnClicked.Remove(OnTaskTitleClicked);
			m_Widgets.m_TaskTitleButtonComponent.m_OnMouseLeave.Remove(OnHoveredTitleEnd);
			m_Widgets.m_TaskTitleButtonComponent.m_OnMouseEnter.Remove(OnHoveredTitleStart);
		}

		if (!m_Task)
			return;

		m_Task.GetOnTaskStateChanged().Remove(UpdateTaskState);
		m_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeAdded);
		m_Task.GetOnTaskAssigneeRemoved().Remove(OnTaskAssigneeAdded);
		m_Task.GetOnDisplayDataChanged().Remove(OnDisplayDataChanged);
		m_Task.GetOnTaskUIVisibilityChanged().Remove(OnTaskUIVisibilityChanged);
		m_Task.GetOnTaskVisibilityChanged().Remove(OnTaskVisibilityChanged);
		m_Task.GetOnTaskOwnershipChanged().Remove(OnTaskOwnershipChanged);
		m_Task.GetOnOwnerFactionAdded().Remove(OnTaskFactionChanged);
		m_Task.GetOnOwnerFactionRemoved().Remove(OnTaskFactionChanged);
		m_Task.GetOnOwnerGroupAdded().Remove(OnTaskGroupChanged);
		m_Task.GetOnOwnerGroupRemoved().Remove(OnTaskGroupChanged);

		if (m_TaskManager)
		{
			m_TaskManager.GetOnTaskColorsUpdated().Remove(SetTaskIconColors);
			m_TaskManager.GetOnTaskBackgroundUpdated().Remove(SetTaskBackground);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns origin of task entity
	//! \return vector
	override vector GetPos()
	{
		if (!m_TaskSystem || !m_Task)
			return vector.Zero;

		return m_TaskSystem.GetTaskLocation(m_Task);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		super.OnMouseEnter(w, x, y);
		
		if (w != m_Widgets.m_wTaskIconButton)
			return false;

		m_bEnableContext = true;
		m_Widgets.m_wTaskTitleButton.SetVisible(true);
		m_Widgets.m_wAssignedPlayers.SetVisible(m_bDisplayAssigned);

		m_wRoot.Update();
		m_TaskManager.SetHoveredTask(m_Task);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		super.OnMouseLeave(w, enterW, x, y);

		m_Widgets.m_wTaskTitleButton.SetVisible(false);

		m_bEnableContext = false;
		m_TaskManager.SetHoveredTask(null);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoveredIconStart(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		if (CheckTitleOverflow())
			m_Widgets.m_TitleFrameComponent.AnimationStart();
		
		GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_wTaskIconButton);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoveredTitleStart(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		if (CheckTitleOverflow())
			m_Widgets.m_TitleFrameComponent.AnimationStart();
		
		GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_wTaskTitleButton);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoveredTitleEnd(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		m_Widgets.m_TitleFrameComponent.AnimationStop();
		m_Widgets.m_TitleFrameComponent.ResetPosition();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnHoveredEnd(SCR_ModularButtonComponent comp, bool mouseInput)
	{
		m_Widgets.m_TitleFrameComponent.AnimationStop();
		m_Widgets.m_TitleFrameComponent.ResetPosition();
		
		GetGame().GetWorkspace().SetFocusedWidget(null);
	}

	//------------------------------------------------------------------------------------------------
	//! \return True when content doesn't fit parent widget.
	protected bool CheckTitleOverflow()
	{
		float frameX, frameY;
		m_Widgets.m_wTitleFrame.GetScreenSize(frameX, frameY);

		float titleX, titleY;
		m_Widgets.m_wTaskTitle.GetScreenSize(titleX, titleY);
		
		bool isOverflow = frameX < titleX;
		
		float witdh;
		if (isOverflow)
			witdh = m_Widgets.m_wTitleSizeLayout.GetMaxDesiredWidth();
		else
			witdh = 0;
		
		m_Widgets.m_wTitleSizeLayout.SetWidthOverride(witdh);
		m_Widgets.m_wTitleSizeLayout.EnableWidthOverride(isOverflow);

		return isOverflow;
	}
	
	//------------------------------------------------------------------------------------------------
	// Sets task as selected on task icon click
	protected void OnTaskIconClicked()
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (mapEntity)
		{
			SCR_MapCursorModule mapCursorModule = SCR_MapCursorModule.Cast(mapEntity.GetMapModule(SCR_MapCursorModule));
			if (mapCursorModule && mapCursorModule.GetCursorState() & SCR_MapCursorModule.STATE_POPUP_RESTRICTED)
				return;
		}

		if (!m_Task)
			return;

		if (m_OnTaskIconClick)
			m_OnTaskIconClick.Invoke();

		if (m_bSelected)
		{
			m_bSelected = false;
			UpdateTaskIconColors();
		}
		else if (m_TaskManager)
		{
			m_TaskManager.SetSelectedTask(m_Task);
			m_TaskManager.SetTaskListVisibility(true);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Assigns task on task title click
	protected void OnTaskTitleClicked()
	{
		if (m_Task && m_TaskManager)
			m_TaskManager.AssignTask(m_Task);
	}

	//------------------------------------------------------------------------------------------------
	// Sets task visuals to selected state
	//! \param[in] task Selected task
	protected void OnTaskSelected(SCR_Task task)
	{
		if (!m_TaskManager || !m_Task)
			return;

		if (task == m_Task)
		{
			UpdateTaskIconColors(UIColors.CONTRAST_COLOR);
			return;
		}

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (m_Task.IsTaskAssignedTo(player) || (faction && faction.IsPlayerCommander(playerID) && m_Task.GetTaskAssigneeCount() > 0))
		{
			UpdateTaskIconColors(m_FactionColor);
			return;
		}

		UpdateTaskIconColors();
	}

	//------------------------------------------------------------------------------------------------
	//! Handles update of assigned entities. If player is assigned, changes color of map marker.
	//! \param[in] task
	//! \param[in] assignee Assigned SCR_TaskExecutor
	//! \param[in] requesterID
	protected void OnTaskAssigneeAdded(SCR_Task task, SCR_TaskExecutor assignee, int requesterID)
	{
		if (!m_Task || !m_TaskSystem)
			return;

		SetMapMarkerAssigneeCount();

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!m_Widgets.m_wTaskIconBackground || !player)
			return;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (faction && (m_Task.IsTaskAssignedTo(player) || (faction.IsPlayerCommander(playerID) && m_Task.GetTaskAssigneeCount() > 0)))
		{
			Color color = faction.GetFactionColor();
			if (!color)
				return;

			m_Widgets.m_wTaskIconBackground.SetColor(color);
		}
		else
		{
			if (m_Task == m_TaskManager.GetSelectedTask())
			{
				UpdateTaskIconColors(UIColors.CONTRAST_COLOR);
				return;
			}

			m_Widgets.m_wTaskIconBackground.SetColor(m_BackgroundColor);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handles update of task visuals
	protected void OnDisplayDataChanged()
	{
		CheckTaskVisibility();
		UpdateTaskInformation();
		SetIconFactionColor();
		SetMapMarkerAssigneeCount();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initializes task, sets its visuals
	//! \param[in] task
	void InitTask(notnull SCR_Task task)
	{	
		m_Task = task;
		
		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (!m_TaskSystem && !m_TaskSystem.IsTaskVisibleOnMap(task))
			return;

		GetGame().GetCallqueue().Call(CheckTaskVisibility);

		int playerID = SCR_PlayerController.GetLocalPlayerId();
		
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (!faction)
			return;
		
		array<string> taskFactions = m_TaskSystem.GetTaskFactions(task);
		if (taskFactions && !taskFactions.IsEmpty() && !taskFactions.Contains(faction.GetFactionKey()))
			return;
		
		SetIconFactionColor();
		SetTask(task);
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
		
		m_BackgroundColor = UIColors.NEUTRAL_INFORMATION;
		
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return;

		if (m_FactionColor && (m_Task.IsTaskAssignedTo(player) || (faction.IsPlayerCommander(playerID) && m_Task.GetTaskAssigneeCount() > 0)))
			UpdateTaskIconColors(m_FactionColor);
		else
			UpdateTaskIconColors();
	}

	//------------------------------------------------------------------------------------------------
	void SetTask(notnull SCR_Task task)
	{
		m_Task = task;
		
		UpdateTaskInformation();

		SetMapMarkerAssigneeCount();
		
		m_Task.GetOnTaskStateChanged().Insert(UpdateTaskState);
		m_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeAdded);
		m_Task.GetOnTaskAssigneeRemoved().Insert(OnTaskAssigneeAdded);
		m_Task.GetOnDisplayDataChanged().Insert(OnDisplayDataChanged);
		m_Task.GetOnTaskUIVisibilityChanged().Insert(OnTaskUIVisibilityChanged);
		m_Task.GetOnTaskVisibilityChanged().Insert(OnTaskVisibilityChanged);
		m_Task.GetOnTaskOwnershipChanged().Insert(OnTaskOwnershipChanged);
		m_Task.GetOnOwnerFactionAdded().Insert(OnTaskFactionChanged);
		m_Task.GetOnOwnerFactionRemoved().Insert(OnTaskFactionChanged);
		m_Task.GetOnOwnerGroupAdded().Insert(OnTaskGroupChanged);
		m_Task.GetOnOwnerGroupRemoved().Insert(OnTaskGroupChanged);
		
		if (m_TaskManager)
			m_TaskManager.GetOnTaskSelected().Insert(OnTaskSelected);
		
		UpdateTaskState(m_Task, m_Task.GetTaskState());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates task title and icon to the one provided in task SCR_TaskUIInfo
	void UpdateTaskInformation()
	{
		SCR_TaskUIInfo info = m_Task.GetTaskUIInfo();
		if (!info)
			return;
		
		info.SetNameTo(m_Widgets.m_wTaskTitle);
		info.SetIconTo(m_Widgets.m_wTaskIconSymbol);
		
		if (m_Task.GetAuthorID() > 0)
		{
			m_Widgets.m_wAuthorLayout.SetVisible(true);
			
			string playerName = GetGame().GetPlayerManager().GetPlayerName(m_Task.GetAuthorID());
			m_Widgets.m_wMarkerAuthor.SetText(playerName);
			
			m_Widgets.m_wPlatformIcon.SetVisible(false);
			
			PlatformKind platformKind = GetGame().GetPlayerManager().GetPlatformKind(m_Task.GetAuthorID());
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageToKind(platformKind, m_Widgets.m_wPlatformIcon, showOnPC: true, showOnXbox: true);
		}
		else
		{
			m_Widgets.m_wAuthorLayout.SetVisible(false);
		}
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
	//! Handles task visuals based on new task state.
	//! \param[in] state New state of task
	protected void UpdateTaskState(SCR_Task task, SCR_ETaskState state)
	{
		if (m_Task != task)
			return;

		m_Widgets.m_wOverlayTaskStates.SetVisible(true);
		switch (state)
		{
			case SCR_ETaskState.CREATED:
			{
				m_Widgets.m_wIconOverlay.SetOpacity(1);
				m_Widgets.m_wOverlayTaskStates.SetVisible(false);
				break;
			}

			case SCR_ETaskState.COMPLETED:
			{
				m_Widgets.m_wIconOverlay.SetOpacity(0.5);
				m_Widgets.m_wIconOverlayFinished.SetVisible(true);

				if (m_Widgets.m_TaskIconButtonComponent)
					m_Widgets.m_TaskIconButtonComponent.m_OnClicked.Remove(OnTaskIconClicked);

				if (m_Widgets.m_TaskTitleButtonComponent)
					m_Widgets.m_TaskTitleButtonComponent.m_OnClicked.Remove(OnTaskTitleClicked);

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
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetMapMarkerAssigneeCount()
	{
		m_Widgets.m_wAssigneesCount.SetText(m_Task.GetTaskAssigneePlayerCount().ToString());
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visual data is changed
	protected void OnTaskUIVisibilityChanged(SCR_Task task, SCR_ETaskUIVisibility visible)
	{
		if (m_Task != task)
			return;

		if (visible == SCR_ETaskUIVisibility.ALL || visible == SCR_ETaskUIVisibility.MAP_ONLY)
			SetVisibility(true);
		else
			SetVisibility(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visibility is changed
	protected void OnTaskVisibilityChanged(SCR_Task task, SCR_ETaskVisibility visible)
	{
		HandleTaskVisibilityChange(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task ownership is changed
	protected void OnTaskOwnershipChanged(SCR_Task task, SCR_ETaskOwnership ownership)
	{
		HandleTaskVisibilityChange(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task faction is changed
	protected void OnTaskFactionChanged(SCR_Task task, FactionKey key)
	{
		HandleTaskVisibilityChange(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task group is changed
	protected void OnTaskGroupChanged(SCR_Task task, int id)
	{	
		HandleTaskVisibilityChange(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Invoked when task visibility is changed
	protected void HandleTaskVisibilityChange(SCR_Task task)
	{
		if (m_Task != task)
			return;

		CheckTaskVisibility();
	}

	//------------------------------------------------------------------------------------------------
	//!
	protected void CheckTaskVisibility()
	{
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!player)
			return;
		
		if (!m_TaskSystem.IsTaskVisibleOnMap(m_Task))
		{
			SetVisibility(false);
			return;
		}
		
		SetVisibility(m_TaskSystem.IsTaskVisibleFor(m_Task, player));
	}

	//------------------------------------------------------------------------------------------------
	//! Sets visibility of entry
	//! \param[in] show New visibility
	protected void SetVisibility(bool show)
	{
		m_wRoot.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	SCR_Task GetTask()
	{
		return m_Task;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnTaskIconClick()
	{
		if (!m_OnTaskIconClick)
			m_OnTaskIconClick = new ScriptInvokerVoid();

		return m_OnTaskIconClick;
	}
}
