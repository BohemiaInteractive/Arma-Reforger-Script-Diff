class SCR_PlayerListButtonNotificationUIComponent: ScriptedWidgetComponent
{
	[Attribute("", desc: "When more then one notification is active, set this icon")]
	protected ResourceName m_sMultipleNotifications;
	
	[Attribute("", desc: "When the player has a group invite, set this icon")]
	protected ResourceName m_sGroupInvite;
	
	[Attribute("", desc: "When there is no Game Master and one can be voted, set this icon")]
	protected ResourceName m_sNoGameMaster;
	
	[Attribute("", desc: "When there is an active vote going on, set this icon")]
	protected ResourceName m_sVoteActive;
	
	[Attribute("AvailibleActionIcon")]
	protected string m_sNotificationIconName;
	
	[Attribute("2")]
	protected float m_RefreshRateInSeconds;
	
	protected ImageWidget m_NotificationIcon;
	
	protected bool m_bHasGroupInvite;
	protected bool m_bNoGameMaster;
	protected bool m_bVoteActive;
	
	protected int m_NotificationTypesActive = 0;
	
	protected SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	protected SCR_PlayerDelegateEditorComponent m_DelegateManager;
	protected SCR_VotingManagerComponent m_VotingManagerComponent;
	
	
	protected void CheckIfShowIcon()
	{
		if (m_NotificationTypesActive == 1)
		{
			if (m_bHasGroupInvite)
				m_NotificationIcon.LoadImageTexture(0, m_sGroupInvite);
			else if (m_bNoGameMaster)
				m_NotificationIcon.LoadImageTexture(0, m_sNoGameMaster);
			else if (m_bVoteActive)
				m_NotificationIcon.LoadImageTexture(0, m_sVoteActive);
			//Safty
			else 
				m_NotificationIcon.LoadImageTexture(0, m_sMultipleNotifications);
		}
		else if (m_NotificationTypesActive > 1)
		{
			m_NotificationIcon.LoadImageTexture(0, m_sMultipleNotifications);
		}
		
		m_NotificationIcon.SetVisible(m_NotificationTypesActive != 0);
	}
	
	protected void RefreshNotificationIcon()
	{
		m_NotificationTypesActive = 0;
		m_bHasGroupInvite = false;
		m_bNoGameMaster = false;
		m_bVoteActive = false;
		
		if (m_PlayerControllerGroupComponent)
		{
			if (m_PlayerControllerGroupComponent.GetGroupInviteID() > -1)
			{
				m_NotificationTypesActive++;
				m_bHasGroupInvite = true;
			}
		}
		
		SCR_GameModeEditor gameMode = SCR_GameModeEditor.Cast(GetGame().GetGameMode());
		if (gameMode && gameMode.GetGameMasterTarget() == EGameModeEditorTarget.VOTE && m_DelegateManager && !m_DelegateManager.HasPlayerWithUnlimitedEditor())
		{
			m_NotificationTypesActive++;
			m_bNoGameMaster = true;
		}
		
		if (m_VotingManagerComponent)
		{
			//Set visible at start
			array<EVotingType> validActiveVotingTypes = new array<EVotingType>;
			int activeVotingCount = m_VotingManagerComponent.GetAllVotingsAboutPlayer(true, validActiveVotingTypes);
			
			if (activeVotingCount > 0)
			{
				m_NotificationTypesActive++;
				m_bVoteActive = true;
			}
		}
		
		CheckIfShowIcon();
	}
	
	override void HandlerAttached(Widget w)
	{
		m_NotificationIcon = ImageWidget.Cast(w.FindAnyWidget(m_sNotificationIconName));
		
		if (!m_NotificationIcon)
		{
			Print("(SCR_PlayerListButtonNotificationUIComponent) Player list pause menu button could not find Notification Icon", LogLevel.ERROR);
			return;
		}
		
		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		m_DelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
		m_VotingManagerComponent = SCR_VotingManagerComponent.GetInstance();
		
		RefreshNotificationIcon();
		GetGame().GetCallqueue().CallLater(RefreshNotificationIcon, m_RefreshRateInSeconds * 1000, true);
	}
	
	override void HandlerDeattached(Widget w)
	{
		if (!m_NotificationIcon)
			return;
		
		GetGame().GetCallqueue().Remove(RefreshNotificationIcon);
	}
};


