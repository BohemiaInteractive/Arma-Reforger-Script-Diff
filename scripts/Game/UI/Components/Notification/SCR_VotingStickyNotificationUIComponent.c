class SCR_VotingStickyNotificationUIComponent : SCR_StickyNotificationUIComponent
{
	[Attribute("#AR-Notification_Sticky_MultiplePlayerVotes", desc: "When there is more then 1 vote in progress", uiwidget: UIWidgets.LocaleEditBox)]
	protected LocalizedString m_sMultipleVotesText;
	
	[Attribute("VotingInstantVote")]
	protected string m_sInstantVoteLayoutName;
	
	[Attribute("Time")]
	protected string m_sRemainingVotingTimeName;
	
	[Attribute("VotingInstantVoteButtons")]
	protected string m_sVotingInstantVoteButtonsName;

	[Attribute("#AR-Voting_PlayerCountFormatting")]
	protected string m_sCurrentVoteCountFormatting;
	
	protected Widget m_InstantVoteLayout;
	protected TextWidget m_RemainingVotingTime;
	protected Widget m_VotingInstantVoteButtons;
	
	protected SCR_VotingManagerComponent m_VotingManagerComponent;
	
	//~ If one vote is set then the duration left is shown
	protected EVotingType m_eActiveSingularVoteType;
	protected int m_eActiveSingularVoteValue;

	//~ If there is a voting about local player that should display the vote timer
	protected EVotingType m_eActiveVotingAboutLocalPlayerVoteType;
	protected int m_iActiveVotingAboutLocalPlayerVoteValue;
	protected bool m_bIsDisplayingLocalVoteTimer;
	
	protected bool m_bIsUpdatingTime;
	protected bool m_bIsUpdatingActionContext;
	
	protected static const float PLATFORM_ICON_SIZE = 2;
	
	//------------------------------------------------------------------------------------------------
	protected void OnVotingChanged(bool isInit = false)
	{		
		array<EVotingType> validActiveVotingTypes = {};
		array<int> votingValues = {};
		int count = m_VotingManagerComponent.GetAllVotingsWithValue(validActiveVotingTypes, votingValues, false, true);
		m_bIsDisplayingLocalVoteTimer = false;
		
		int value;
		EVotingType voteType;
		for (int i = count - 1; i >= 0; i--)
		{
			voteType = validActiveVotingTypes[i];
			value = votingValues[i];

			//~ Ignore any abstained, unavailible or voted votes
			if (!m_VotingManagerComponent.IsVotingAvailable(voteType, value) || m_VotingManagerComponent.HasAbstainedLocally(voteType, value) || m_VotingManagerComponent.IsLocalVote(voteType, value))
			{
				if (m_VotingManagerComponent.GetVoteAlwaysDisplayVoteSubjectVotingTimer(voteType, value) && value == SCR_PlayerController.GetLocalPlayerId())
				{
					m_bIsDisplayingLocalVoteTimer = true;
					m_eActiveVotingAboutLocalPlayerVoteType = voteType;
					m_iActiveVotingAboutLocalPlayerVoteValue = value;
					continue;
				}

				validActiveVotingTypes.RemoveOrdered(i);
				votingValues.RemoveOrdered(i);
			}	
		}
		
		int votingCount = validActiveVotingTypes.Count();
		
		ScriptInvokerMenu menuInvoker = SCR_MenuHelper.GetOnMenuClose();
		//If no votes active
		if (votingCount <= 0)
		{
			//Hide notification
			SetStickyActive(false, !isInit);
			
			EnableVotingTimerUI(false);
			EnableContextAction(false);
			menuInvoker.Remove(OnSettingsMenuClosed);
			return;
		}
		//Multiple votes active
		else if (votingCount > 1)
		{
			m_Text.SetTextFormat(m_sMultipleVotesText, votingCount.ToString());
			
			if (m_OptionalMessageLayout)
				m_OptionalMessageLayout.SetVisible(true);

			if (m_InstantVoteLayout)
				m_InstantVoteLayout.SetVisible(m_bIsDisplayingLocalVoteTimer);

			if (m_VotingInstantVoteButtons)
				m_VotingInstantVoteButtons.SetVisible(false);

			if (m_RemainingVotingTime)
				m_RemainingVotingTime.SetVisible(m_bIsDisplayingLocalVoteTimer);

			EnableVotingTimerUI(m_bIsDisplayingLocalVoteTimer);
			EnableContextAction(false);
			menuInvoker.Remove(OnSettingsMenuClosed);
		}
		//One vote active
		else
		{
			m_eActiveSingularVoteType = validActiveVotingTypes[0];
			m_eActiveSingularVoteValue = votingValues[0];
			
			if (m_eActiveSingularVoteValue != SCR_VotingBase.DEFAULT_VALUE)
			{
				string playerName = GetPlayerName(m_eActiveSingularVoteValue);
				string voteAuthorName = GetPlayerName(m_VotingManagerComponent.GetVoteAuthorId(m_eActiveSingularVoteType, m_eActiveSingularVoteValue));

				SCR_VotingUIInfo voteUIInfo = m_VotingManagerComponent.GetVotingInfo(m_eActiveSingularVoteType);
				if (!voteAuthorName.IsEmpty())
					voteAuthorName = WidgetManager.Translate(voteUIInfo.GetAuthorText(), voteAuthorName);
				
				string text = WidgetManager.Translate(voteUIInfo.GetStickyNotificationText(), playerName, voteAuthorName);
				
				int currentVotes, VotesRequired;
				if (m_VotingManagerComponent.GetVoteCounts(m_eActiveSingularVoteType, m_eActiveSingularVoteValue, currentVotes, VotesRequired))
					m_Text.SetTextFormat(m_sCurrentVoteCountFormatting, text, currentVotes, VotesRequired);
				else 
					m_Text.SetTextFormat(text);
			}

			if (m_InstantVoteLayout)
				m_InstantVoteLayout.SetVisible(true);

			if (m_OptionalMessageLayout)
				m_OptionalMessageLayout.SetVisible(false);

			if (m_VotingInstantVoteButtons)
				m_VotingInstantVoteButtons.SetVisible(!m_bIsDisplayingLocalVoteTimer);

			if (m_RemainingVotingTime)
				m_InstantVoteLayout.SetVisible(true);
			
			EnableVotingTimerUI(true);
			EnableContextAction(!m_bIsDisplayingLocalVoteTimer);

			menuInvoker.Insert(OnSettingsMenuClosed);
		}		
		
		//Show Notification
		SetStickyActive(true, !isInit);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method used to force refresh notification content (f.e. after player changed the language)
	//! \param[in] menu which was just closed
	protected void OnSettingsMenuClosed(ChimeraMenuBase menu)
	{
		if (SCR_SettingsSuperMenu.Cast(menu))
			OnVotingChanged();
	}

	//------------------------------------------------------------------------------------------------
	//! Finds filtered player display name and adds platform icon if such is desired
	//! \param[in] playerId
	//! \return player name with optional rich text format that contains a platform icon image
	protected string GetPlayerName(int playerId)
	{
		string playerName = SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerId);
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(playerName))
			return string.Empty;

		BaseContainer settings = GetGame().GetGameUserSettings().GetModule("SCR_GameplaySettings");
		bool showIconNametag;
		if (settings)
			settings.Get("m_bPlatformIconNametag", showIconNametag);

		PlatformKind localPlatform = GetGame().GetPlatformService().GetLocalPlatformKind();
		if (!showIconNametag && localPlatform != PlatformKind.PSN)
			return playerName;

		PlatformKind playerPlatform = GetGame().GetPlayerManager().GetPlatformKind(playerId);
		string iconName;
		if (localPlatform == PlatformKind.PSN)
		{
			if (playerPlatform == PlatformKind.PSN)
				iconName = UIConstants.PLATFROM_PLAYSTATION_ICON_NAME;
			else
				iconName = UIConstants.PLATFROM_GENERIC_ICON_NAME;
		}
		else
		{
			switch (playerPlatform)
			{
				case PlatformKind.STEAM:
					iconName = UIConstants.PLATFROM_PC_ICON_NAME;
				break;
				case PlatformKind.XBOX:
					iconName = UIConstants.PLATFROM_XBOX_ICON_NAME;
				break;
				case PlatformKind.PSN:
					iconName = UIConstants.PLATFROM_PLAYSTATION_ICON_NAME;
				break;
				case PlatformKind.NONE:
					iconName = UIConstants.PLATFROM_PC_ICON_NAME;
				break;
			}
		}

		return string.Format(UIConstants.NAME_WITH_PLATFORM_ICON_FORMAT, UIColors.FormatColor(GUIColors.ENABLED), UIConstants.ICONS_IMAGE_SET, iconName, PLATFORM_ICON_SIZE, playerName);
	}

	//------------------------------------------------------------------------------------------------
	//! When the UI is visible the player can use the instant vote actions (If there is only one vote active)
	//! \param[in] enable
	protected void EnableContextAction(bool enable)
	{
		if (enable)
		{
			if (m_bIsUpdatingActionContext)
				return;
			
			GetGame().GetInputManager().AddActionListener("InstantVote", EActionTrigger.DOWN, SCR_VoterComponent.InstantVote);
			GetGame().GetInputManager().AddActionListener("InstantVoteAbstain", EActionTrigger.DOWN, SCR_VoterComponent.InstantRemoveAndAbstainVote);
			GetGame().GetCallqueue().CallLater(InstantVotingContextUpdate, repeat: true);
		}
		else 
		{
			if (!m_bIsUpdatingActionContext)
				return;
			
			GetGame().GetInputManager().RemoveActionListener("InstantVote", EActionTrigger.DOWN, SCR_VoterComponent.InstantVote);
			GetGame().GetInputManager().RemoveActionListener("InstantVoteAbstain", EActionTrigger.DOWN, SCR_VoterComponent.InstantRemoveAndAbstainVote);
			GetGame().GetCallqueue().Remove(InstantVotingContextUpdate);
		}
		
		m_bIsUpdatingActionContext = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Updates each frame to keep InstantVotingContext active
	protected void InstantVotingContextUpdate()
	{
		GetGame().GetInputManager().ActivateContext("InstantVotingContext");
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enable the voting timer UI
	//! \param[in] enable
	protected void EnableVotingTimerUI(bool enable)
	{
		if (!m_RemainingVotingTime)
			return;
		
		if (enable)
		{
			if (m_bIsUpdatingTime)
				return;
			
			VotingTimeUpdate();
			m_RemainingVotingTime.SetVisible(true);
			GetGame().GetCallqueue().CallLater(VotingTimeUpdate, 1000, true);
		}
		else 
		{
			if (!m_bIsUpdatingTime)
				return;
			
			m_RemainingVotingTime.SetVisible(false);
			GetGame().GetCallqueue().Remove(VotingTimeUpdate);
		}
		
		m_bIsUpdatingTime = enable;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when only 1 vote is active or when local player is the subject of the vote and timer is displayed. Called every 1 second to update
	protected void VotingTimeUpdate()
	{
		if (m_bIsDisplayingLocalVoteTimer)
			m_RemainingVotingTime.SetText(SCR_FormatHelper.GetTimeFormatting(Math.Clamp(m_VotingManagerComponent.GetRemainingDurationOfVote(m_eActiveVotingAboutLocalPlayerVoteType, m_iActiveVotingAboutLocalPlayerVoteValue), 0, float.MAX), ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES));
		else
			m_RemainingVotingTime.SetText(SCR_FormatHelper.GetTimeFormatting(Math.Clamp(m_VotingManagerComponent.GetRemainingDurationOfVote(m_eActiveSingularVoteType, m_eActiveSingularVoteValue), 0, float.MAX), ETimeFormatParam.DAYS | ETimeFormatParam.HOURS, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS | ETimeFormatParam.MINUTES));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoteStartOrChanged(EVotingType type, int value)
	{
		//~ One frame later to be sure that all votes were correctly counted
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVoteCountChanged(EVotingType type, int value, int voteCount)
	{
		//~ One frame later to be sure that all votes were correctly counted
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnVotingEnd(EVotingType type, int value, int winner)
	{
		//~ One frame later to be sure that all votes were correctly counted
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerJoinOrLeaveServer(int playerId)
	{
		//~ No need to do anything if sticky notification is not active
		if (!m_bStickyNotificationActive)
			return;
		
		//~ Call later to make sure everything is correctly setup
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionChanged(Faction faction, int newCount)
	{
		//~ No need to do anything if sticky notification is not active
		if (!m_bStickyNotificationActive)
			return;

		//~ Call later to make sure everything is correctly setup
		GetGame().GetCallqueue().CallLater(OnVotingChanged, param1: false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(SCR_NotificationsLogComponent notificationLog)
	{					
		super.OnInit(notificationLog);
		
		m_InstantVoteLayout = m_Root.FindAnyWidget(m_sInstantVoteLayoutName);
		if (m_InstantVoteLayout)
		{
			m_RemainingVotingTime = TextWidget.Cast(m_InstantVoteLayout.FindAnyWidget(m_sRemainingVotingTimeName));	
			m_VotingInstantVoteButtons = m_InstantVoteLayout.FindAnyWidget(m_sVotingInstantVoteButtonsName);
		}

		m_VotingManagerComponent = SCR_VotingManagerComponent.GetInstance();
		if (m_VotingManagerComponent)
		{			
			m_VotingManagerComponent.GetOnVotingStart().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVotingEnd().Insert(OnVotingEnd);
			m_VotingManagerComponent.GetOnVoteLocal().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnRemoveVoteLocal().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnAbstainVoteLocal().Insert(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVoteCountChanged().Insert(OnVoteCountChanged);
			OnVotingChanged(true);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerRegistered().Insert(OnPlayerJoinOrLeaveServer);
			gameMode.GetOnPlayerDisconnected().Insert(OnPlayerJoinOrLeaveServer);
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionCountChanged().Insert(OnPlayerFactionChanged);
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
		if (m_VotingManagerComponent)
		{
			m_VotingManagerComponent.GetOnVotingStart().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVotingEnd().Remove(OnVotingEnd);
			m_VotingManagerComponent.GetOnVoteLocal().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnRemoveVoteLocal().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnAbstainVoteLocal().Remove(OnVoteStartOrChanged);
			m_VotingManagerComponent.GetOnVoteCountChanged().Remove(OnVoteCountChanged);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
		{
			gameMode.GetOnPlayerRegistered().Remove(OnPlayerJoinOrLeaveServer);
			gameMode.GetOnPlayerDisconnected().Remove(OnPlayerJoinOrLeaveServer);
		}
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
			factionManager.GetOnPlayerFactionCountChanged().Remove(OnPlayerFactionChanged);
		
		if (m_bIsUpdatingActionContext)
		{
			InputManager inputManager = GetGame().GetInputManager();
			
			if (inputManager)
			{
				inputManager.RemoveActionListener("InstantVote", EActionTrigger.DOWN, SCR_VoterComponent.InstantVote);
				inputManager.RemoveActionListener("InstantVoteAbstain", EActionTrigger.DOWN, SCR_VoterComponent.InstantRemoveAndAbstainVote);
			}
			
			GetGame().GetCallqueue().Remove(InstantVotingContextUpdate);
		}

		if (m_bIsUpdatingTime)
			GetGame().GetCallqueue().Remove(VotingTimeUpdate);
	}	
}
