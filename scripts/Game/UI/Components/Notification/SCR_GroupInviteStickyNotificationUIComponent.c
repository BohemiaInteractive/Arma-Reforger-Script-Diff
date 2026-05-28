class SCR_GroupInviteStickyNotificationUIComponent : SCR_StickyNotificationUIComponent
{
	[Attribute("#AR-Notification_GROUPS_PLAYER_INVITED_GROUP_NAME")]
	protected LocalizedString m_sGroupInviteText;
	
	SCR_GroupsManagerComponent m_GroupManager;
	SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	
	//How long the group invite notification exists before it gets deleted, Time in seconds
	static const int NOTIFICATION_DELETE_TIME = 20;

	protected static const float PLATFORM_ICON_SIZE = 2;

	protected ref array<int> m_aGroupIdQueue = {};
	protected int m_iGroupId = -1;
	
	//------------------------------------------------------------------------------------------------
	//Shows the sticky notification including player who invited you and the group callsign
	protected void ShowGroupInviteNotification(int groupId, bool isInit = false)
	{		
		if (m_iGroupId > -1 && m_iGroupId != groupId)
		{
			//Add the invite to the queue if another invite is currently being displayed
			if (!m_aGroupIdQueue.Contains(groupId))
				m_aGroupIdQueue.Insert(groupId);

			return;
		}

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
			if (GetGame().GetPlayerManager().GetPlatformKind(m_PlayerControllerGroupComponent.GetPlayerIdFromGroupInvite(groupId)) == PlatformKind.PSN)
				playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_PLAYSTATION_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
			else
				playerName = string.Format("<color rgba=%1><image set='%2' name='%3' scale='%4'/></color>", UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, UIConstants.PLATFROM_GENERIC_ICON_NAME, PLATFORM_ICON_SIZE) + playerName;
		}
		
		string squadName = WidgetManager.Translate(format, company, platoon, squad);
		m_Text.SetTextFormat(m_sGroupInviteText, company, platoon, squad, squadName, playerName);
		
		SetStickyActive(true, !isInit);

		//Hiding the notification after a certain amount of time has passed
		GetGame().GetCallqueue().Remove(RemoveInvite);
		GetGame().GetCallqueue().CallLater(RemoveInvite, NOTIFICATION_DELETE_TIME * 1000, false, groupId);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInviteReceived(int groupId, int fromPlayerId)
	{
		ShowGroupInviteNotification(groupId);
		GetGame().OnUserSettingsChangedInvoker().Insert(OnUserSettingsChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveInvite(int groupId)
	{
		//Remove the invite from the queue
		m_aGroupIdQueue.RemoveItem(groupId);

		// If the invite to be removed is the notification currently being displayed, deactivate it and show the next notification in queue (if valid)
		if (m_iGroupId == groupId)
		{
			GetGame().GetCallqueue().Remove(RemoveInvite);
			SetStickyActive(false);
			m_FadeUIComponent.GetOnFadeDone().Insert(ShowNextInviteInQueue);
		}

		GetGame().OnUserSettingsChangedInvoker().Remove(OnUserSettingsChanged);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ShowNextInviteInQueue()
	{
		m_FadeUIComponent.GetOnFadeDone().Remove(ShowNextInviteInQueue);

		m_iGroupId = -1;

		//If there are invites in the queue, show the next one
		if (!m_aGroupIdQueue.IsEmpty())
			ShowGroupInviteNotification(m_aGroupIdQueue.Get(0));
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
		m_PlayerControllerGroupComponent.GetOnInviteAccepted().Insert(RemoveInvite);
		m_PlayerControllerGroupComponent.GetOnInviteCancelled().Insert(RemoveInvite);

		bool shouldShowInvite = m_PlayerControllerGroupComponent.HasGroupInvites();
		
		if (shouldShowInvite)
			ShowGroupInviteNotification(m_PlayerControllerGroupComponent.GetGroupInviteIdFromIndex(0), true);
		
		SetVisible(shouldShowInvite);
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
		m_PlayerControllerGroupComponent.GetOnInviteAccepted().Remove(RemoveInvite);
		m_PlayerControllerGroupComponent.GetOnInviteCancelled().Remove(RemoveInvite);
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
