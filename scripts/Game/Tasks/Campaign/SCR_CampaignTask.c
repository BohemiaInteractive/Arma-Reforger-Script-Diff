//------------------------------------------------------------------------------------------------
//class SCR_CampaignTaskClass: SCR_CampaignBaseTaskClass
//{
//};

//------------------------------------------------------------------------------------------------
//class SCR_CampaignTask : SCR_CampaignBaseTask
//{
//	static const string CAMPAIGN_TASK_RECONFIGURED_TEXT = "#AR-CampaignTasks_Reconfigured-UC";
//	static const string CAMPAIGN_TASK_RECONFIGURED_BY_TEXT = "#AR-CampaignTasks_ReconfiguredBy-UC";
//	
//	//*****************//
//	//MEMBER ATTRIBUTES//
//	//*****************//
//	
//	[Attribute(defvalue: "Task title for reconfigure.", desc: "The task title visible to the player.")]
//	protected string m_sNameReconfigure;
//	[Attribute(defvalue: "Task description.", desc: "The task description visible to the player.")]
//	protected string m_sDescriptionReconfigure;
//	
//	protected string m_sAffix;
//	
//	//**************************//
//	//PROTECTED MEMBER VARIABLES//
//	//**************************//
//	
//	protected SCR_CampaignTaskType m_eType;
//	
//	//*********************************//
//	//PROTECTED OVERRIDE MEMBER METHODS//
//	//*********************************//
//	
//	//------------------------------------------------------------------------------------------------
//	protected override bool DoneByAssignee()
//	{
//		SCR_BaseTaskExecutor assignee = GetAssignee();
//		if (assignee && m_TargetBase)
//		{
//			int assigneeID = SCR_BaseTaskExecutor.GetTaskExecutorID(assignee);
//			int reconfigurerID = m_TargetBase.GetReconfiguredByID();
//			if (assigneeID == reconfigurerID)
//				return true;
//		}
//		
//		return false;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Shows a message about this task being available again
//	protected override void ShowAvailableTask(bool afterAssigneeRemoved = false)
//	{
//		SCR_ECannotAssignReasons reason;
//		if (CanBeAssigned(reason))
//		{
//			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//			if (campaign)
//			{
//				string text = TASK_AVAILABLE_TEXT + " " + GetTitle();
//				string baseName;
//				
//				if (m_TargetBase)
//					baseName = GetBaseNameWithCallsign();
//				
//				SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_AVAILABLE, param1: baseName, text2: TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT, sound: SCR_SoundEvent.TASK_CREATED);
//			}
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override protected void ShowTaskProgress(bool showMsg = true)
//	{
//		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//		if (campaign && showMsg)
//		{
//			string baseName;
//			
//			if (m_TargetBase)
//				baseName = GetBaseNameWithCallsign();
//			
//			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_PROGRESS_TEXT + " " + GetTitle() + " " + TASK_AMOUNT_COMPLETED_TEXT, prio: SCR_ECampaignPopupPriority.TASK_PROGRESS, param1: baseName);
//		}
//	}
//	
//	//******************************//
//	//PUBLIC OVERRIDE MEMBER METHODS//
//	//******************************//
//	
//	//------------------------------------------------------------------------------------------------
//	override string GetIconSuffix()
//	{
//		if (m_TargetBase && m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
//			return "_Relay";
//		
//		return "";
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void DoNotifyAssignment(int assigneeID)
//	{
//		if (m_TargetBase.GetType() != SCR_ECampaignBaseType.BASE)
//			return;
//
//		SCR_CampaignNetworkComponent assigneeNetworkComponent = SCR_CampaignNetworkComponent.GetCampaignNetworkComponent(assigneeID);
//		if (!assigneeNetworkComponent)
//			return;
//		
//		assigneeNetworkComponent.SendPlayerMessage(GetAssignMessage(), m_TargetBase.GetCallsign(), public: false);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Return the title of this task.
//	override string GetTitle()
//	{
//		if (m_TargetBase && m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
//			return m_sNameReconfigure;
//		
//		return m_sName;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetIsPriority(bool isPrio)
//	{
//		super.SetIsPriority(isPrio);
//		
//		if (isPrio)
//			m_sAffix = "<br/><br/>#AR-Campaign_PriorityTaskInfo";
//		else
//			m_sAffix = string.Empty;
//		
//		RegisterTaskUpdate(SCR_ETaskEventMask.TASK_PROPERTY_CHANGED);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! Return the description of this task.
//	override string GetDescription()
//	{
//		if (m_TargetBase && m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
//			return m_sDescriptionReconfigure + m_sAffix;
//		
//		return m_sDescription + m_sAffix;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Finish(bool showMsg = true)
//	{
//		SCR_MilitaryBaseSystem system = SCR_MilitaryBaseSystem.GetInstance();
//		
//		if (system)
//			system.GetOnBaseFactionChanged().Remove(OnTargetBaseCaptured);
//
//		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
//		super.Finish(showMsg);
//		
//		SCR_XPHandlerComponent comp = SCR_XPHandlerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_XPHandlerComponent));
//		
//		// Reward XP for seizing a base or reconfiguring a relay
//		if (comp && !GetTaskManager().IsProxy() && GetType() == SCR_CampaignTaskType.CAPTURE)
//		{
//			PlayerManager playerManager = GetGame().GetPlayerManager();
//			array<int> players = {};
//			playerManager.GetPlayers(players);
//			array<SCR_BaseTaskExecutor> assignees = {};
//			GetAssignees(assignees);
//			vector baseOrigin = m_TargetBase.GetOwner().GetOrigin();
//			int radius = m_TargetBase.GetRadius();
//			int radiusSq;
//			Faction playerFaction;
//			IEntity playerEntity;
//			bool isAssignee;
//			int assigneeID;
//			SCR_EXPRewards rewardID;
//			
//			if (m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
//				rewardID = SCR_EXPRewards.RELAY_RECONFIGURED;
//			else
//				rewardID = SCR_EXPRewards.BASE_SEIZED;
//			
//			if (m_TargetBase.GetType() == SCR_ECampaignBaseType.RELAY)
//				radiusSq = 50 * 50;
//			else
//				radiusSq = radius * radius;
//			
//			foreach (int playerId : players)
//			{
//				playerEntity = playerManager.GetPlayerControlledEntity(playerId);
//				
//				if (!playerEntity)
//					continue;
//				
//				playerFaction = SCR_CampaignReconfigureRelayUserAction.GetPlayerFaction(playerEntity);
//				
//				if (playerFaction != m_TargetFaction)
//					continue;
//				
//				if (vector.DistanceSq(playerEntity.GetOrigin(), baseOrigin) < radiusSq)
//				{
//					isAssignee = false;
//					
//					foreach (SCR_BaseTaskExecutor assignee : assignees)
//					{
//						assigneeID = SCR_BaseTaskExecutor.GetTaskExecutorID(assignee);
//						
//						if (assigneeID == playerId)
//						{
//							isAssignee = true;
//							break;
//						}
//					}
//					
//					float multiplier = 1.0;
//					
//					if (m_bIsPriority)
//						multiplier = 1.5;
//
//					comp.AwardXP(playerId, rewardID, multiplier, isAssignee);
//				}
//			}
//		}
//		
//		string baseName;
//		
//		if (m_TargetBase)
//			baseName = GetBaseNameWithCallsign();
//		
//		if (showMsg)
//		{
//			// TODO make this nicer
//			if (m_bIndividualTask)
//			{
//				if (IsAssignedToLocalPlayer())
//				{
//					if (DoneByAssignee())
//					{
//						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
//					}
//					else
//					{
//						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_FAILED);
//					}
//				}
//				else
//				{
//					if (m_aAssignees && m_aAssignees.Count() > 0)
//					{
//						string text;
//						string par1;
//						string par2;
//						string par3;
//						string par4;
//						string par5;
//						GetFinishTextData(text, par1, par2, par3, par4, par5);
//						SCR_PopUpNotification.GetInstance().PopupMsg(text, prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: par1, param2: par2, param3: par3, param4: par4, sound: SCR_SoundEvent.TASK_SUCCEED);
//					}
//					else
//						SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
//				}
//			}
//			else
//				SCR_PopUpNotification.GetInstance().PopupMsg(TASK_COMPLETED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_SUCCEED, text2: SCR_BaseTask.TASK_HINT_TEXT, text2param1: SCR_PopUpNotification.TASKS_KEY_IMAGE_FORMAT);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Fails the task.
//	override void Fail(bool showMsg = true)
//	{
//		showMsg = SCR_FactionManager.SGetLocalPlayerFaction() == m_TargetFaction;
//		super.Fail(showMsg);
//		
//		/*SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//		if (campaign && showMsg)
//		{
//			string baseName;
//		
//			if (m_TargetBase)
//				baseName = GetBaseNameWithCallsign();
//			
//			SCR_PopUpNotification.GetInstance().PopupMsg(TASK_FAILED_TEXT + " " + GetTitle(), prio: SCR_ECampaignPopupPriority.TASK_DONE, param1: baseName, sound: SCR_SoundEvent.TASK_FAILED);
//		}*/
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns a text informing about the task being finished
//	override string GetFinishText()
//	{
//		// TODO Add other types of tasks here once needed...
//		string text = "";
//		
//		switch (m_eType)
//		{
//			case SCR_CampaignTaskType.CAPTURE:
//			{
//				if (m_TargetBase)
//				{
//					//if (m_bIndividualTask && DoneByAssignee())
//					if (false)
//						text = string.Format(CAMPAIGN_TASK_RECONFIGURED_BY_TEXT, GetBaseNameWithCallsign(), GetAllAssigneeNamesString());
//					else
//						text = string.Format(CAMPAIGN_TASK_RECONFIGURED_TEXT, GetBaseNameWithCallsign());
//				}
//				break;
//			};
//		}
//			
//		return text;
//	}
//
//	//------------------------------------------------------------------------------------------------
//	//! Returns text data informing about the task being finished
//	void GetFinishTextData(out string text = "", out string param1 = "", out string param2 = "", out string param3 = "", out string param4 = "", out string param5 = "")
//	{
//		switch (m_eType)
//		{
//			case SCR_CampaignTaskType.CAPTURE:
//			{
//				if (m_TargetBase)
//				{
//					//if (m_bIndividualTask && DoneByAssignee())
//					if (false)
//					{
//						text = CAMPAIGN_TASK_RECONFIGURED_BY_TEXT;
//						param1 = GetBaseNameWithCallsign();
//						param2 = GetAllAssigneeNamesString();
//					}
//					else
//					{
//						text = CAMPAIGN_TASK_RECONFIGURED_TEXT;
//						param1 = GetBaseNameWithCallsign();
//					}
//				}
//
//				break;
//			};
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override string GetMapDescriptorText()
//	{
//		return GetTaskListTaskText();
//	}
//	
//	//*********************//
//	//PUBLIC MEMBER METHODS//
//	//*********************//
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetTitleWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		string baseName;
//
//		if (m_TargetBase)
//		{
//			baseName = GetBaseNameWithCallsign();
//			textWidget.SetTextFormat(taskText, baseName);
//		}
//		else
//		{
//			// m_TargetBase has not been set in time, delay another call
//			GetGame().GetCallqueue().CallLater(SetTitleWidgetText, SCR_GameModeCampaign.UI_UPDATE_DELAY, false, textWidget, taskText);
//		}
//	}
//
//	//------------------------------------------------------------------------------------------------
//	override string GetTitleText()
//	{
//		string baseName;
//
//		if (m_TargetBase)
//			baseName = GetBaseNameWithCallsign();
//
//		return string.Format("%1 %2", m_sName, baseName);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetDescriptionWidgetText(notnull TextWidget textWidget, string taskText)
//	{
//		string baseName;
//		
//		if (m_TargetBase)
//			baseName = GetBaseNameWithCallsign();
//		
//		textWidget.SetTextFormat(taskText, baseName);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Returns the type of this task.
//	SCR_CampaignTaskType GetType()
//	{
//		return m_eType;
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	//! Sets the type of this task.
//	void SetType(SCR_CampaignTaskType type)
//	{
//		m_eType = type;
//	}
//	
//	//***************************//
//	//PUBLIC MEMBER EVENT METHODS//
//	//***************************//
//	
//	//------------------------------------------------------------------------------------------------
//	override void Deserialize(ScriptBitReader reader)
//	{
//		super.Deserialize(reader);
//		
//		SCR_CampaignTaskType type;
//		reader.ReadIntRange(type, 0, SCR_CampaignTaskType.LAST-1);
//		SetType(type);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void Serialize(ScriptBitWriter writer)
//	{
//		super.Serialize(writer);
//		
//		writer.WriteIntRange(GetType(), 0, SCR_CampaignTaskType.LAST-1);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	override void SetTargetBase(notnull SCR_CampaignMilitaryBaseComponent targetBase)
//	{
//		super.SetTargetBase(targetBase);
//		SCR_MilitaryBaseSystem.GetInstance().GetOnBaseFactionChanged().Insert(OnTargetBaseCaptured);
//	}
//
//	//------------------------------------------------------------------------------------------------
//	void OnTargetBaseCaptured(SCR_MilitaryBaseComponent base, Faction faction)
//	{
//		if (base != m_TargetBase || !m_TargetFaction || m_eType != SCR_CampaignTaskType.CAPTURE || faction != m_TargetFaction)
//			return;
//		
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		
//		if (!supportEntity)
//			return;
//		
//		supportEntity.FinishTask(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void OnCampaignBaseSignalChanged(SCR_CampaignMilitaryBaseComponent base)
//	{
//		if (!m_TargetBase || !base || !m_TargetFaction || m_eType != SCR_CampaignTaskType.CAPTURE || base != m_TargetBase)
//			return;
//		
//		SCR_CampaignFaction castFaction = SCR_CampaignFaction.Cast(m_TargetFaction);
//		
//		if (!castFaction)
//			return;
//		
//		if (!GetTaskManager())
//			return;
//		
//		SCR_BaseTaskSupportEntity supportEntity = GetTaskManager().FindSupportEntity(SCR_BaseTaskSupportEntity);
//		
//		if (!supportEntity)
//			return;
//		
//		if (!m_TargetBase.IsHQRadioTrafficPossible(castFaction))
//			supportEntity.FailTask(this);
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void SCR_CampaignTask(IEntitySource src, IEntity parent)
//	{
//		if (SCR_Global.IsEditMode(this))
//			return;
//		
//		if (!GetTaskManager().IsProxy())
//		{
//			SCR_GameModeCampaign.GetInstance().GetBaseManager().GetOnSignalChanged().Insert(OnCampaignBaseSignalChanged);
//		}
//	}
//	
//	//------------------------------------------------------------------------------------------------
//	void ~SCR_CampaignTask()
//	{
//		if (SCR_Global.IsEditMode(this) || !GetGame().GetGameMode())
//			return;
//		
//		if (!GetTaskManager().IsProxy())
//		{
//			if (m_TargetBase)
//			{
//				SCR_MilitaryBaseSystem system = SCR_MilitaryBaseSystem.GetInstance();
//		
//				if (system)
//					system.GetOnBaseFactionChanged().Remove(OnTargetBaseCaptured);
//			}
//			
//			SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
//			
//			if (campaign)
//				campaign.GetBaseManager().GetOnSignalChanged().Remove(OnCampaignBaseSignalChanged);
//		}
//	}
//};
