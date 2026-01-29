[EntityEditorProps(category: "GameScripted/Tasks", description: "This is a task manager related component that allows you to display the available tasks.", color: "0 0 255 255")]
class SCR_TaskManagerUIComponentClass : SCR_BaseGameModeComponentClass
{
}

void TaskAssigned(SCR_Task task);
typedef func TaskAssigned;
typedef ScriptInvokerBase<TaskAssigned> SCR_TaskAssignedInvoker;

void TaskSelected(SCR_Task task);
typedef func TaskSelected;
typedef ScriptInvokerBase<TaskSelected> SCR_TaskSelectedInvoker;

void TaskHovered(SCR_Task task);
typedef func TaskHovered;
typedef ScriptInvokerBase<TaskHovered> SCR_TaskHoveredInvoker;

void TaskColorsUpdated(Color backgroundColor, Color iconColor, Color outlineColor);
typedef func TaskColorsUpdated;
typedef ScriptInvokerBase<TaskColorsUpdated> SCR_TaskColorsUpdatedInvoker;

void TaskBackgroundUpdated(ResourceName imageset, string name);
typedef func TaskBackgroundUpdated;
typedef ScriptInvokerBase<TaskBackgroundUpdated> SCR_TaskBackgroundUpdatedInvoker;

class SCR_TaskManagerUIComponent : SCR_BaseGameModeComponent
{
	[Attribute("{EE9497AB43F556F9}UI/layouts/Tasks/CurrentTask.layout")]
	protected ResourceName m_sTaskHintLayout;

	[Attribute("5", UIWidgets.EditBox, "Defines duration of current task display (in seconds)")]
	protected float m_fCurrentTaskShowTime;
	
	[Attribute("{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset")]
	protected ResourceName m_sDefaultImageset;
	
	[Attribute("Icon_M_Task_Seize")]
	protected string m_sDefaultIcon;
	
	[Attribute("1", desc: "Defines whether number of people assigned to task should be displayed on map task marker")]
	protected bool m_bDisplayOfAssigneesOnMap;
	
	[Attribute("0")]
	protected bool m_bShowCurrentTaskOnRespawn;

	[Attribute("{C9D32771B2166F77}Configs/Task/TasksTabConfig_Base.conf", UIWidgets.ResourceNamePicker, params: "conf class=SCR_TasksTabConfig")]
	protected string m_sTaskTabConfig;
	
	static SCR_TaskManagerUIComponent s_Instance;

	protected ref SCR_TaskSelectedInvoker m_OnTaskSelected;
	protected ref SCR_TaskAssignedInvoker m_OnTaskAssigned;
	protected ref SCR_TaskHoveredInvoker m_OnTaskHovered;
	protected ref SCR_TaskColorsUpdatedInvoker m_OnTaskColorsUpdate;
	protected ref SCR_TaskBackgroundUpdatedInvoker m_OnTaskBackgroundUpdate;
	protected ref ScriptInvokerBool m_OnTaskHUDVisible;
	protected ref SCR_TasksTabConfig m_TaskTabConfig;
	protected bool m_bIsUnconscious;	//Character is unconscious --> Task menu control is disabled
	protected bool m_bCurrentTaskVisible;

	protected Widget m_wTaskHint;
	protected SCR_Task m_SelectedTask;
	protected SCR_Task m_AssignedTask;
	protected SCR_Task m_HoveredTask;

	protected SCR_MapEntity m_MapEntity;
	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_TaskListUIComponent m_TaskListComponent;
	protected SCR_MapJournalUI m_JournalComponent;
	
	protected const string CONTENT_TASK_LIST_MAP = "TaskListMapContext";
	protected const string ACTION_CLOSE_TASK_LIST = "TasksClose";
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_TaskManagerUIComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		SetEventMask(GetOwner(), EntityEvent.INIT | EntityEvent.FRAME);

