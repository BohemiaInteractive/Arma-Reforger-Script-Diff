[EntityEditorProps(category: "GameScripted/Tasks", description: "Move task.", color: "0 0 255 255")]
class SCR_EditorTaskClass: SCR_ExtendedTaskClass
{
}

class SCR_EditorTask : SCR_ExtendedTask
{	
	[Attribute(SCR_Enum.GetDefault(ETaskTextType.NONE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ETaskTextType))]
	protected ETaskTextType m_TextType;
	
	[Attribute("0", category: "Editor Task")]
	protected int m_iTextIndex;
	
	[Attribute("0", UIWidgets.ComboBox, "Task completion", "", ParamEnumArray.FromEnum(EEditorTaskCompletionType) )]
	protected EEditorTaskCompletionType m_iTaskCompletionType;
	
	protected LocalizedString m_sLocationName;
	
	protected static int s_iNextAutomatedTaskID = 0;
	protected const string GENERATED_TASK_PREFIX = "$EDITOR_TASK_";
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set name of location to which this task relates to.
	\param locationName Name of the location
	*/
	void SetLocationName(LocalizedString locationName)
	{
		m_sLocationName = locationName;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get name of location to which this task relates to.
	\return string locationName Name of the location
	*/
	string GetLocationName()
	{
		return m_sLocationName;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get type of custom texts this task should use.
	\return Type of texts
	*/
	ETaskTextType GetTextType()
	{
		return m_TextType;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get index of custom text from SCR_TextsTaskManagerComponent.
	\return Index from the array of texts
	*/
	int GetTextIndex()
	{
		return m_iTextIndex;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set index of custom text from SCR_TextsTaskManagerComponent.
	\param index Index from the array of texts
	*/
	void SetTextIndex(int index)
	{
		m_iTextIndex = index;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get the task completion type
	\return MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	EEditorTaskCompletionType GetTaskCompletionType()
	{
		return m_iTaskCompletionType;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set the task completion type
	\param newTaskCompletionType MANUAL and ALWAYS_MENUAL means only the GM can complete the task. AUTOMATIC means that the task can auto complete and/or fail depending on the task
	*/
	void SetTaskCompletionType(EEditorTaskCompletionType newTaskCompletionType)
	{
		if (m_iTaskCompletionType == newTaskCompletionType)
			return;
		
		m_iTaskCompletionType = newTaskCompletionType;
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetTaskState(SCR_ETaskState state)
	{
		if (state == GetTaskState())
			return;
		
		super.SetTaskState(state);
		
		string text;
		ENotification notification;
		switch (state)
		{
			case SCR_ETaskState.CREATED:
				text = SCR_TextsTaskManagerComponent.TASK_AVAILABLE_TEXT;
				notification = ENotification.EDITOR_TASK_PLACED;
				break;
			case SCR_ETaskState.COMPLETED:
				text = SCR_TextsTaskManagerComponent.TASK_COMPLETED_TEXT;
				notification = ENotification.EDITOR_TASK_COMPLETED;
				break;
			case SCR_ETaskState.FAILED:
				text = SCR_TextsTaskManagerComponent.TASK_FAILED_TEXT;
				notification = ENotification.EDITOR_TASK_FAILED;
				break;
			case SCR_ETaskState.CANCELLED:
				text = SCR_TextsTaskManagerComponent.TASK_CANCELLED_TEXT;
				notification = ENotification.EDITOR_TASK_CANCELED;
				break;
			
			default:
				return;
		}
		
		Rpc_PopUpNotification(text, true);
		Rpc(Rpc_PopUpNotification, text, true);
		
		ShowTaskNotification(notification);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void Rpc_PopUpNotification(string prefix, bool alwaysInEditor)
	{
		//--- Get player faction (prioritize respawn faction, because it's defined even when player is waiting for respawn)
		Faction playerFaction;
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			playerFaction = factionManager.GetLocalPlayerFaction();
				
		if (!playerFaction)
			playerFaction = SCR_PlayerController.GetLocalMainEntityFaction();
		
		string playerFactionKey;
		if (playerFaction)
			playerFactionKey = playerFaction.GetFactionKey();
		
		//--- Show notification when player is assigned, of the same faction, or has unlimited editor (i.e., is Game Master)
		if (IsTaskAssignedTo(SCR_TaskExecutor.FromLocalPlayer()) || GetOwnerFactionKeys().Contains(playerFactionKey) || (alwaysInEditor && !SCR_EditorManagerEntity.IsLimitedInstance()))
		{
			//--- SCR_PopUpNotification.GetInstance() is never null, as it creates the instance if it doesn't exist yet
			SCR_PopUpNotification.GetInstance().PopupMsg(prefix + " " + GetTaskName(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: m_sLocationName, sound: SCR_SoundEvent.TASK_SUCCEED);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowPopUpNotification(string subtitle)
	{
		SCR_PopUpNotification.GetInstance().PopupMsg(GetTaskName(), text2: subtitle, param1: m_sLocationName);
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Show notification related to the task state. Delete notification is called by SCR_EditableTaskComponent
	\param taskNotification notification to show
	*/
	void ShowTaskNotification(ENotification taskNotification, bool SendOverNetwork = false)
	{
		SCR_EditableEntityComponent editableTask = SCR_EditableEntityComponent.Cast(FindComponent(SCR_EditableEntityComponent));
		if (!editableTask)
			return;
	
		int taskID = Replication.FindId(editableTask);
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;
		
		Faction faction = factionManager.GetFactionByKey(GetOwnerFactionKeys()[0]);
		if (!faction)
			return;
		
		vector position = GetTaskPosition();
		
		int factionIndex = factionManager.GetFactionIndex(faction);
		
		//Send local GM
		if (!SendOverNetwork)
		{
			if (taskNotification == ENotification.EDITOR_TASK_PLACED)
				GetGame().GetCallqueue().CallLater(DelayedPlacedNotification, 1, false, position, taskID, factionIndex);
			else
				SCR_NotificationsComponent.SendLocalUnlimitedEditor(taskNotification, position, taskID, factionIndex);
		}
		else 
		{
			SCR_NotificationsComponent.SendToUnlimitedEditorPlayers(taskNotification, position, taskID, factionIndex);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DelayedPlacedNotification(vector position, int taskID, int factionIndex)
	{
		SCR_NotificationsComponent.SendLocalUnlimitedEditor(ENotification.EDITOR_TASK_PLACED, position, taskID, factionIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		if (Replication.IsClient())
			return;

		SetTaskID(GENERATED_TASK_PREFIX + s_iNextAutomatedTaskID);
		s_iNextAutomatedTaskID++;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_EditorTask()
	{
		s_iNextAutomatedTaskID--;
	}
}