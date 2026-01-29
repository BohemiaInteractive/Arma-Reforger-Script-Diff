[BaseContainerProps(configRoot: true)]
class SCR_TaskNotificationsRootConfig
{
	[Attribute(desc:"Root config data list with notification settings")]
	ref array<ref SCR_TaskNotificationConfig> m_aTaskNotificationList;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotification, "m_eTaskNotification", "Notification: %1")]
class SCR_TaskNotificationConfig
{
	[Attribute(desc:"Task notification type", uiwidget: UIWidgets.ComboBox, enumType: SCR_ETaskNotification)]
	SCR_ETaskNotification m_eTaskNotification;

	[Attribute(desc:"Task name", uiwidget: UIWidgets.LocaleEditBox)]
	LocalizedString m_sTaskName;

	[Attribute(desc:"If checked, notifications will be shown only after the interval; before the interval, they will be ignored.")]
	bool m_bAllowedNotificationInterval;

	[Attribute(desc:"Notification interval", params:"0 inf")]
	float m_fNotificationInterval;

	[Attribute(desc:"Data list with notification settings")]
	ref array<ref SCR_BaseTaskNotificationData> m_aTaskNotificationDataList;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotificationMsg, "m_eTaskNotificationMsg")]
class SCR_BaseTaskNotificationData
{
	[Attribute(desc:"Task notification msg type", uiwidget: UIWidgets.ComboBox, enumType: SCR_ETaskNotificationMsg)]
	SCR_ETaskNotificationMsg m_eTaskNotificationMsg;

	[Attribute("-1", desc:"Allowed roles for which notifications are sent", uiwidget: UIWidgets.Flags, enumType: SCR_ETaskNotificationAllowedRole)]
	protected SCR_ETaskNotificationAllowedRole m_eAllowedCharacterRole;

	[Attribute(desc: "Skip sending message to the commander frequency")]
	protected bool m_bSkipCommandFrequency;

	[Attribute(desc: "If checked, notifications within the defined distance from the task will be send, the rest will be ignored.")]
	protected bool m_bAllowedDistance;

	[Attribute(desc: "Distance[m] from task", params: "0 inf")]
	protected float m_fDistance;

	//------------------------------------------------------------------------------------------------
	//!	Checks if to send a message by group role
	//! \param[in] receiverGroup
	//! \return true if the notification can be sent
	bool CanSendNotification(notnull SCR_AIGroup receiverGroup)
	{
		if (m_bSkipCommandFrequency && receiverGroup.GetGroupRole() == SCR_EGroupRole.COMMANDER)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks whether the message can be sent to the player based on the character role and distance from the task
	//! \param[in] faction
	//! \param[in] playerId
	//! \param[in] taskPosition
	//! \return true if the notification can be sent to specific players
	bool CanSendToPlayer(notnull SCR_Faction faction, int playerId, vector taskPosition)
	{
		if (faction.IsPlayerCommander(playerId))
		{
			if (!SCR_Enum.HasFlag(m_eAllowedCharacterRole, SCR_ETaskNotificationAllowedRole.COMMANDER))
				return false;
		}
		else
		{
			SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(playerId);
			if (groupController)
			{
				bool isPlayerSquadLeader = groupController.IsPlayerLeaderOwnGroup();
				if (isPlayerSquadLeader && !SCR_Enum.HasFlag(m_eAllowedCharacterRole, SCR_ETaskNotificationAllowedRole.SQUAD_LEADER))
					return false;

				if (!isPlayerSquadLeader && !SCR_Enum.HasFlag(m_eAllowedCharacterRole, SCR_ETaskNotificationAllowedRole.SQUAD_MEMBER))
					return false;
			}
		}

		if (m_bAllowedDistance)
		{
			// distance check - player and task
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			if (!playerController)
				return false;

			IEntity player = playerController.GetMainEntity();
			if (!player || vector.DistanceSqXZ(player.GetOrigin(), taskPosition) > m_fDistance * m_fDistance)
				return false;
		}

		return true;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotificationMsg, "m_eTaskNotificationMsg")]
class SCR_MajorTaskNotificationData : SCR_BaseTaskNotificationData
{
	[Attribute(desc:"Notification text", uiwidget: UIWidgets.LocaleEditBox)]
	LocalizedString m_sText;

	[Attribute(desc:"Sound event name for voiceover or other sound")]
	string m_sSoundEventName;

	//------------------------------------------------------------------------------------------------
	//! \return notification text for newly created task
	string GetCreationText()
	{
		return m_sText;
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotificationMsg, "m_eTaskNotificationMsg")]
class SCR_MajorBaseTaskNotificationData : SCR_MajorTaskNotificationData
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] faction
	//! \param[in] base
	//! \return notification text for newly created task
	string GetCreationText(notnull Faction faction, notnull SCR_CampaignMilitaryBaseComponent base)
	{
		return WidgetManager.Translate(m_sText, base.GetFormattedBaseNameWithCallsign(faction));
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotificationMsg, "m_eTaskNotificationMsg")]
class SCR_MajorRequestedTaskNotificationData : SCR_MajorTaskNotificationData
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] groupId
	//! \return notification text for newly created task
	string GetCreationText(int groupId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return string.Empty;

		SCR_AIGroup aiGroup = groupsManager.FindGroup(groupId);
		if (!aiGroup)
			return string.Empty;

		return WidgetManager.Translate(m_sText, SCR_GroupHelperUI.GetTranslatedGroupName(aiGroup));
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotificationMsg, "m_eTaskNotificationMsg")]
class SCR_MinorTaskNotificationData : SCR_BaseTaskNotificationData
{
	[Attribute(ENotification.GROUP_TASK_GROUP_ASSIGNED_TO_SAME_TASK.ToString(), desc:"Minor notification type", uiwidget: UIWidgets.ComboBox, enumType: ENotification)]
	ENotification m_eNotification;

	[Attribute(desc:"Text required by some minor notifications", uiwidget: UIWidgets.LocaleEditBox)]
	LocalizedString m_sText;
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(SCR_ETaskNotificationMsg, "m_eTaskNotificationMsg")]
class SCR_MinorRequestedTaskNotificationData : SCR_MinorTaskNotificationData
{
	//------------------------------------------------------------------------------------------------
	//! \param[in] groupId
	//! \return notification text for newly created task
	string GetCreationText(int groupId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return string.Empty;

		SCR_AIGroup aiGroup = groupsManager.FindGroup(groupId);
		if (!aiGroup)
			return string.Empty;

		return WidgetManager.Translate(m_sText, SCR_GroupHelperUI.GetTranslatedGroupName(aiGroup));
	}
}
