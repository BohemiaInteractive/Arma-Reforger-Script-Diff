//------------------------------------------------------------------------------------------------
//class SCR_BaseTaskClass: GenericEntityClass
//{
//};
//
//------------------------------------------------------------------------------------------------
//! A base class for tasks.
//class SCR_BaseTask : GenericEntity
//{
//
//	//****************//
//	//STATIC VARIABLES//
//	//****************//
//	
//	static const string TASK_AMOUNT_COMPLETED_TEXT = "#AR-Tasks_AmountCompleted";
//	static const string TASK_PROGRESS_TEXT = "#AR-Tasks_StatusProgress-UC";
//	static const string TASK_FINISHED_TEXT = "#AR-Tasks_StatusFinished-UC";
//	static const string TASK_AVAILABLE_TEXT = "#AR-Tasks_StatusNew-UC";
//	static const string TASK_HINT_TEXT = "#AR-Tasks_Hint";
//	static const string TASK_CANCELLED_TEXT = "#AR-Tasks_StatusCancelled-UC";
//	static const string TASK_COMPLETED_TEXT = "#AR-Tasks_StatusCompleted-UC";
//	static const string TASK_FAILED_TEXT = "#AR-Tasks_StatusFailed-UC";
//	static const float DEFAULT_ASSIGNEE_TIME_LIMIT = 1800;
//	static const float DEFAULT_ASSIGNEE_TIMEOUT_TIME = 60;
//	static const int INVALID_TIMESTAMP = -1;
//	static const int INVALID_TASK_ID = -1;
//	protected static const float PLATFORM_ICON_SIZE = 2;
//	
//	static int s_iCurrentTaskID = 0;
//	
//	//*****************//
//	//MEMBER ATTRIBUTES//
//	//*****************//
//	
//	[Attribute(defvalue: "Task name.", desc: "The task name visible to the player.")]
//	protected string m_sName;
//	
//	[Attribute(defvalue: "Task description.", desc: "The task description visible to the player.")]
//	protected string m_sDescription;
//	
//	[Attribute("1", desc: "Can this task be assigned?")]
//	protected bool m_bAssignable;
//	
//	[Attribute(desc: "Is this task supposed to be completed just by an individual?")]
//	protected bool m_bIndividualTask;
//	
//	[Attribute("Task_Seize")]
//	protected string m_sMapIconName;
//	
//	[Attribute("Icon_")]
//	protected string m_sTaskListIconNamePrefix;
//	
//	[Attribute("Seize")]
//	protected string m_sTaskListIconName;
//	
//	[Attribute("M_")]
//	protected string m_sMajorModifier;
//
//	[Attribute("{EAB5D9841F081D07}UI/layouts/Campaign/TaskElementNew.layout")]
//	protected ResourceName m_sMapUIElementResourceName;
//	
//	[Attribute("{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset", category: "Task icon")]
//	protected ResourceName m_sIconImageset;
//	
//	[Attribute(SCR_ERadioMsg.CONFIRM.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ERadioMsg))]
//	protected SCR_ERadioMsg m_eAssignMessage;
//	
//	[Attribute("0")]
//	protected bool m_bNotifyAssignment;
//	
//	[Attribute(SCR_ERadioMsg.TASK_UNASSIGN_EVAC.ToString(), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ERadioMsg))]
//	protected SCR_ERadioMsg m_eUnassignMessage;
//	
//	[Attribute("0")]
//	protected bool m_bNotifyUnassign;
//	
//	[Attribute("{6FE28246710CA6B1}UI/layouts/Tasks/TaskHUDIcon.layout")]
//	protected ResourceName m_sHUDIcon;
//	
//	//**************************//
//	//PROTECTED MEMBER VARIABLES//
//	//**************************//
//	
//	protected Faction m_TargetFaction = null;
//	protected int m_iTaskID = INVALID_TASK_ID;
//	protected float m_fAssigneeTimeLimit = DEFAULT_ASSIGNEE_TIME_LIMIT;
//	protected float m_fLastAssigneeAddedTimestamp = INVALID_TIMESTAMP;
//	protected ref array<SCR_BaseTaskExecutor> m_aAssignees = new array<SCR_BaseTaskExecutor>();
//	protected SCR_TaskState m_eState;
//	protected SCR_BaseTaskExecutor m_TimedOutAssignee = null;
//	protected float m_fAssigneeTimeoutTimestamp = INVALID_TIMESTAMP;
//	protected SCR_MapDescriptorComponent m_MapDescriptor;
//	protected ImageWidget m_wHUDIcon;
//	protected Widget m_wMapTaskIcon;
//	protected Widget m_wTaskListDescription;
//	protected bool m_bIsPriority;
//	protected SCR_PlayerFactionAffiliationComponent m_sPlyFactionAffilComp;
//	protected float m_fMajorSize = 80;
//	
//	const string TASK_BG_M = "Icon_M_Task_BG";
//	const string TASK_O_M = "Icon_M_Task_Outline";
//	const string TASK_BG = "Icon_Task_BG";
//	const string TASK_O = "Icon_Task_Outline";
//	const string TASK_H = "Icon_M_Task_Hover";
//	const string Task_S = "TaskIconSymbol";
//	
//	//***************************//
//	//PUBLIC MEMBER EVENT METHODS//
//	//***************************//
//	
//	//------------------------------------------------------------------------------------------------
//	void OnDelete()
//	{
//	}
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	bool NotifyAssignment()
//	{
//		return m_bNotifyAssignment;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool NotifyUnassign()
//	{
//		return m_bNotifyUnassign;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void DoNotifyAssignment(int assigneeID)
//	{
//		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
//		if (!assigneeNetworkComponent)
//			return;
//		
//		assigneeNetworkComponent.SendPlayerMessage(GetAssignMessage());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void DoNotifyUnassign(int assigneeID)
//	{
//		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
//		if (!assigneeNetworkComponent)
//			return;
//		
//		assigneeNetworkComponent.SendPlayerMessage(GetUnassignMessage());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_ERadioMsg GetAssignMessage()
//	{
//		return m_eAssignMessage;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_ERadioMsg GetUnassignMessage()
//	{
//		return m_eUnassignMessage;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	ResourceName GetMapUIElementResourceName()
//	{
//		return m_sMapUIElementResourceName;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetMapDescriptorText()
//	{
//		return GetTaskListTaskText();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnMapOpen(MapConfiguration config)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnHoverItem(MapItem item)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetFullTaskIconName()
//	{
//		return string.Format(GetTaskListIconNamePrefix() + GetTaskListIconName() + GetIconSuffix());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetTaskMapIconName()
//	{
//		return m_sMapIconName;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetTaskListIconNamePrefix()
//	{
//		return m_sTaskListIconNamePrefix;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetTaskListIconName()
//	{
//		return m_sTaskListIconName;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetTaskListTaskTitle()
//	{
//		return GetTitle();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetMajorModifier()
//	{
//		return m_sMajorModifier;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//Description
//	string GetTaskListTaskText()
//	{
//		return GetDescription();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	Widget GetParentWidget(notnull Widget rootWidget)
//	{
//		return rootWidget.FindAnyWidget("Tasks");
//	}
//	
//	ResourceName GetIconImageset()
//	{
//		return m_sIconImageset;
//	}	
//		
//	//------------------------------------------------------------------------------------------------
//	void SetHUDIcon()
//	{
//		SetWidgetIcon(m_wHUDIcon);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetWidgetIcon(ImageWidget image)
//	{
//		if (!image || !m_wMapTaskIcon)
//			return;
//		//Setting the regular task icon
//		image.LoadImageFromSet(0, m_sIconImageset, string.Format(GetTaskListIconNamePrefix() + GetTaskListIconName() + GetIconSuffix()));
//		UpdateMapTaskIcon();
//		//Setting the priority task icon
//		image.LoadImageFromSet(0, m_sIconImageset, string.Format(GetTaskListIconNamePrefix() + GetMajorModifier() + GetTaskListIconName() + GetIconSuffix()));
//		UpdatePriorityMapTaskIcon();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Remove icon assigned to this task.
//	void ClearWidgetIcon()
//	{
//		if (m_wMapTaskIcon && m_wMapTaskIcon.GetParent())
//			m_wMapTaskIcon.GetParent().RemoveFromHierarchy();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void UpdateMapTaskIcon()
//	{
//		if(m_bIsPriority)
//			return;	
//		// Set color for default unnasigned task
//		if(!m_wMapTaskIcon)	
//			return;	
//		
//		m_wMapTaskIcon.SetColor(Color.FromInt(Color.WHITE));
//		
//		if (m_TargetFaction && (IsAssignedToLocalPlayer() || SCR_EditorManagerEntity.IsOpenedInstance(false)) && !m_bIsPriority)
//			m_wMapTaskIcon.SetColor(m_TargetFaction.GetFactionColor());
//	}
//	//------------------------------------------------------------------------------------------------
//	void UpdatePriorityMapTaskIcon()
//	{
//		if(!m_bIsPriority)
//			return;
//		
//		GetTaskIconkWidget();
//		//Casting widgets, layouts and variables for Icon handling
//		
//		if (!m_wMapTaskIcon)
//			return;
//		
//		ImageWidget outline = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconOutline"));
//		ImageWidget background = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconBackground"));
//		ImageWidget hover = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconHover"));
//		ImageWidget symbol = ImageWidget.Cast(m_wMapTaskIcon.GetParent().FindAnyWidget("TaskIconSymbol"));
//		
//		PlayerController playerController = GetGame().GetPlayerController();	
//		if (!playerController)
//			return;
//		
//		m_sPlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
//		if (!m_sPlyFactionAffilComp)
//			return;
//		
//		SCR_Faction m_sfactionCol = SCR_Faction.Cast(m_sPlyFactionAffilComp.GetAffiliatedFaction());
//		if (!m_sfactionCol)
//			return;
//		
//		background.LoadImageFromSet(0, m_sIconImageset,"Icon_M_Task_BG");
//		outline.LoadImageFromSet(0, m_sIconImageset,"Icon_M_Task_Outline");
//		hover.LoadImageFromSet(0,m_sIconImageset,TASK_H);
//		
//		background.SetSize(m_fMajorSize,m_fMajorSize);
//		outline.SetSize(m_fMajorSize,m_fMajorSize);
//		symbol.SetSize(m_fMajorSize,m_fMajorSize);
//		hover.SetSize(m_fMajorSize,m_fMajorSize);
//		
//		// Set color for assigned tasks
//		if (IsAssignedToLocalPlayer())
//		{		
//			if (!outline || !background || !hover || !symbol)
//				return;
//			
//			background.SetColor(UIColors.CONTRAST_COLOR);
//			outline.SetColor(m_sfactionCol.GetOutlineFactionColor()); 
//			symbol.SetColor(m_sfactionCol.GetOutlineFactionColor()); 
//			return;
//		}
//		//Set color for unassigned priority task
//		if (!IsAssignedToLocalPlayer())
//		{
//			background.SetColor(m_sfactionCol.GetOutlineFactionColor()); 
//			outline.SetColor(m_sfactionCol.GetFactionColor());
//			symbol.SetColor(m_sfactionCol.GetFactionColor());
//		}
//		
//	}
//	//------------------------------------------------------------------------------------------------
//	bool AssignTaskToAI(AIAgent agent)
//	{
//		// Find entities in hierarchy - AIWaypoint type
//		// On agent add all waypoints
//		// Order matters! Check if it's deterministic
//		// Return false if no agent, no waypoint
//		// Return true at the end
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetIconName()
//	{
//		return GetTaskMapIconName() + GetIconSuffix();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	ResourceName GetImageSetName()
//	{
//		return GetMapUIElementResourceName();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetIconSuffix()
//	{
//		return "";
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetTitleIconString()
//	{
//		return string.Format("<image set='%1' name='%2' />", GetMapUIElementResourceName(), GetIconName() + GetIconSuffix());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		textWidget.SetTextFormat(taskText);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	string GetTitleText()
//	{
//		return m_sName; // todo(koudelkaluk): merge with GetTitle()
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		textWidget.SetTextFormat(taskText);
//	}
//		
//	//------------------------------------------------------------------------------------------------
//	Widget GenerateTaskDescriptionUI(notnull Widget rootWidget, array<Widget> widgets)
//	{		
//		Widget parentWidget = GetParentWidget(rootWidget);
//		if (!parentWidget)
//			return null;
//				
//		WorkspaceWidget workspaceWidget = rootWidget.GetWorkspace();
//		if (!workspaceWidget)
//			return null;
//		
//		SCR_BaseTaskSupportEntity supportClass = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportClass)
//			return null;
//		
//		ResourceName taskDescriptionWidgetResourceName = supportClass.GetTaskDescriptionWidgetResource();
//		if (taskDescriptionWidgetResourceName == string.Empty)
//			return null;
//		
//		Widget taskDescriptionWidget = workspaceWidget.CreateWidgets(taskDescriptionWidgetResourceName, parentWidget);
//		widgets.Insert(taskDescriptionWidget);
//		VerticalLayoutSlot.SetPadding(taskDescriptionWidget, 4, 4, 4, 4);
//		
//		 m_wTaskListDescription = taskDescriptionWidget;
//		
//		SCR_TaskListEntryHandler handler = SCR_TaskListEntryHandler.Cast(taskDescriptionWidget.FindHandler(SCR_TaskListEntryHandler));
//		if (handler)
//		{	
//			//handler.SetTask(this);
//			//handler.UpdateTask(this);
//		}
//		
//		Widget doneLayout = taskDescriptionWidget.FindAnyWidget("DoneLayout");
//		if (doneLayout)
//		{
//			if (m_eState == SCR_TaskState.FINISHED)
//				doneLayout.SetOpacity(1);
//			else
//				doneLayout.SetOpacity(0);
//		}
//		
//		RichTextWidget textWidgetRich = RichTextWidget.Cast(taskDescriptionWidget.FindAnyWidget("TaskDescription"));
//		if (!textWidgetRich)
//			return taskDescriptionWidget;
//		
//		string taskText = GetTaskListTaskText();
//
//		textWidgetRich.SetText(taskText);
//										
//		TextWidget textWidget = TextWidget.Cast(taskDescriptionWidget.FindAnyWidget("TaskTitle"));
//		if (!textWidget)
//			return taskDescriptionWidget;
//		
//		string taskTextTitle = GetTaskListTaskTitle();
//
//		SetTitleWidgetText(textWidget, taskTextTitle);
//		SetWidgetIcon(ImageWidget.Cast(taskDescriptionWidget.FindAnyWidget("TaskIcon")));
//		
//		 UpdateTaskListAssignee();
//		
//		return taskDescriptionWidget;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void UpdateTaskListAssignee()
//	{
//		if (!m_wTaskListDescription)
//			return;
//		
//		TextWidget textWidget = TextWidget.Cast(m_wTaskListDescription.FindAnyWidget("TaskAssignee"));
//		if (!textWidget)
//			return;
//		
//		const string assigneeNames;
//		
//		//Names of assignees are disabled temporarily to be fixed later
//		/*if (IsAssigned())
//		{
//			textWidget.SetTextFormat("%1", GetAllAssigneeNamesString());
//		} 
//		else 
//		{
//			textWidget.SetTextFormat(assigneeNames);
//		}*/
//		
//		textWidget.SetTextFormat(assigneeNames);		
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnAssigneeKilled()
//	{
//		
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void CreateMapUIIcon()
//	{
//
//	}	
//		
//	//------------------------------------------------------------------------------------------------
//	//! Returns the faction that should complete this task.
//	Faction GetTargetFaction()
//	{
//		return m_TargetFaction;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Sets the faction that should complete this task.
//	void SetTargetFaction(Faction targetFaction)
//	{
//		m_TargetFaction = targetFaction;
//		SCR_BaseTaskManager.s_OnTaskFactionAssigned.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_PROPERTY_CHANGED);
//		
//		SetHUDIcon();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns a text informing about the task being finished
//	string GetFinishText()
//	{
//		return TASK_FINISHED_TEXT;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns whether the task is assigned to someone or not
//	bool IsAssigned()
//	{
//		if (!m_aAssignees || m_aAssignees.Count() <= 0)
//			return false;
//		
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns a string with all the assignees on this task in following format: "PlayerA, PlayerB, ..., PlayerN"
//	string GetAllAssigneeNamesString()
//	{
//		string names = "";
//		if (!IsAssigned())
//			return names;
//		
//		string playerName;
//		for (int i = 0, count = m_aAssignees.Count(); i < count; i++)
//		{
//			playerName = m_aAssignees[i].GetPlayerName();
//			//if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
//			if (true)
//			{
//				if (GetGame().GetPlayerManager().GetPlatformKind(m_aAssignees[i].GetExecutorID()) == PlatformKind.PSN)
//					playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
//				else
//					playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
//			}
//			
//			names += playerName;
//			if (i < count - 1)
//				names += ", ";
//		}
//		Print(names);
//		return names;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Checks all the assignees for timeout, if their time is up, they get unassigned
//	void CheckAssigneeTimeout()
//	{
//		if (!m_bIndividualTask || !m_aAssignees || !IsAssigned() || !GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		if (!supportEntity)
//			return;
//		
//		if (GetAssigneeTimeLeft() <= 0)
//		{
//			foreach (SCR_BaseTaskExecutor assignee : m_aAssignees)
//			{
//				if (!assignee)
//					continue;
//				
//				supportEntity.UnassignTask(this, assignee, SCR_EUnassignReason.ASSIGNEE_TIMEOUT);
//			}
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetLastAssigneeAddedTimestamp(float timestamp)
//	{
//		m_fLastAssigneeAddedTimestamp = timestamp;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Set this task as task for an individual
//	void SetIndividual(bool individual)
//	{
//		m_bIndividualTask = individual;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns whether this task is for an individual or not
//	bool IsIndividual()
//	{
//		return m_bIndividualTask;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetIsPriority(bool isPrio)
//	{
//		m_bIsPriority = isPrio;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool IsPriority()
//	{
//		return m_bIsPriority;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool IsAssignable()
//	{
//		return m_bAssignable;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns whether this task can be assigned to the player with the given player ID
//	//! If playerID == -1, the local player is used as the target
//	bool CanBeAssigned(out SCR_ECannotAssignReasons reason, int playerID = -1)
//	{
//		SCR_BaseTaskExecutor executor = null;
//		SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
//		if (!localExecutor)
//			return false;
//		
//		SCR_BaseTask localExecutorTask = localExecutor.GetAssignedTask();
//		if (localExecutorTask)
//		{
//			if (localExecutorTask != this)
//				reason = SCR_ECannotAssignReasons.LOCAL_EXECUTOR_IS_ASSIGNED;
//			else
//				reason = SCR_ECannotAssignReasons.TASK_IS_ASSIGNED_TO_LOCAL_EXECUTOR;
//			
//			return false;
//		}
//		
//		if (playerID == -1)
//			executor = localExecutor;
//		else
//			executor = SCR_BaseTaskExecutor.GetTaskExecutorByID(playerID);
//		
//		if (m_bAssignable && !m_bIndividualTask)
//			return true;
//
//		if (m_bIndividualTask)
//		{
//			if (IsAssigned())
//			{
//				reason = SCR_ECannotAssignReasons.TASK_IS_ASSIGNED;
//				return false;
//			}
//			
//			if (m_TimedOutAssignee == executor && m_fAssigneeTimeoutTimestamp > GetTaskManager().GetTimestamp())
//			{
//				reason = SCR_ECannotAssignReasons.ASSIGNEE_TIMEOUT;
//				return false;
//			}
//
//			if (GetTaskManager().GetAssigneeAbandoned(executor))
//			{
//				reason = SCR_ECannotAssignReasons.TASK_ABANDONED;
//				return false;
//			}
//			
//			// Is individual task, that is not assigned yet, can be assigned
//			return true;
//		}
//		else
//		{
//			// Cannot be assigned
//			reason = SCR_ECannotAssignReasons.TASK_NOT_ASSIGNABLE;
//			return false;
//		}
//
//		// Is not individual task, can be assigned
//		reason = SCR_ECannotAssignReasons.TASK_NOT_ASSIGNABLE;
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the timestamp of last add assignee event
//	float GetLastAssigneeAddedTimestamp()
//	{
//		return m_fLastAssigneeAddedTimestamp;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns time left the assignee of this task has to finish it, after that they get automatically unassigned
//	float GetAssigneeTimeLeft()
//	{
//		float currentTimestamp = GetTaskManager().GetTimestamp();
//		float timeLeft = m_fAssigneeTimeLimit - (currentTimestamp - m_fLastAssigneeAddedTimestamp);
//		
//		return timeLeft;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Sets an ID of this task
//	void SetTaskID(int taskID)
//	{
//		m_iTaskID = taskID;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns an ID of this task
//	int GetTaskID()
//	{
//		return m_iTaskID;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the state of this task
//	SCR_TaskState GetTaskState()
//	{
//		return m_eState;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTitle(string title)
//	{
//		m_sName = title;
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_PROPERTY_CHANGED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Return the title of this task.
//	string GetTitle()
//	{
//		return m_sName;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetDescription(string description)
//	{
//		m_sDescription = description;
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_PROPERTY_CHANGED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Return the description of this task.
//	string GetDescription()
//	{
//		return m_sDescription;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the first assignee of this task.
//	SCR_BaseTaskExecutor GetAssignee()
//	{
//		if (!m_aAssignees || m_aAssignees.Count() <= 0)
//			return null;
//		
//		return m_aAssignees[0];
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns assignees of this task.
//	int GetAssignees(out array<SCR_BaseTaskExecutor> assignees)
//	{
//		return assignees.Copy(m_aAssignees);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	int GetAssigneeCount()
//	{
//		return m_aAssignees.Count();
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void Cancel(bool showMsg = true)
//	{
//		SetState(SCR_TaskState.CANCELLED);
//		
//		SCR_BaseTaskManager.s_OnTaskCancelled.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_CANCELED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Marks the task as removed.
//	void Remove()
//	{
//		SetState(SCR_TaskState.REMOVED);
//		
//		if (m_aAssignees)
//		{
//			foreach (SCR_BaseTaskExecutor assignee: m_aAssignees)
//				assignee.OnAssignedTaskRemoved();
//		}
//		
//		SCR_BaseTaskManager.s_OnTaskRemoved.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_REMOVED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! When the task is created
//	void Create(bool showMsg = true)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Marks the task as finished.
//	void Finish(bool showMsg = true)
//	{
//		SetState(SCR_TaskState.FINISHED);
//		
//		if (m_aAssignees)
//		{
//			foreach (SCR_BaseTaskExecutor assignee: m_aAssignees)
//				assignee.OnAssignedTaskFinished();
//		}
//		
//		SCR_BaseTaskManager.s_OnTaskFinished.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_FINISHED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Fails the task.
//	void Fail(bool showMsg = true)
//	{
//		SetState(SCR_TaskState.CANCELLED);
//		
//		if (m_aAssignees)
//		{
//			foreach (SCR_BaseTaskExecutor assignee: m_aAssignees)
//				assignee.OnAssignedTaskFailed();
//		}
//		
//		SCR_BaseTaskManager.s_OnTaskFailed.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_FAILED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Removes every assignee from this task.
//	void RemoveAllAssignees()
//	{
//		if (!IsAssigned())
//			return;
//		
//		SCR_BaseTaskExecutor removedAssignee = null;
//		
//		for (int i = m_aAssignees.Count() - 1; i >= 0; i --)
//		{
//			removedAssignee = m_aAssignees[i];
//			m_aAssignees[i].OnAssignedTaskRemoved();
//			m_aAssignees.Remove(i);
//		}
//		
//		if (m_bIndividualTask)
//		{
//			// We automatically set timeout for the assignee, since we don't want them to be immediately able to assign the task to themselves again
//			m_fAssigneeTimeoutTimestamp = GetTaskManager().GetTimestamp() + DEFAULT_ASSIGNEE_TIMEOUT_TIME;
//			m_TimedOutAssignee = removedAssignee;
//			
//			ShowAvailableTask();
//		}
//		
//		SCR_BaseTaskManager.s_OnTaskUnassigned.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Removes an assignee, specified as a parameter, from this task.
//	void RemoveAssignee(SCR_BaseTaskExecutor assignee, SCR_EUnassignReason reason)
//	{
//		if (!m_aAssignees || !assignee)
//			return;
//		
//		int index = m_aAssignees.Find(assignee);
//		
//		if (index == -1)
//			return;
//		
//		m_aAssignees.Remove(index);
//		if (assignee == SCR_BaseTaskExecutor.GetLocalExecutor())
//			ShowPopUpNotification("#AR-Tasks_UnassignPopup");
//
//		OnAssigneeRemoved(assignee);
//		
//		if (m_bIndividualTask)
//		{
//			switch (reason)
//			{
//				case SCR_EUnassignReason.ASSIGNEE_TIMEOUT:
//					// We set timeout for the assignee, since we don't want them to be immediately able to assign the task to themselves again
//					m_fAssigneeTimeoutTimestamp = GetTaskManager().GetTimestamp() + DEFAULT_ASSIGNEE_TIMEOUT_TIME;
//					m_TimedOutAssignee = assignee;
//					break;
//				case SCR_EUnassignReason.ASSIGNEE_DISCONNECT:
//					break;
//				case SCR_EUnassignReason.ASSIGNEE_ABANDON:
//					break;
//				case SCR_EUnassignReason.GM_REASSIGN:
//					break;
//			}
//			
//			ShowAvailableTask(true);
//		}
//		
//		SCR_BaseTaskManager.s_OnTaskUnassigned.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Adds a new assignee, specified as a parameter, to this task.
//	void AddAssignee(SCR_BaseTaskExecutor assignee, float timestamp)
//	{
//		if (!m_aAssignees || !assignee)
//			return;
//		
//		IEntity assigneeEntity = assignee.GetControlledEntity();
//		if (assigneeEntity)
//		{
//			SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(assigneeEntity.FindComponent(SCR_CharacterControllerComponent));
//			if (characterControllerComponent)
//				characterControllerComponent.GetOnPlayerDeath().Insert(OnAssigneeKilled);
//		}
//		else
//		{
//			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//			if (!gameMode)
//				return;
//			gameMode.GetOnPlayerSpawned().Insert(InitOnSpawn);
//		}
//		
//		// Is this task for an individual & we already have someone assigned?
//		if (m_bIndividualTask && m_aAssignees.Count() > 0)
//			return;
//		
//		int index = m_aAssignees.Find(assignee);
//		
//		if (index != -1)
//			return;
//		
//		SetLastAssigneeAddedTimestamp(timestamp);
//		
//		m_aAssignees.Insert(assignee);
//		if (assignee == SCR_BaseTaskExecutor.GetLocalExecutor())
//			ShowPopUpNotification("#AR-Tasks_AssignPopup");
//
//		OnAssigneeAdded(assignee);
//		
//		SCR_BaseTaskManager.s_OnTaskAssigned.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_ASSIGNEE_CHANGED);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void InitOnSpawn(int pid, IEntity ent)
//	{
//		SCR_CharacterControllerComponent charCtrlComp = SCR_CharacterControllerComponent.Cast(ent.FindComponent(SCR_CharacterControllerComponent));
//		if (charCtrlComp)
//			charCtrlComp.GetOnPlayerDeath().Insert(OnAssigneeKilled);
//		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
//		if (gameMode)
//			gameMode.GetOnPlayerSpawned().Remove(InitOnSpawn);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	[Obsolete("Use new SCR_TaskManagerUIComponent methods instead")]
//	void ToggleHUDIcon(bool show, bool fade = true)
//	{
//		if (show)
//		{
//			AnimateWidget.StopAnimation(m_wHUDIcon, WidgetAnimationOpacity);
//			m_wHUDIcon.SetOpacity(1);
//		}
//		else
//		{
//			if (fade)
//			{
//				//SCR_UITaskManagerComponent.GetInstance().KeepHUDIconUpdated(1000, this);
//				AnimateWidget.Opacity(m_wHUDIcon, 0, 1);
//			}
//			else
//			{
//				AnimateWidget.StopAnimation(m_wHUDIcon, WidgetAnimationOpacity);
//				m_wHUDIcon.SetOpacity(0);
//			}
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void UpdateHUDIcon()
//	{
//		vector pos = GetGame().GetWorkspace().ProjWorldToScreen(GetOrigin(), GetWorld());
//		if (pos[2] > 0)
//		{
//			if (!AnimateWidget.IsAnimating(m_wHUDIcon))
//				m_wHUDIcon.SetOpacity(1);
//			FrameSlot.SetPos(m_wHUDIcon, pos[0], pos[1]);
//		}
//		else
//			m_wHUDIcon.SetOpacity(0);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	Widget GetTaskIconkWidget()
//	{
//		return m_wMapTaskIcon;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTaskIconWidget(Widget w)
//	{
//		m_wMapTaskIcon = w;
//	}
//	
//	//************************//
//	//PROTECTED MEMBER METHODS//
//	//************************//
//	
//	protected void ShowPopUpNotification(string subtitle)
//	{
//		SCR_PopUpNotification.GetInstance().PopupMsg(GetTitleText(), text2: subtitle);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void UpdateMapInfo()
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected bool DoneByAssignee()
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ShowTaskProgress(bool showMsg = true)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Shows a message informing about this task being available
//	protected void ShowAvailableTask(bool afterAssigneeRemoved = false)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns whether this task is assigned to the local player
//	protected bool IsAssignedToLocalPlayer()
//	{
//		if (!IsAssigned())
//			return false;
//		
//		SCR_BaseTaskExecutor localExecutor = SCR_BaseTaskExecutor.GetLocalExecutor();
//		int localExecutorID = SCR_BaseTaskExecutor.GetTaskExecutorID(localExecutor);
//		
//		foreach (SCR_BaseTaskExecutor assignee : m_aAssignees)
//		{
//			int assigneeID = SCR_BaseTaskExecutor.GetTaskExecutorID(assignee);
//			if (assigneeID == localExecutorID)
//				return true;
//		}
//		
//		return false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Changes the state of this task to the parameter.
//	//! Don't use this directly, use the following methods instead: SCR_BaseTaskManager.FinishTask/CancelTask/FailTask
//	void SetState(SCR_TaskState state)
//	{
//		if (state == m_eState)
//			return;
//		
//		// Let the script know that the state of this task has changed.
//		OnStateChanged(m_eState, state);
//		
//		m_eState = state;
//	}
//	
//	//******************************//
//	//PROTECTED MEMBER EVENT METHODS//
//	//******************************//
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when an assignee has been removed from this task.
//	protected void OnAssigneeRemoved(SCR_BaseTaskExecutor oldAssignee)
//	{
//		if (oldAssignee)
//			oldAssignee.AssignNewTask(null);
//		
//		if (m_aAssignees.Count() <= 0)
//			SetState(SCR_TaskState.OPENED);
//		
//		UpdateMapTaskIcon();	
//		UpdatePriorityMapTaskIcon();
//		UpdateTaskListAssignee();
//
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when a new assignee has been added to this task.
//	protected void OnAssigneeAdded(SCR_BaseTaskExecutor newAssignee)
//	{
//		if (newAssignee)
//			newAssignee.AssignNewTask(this);
//			UpdateMapTaskIcon();
//			UpdatePriorityMapTaskIcon();
//			UpdateTaskListAssignee();	
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! An event called when the state of this task has been changed. 
//	protected void OnStateChanged(SCR_TaskState previousState, SCR_TaskState newState)
//	{
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Registers task event mask for OnTaskUpdate
//	//void RegisterTaskUpdate(SCR_ETaskEventMask TaskEventMask)
//	//{
//		//if (GetTaskManager())
//			//GetTaskManager().OnTaskUpdate(this, TaskEventMask);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void Serialize(ScriptBitWriter writer)
//	{
//		// Make sure that the server tells clients whether it had the faction manager
//		// during serialization or not. Because tasks can't some from non-replicated
//		// prefabs, this will help us avoid deserialization issues mentioned in Deserialize.
//		FactionManager factionManager = GetGame().GetFactionManager();
//		if (!factionManager)
//		{
//			writer.WriteBool(false);
//			return;
//		}
//		writer.WriteBool(true);
//		
//		int factionIndex = factionManager.GetFactionIndex(m_TargetFaction);
//		writer.Write(factionIndex, 4);
//		writer.WriteInt(m_iTaskID);
//		writer.WriteInt(GetTaskState());
//		writer.WriteInt(GetTaskManager().m_mTaskEventMaskMap.Get(this));
//		writer.WriteBool(m_bIndividualTask);
//		writer.WriteFloat(GetLastAssigneeAddedTimestamp());
//		writer.WriteString(GetTitle());
//		writer.WriteString(GetDescription());
//		
//		int assigneesCount = m_aAssignees.Count();
//		writer.Write(assigneesCount, 7);
//		
//		for (int i = 0; i < assigneesCount; i++)
//		{
//			int assigneeID = SCR_BaseTaskExecutor.GetTaskExecutorID(m_aAssignees[i]);
//			writer.WriteInt(assigneeID);
//		}
//		
//		writer.WriteVector(GetOrigin());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void Deserialize(ScriptBitReader reader)
//	{
//		// Make sure that the server actually had the faction manager
//		// during serialization. If it did not, we can safely return
//		// because the server is guaranteed to not have written anything.
//		bool serverHadFactionManager;
//		reader.ReadBool(serverHadFactionManager);
//		if (!serverHadFactionManager)
//			return;
//
//		// https://jira.bistudio.com/browse/ARMA4-62494
//		// If there's no faction manager we won't be able to update the faction
//		// index. However, if we simply returned right now, we'd create a huge problem
//		// because we wouldn't read the rest of our data.
//		// A task following after this one would therefore read data that would still
//		// partially belong to this one. That would result in undefined behavior and
//		// most likely in crashes during ScriptBitReader's data serialization.
//		// For this reason, even if the faction manager is not present, we proceed.
//		// We simply won't be able to set the faction properly.
//		FactionManager factionManager = GetGame().GetFactionManager();
//		//if (!factionManager)
//		//	return; <<< DON'T DO THIS
//		
//		// Reading factionIndex
//		int factionIndex = 0;
//		reader.Read(factionIndex, 4);
//		
//		if (factionManager)
//			SetTargetFaction(factionManager.GetFactionByIndex(factionIndex));
//
//		// Reading m_iTaskID
//		reader.ReadInt(m_iTaskID);
//		
//		// Reading Task State
//		int taskState;
//		reader.ReadInt(taskState);
//		SetState(taskState);
//		
//		// Reading Task Mask
//		int taskMask;
//		reader.ReadInt(taskMask);
//		GetTaskManager().m_mTaskEventMaskMap.Set(this, taskMask);
//
//		// Reading m_bIndividualTask
//		reader.ReadBool(m_bIndividualTask);
//
//		// Reading m_fLastAssigneeAddedTimestamp
//		reader.ReadFloat(m_fLastAssigneeAddedTimestamp);
//		
//		string text;
//		// Reading title
//		reader.ReadString(text);
//		SetTitle(text);
//		
//		// Reading description
//		reader.ReadString(text);
//		SetDescription(text);
//		
//		// Reading target m_aAssignees.Count()
//		int assigneesCount;
//		reader.Read(assigneesCount, 7);
//		
//		int assigneeID;
//		for (int i = 0; i < assigneesCount; i++)
//		{
//			// Reading assignee ID
//			reader.ReadInt(assigneeID);
//			SCR_BaseTaskExecutor assignee = SCR_BaseTaskExecutor.GetTaskExecutorByID(assigneeID);
//			m_aAssignees.Insert(assignee);
//		}
//		
//		// Reading position
//		vector origin;
//		reader.ReadVector(origin);
//		SetOrigin(origin);
//
//		GetTaskManager().RegisterTask(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void EOnInit(IEntity owner)
//	{
//		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
//			return;
//		
//		if (!GetTaskManager().IsProxy())
//		{
//			m_iTaskID = s_iCurrentTaskID;
//			s_iCurrentTaskID++;
//		}
//		
//		GetTaskManager().RegisterTask(this);
//		
//		SCR_BaseTaskManager.s_OnTaskCreated.Invoke(this);
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_CREATED);
//		
//		m_MapDescriptor = SCR_MapDescriptorComponent.Cast(FindComponent(SCR_MapDescriptorComponent));
//		
//		if (m_MapDescriptor)
//		{
//			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
//			SCR_MapEntity.GetOnHoverItem().Insert(OnHoverItem);
//		}
//	}
//	
//	//************************//
//	//CONSTRUCTOR & DESTRUCTOR//
//	//************************//
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_BaseTask(IEntitySource src, IEntity parent)
//	{
//		SetEventMask(EntityEvent.INIT);
//		
//		CreateMapUIIcon();
//		
//		//HUD Icon setup
//		Widget HUDIconRoot = GetGame().GetWorkspace().CreateWidgets(m_sHUDIcon);
//		m_wHUDIcon = ImageWidget.Cast(HUDIconRoot.FindAnyWidget("TaskIcon"));
//		m_wHUDIcon.SetOpacity(0);
//		SetHUDIcon();
//		//End of HUD Icon setup
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_BaseTask()
//	{
//		if (m_aAssignees)
//		{
//			m_aAssignees.Clear();
//			m_aAssignees = null;
//		}
//		
//		if (GetTaskManager())
//			SCR_BaseTaskManager.s_OnTaskDeleted.Invoke(this);
//	}
//}
