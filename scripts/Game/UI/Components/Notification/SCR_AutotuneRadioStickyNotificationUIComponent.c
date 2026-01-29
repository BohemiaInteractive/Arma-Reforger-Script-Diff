class SCR_AutotuneStickyNotificationUIComponent : SCR_StickyNotificationUIComponent
{
	SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	
	SCR_EAutotuneReason m_eAutotuneReason;
	
	//------------------------------------------------------------------------------------------------
	//Shows the sticky notification
	protected void ShowAutotuneRadioNotification(bool isInit = false)
	{		
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
		string notificationText;
		switch (m_eAutotuneReason)
		{
			case SCR_EAutotuneReason.GROUP_CHANGED:
			{
				notificationText = "#AR-Notification_Radio_New_Group";
				break;
			}
			case SCR_EAutotuneReason.TASK_ASSIGNED:
			{
				notificationText = "#AR-Notification_Radio_New_Task";
				break;
			}
			case SCR_EAutotuneReason.BECAME_LEADER:
			{
				notificationText = "#AR-Notification_Radio_Squad_Leader";
				break;
			}
			case SCR_EAutotuneReason.BECAME_COMMANDER:
			{
				notificationText = "#AR-Notification_commander";
				break;
			}			
		}
		
		m_Text.SetTextFormat(notificationText);
		
		SetStickyActive(true, !isInit);
		
		GetGame().GetInputManager().AddActionListener("AutotuneRadio", EActionTrigger.DOWN, TuneRadio);
		
		//call later to hide the notification in case it is still there
		GetGame().GetCallqueue().CallLater(DisableSticky, 10000);
	}
	
	//------------------------------------------------------------------------------------------------
	void TuneRadio()
	{
		SCR_PlayerController controller = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!controller)
			return;
		
		SCR_VONController vonController = SCR_VONController.Cast(controller.FindComponent(SCR_VONController));
		if (!vonController)
			return;
		
		SCR_VONAutoTune autotune = vonController.GetAutoTune();
		if (!autotune)
			return;
		
		//if we want more control over specific channels which to change, we could go this way, but for now lets let SetRadios to handle it
		/*
		switch (m_eAutotuneReason)
		{
			case SCR_EAutotuneReason.GROUP_CHANGED:
			{
				autotune.SetRadios(controller.GetLocalMainEntity(), SCR_EVONAutoTuneFrequency.SQUAD);
				break;
			}
			case SCR_EAutotuneReason.TASK_ASSIGNED:
			{
				autotune.SetRadios(controller.GetLocalMainEntity(), SCR_EVONAutoTuneFrequency.OBJECTIVE_PLATOON);
				break;
			}
			case SCR_EAutotuneReason.BECAME_LEADER:
			{
				autotune.SetRadios(controller.GetLocalMainEntity(), SCR_EVONAutoTuneFrequency.COMMANDER);
				autotune.SetRadios(controller.GetLocalMainEntity(), SCR_EVONAutoTuneFrequency.SQUAD);
				break;
			}
			case SCR_EAutotuneReason.BECAME_COMMANDER:
			{
				//get what to tune from config
				autotune.SetRadios(controller.GetLocalMainEntity(), SCR_EVONAutoTuneFrequency.COMMANDER);
				break;
			}
			
			default:
				break;
			
		}
		*/
		
		autotune.SetRadios(controller.GetLocalMainEntity());
		
		DisableSticky();
	}
	
	//------------------------------------------------------------------------------------------------
	void DisableSticky()
	{
		m_eAutotuneReason = SCR_EAutotuneReason.UNKNOWN;
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		GetGame().GetInputManager().RemoveActionListener("AutotuneRadio", EActionTrigger.DOWN, TuneRadio);
		SetStickyActive(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnGroupChanged()
	{		
		//we dont care if the player doesnt have entity as there is no radio to tune
		IEntity controlled = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());		
		if (!controlled)
			return;
		
		m_eAutotuneReason = SCR_EAutotuneReason.GROUP_CHANGED;
		
		ShowAutotuneRadioNotification();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{	
		super.OnInit(notificationLog);
		
		return;// disabled showing auto tune notification

		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerControllerGroupComponent)
		{
			Print("SCR_GroupInviteStickyNotificationUIComponent could not find SCR_PlayerControllerGroupComponent!", LogLevel.ERROR);
			return;
		}
		
		m_PlayerControllerGroupComponent.GetOnGroupChanged().Insert(OnGroupChanged);
		SCR_Task.GetOnTaskAssigneeAdded().Insert(OnTaskAssigneeChanged);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Insert(OnGroupLeaderChanged);
		
		SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnTaskAssigneeChanged(SCR_Task task, SCR_TaskExecutor executor, int requesterID)
	{
		//we dont care if the player doesnt have entity as there is no radio to tune
		IEntity controlled = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());		
		if (!controlled)
			return;
		
		m_eAutotuneReason = SCR_EAutotuneReason.TASK_ASSIGNED;
		ShowAutotuneRadioNotification();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnGroupLeaderChanged(int groupId, int playerId)
	{
		//becoming leader notification overrides previous ones as its important
		if (m_eAutotuneReason != SCR_EAutotuneReason.UNKNOWN && m_eAutotuneReason != SCR_EAutotuneReason.BECAME_COMMANDER)
			DisableSticky();
	
		IEntity controlled = GetGame().GetPlayerManager().GetPlayerControlledEntity(SCR_PlayerController.GetLocalPlayerId());
		if (!controlled)
			return;
		
		SCR_Faction faction = SCR_Faction.Cast(SCR_Faction.GetEntityFaction(controlled));	
		if (!faction)
			return;
		
		if (faction.IsPlayerCommander(SCR_PlayerController.GetLocalPlayerId()))
			m_eAutotuneReason =	SCR_EAutotuneReason.BECAME_COMMANDER;
		else
			m_eAutotuneReason = SCR_EAutotuneReason.BECAME_LEADER;	
				
		ShowAutotuneRadioNotification();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDestroy()
	{
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		if (!m_PlayerControllerGroupComponent)
			return;
		
		m_PlayerControllerGroupComponent.GetOnGroupChanged().Remove(OnGroupChanged);
		SCR_Task.GetOnTaskAssigneeAdded().Remove(OnTaskAssigneeChanged);
		SCR_AIGroup.GetOnPlayerLeaderChanged().Remove(OnGroupLeaderChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback used to reload the text of the notification in case that language changes
	protected void OnUserSettingsChanged()
	{
		SCR_EAutotuneReason autotuneReasonTemp = m_eAutotuneReason;
		DisableSticky();
		m_eAutotuneReason = autotuneReasonTemp;
		ShowAutotuneRadioNotification();
	}
}

enum SCR_EAutotuneReason
{
	UNKNOWN,
	GROUP_CHANGED,
	BECAME_LEADER,
	TASK_ASSIGNED,
	BECAME_COMMANDER
}
