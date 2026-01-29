class SCR_GroupInviteStickyNotificationUIComponent : SCR_StickyNotificationUIComponent
{
	[Attribute("#AR-Notification_GROUPS_PLAYER_INVITED_GROUP_NAME")]
	protected LocalizedString m_sGroupInviteText;
	
	SCR_GroupsManagerComponent m_GroupManager;
	SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	
	protected static const float PLATFORM_ICON_SIZE = 2;

	protected int m_iGroupId = -1;
	
	//------------------------------------------------------------------------------------------------
	//Shows the sticky notification including player who invited you and the group callsign
	protected void ShowGroupInviteNotification(int groupId, bool isInit = false)
	{		
		m_iGroupId = groupId;
		SCR_AIGroup playerGroup = m_GroupManager.FindGroup(groupId);
		if (!playerGroup)
		{
			Print(string.Format("SCR_GroupInviteStickyNotificationUIComponent could not find group with ID %1!", groupId.ToString()), LogLevel.ERROR);
			return;
		}
		
		string company, platoon, squad, character, format;
		playerGroup.GetCallsigns(company, platoon, squad, character, format);
		
		//string invitingPlayerName = m_PlayerControllerGroupComponent.GetGroupInviteFromPlayerName();
		
		string playerName = m_PlayerControllerGroupComponent.GetGroupInviteFromPlayerName();
		if (GetGame().GetPlatformService().GetLocalPlatformKind() == PlatformKind.PSN)
		{
			if (GetGame().GetPlayerManager().GetPlatformKind(m_PlayerControllerGroupComponent.GetGroupInviteFromPlayerID()) == PlatformKind.PSN)
				playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
			else
				playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
		}
		
		string squadName = WidgetManager.Translate(format, company, platoon, squad);
		m_Text.SetTextFormat(m_sGroupInviteText, company, platoon, squad, squadName, playerName);
		
		SetStickyActive(true, !isInit);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInviteReceived(int groupId, int fromPlayerId)
	{
		ShowGroupInviteNotification(groupId);
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInviteAcceptedOrCancelled(int groupId)
	{
		m_iGroupId = -1;
		SetStickyActive(false);
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{	
		super.OnInit(notificationLog);
				
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		
		if (!m_GroupManager)
		{
			//Print("SCR_GroupInviteStickyNotificationUIComponent could not find SCR_GroupsManagerComponent!", LogLevel.ERROR);
			return;
		}
		
		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerControllerGroupComponent)
		{
			Print("SCR_GroupInviteStickyNotificationUIComponent could not find SCR_PlayerControllerGroupComponent!", LogLevel.ERROR);
			return;
		}
		
		m_PlayerControllerGroupComponent.GetOnInviteReceived().Insert(OnInviteReceived);
		m_PlayerControllerGroupComponent.GetOnInviteAccepted().Insert(OnInviteAcceptedOrCancelled);
		m_PlayerControllerGroupComponent.GetOnInviteCancelled().Insert(OnInviteAcceptedOrCancelled);
		
		int groupInviteID = m_PlayerControllerGroupComponent.GetGroupInviteID();
		
		if (groupInviteID > -1)
			ShowGroupInviteNotification(groupInviteID, true);
		
		SetVisible(groupInviteID > -1);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnButton()
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (menuManager)
			menuManager.OpenDialog(ChimeraMenuPreset.PlayerListMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnDestroy()
	{
		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
		if (!m_PlayerControllerGroupComponent)
			return;
		
		m_PlayerControllerGroupComponent.GetOnInviteReceived().Remove(OnInviteReceived);
		m_PlayerControllerGroupComponent.GetOnInviteAccepted().Remove(OnInviteAcceptedOrCancelled);
		m_PlayerControllerGroupComponent.GetOnInviteCancelled().Remove(OnInviteAcceptedOrCancelled);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback used to reload the text of the notification in case that language changes
	protected void OnUserSettingsChanged()
	{
		if (m_iGroupId < 0)
			return;

		ShowGroupInviteNotification(m_iGroupId);
	}
}
