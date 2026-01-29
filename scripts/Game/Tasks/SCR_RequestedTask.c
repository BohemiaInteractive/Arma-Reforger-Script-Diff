//[EntityEditorProps(category: "GameScripted/Tasks", description: "A requested task.", color: "0 0 255 255")]
//class SCR_RequestedTaskClass: SCR_BaseTaskClass
//{
//};
//
////THIS TASK TYPE IS NOT SYNCHRONIZED BY ITSELF YET. WHEREAS IT'S INHERITED (E.G.) SCR_EvacuateTask IS.
////------------------------------------------------------------------------------------------------
//class SCR_RequestedTask : SCR_BaseTask
//{
//	[Attribute(SCR_EXPRewards.SUPPORT_EVAC.ToString(), uiwidget: UIWidgets.ComboBox, enums:ParamEnumArray.FromEnum(SCR_EXPRewards))]
//	SCR_EXPRewards m_AssigneeXPReward;
//	
//	int m_iRequesterID = -1;
//	SCR_TaskExecutor m_Requester;
//	
//	static const string TASK_REQUEST_TRANSMITTED_TEXT = "#AR-Tasks_StatusTransmitted-UC";
//	static const string TASK_SUPPORT_CANCELLED_TEXT = "#AR-Tasks_StatusCancelled-UC";
//	protected static const string LOCAL_PLAYER_REQUEST = "#AR-Tasks_TitleRequest";
//	
//	//------------------------------------------------------------------------------------------------
//	protected void ShowAvailableTask(bool afterAssigneeRemoved = false)
//	{
//		SCR_TaskExecutor localExecutor = SCR_TaskExecutor.FromLocalPlayer();
//		if (localExecutor == m_Requester)
//		{
//			if (!afterAssigneeRemoved)
//				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_REQUEST_TRANSMITTED_TEXT + " " + GetTaskName(), prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE);
//			return;
//		}
//		
//		Faction localPlayerFaction;
//		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
//		if (factionManager)
//			localPlayerFaction = factionManager.GetLocalPlayerFaction();
//		
//		if (GetOwnerFactionKeys().Contains(localPlayerFaction.GetFactionKey()))
//			SCR_PopUpNotification.GetInstance().PopupMsg(SCR_TextsTaskManagerComponent.TASK_AVAILABLE_TEXT + " " + GetTaskName(), prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, text2: SCR_TextsTaskManagerComponent.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: SCR_SoundEvent.TASK_CREATED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	protected bool IsLocallyRequestedTask()
//	{
//		if (!GetTaskManager())
//			return false;
//		
//		SCR_RequestedTaskSupportEntity supportEntity = SCR_RequestedTaskSupportEntity.Cast(GetTaskManager().FindSupportEntity(SCR_RequestedTaskSupportEntity));
//		if (!supportEntity)
//			return false;
//		
//		return this == supportEntity.GetLocallyRequestedTask();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		if (IsLocallyRequestedTask())
//		{
//			textWidget.SetTextFormat(GetTaskManager().m_sLocalRequestTitle);
//			return;
//		}
//		if (m_Requester)
//			textWidget.SetTextFormat(taskText, m_Requester.GetPlayerName());
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override LocalizedString GetTaskName(out array<LocalizedString> params)
//	{
//		//if (IsLocallyRequestedTask())
//			//return GetTaskManager().m_sLocalRequestTitle;
//
////		if (m_Requester)
////			return string.Format("%1 %2", m_TaskData.m_UIInfo.GetUnformattedName(params), m_Requester.GetPlayerName());
//
//		return super.GetTaskName(params);
//	}
//	
//	//------------------------------------------------------------------------------------------------
////	Widget GenerateTaskDescriptionUI(notnull Widget rootWidget, array<Widget> widgets)
////	{
////		Widget w = super.GenerateTaskDescriptionUI(rootWidget, widgets);
////		
////		if (IsLocallyRequestedTask())
////		{
////			Widget description = w.FindAnyWidget("TaskDescription");
////		}
////		
////		return w;
////	}
//	
//	//------------------------------------------------------------------------------------------------
//	void DoNotifyAssignment(int assigneeID)
//	{
////		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
////		
////		if (assigneeNetworkComponent)
////			assigneeNetworkComponent.SendPlayerMessage(GetAssignMessage(), calledID: m_iRequesterID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void DoNotifyUnassign(int assigneeID)
//	{
////		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
////		
////		if (assigneeNetworkComponent)
////			assigneeNetworkComponent.SendPlayerMessage(GetUnassignMessage(), calledID: m_iRequesterID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		textWidget.SetTextFormat(taskText, GetRequesterName());
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void RequesterDied()
//	{
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();		
//		if (!taskSystem)
//			return;
//		
//		taskSystem.SetTaskState(this, SCR_ETaskState.FAILED);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	SCR_TaskExecutor GetRequester()
//	{
//		return m_Requester;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	string GetRequesterName()
//	{
//		return SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(m_iRequesterID);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnEntityDeath(IEntity killedEntity)
//	{
//		if (!m_Requester)
//			return;
//		
//		if (killedEntity == GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_TaskExecutorPlayer.Cast(m_Requester).GetPlayerID()))
//			SetTaskState(SCR_ETaskState.FAILED); //TODO
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetRequesterID(int requesterID)
//	{
//		m_iRequesterID = requesterID;
//		SetEventMask(EntityEvent.FRAME);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	bool AutoSetRequester()
//	{
//		if (m_iRequesterID == -1)
//			return false;
//		
//		SCR_TaskExecutor requester = SCR_TaskExecutor.FromPlayerID(m_iRequesterID);
//		if (requester)
//			SetRequester(requester);
//		else
//			return false;
//		
//		return true;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SetRequester(SCR_TaskExecutor requester)
//	{
//		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
//		if (!taskSystem)
//			return;
//		
//		if (!requester)
//			return;
//		
//		m_Requester = requester;
//		m_iRequesterID = SCR_TaskExecutorPlayer.Cast(requester).GetPlayerID();
//		
//		IEntity requesterEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(m_iRequesterID);
//		if (!requesterEntity)
//			return;
//		
//		SCR_CharacterControllerComponent characterControllerComponent = SCR_CharacterControllerComponent.Cast(requesterEntity.FindComponent(SCR_CharacterControllerComponent));
//		if (!characterControllerComponent)
//			return;
//		
//		characterControllerComponent.GetOnPlayerDeath().Insert(RequesterDied);
//		
//		SCR_TaskExecutor localTaskExecutor = SCR_TaskExecutor.FromLocalPlayer();
//		//if (localTaskExecutor.IsClonedBy(requester))
//			//SetLocallyRequestedTask(this);
//		
//		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(requesterEntity.FindComponent(FactionAffiliationComponent));
//		//if (factionAffiliationComponent)
//			//m_TargetFaction = factionAffiliationComponent.GetAffiliatedFaction();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void EOnFrame(IEntity owner, float timeSlice)
//	{
//		if (!m_Requester)
//			AutoSetRequester();
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override protected bool RplLoad(ScriptBitReader reader)
//	{
//		int requesterID;
//		reader.ReadInt(requesterID);
//		SetRequesterID(requesterID);
//		
//		return super.RplLoad(reader);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override protected bool RplSave(ScriptBitWriter writer)
//	{
//		writer.WriteInt(m_iRequesterID);
//		
//		return super.RplSave(writer);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void EOnInit(IEntity owner)
//	{
//		super.EOnInit(owner);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_RequestedTask(IEntitySource src, IEntity parent)
//	{
//		if (SCR_Global.IsEditMode(this))
//			return;
//		
//		SetEventMask(EntityEvent.FRAME);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_RequestedTask()
//	{
//	}
//
//};