		if (!s_Instance)
			s_Instance = this;
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_TaskManagerUIComponent()
	{
		s_Instance = null;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_MapEntity = SCR_MapEntity.GetMapInstance();

		m_TaskSystem = SCR_TaskSystem.GetInstance();

		GetGame().GetInputManager().AddActionListener("TasksShowHint", EActionTrigger.DOWN, ToggleCurrentTask);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		gameMode.GetOnPlayerConnected().Insert(OnPlayerConnected);

		if (System.IsConsoleApp())
			return;

		gameMode.GetOnPlayerRegistered().Insert(OnClientPlayerRegistered);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{	
		if (m_MapEntity && m_MapEntity.IsOpen() && IsTaskListOpen())
			GetGame().GetInputManager().ActivateContext(CONTENT_TASK_LIST_MAP);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Displays a hint with currently selected task
	void ToggleCurrentTask()
	{	
		if (m_bIsUnconscious)
			return;
		
		GetGame().GetCallqueue().Remove(ToggleCurrentTask);
		
		int playerID = SCR_PlayerController.GetLocalPlayerId();
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerID));
		if (playerID <= 0 || !faction)
			return;

		m_bCurrentTaskVisible = !m_bCurrentTaskVisible;
		
		if (!m_wTaskHint)
			m_wTaskHint = GetGame().GetWorkspace().CreateWidgets(m_sTaskHintLayout);

		if (!m_wTaskHint)
			return;
		
		if (m_bCurrentTaskVisible)
		{
			SCR_CurrentTaskUIComponent currentTaskComp = SCR_CurrentTaskUIComponent.Cast(m_wTaskHint.FindHandler(SCR_CurrentTaskUIComponent));
			if (!currentTaskComp)
				return;
			
			SCR_TaskExecutor player = SCR_TaskExecutor.FromPlayerID(playerID);
			if (!player)
				return;
			
			m_AssignedTask = m_TaskSystem.GetTaskAssignedTo(player);
			currentTaskComp.UpdateTask(m_AssignedTask);

			AnimateWidget.Opacity(m_wTaskHint, 1, 1);
			GetGame().GetCallqueue().CallLater(ToggleCurrentTask, m_fCurrentTaskShowTime * 1000);
		}
		else
		{
			AnimateWidget.Opacity(m_wTaskHint, 0, 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Requests panning map to task. If map is not opened - opens it.
	void ShowTaskOnMap()
	{
		if (!m_MapEntity)
			return;

		if (!m_MapEntity.IsOpen())
		{
			IEntity player = SCR_PlayerController.GetLocalControlledEntity();
			if (!player)
				return;

			SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(player);
			if (!gadgetManager)
				return;

			IEntity mapGadget = gadgetManager.GetGadgetByType(EGadgetType.MAP);
			if (!mapGadget)
				return;

			gadgetManager.SetGadgetMode(mapGadget, EGadgetMode.IN_HAND);
			
			GetGame().GetCallqueue().CallLater(PanMapToTask, 1000, false, m_SelectedTask.GetTaskID(), m_SelectedTask);
		}
		else
		{
			PanMapToTask(task: m_SelectedTask);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Moves camera to task position in map view
	//! \param[in] taskID
	//! \param[in] task
	protected void PanMapToTask(string taskID = string.Empty, SCR_Task task = null)
	{
		if (!m_MapEntity || !m_MapEntity.IsOpen())
			return;

		if (!task)
		{
			task = m_TaskSystem.GetTaskFromTaskID(taskID);
			if (!task)
				return;
		}

		vector taskPos = task.GetOrigin();
		float screenX, screenY;
		m_MapEntity.WorldToScreen(taskPos[0], taskPos[2], screenX, screenY);
		m_MapEntity.PanSmooth(screenX, screenY);
	}

	//------------------------------------------------------------------------------------------------
	//! Assigns provided task to the player.
	//! \param[in] SCR_Task
	void AssignTask(SCR_Task task = null)
	{	
		PlayerController localPlayerController = GetGame().GetPlayerController();
		if (!localPlayerController)
			return;
		
		SCR_TaskSystemNetworkComponent networkComponent = SCR_TaskSystemNetworkComponent.Cast(localPlayerController.FindComponent(SCR_TaskSystemNetworkComponent));
		if (!networkComponent)
			return;
		
		int playerID = localPlayerController.GetPlayerId();
		SCR_TaskExecutor executor = SCR_TaskExecutor.FromPlayerID(playerID);
		if (!executor)
			return;

		if (!task)
		{
			if (!m_SelectedTask)
				return;

			task = m_SelectedTask;
		}

		if (task == m_AssignedTask)
		{
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_CANCELED);
			networkComponent.UnassignTask(m_AssignedTask, executor, playerID);

			m_AssignedTask = null;
		}
		else
		{	
			if (m_AssignedTask)
				networkComponent.UnassignTask(m_AssignedTask, executor);

			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.TASK_ACCEPT);
			networkComponent.AssignTask(task, executor, true, playerID);

			m_AssignedTask = task;
		}

		if (m_OnTaskAssigned)
			m_OnTaskAssigned.Invoke(task);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Closes task list
	protected void CloseTaskList()
	{
		SetTaskListVisibility(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Invokes whenever task list on HUD is shown.
	void TaskHUDVisibilityChanged(bool newVisibility)
	{	
		if (m_OnTaskHUDVisible)
			m_OnTaskHUDVisible.Invoke(newVisibility);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets provided task as selected.
	//! \param[in] task
	void SetSelectedTask(SCR_Task task)
	{
		m_SelectedTask = task;
		if (m_OnTaskSelected)
			m_OnTaskSelected.Invoke(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets provided task as hovered.
	//! \param[in] task
	void SetHoveredTask(SCR_Task task)
	{
		m_HoveredTask = task;
		if (m_OnTaskHovered)
			m_OnTaskHovered.Invoke(task);
	}

	//------------------------------------------------------------------------------------------------
	//! SCR_PlayerController Event
	//! Used to reinit Task manager Component when new entity is controlled
	protected void OnControlledEntityChanged(IEntity from, IEntity to)
	{
		if (from)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(from);
			if (!character)
				return;

			SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
			if (controller)
				controller.m_OnLifeStateChanged.Remove(LifeStateChanged);
		}

		ChimeraCharacter character = ChimeraCharacter.Cast(to);
		if (!character)
			return;

		SCR_CharacterControllerComponent characterController = SCR_CharacterControllerComponent.Cast(character.GetCharacterController());
		if (!characterController)
			return;

		m_bIsUnconscious = characterController.IsUnconscious();
		characterController.m_OnLifeStateChanged.Insert(LifeStateChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void LifeStateChanged(ECharacterLifeState previousLifeState, ECharacterLifeState newLifeState)
	{
		m_bIsUnconscious = newLifeState == ECharacterLifeState.INCAPACITATED;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerConnected(int playerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		int localPlayerID = playerController.GetLocalPlayerId();
		if (playerId != localPlayerID)
			return;
		
		playerController.m_OnControlledEntityChanged.Insert(OnControlledEntityChanged);
		
		SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.Cast(playerController.FindComponent(SCR_PlayerControllerGroupComponent));
		if (groupComponent)
			groupComponent.GetOnGroupChanged().Insert(OnGroupChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnClientPlayerRegistered(int playerId)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		if (playerId != playerController.GetLocalPlayerId())
			return;

		playerController.m_OnControlledEntityChanged.Insert(OnClientControlledEntityChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClientControlledEntityChanged(IEntity from, IEntity to)
	{
		// show current task on respawn
		if (m_bShowCurrentTaskOnRespawn && !m_bCurrentTaskVisible)
			ToggleCurrentTask();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupChanged(int groupID)
	{
		m_SelectedTask = null;
		m_AssignedTask = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets new colors on task icons. If provided color is null, its being skipped.
	//! \param[in] backgroundColor - color of task shield background.
	//! \param[in] iconColor - color of task symbol.
	//! \param[in] outlineColor - color of task shield outline.
	void UpdateTaskColors(Color backgroundColor, Color iconColor, Color outlineColor)
	{
		m_OnTaskColorsUpdate.Invoke(backgroundColor, iconColor, outlineColor);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets new background of task.
	//! \param[in] imageset with the background.
	//! \param[in] name in imageset.
	void UpdateTaskBackground(ResourceName imageset, string name)
	{
		m_OnTaskBackgroundUpdate.Invoke(imageset, name);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Instance of task manager.
	static SCR_TaskManagerUIComponent GetInstance()
	{
		return s_Instance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Currently selected task.
	SCR_Task GetSelectedTask()
	{
		return m_SelectedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Currently assigned task.
	SCR_Task GetAssigedTask()
	{
		return m_AssignedTask;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Whether task list is opened in map view.
	bool IsTaskListOpen()
	{	
		if (!m_TaskListComponent)
			return false;
		
		return m_TaskListComponent.GetTaskListVisibility();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register task list component.
	//! \param[in] component to be registered
	void RegisterTaskList(SCR_TaskListUIComponent component)
	{
		m_TaskListComponent = component;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Register task list component.
	//! \return whether task list was refreshed or not
	bool RefreshTaskList()
	{
		if (!m_TaskListComponent)
			return false;
		
		m_TaskListComponent.RefreshTaskList();
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] visibility of task description.
	bool SetTaskDescriptionVisiblity(bool newVisibility)
	{
		if (!m_TaskListComponent)
			return false;
		
		m_TaskListComponent.SetTaskDescriptionVisiblity(newVisibility);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets new visiblity of journal and hides task list.
	//! \param[in] visibility of journal to be set.
	void SetJournalVisibility(bool newVisibility)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		SCR_MapJournalUI journalUI = SCR_MapJournalUI.Cast(mapEntity.GetMapUIComponent(SCR_MapJournalUI));
		if (!journalUI)
			return;
		
		journalUI.SetJournalVisibility(newVisibility);
		
		SCR_MapTaskListUI taskUI = SCR_MapTaskListUI.Cast(mapEntity.GetMapUIComponent(SCR_MapTaskListUI));
		if (!taskUI)
			return;
		
		taskUI.ToggleTaskList(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Sets new visiblity of task list and hides journal.
	//! \param[in] visibility of task list to be set.
	void SetTaskListVisibility(bool newVisibility)
	{
		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();
		if (!mapEntity)
			return;
		
		SCR_MapTaskListUI taskUI = SCR_MapTaskListUI.Cast(mapEntity.GetMapUIComponent(SCR_MapTaskListUI));
		if (!taskUI)
			return;
		
		taskUI.ToggleTaskList(newVisibility);
		
		if (m_TaskListComponent && m_SelectedTask)
			m_TaskListComponent.FocusOnEntry(m_SelectedTask);
		
		SCR_MapJournalUI journalUI = SCR_MapJournalUI.Cast(mapEntity.GetMapUIComponent(SCR_MapJournalUI));
		if (!journalUI)
			return;
		
		journalUI.SetJournalVisibility(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Currently assigned task.
	SCR_TaskListUIComponent GetTaskListComponent()
	{	
		if (!m_TaskListComponent)
			return null;
		
		return m_TaskListComponent;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return task description ui component
	SCR_TaskListEntryDescriptionUIComponent GetTaskDescriptionComponent()
	{	
		if (!m_TaskListComponent)
			return null;
		
		SCR_TaskListEntryDescriptionUIComponent desc = m_TaskListComponent.GetTaskDescription();
		if (desc)
			return desc;
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return default imageset.
	ResourceName GetDefaultImageset()
	{	
		return m_sDefaultImageset;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return default icon name.
	string GetDefaultIcon()
	{	
		return m_sDefaultIcon;
	}	
	
	//------------------------------------------------------------------------------------------------
	//! \return default icon name.
	bool GetDisplayAssigneesOnMap()
	{	
		return m_bDisplayOfAssigneesOnMap;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return task tabs to be displayed
	array<SCR_ETaskTabType> GetTaskTabs()
	{
		if (!m_TaskTabConfig)
			m_TaskTabConfig = SCR_TasksTabConfig.Cast(SCR_BaseContainerTools.CreateInstanceFromPrefab(m_sTaskTabConfig));
		
		return m_TaskTabConfig.GetTaskTabs();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnTaskHUDVisible()
	{
		if (!m_OnTaskHUDVisible)
			m_OnTaskHUDVisible = new ScriptInvokerBool();

		return m_OnTaskHUDVisible;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TaskColorsUpdatedInvoker GetOnTaskColorsUpdated()
	{
		if (!m_OnTaskColorsUpdate)
			m_OnTaskColorsUpdate = new SCR_TaskColorsUpdatedInvoker();

		return m_OnTaskColorsUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TaskBackgroundUpdatedInvoker GetOnTaskBackgroundUpdated()
	{
		if (!m_OnTaskBackgroundUpdate)
			m_OnTaskBackgroundUpdate = new SCR_TaskBackgroundUpdatedInvoker();

		return m_OnTaskBackgroundUpdate;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_TaskSelectedInvoker GetOnTaskSelected()
	{
		if (!m_OnTaskSelected)
			m_OnTaskSelected = new SCR_TaskSelectedInvoker();

		return m_OnTaskSelected;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskAssignedInvoker GetOnTaskAssigned()
	{
		if (!m_OnTaskAssigned)
			m_OnTaskAssigned = new SCR_TaskAssignedInvoker();

		return m_OnTaskAssigned;
	}

	//------------------------------------------------------------------------------------------------
	SCR_TaskHoveredInvoker GetOnTaskHovered()
	{
		if (!m_OnTaskHovered)
			m_OnTaskHovered = new SCR_TaskHoveredInvoker();

		return m_OnTaskHovered;
	}
}
