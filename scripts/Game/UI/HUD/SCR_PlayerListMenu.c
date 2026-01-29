enum EPlayerListTab
{
	ALL = 0,
	GROUPS = 1
}

class SCR_PlayerListEntry
{
	SCR_ScoreInfo m_Info;
	int m_iID;
	Widget m_wRow;
	Faction m_Faction;
	int m_iSortFrequency;

	SCR_ButtonBaseComponent m_Mute;
	SCR_ComboBoxComponent m_PlayerActionList;

	TextWidget m_wName;
	TextWidget m_wKills;
	TextWidget m_wFreq;
	TextWidget m_wDeaths;
	TextWidget m_wScore;
	ImageWidget m_wLoadoutIcon;
	ImageWidget m_wPlatformIcon;
	Widget m_wTaskIcon;
	Widget m_wFactionImage;
	Widget m_wVotingNotification;
	Widget m_wBlockedIcon;
};

//------------------------------------------------------------------------------------------------
class SCR_PlayerListMenu : SCR_SuperMenuBase
{
	protected ResourceName m_sScoreboardRow = "{65369923121A38E7}UI/layouts/Menus/PlayerList/PlayerListEntry.layout";

	protected ref array<ref SCR_PlayerListEntry> m_aEntries = new array<ref SCR_PlayerListEntry>();
	protected ref map<int, SCR_ScoreInfo> m_aAllPlayersInfo = new map<int, SCR_ScoreInfo>();
	protected ref array<Faction> m_aFactions = {null};

	protected SCR_InputButtonComponent m_Mute;
	protected SCR_InputButtonComponent m_Block;
 	protected SCR_InputButtonComponent m_Unblock;
	protected SCR_InputButtonComponent m_Vote;
	protected SCR_InputButtonComponent m_Invite;
	protected SCR_InputButtonComponent m_ViewProfile;

	protected SCR_VotingManagerComponent m_VotingManager;
	protected SCR_VoterComponent m_VoterComponent;
	protected SCR_BaseScoringSystemComponent m_ScoringSystem;
	protected SCR_PlayerListEntry m_SelectedEntry;
	protected SCR_PlayerControllerGroupComponent m_PlayerGroupController;
	protected PlayerController m_PlayerController;
	protected SocialComponent m_SocialComponent;
	SCR_SortHeaderComponent m_Header;
	protected Widget m_wTable;
	protected bool m_bFiltering;
	protected float m_fTimeSkip;

	protected const float TIME_STEP = 1.0;

	protected const string MUTE = "#AR-PlayerList_Mute";
	protected const string UNMUTE = "#AR-PlayerList_Unmute";
	protected const string BLOCK = "#AR-PlayerList_Block";
	protected const string UNBLOCK = "#AR-PlayerList_Unblock";
	protected const string INVITE_PLAYER_VOTE = "#AR-PlayerList_Invite";
	protected const string MUTE_TEXTURE = "sound-off";
	protected const string OPTIONS_COMBO_ACCEPT = "#AR-Group_AcceptJoinPrivateGroup";
	protected const string OPTIONS_COMBO_CANCEL = "#AR-Group_RefuseJoinPrivateGroup";
	protected const string VOTING_PLAYER_COUNT_FORMAT = "#AR-Voting_PlayerCountFormatting";

	protected const string FILTER_FAV = "Favourite";
	protected const string FILTER_NAME = "Name";
	protected const string FILTER_FREQ = "Freq";
	protected const string FILTER_KILL = "Kills";
	protected const string FILTER_DEATH = "Deaths";
	protected const string FILTER_SCORE = "Score";
	protected const string FILTER_MUTE = "Mute";
	protected const string FILTER_BLOCK = "Block";

	protected static const ResourceName FACTION_COUNTER_LAYOUT = "{5AD2CE85825EDA11}UI/layouts/Menus/PlayerList/FactionPlayerCounter.layout";
	
	protected const int DEFAULT_SORT_INDEX = 1;

	protected string m_sGameMasterIndicatorName = "GameMasterIndicator";
	protected string m_sCommanderIndicatorName = "CommanderIndicator";

	protected static ref ScriptInvoker s_OnPlayerListMenu = new ScriptInvoker();

	protected ref Color m_PlayerNameSelfColor = new Color(0.898, 0.541, 0.184, 1);
	
	protected ref map<EVotingType, int> m_mVotingTypesOnCooldown = new map<EVotingType, int>();
	
	protected ref BackendCallback m_BlockCallback = null;

	/*!
	Get event called when player list opens or closes.
	\return Script invoker
	*/
	static ScriptInvoker GetOnPlayerListMenu()
	{
		return s_OnPlayerListMenu;
	}

	//------------------------------------------------------------------------------------------------
	protected void InitSorting()
	{
		if (!GetRootWidget())
			return;

		Widget w = GetRootWidget().FindAnyWidget("SortHeader");
		if (!w)
			return;

		m_Header = SCR_SortHeaderComponent.Cast(w.FindHandler(SCR_SortHeaderComponent));
		if (!m_Header)
			return;

		m_Header.m_OnChanged.Insert(OnHeaderChanged);

		if (m_ScoringSystem)
			return;

		SCR_AIGroup.GetOnJoinPrivateGroupConfirm().Insert(SetEntryBackgrounColor);
		SCR_AIGroup.GetOnJoinPrivateGroupCancel().Insert(SetEntryBackgrounColor);

		// Hide K/D/S sorting headers if the re is no scoreboard
		ButtonWidget sortKills = ButtonWidget.Cast(w.FindAnyWidget("sortKills"));
		ButtonWidget sortDeaths = ButtonWidget.Cast(w.FindAnyWidget("sortDeaths"));
		ButtonWidget sortScore = ButtonWidget.Cast(w.FindAnyWidget("sortScore"));

		if (sortKills)
			sortKills.SetOpacity(0);
		if (sortDeaths)
			sortDeaths.SetOpacity(0);
		if (sortScore)
			sortScore.SetOpacity(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHeaderChanged(SCR_SortHeaderComponent sortHeader)
	{
		string filterName = sortHeader.GetSortElementName();
		bool sortUp = sortHeader.GetSortOrderAscending();
		Sort(filterName, sortUp);
	}


	//------------------------------------------------------------------------------------------------
	protected void Sort(string filterName, bool sortUp)
	{
		if (filterName == FILTER_NAME)
			SortByName(sortUp);
		else if (filterName == FILTER_FREQ)
			SortByFrequency(sortUp);
		else if (filterName == FILTER_KILL)
			SortByKills(sortUp);
		else if (filterName == FILTER_DEATH)
			SortByDeaths(sortUp);
		else if (filterName == FILTER_SCORE)
			SortByScore(sortUp);
		else if (filterName == FILTER_MUTE)
			SortByMuted(sortUp);
	}

	//------------------------------------------------------------------------------------------------
	void SortByMuted(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_Mute.IsToggled() * direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByName(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		array<string> names = {};
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_wName)
				names.Insert(entry.m_wName.GetText());
		}

		names.Sort();

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wName)
				continue;

			string text = entry.m_wName.GetText();

			foreach (int i, string s : names)
			{
				if (s != text)
					continue;

				if (entry.m_wRow)
					entry.m_wRow.SetZOrder(i * direction);
				continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByFrequency(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			entry.m_wRow.SetZOrder(entry.m_iSortFrequency * direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByKills(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			SCR_ScoreInfo score = entry.m_Info;
			if (score)
				entry.m_wRow.SetZOrder(score.m_iKills * direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByDeaths(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			SCR_ScoreInfo score = entry.m_Info;
			if (score)
				entry.m_wRow.SetZOrder(score.m_iDeaths * direction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void SortByScore(bool reverseSort = false)
	{
		int direction = 1;
		if (reverseSort)
			direction = -1;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			SCR_ScoreInfo info = entry.m_Info;
			if (info)
			{
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(entry.m_iID);
				else
					score = 0;

				entry.m_wRow.SetZOrder(score * direction);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnBlock(SCR_InputButtonComponent comp, string actionName)
	{
		if (!m_SelectedEntry || m_BlockCallback)
			return;

		m_BlockCallback = new BackendCallback();
		m_BlockCallback.SetOnError(OnBlockError);
		m_BlockCallback.SetOnSuccess(OnBlockSuccess);
		GameBlocklist blckList = GetGame().GetGameBlocklist();
		blckList.Block(m_BlockCallback, m_SelectedEntry.m_iID);
	}

	//------------------------------------------------------------------------------------------------
	void OnUnblock(SCR_InputButtonComponent comp, string actionName)
	{
		if (!m_SelectedEntry || m_BlockCallback)
			return;
		
		m_BlockCallback = new BackendCallback();
		m_BlockCallback.SetOnError(OnBlockError);
		m_BlockCallback.SetOnSuccess(OnBlockSuccess);
		GameBlocklist blckList = GetGame().GetGameBlocklist();
		blckList.Unblock(m_BlockCallback, m_SelectedEntry.m_iID);
	}

	//------------------------------------------------------------------------------------------------
	void OnMute()
	{
		if (!m_SelectedEntry)
			return;
		
		SCR_ButtonBaseComponent mute = m_SelectedEntry.m_Mute;
		mute.SetToggled(!mute.IsToggled());
	}

	//------------------------------------------------------------------------------------------------
	void OnTabChanged(SCR_TabViewComponent comp, Widget w, int selectedTab)
	{
		if (selectedTab < 0)
			return;

		Faction faction = null;
		foreach (Faction playableFaction : m_aFactions)
		{
			if (comp.GetShownTabComponent().m_sTabButtonContent == playableFaction.GetFactionName())
				faction = playableFaction;
		}

		int lowestZOrder = int.MAX;
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wRow)
				continue;

			//if the tab is the first one, it's the All tab for now
			if (comp.GetShownTab() == 0)
				entry.m_wRow.SetVisible(true);
			else if (faction == entry.m_Faction)
				entry.m_wRow.SetVisible(true);
			else
				entry.m_wRow.SetVisible(false);
		}

		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);
		CloseAllVoting();
	}

	//------------------------------------------------------------------------------------------------
	void OnBack()
	{
			Close();
	}

	//------------------------------------------------------------------------------------------------
	void OnVoting()
	{
		if (!m_SelectedEntry)
			return;

		SCR_ComboBoxComponent comp = m_SelectedEntry.m_PlayerActionList;
		if (!comp)
			return;

		if (comp.IsOpened())
			comp.CloseList();
		else
			comp.OpenList();
	}

	//------------------------------------------------------------------------------------------------
	void OnInvite()
	{
		if (!m_SelectedEntry)
			return;
		SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent().InvitePlayer(m_SelectedEntry.m_iID);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnViewProfile()
	{
		if (!m_SelectedEntry)
			return;
		
		GetGame().GetPlayerManager().ShowUserProfile(m_SelectedEntry.m_iID);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnMuteClick(SCR_ButtonBaseComponent comp, bool state)
	{
		if (!m_SocialComponent)
			return;

		int id = -1;
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Mute != comp)
				continue;

			id = entry.m_iID;
			break;
		}
		
		if (id < 0)
			return;
		
		m_SocialComponent.SetMuted(id, state);
		UpdateShownUIElements();
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryFocused(notnull Widget w)
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry)
				continue;

			Widget row = entry.m_wRow;
			if (row != w)
				continue;

			m_SelectedEntry = entry;
			break;
		}

		bool enablePlayerOptionList = CanOpenPlayerActionList(m_SelectedEntry);

		if (m_SelectedEntry && m_SelectedEntry.m_PlayerActionList)
			m_SelectedEntry.m_PlayerActionList.SetEnabled(enablePlayerOptionList);
		
		if (m_Vote)
			m_Vote.SetEnabled(enablePlayerOptionList);
		
		if (m_Invite && m_PlayerGroupController)
			m_Invite.SetEnabled(m_PlayerGroupController.CanInvitePlayer(m_SelectedEntry.m_iID));
		
		if (m_SelectedEntry)
			UpdateViewProfileButton(m_SelectedEntry.m_iID);
		UpdateShownUIElements();
	}

	//------------------------------------------------------------------------------------------------
	void OnEntryFocusLost(Widget w)
	{
		UpdateViewProfileButton(0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void FocusFirstItem()
	{
		Widget firstEntry;
		int lowestZOrder = int.MAX;
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (!entry.m_wRow.IsVisible())
				continue;

			int z = entry.m_wRow.GetZOrder();
			if (z < lowestZOrder)
			{
				lowestZOrder = z;
				firstEntry = entry.m_wRow;
			}
		}

		if (firstEntry)
			GetGame().GetWorkspace().SetFocusedWidget(firstEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateViewProfileButton(int playerId, bool forceHidden = false)
	{
		if (!m_ViewProfile)
			return;
		
		bool isLocal = IsLocalPlayer(playerId);
		bool isProfileAvailable = GetGame().GetPlayerManager().IsUserProfileAvailable(playerId);

		m_ViewProfile.SetVisible(!forceHidden && !isLocal && isProfileAvailable, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupPlayerActionList(notnull SCR_ComboBoxComponent combo)
	{
		if (!m_VotingManager || !m_VoterComponent)
			return;

		combo.ClearAll();
		
		m_mVotingTypesOnCooldown.Clear();

		int playerID = GetVotingPlayerID(combo);
		SCR_VotingUIInfo info;
		array<EVotingType> votingTypes = {};
		for (int i, count = m_VotingManager.GetVotingsAboutPlayer(playerID, votingTypes, true, true); i < count; i++)
		{
			EVotingType votingType = votingTypes[i];
			info = m_VotingManager.GetVotingInfo(votingType);
			
			if (!info)
			{
				Print("'SCR_PlayerListMenu' function 'SetupPlayerActionList' could not find votingInfo for vote: '" + typename.EnumToString(EVotingType, votingType) + "' is it added to the voting component?", LogLevel.ERROR); 
				continue;
			}
				
			if (!m_VotingManager.IsVoting(votingType, playerID))
			{				
				int cooldown = m_VotingManager.GetCurrentVoteCooldownForLocalPlayer(votingType);
				
				//--- Voting not in progress, start it
				if (cooldown <= 0)
				{
					combo.AddItem(info.GetStartVotingName(), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.START_VOTE));				
				}
				//~ Currently the player cannot vote as there is a cooldown
				else 
				{		
					combo.AddItem(WidgetManager.Translate(m_VotingManager.VOTE_TIMEOUT_FORMAT, info.GetStartVotingName(), SCR_FormatHelper.GetTimeFormatting(cooldown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS)), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.START_VOTE));
					m_mVotingTypesOnCooldown.Insert(votingType, combo.GetNumItems() -1);
				}
			}
			//--- Voting in progress
			else
			{
				//~ Did cast a vote, withdraw it
				if (m_VoterComponent.DidVote(votingType, playerID))
				{
					int currentVotes, VotesRequired;
					if (m_VotingManager.GetVoteCounts(votingType, playerID, currentVotes, VotesRequired))
						combo.AddItem(WidgetManager.Translate(VOTING_PLAYER_COUNT_FORMAT, info.GetCancelVotingName(), currentVotes, VotesRequired), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.CANCEL_VOTE));
					else 
						combo.AddItem(info.GetCancelVotingName(), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.CANCEL_VOTE));
				}
				else
				{
					//~ The player did not abstain from voting
					if (!m_VoterComponent.HasAbstained(votingType, playerID))
					{
						//~ Did not cast a vote, do it or abstain from doing it
						int currentVotes, VotesRequired;
						if (m_VotingManager.GetVoteCounts(votingType, playerID, currentVotes, VotesRequired))
						{
							combo.AddItem(WidgetManager.Translate(VOTING_PLAYER_COUNT_FORMAT, info.GetName(), currentVotes, VotesRequired), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.VOTE));
							combo.AddItem(info.GetAbstainVoteName(), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.ABSTAIN_VOTE));
						}
						else 
						{
							combo.AddItem(info.GetName(), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.VOTE));
							combo.AddItem(info.GetAbstainVoteName(), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.ABSTAIN_VOTE));
						}
					}
					//~ The Player abstained from voting. Revote again
					else 
					{
						int currentVotes, VotesRequired;
						if (m_VotingManager.GetVoteCounts(votingType, playerID, currentVotes, VotesRequired))
							combo.AddItem(WidgetManager.Translate(VOTING_PLAYER_COUNT_FORMAT, info.GetRevoteName(), currentVotes, VotesRequired), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.VOTE));
						else 
							combo.AddItem(info.GetRevoteName(), false, new SCR_PlayerListComboEntryData(votingType, SCR_EPlayerListComboAction.VOTE));
					}
				}
			}
		}

		//~ Group actions
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupManager)
		{
			SCR_AIGroup group = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
			if (group)
			{
				array<int> requesters = {};
				group.GetRequesterIDs(requesters);

				if (requesters.Contains(playerID))
				{
					combo.AddItem(OPTIONS_COMBO_ACCEPT, false, new SCR_PlayerListComboEntryData(SCR_EPlayerListComboType.GROUP, SCR_EPlayerListComboAction.COMFIRM_JOIN_PRIVATE_GROUP));
					combo.AddItem(OPTIONS_COMBO_CANCEL, false, new SCR_PlayerListComboEntryData(SCR_EPlayerListComboType.GROUP, SCR_EPlayerListComboAction.CANCEL_JOIN_PRIVATE_GROUP));
				}
				else if (m_PlayerGroupController.CanInvitePlayer(playerID))
					combo.AddItem(INVITE_PLAYER_VOTE, false, new SCR_PlayerListComboEntryData(SCR_EPlayerListComboType.GROUP, SCR_EPlayerListComboAction.INVITE_TO_GROUP));
			}
		}
		
		bool isLocalPlayer = IsLocalPlayer(playerID);
		
		//~ Reporting action
		if (!isLocalPlayer)
			combo.AddItem("#AR-Report_Dialog_Title", false,
		 		new SCR_PlayerListComboEntryData(SCR_EPlayerListComboType.REPORT,
			 		SCR_EPlayerListComboAction.REPORT_PLAYER
				)
			);
		
		//Blockin system action
		//~ Reporting action
		if (!isLocalPlayer)
		{
			if (!m_SocialComponent.IsBlocked(playerID))
			{
				combo.AddItem("#AR-PlayerList_Block", false,
			 		new SCR_PlayerListComboEntryData(
						SCR_EPlayerListComboType.BLOCK,
				 		SCR_EPlayerListComboAction.BLOCK_PLAYER
					)
				);
			}
			else if (m_SocialComponent.CanUnblock(playerID))
			{
				combo.AddItem("#AR-PlayerList_Unblock", false,
			 		new SCR_PlayerListComboEntryData(
						SCR_EPlayerListComboType.BLOCK,
				 		SCR_EPlayerListComboAction.UNBLOCK_PLAYER
					)
				);
			}
		}
		
		//~ Update any votes that have a cooldown
		if (!m_mVotingTypesOnCooldown.IsEmpty())
		{
			//~ Disable the HUD elements of the votes that are disabled
			foreach (EVotingType votingType, int index : m_mVotingTypesOnCooldown)
			{
				combo.SetElementWidgetEnabled(index, false, false);
			}
			
			UpdatePlayerActionList(combo , false);
			GetGame().GetCallqueue().CallLater(UpdatePlayerActionList, 1000, true, combo, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdatePlayerActionList(notnull SCR_ComboBoxComponent combo, bool isCallqueue)
	{
		//~ Check if there is a combo and any votes have a cooldown
		if (m_mVotingTypesOnCooldown.IsEmpty())
		{
			if (isCallqueue)
				GetGame().GetCallqueue().Remove(UpdatePlayerActionList);
			
			return;
		}
		
		//~ Check if combo has elements
		array<Widget> elementWidgets = {};
		combo.GetElementWidgets(elementWidgets);
		
		if (elementWidgets.IsEmpty())
		{
			if (isCallqueue)
				GetGame().GetCallqueue().Remove(UpdatePlayerActionList);
			
			return;
		}
		
		int cooldown;
		SCR_VotingUIInfo info;
		
		array<EVotingType> votingTypesToRemove = {};
		TextWidget textWidget;
		
		//~ For each disbled voting entry
		foreach (EVotingType votingType, int index : m_mVotingTypesOnCooldown)
		{
			info = m_VotingManager.GetVotingInfo(votingType);
			if (!info)
				continue;
			
			cooldown = m_VotingManager.GetCurrentVoteCooldownForLocalPlayer(votingType);
			
			//~ Entry still disabled so update the timer
			if (cooldown > 0)
			{
				if (!elementWidgets.IsIndexValid(index))
					continue;
				
				textWidget = TextWidget.Cast(elementWidgets[index].FindAnyWidget("Text"));
				if (!textWidget)
					continue;
				
				textWidget.SetTextFormat(m_VotingManager.VOTE_TIMEOUT_FORMAT, info.GetStartVotingName(), SCR_FormatHelper.GetTimeFormatting(cooldown, ETimeFormatParam.DAYS | ETimeFormatParam.HOURS));
			}
			//~ Voting is possible so set the entry active
			else 
			{
				votingTypesToRemove.Insert(votingType);
				combo.SetElementWidgetEnabled(index, true, true);
				
				if (!elementWidgets.IsIndexValid(index))
					continue;
				
				textWidget = TextWidget.Cast(elementWidgets[index].FindAnyWidget("Text"));
				if (!textWidget)
					continue;
				
				textWidget.SetText(info.GetStartVotingName());
			}
		}
		
		//~ Any entries that need to be removed
		foreach (EVotingType votingType : votingTypesToRemove)
		{
			m_mVotingTypesOnCooldown.Remove(votingType);
		}
		
		//~ No longer any votes with active cooldowns
		if (isCallqueue && m_mVotingTypesOnCooldown.IsEmpty())
			GetGame().GetCallqueue().Remove(UpdatePlayerActionList);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxConfirm(notnull SCR_ComboBoxComponent combo, int index)
	{
		if (!m_VoterComponent)
			return;
		
		int playerID = GetVotingPlayerID(combo);
		
		SCR_PlayerListComboEntryData comboData = SCR_PlayerListComboEntryData.Cast(combo.GetItemData(index));
		if (comboData)
		{
			switch (comboData.GetComboEntryAction())
			{
				case SCR_EPlayerListComboAction.VOTE: case SCR_EPlayerListComboAction.START_VOTE:
				{
					m_VoterComponent.Vote(comboData.GetComboEntryType(), playerID);
					break;
				}
				case SCR_EPlayerListComboAction.CANCEL_VOTE:
				{
					if (m_VoterComponent.DidVote(comboData.GetComboEntryType(), playerID))
						m_VoterComponent.RemoveVote(comboData.GetComboEntryType(), playerID);
					
					break;
				}	
				case SCR_EPlayerListComboAction.ABSTAIN_VOTE:
				{
					m_VoterComponent.AbstainVote(comboData.GetComboEntryType(), playerID);
					break;
				}
				case SCR_EPlayerListComboAction.INVITE_TO_GROUP:
				{
					SCR_PlayerControllerGroupComponent groupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
					if (groupComponent)
						groupComponent.InvitePlayer(playerID);
					
					break;
				}
				case SCR_EPlayerListComboAction.COMFIRM_JOIN_PRIVATE_GROUP:
				{
					m_PlayerGroupController.AcceptJoinPrivateGroup(playerID, true);
					break;
				}
				case SCR_EPlayerListComboAction.CANCEL_JOIN_PRIVATE_GROUP:
				{
					m_PlayerGroupController.AcceptJoinPrivateGroup(playerID, false);
					break;
				}
				case SCR_EPlayerListComboAction.REPORT_PLAYER:
				{
					new SCR_ReportPlayerDialog(playerID);
					break;
				}
				case SCR_EPlayerListComboAction.BLOCK_PLAYER:
				{
					OnBlock(null, string.Empty);
					break;
				}
				case SCR_EPlayerListComboAction.UNBLOCK_PLAYER:
				{
					OnUnblock(null, string.Empty);
					break;
				}
			}
		}

		combo.SetCurrentItem(-1, false, false);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetVotingPlayerID(SCR_ComboBoxComponent combo)
	{
		for (int i, count = m_aEntries.Count(); i < count; i++)
		{
			if (m_aEntries[i].m_PlayerActionList == combo)
				return m_aEntries[i].m_iID;
		}
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	void SetEntryBackgrounColor(Color color)
	{
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;
		
		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());
		if (!group)
			return;

		array<int> requesters = {};
		array<string> reqNames = {};

		group.GetRequesterIDs(requesters);

		foreach (int req : requesters)
		{
			reqNames.Insert(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(req));
		}

		if (!requesters.Contains(m_SelectedEntry.m_iID))
			return;
		
		Widget w = GetGame().GetWorkspace().FindAnyWidget("Button");

		SCR_ButtonBaseComponent button = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (!button)
			return;

		ImageWidget background = ImageWidget.Cast(m_SelectedEntry.m_wRow.FindAnyWidget("Background"));

		background.SetColor(color);
	}
	
	//------------------------------------------------------------------------------------------------
	void CreateEntry(int id, SCR_PlayerDelegateEditorComponent editorDelegateManager)
	{
		//check for existing entry, return if it exists already
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_iID == id)
				return;
		}

		ImageWidget badgeTop, badgeMiddle, badgeBottom;

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sScoreboardRow, m_wTable);
		if (!w)
			return;

		SCR_PlayerListEntry entry = new SCR_PlayerListEntry();
		entry.m_iID = id;
		entry.m_wRow = w;

		//--- Initialize voting combo box
		entry.m_PlayerActionList = SCR_ComboBoxComponent.GetComboBoxComponent("VotingCombo", w);
		if (m_VotingManager)
		{
			entry.m_PlayerActionList.m_OnOpened.Insert(SetupPlayerActionList);
			entry.m_PlayerActionList.m_OnChanged.Insert(OnComboBoxConfirm);
			entry.m_PlayerActionList.SetEnabled(CanOpenPlayerActionList(entry));

			entry.m_wVotingNotification = entry.m_wRow.FindAnyWidget("VotingNotification");
			entry.m_wVotingNotification.SetVisible(IsVotedAbout(entry));
		}
		else
		{
			entry.m_PlayerActionList.SetVisible(false);
		}

		SCR_ButtonBaseComponent handler = SCR_ButtonBaseComponent.Cast(w.FindHandler(SCR_ButtonBaseComponent));
		if (handler)
		{
			handler.m_OnFocus.Insert(OnEntryFocused);
			handler.m_OnFocusLost.Insert(OnEntryFocusLost);
		}

		if (m_aAllPlayersInfo)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != id)
					continue;

				entry.m_Info = info;
				break;
			}
		}

		// Find faction
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (factionManager)
		{
			Faction faction = factionManager.GetPlayerFaction(entry.m_iID);
			entry.m_Faction = faction;
		}

		Widget factionImage = w.FindAnyWidget("FactionImage");

		if (factionImage)
		{
			if (entry.m_Faction)
				factionImage.SetColor(entry.m_Faction.GetFactionColor());
			else
				factionImage.SetVisible(false);
		}

		entry.m_wName = TextWidget.Cast(w.FindAnyWidget("PlayerName"));

		if (entry.m_wName)
		{
			entry.m_wName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(id));
			if (entry.m_iID == m_PlayerController.GetPlayerId())
				entry.m_wName.SetColor(m_PlayerNameSelfColor);
		}

		if (editorDelegateManager)
		{
			SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(id);

			if (playerEditorDelegate)
			{
				playerEditorDelegate.GetOnLimitedEditorChanged().Insert(OnEditorRightsChanged);
				UpdateGameMasterIndicator(entry, playerEditorDelegate.HasLimitedEditor());
			}
		}

		SCR_Faction scrFaction = SCR_Faction.Cast(entry.m_Faction);
		if (scrFaction)
		{
			SCR_FactionCommanderHandlerComponent commanderHandler = SCR_FactionCommanderHandlerComponent.GetInstance();

			if (commanderHandler)
			{
				ToggleCommanderIndicator(entry, scrFaction.GetCommanderId() == entry.m_iID);
				commanderHandler.GetOnFactionCommanderChanged().Insert(OnFactionCommanderChanged);
			}
		}

		entry.m_wFreq = TextWidget.Cast(w.FindAnyWidget("Freq"));
		entry.m_wKills = TextWidget.Cast(w.FindAnyWidget("Kills"));
		entry.m_wDeaths = TextWidget.Cast(w.FindAnyWidget("Deaths"));
		entry.m_wScore = TextWidget.Cast(w.FindAnyWidget("Score"));
		if (entry.m_Info)
		{
			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(id);

				entry.m_wScore.SetText(score.ToString());
			}
		}
		else
		{

			if (entry.m_wKills)
				entry.m_wKills.SetText("");
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText("");
			if (entry.m_wScore)
				entry.m_wScore.SetText("");
		}

		entry.m_Mute = SCR_ButtonBaseComponent.GetButtonBase("Mute", w);
		entry.m_wTaskIcon = entry.m_wRow.FindAnyWidget("TaskIcon");
		entry.m_wBlockedIcon = entry.m_wRow.FindAnyWidget("BlockedIcon");
		entry.m_wLoadoutIcon = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("LoadoutIcon"));
		entry.m_wPlatformIcon = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("PlatformIcon"));

		ImageWidget background = ImageWidget.Cast(w.FindAnyWidget("Background"));
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupManager)
			return;

		SCR_AIGroup group = groupManager.GetPlayerGroup(m_PlayerController.GetPlayerId());

		// Saphyr TODO: temporary before definition from art dept.
		if (group && group.HasRequesterID(id))
			background.SetColor(m_PlayerNameSelfColor);

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (entry.m_wTaskIcon && taskSystem)
		{
			SCR_TaskExecutor taskExecutor = SCR_TaskExecutor.FromPlayerID(entry.m_iID);
			if (taskSystem.GetTaskAssignedTo(taskExecutor))
			{
				entry.m_wTaskIcon.SetColor(entry.m_Faction.GetFactionColor());
			}
			else
			{
				entry.m_wTaskIcon.SetOpacity(0);
			}
		}

		Faction playerFaction;		
		Faction entryPlayerFaction;
		if (factionManager)
		{
			playerFaction = factionManager.GetPlayerFaction(m_PlayerController.GetPlayerId());	
			entryPlayerFaction = factionManager.GetPlayerFaction(entry.m_iID);
		}
		 
		
		SCR_BasePlayerLoadout playerLoadout;
		SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
		if (loadoutManager)
			playerLoadout = loadoutManager.GetPlayerLoadout(entry.m_iID);		 

				
		if (entry.m_wBlockedIcon && m_SocialComponent)
			entry.m_wBlockedIcon.SetOpacity(m_SocialComponent.IsBlocked(entry.m_iID));
		
		if (entry.m_wLoadoutIcon && (playerFaction != entryPlayerFaction))
			entry.m_wLoadoutIcon.SetVisible(false);

		// Enable GM to see everyones loadout icon, temporary solution until we get improved ways to say who's GM
		if (SCR_EditorManagerEntity.IsOpenedInstance())
			entry.m_wLoadoutIcon.SetVisible(true);

		if (entry.m_wLoadoutIcon && playerLoadout && entry.m_wLoadoutIcon.IsVisible())
		{
			Resource res = Resource.Load(playerLoadout.GetLoadoutResource());
			IEntityComponentSource source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
			if (!source)
				return;
			BaseContainer container = source.GetObject("m_UIInfo");
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
			info.SetIconTo(entry.m_wLoadoutIcon);
		}

		if (entry.m_wPlatformIcon)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (playerController)
				playerController.SetPlatformImageTo(entry.m_iID, entry.m_wPlatformIcon, showOnPC: true, showOnXbox: true)
		}

		badgeTop = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(entry.m_wRow.FindAnyWidget("BadgeBottom"));
		Color factionColor;

		if (badgeTop && badgeMiddle && badgeBottom && entry.m_Faction)
		{
			factionColor = entry.m_Faction.GetFactionColor();
			badgeTop.SetColor(factionColor);
			badgeMiddle.SetColor(factionColor);
			badgeBottom.SetColor(factionColor);
		}

		// Handle mute icon/action based on blocked state
		if (m_SocialComponent && !IsLocalPlayer(entry.m_iID))
		{
			// Fore "muted" for blocked players
			if (m_SocialComponent.IsBlocked(entry.m_iID))
			{
				entry.m_Mute.SetEnabled(false);
				entry.m_Mute.SetToggled(true);
			}
			else
			{
				entry.m_Mute.SetEnabled(true);
				entry.m_Mute.SetToggled(m_SocialComponent.IsMuted(entry.m_iID));
			}
			entry.m_Mute.m_OnToggled.Insert(OnMuteClick);
		}

		m_aEntries.Insert(entry);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditorRightsChanged(int playerID, bool newLimited)
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_iID == playerID)
			{
				UpdateGameMasterIndicator(entry, newLimited);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateGameMasterIndicator(notnull SCR_PlayerListEntry entry, bool editorIslimited)
	{
		Widget gameMasterIndicator = entry.m_wRow.FindAnyWidget(m_sGameMasterIndicatorName);
		if (gameMasterIndicator)
			gameMasterIndicator.SetVisible(!editorIslimited);
	}

	//------------------------------------------------------------------------------------------------
	// IsLocalPlayer would be better naming
	protected bool IsLocalPlayer(int id)
	{
		if (id <= 0)
			return false;

		return SCR_PlayerController.GetLocalPlayerId() == id;
	}

	//------------------------------------------------------------------------------------------------
	void RemoveEntry(notnull SCR_PlayerListEntry entry)
	{
		if (entry.m_wRow)
			entry.m_wRow.RemoveFromHierarchy();

		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));

		//Remove the subscription to player right changed
		if (editorDelegateManager)
		{
			SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(entry.m_iID);

			if (playerEditorDelegate)
			{
				playerEditorDelegate.GetOnLimitedEditorChanged().Remove(OnEditorRightsChanged);
			}
		}

		m_aEntries.RemoveItem(entry);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnVotingChanged(EVotingType type, int value, int playerID)
	{
		UpdateVoting();
	}
	protected void GetOnVotingStart(EVotingType type, int value)
	{
		UpdateVoting();
	}
	protected void UpdateVoting()
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			entry.m_PlayerActionList.SetEnabled(CanOpenPlayerActionList(entry));
			entry.m_wVotingNotification.SetVisible(IsVotedAbout(entry));
		}

		if (m_SelectedEntry)
			m_Vote.SetEnabled(CanOpenPlayerActionList(m_SelectedEntry));
	}
	protected bool IsVotedAbout(SCR_PlayerListEntry entry)
	{
		if (!entry || !m_VotingManager)
			return false;

		array<EVotingType> votingTypes = {};
		int  count = m_VotingManager.GetVotingsAboutPlayer(entry.m_iID, votingTypes, false, true);
		int validEntries = count;
		
		//~ Remove any votes that cannot be shown to the player. Eg faction specific votes
		foreach (EVotingType voteType : votingTypes)
		{
			if (!m_VotingManager.IsVotingAvailable(voteType, entry.m_iID))
				validEntries--;
		}
		
		return validEntries > 0;
	}
	
	protected bool CanOpenPlayerActionList(notnull SCR_PlayerListEntry entry)
	{		
		if (m_VotingManager)
		{
			//~ Check if can vote, if yes return true
			array<EVotingType> votingTypes = {};
			m_VotingManager.GetVotingsAboutPlayer(entry.m_iID, votingTypes, true, true);
			
			//~ Check if UI info can be found
			foreach(EVotingType votingType : votingTypes)
			{
				if (m_VotingManager.GetVotingInfo(votingType))
					return true;
			}
		}
		
		//~ Can invite the player so return true
		if (m_PlayerGroupController.CanInvitePlayer(entry.m_iID))
			return true;
		
		//~ Check if Player actions have group dropdown
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();
		if (groupManager)
		{
			//~ No group so no drop down
			SCR_AIGroup group = groupManager.GetPlayerGroup(SCR_PlayerController.GetLocalPlayerId());
			if (group)
			{
				array<int> requesters = {};
				group.GetRequesterIDs(requesters);
		
				if (requesters.Contains(entry.m_iID))
					return true;
			}
		}
		
		//Check if reporting is available
		if (!IsLocalPlayer(entry.m_iID))
			return true;
		
		//~ None of the conditions met
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnPlayerAdded(int playerId)
	{
		UpdatePlayerList(true, playerId);
	}
	private void OnPlayerRemoved(int playerId)
	{
		UpdatePlayerList(false, playerId);
	}
	private void OnScoreChanged()
	{
		UpdateScore();
	}
	private void OnPlayerScoreChanged(int playerId, SCR_ScoreInfo scoreInfo)
	{
		OnScoreChanged();
	}
	private void OnFactionScoreChanged(Faction faction, SCR_ScoreInfo scoreInfo)
	{
		OnScoreChanged();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		GameBlocklist blocklist = GetGame().GetGameBlocklist();
		blocklist.OnBlockListUpdateInvoker.Insert(OnBlocklistUpdate);
		
		m_PlayerController = GetGame().GetPlayerController();
		if (m_PlayerController)
		{
			m_SocialComponent = SocialComponent.Cast(m_PlayerController.FindComponent(SocialComponent));
			
			SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(m_PlayerController.FindComponent(SCR_HUDManagerComponent));
			hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() & ~EHudLayers.HIGH);
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;
		m_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		m_VoterComponent = SCR_VoterComponent.GetInstance();
		m_VotingManager = SCR_VotingManagerComponent.GetInstance();

		if (m_VotingManager)
		{
			m_VotingManager.GetOnVotingEnd().Insert(OnVotingChanged);
			m_VotingManager.GetOnVotingStart().Insert(GetOnVotingStart);
			m_VotingManager.GetOnRemoveVote().Insert(OnVotingChanged);
		}

		gameMode.GetOnPlayerRegistered().Insert(OnPlayerConnected);

		m_ScoringSystem = gameMode.GetScoringSystemComponent();
		if (m_ScoringSystem)
		{
			m_ScoringSystem.GetOnPlayerAdded().Insert(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Insert(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Insert(OnFactionScoreChanged);

			array<int> players = {};
			PlayerManager playerManager = GetGame().GetPlayerManager();
			playerManager.GetPlayers(players);

			m_aAllPlayersInfo.Clear();
			foreach (int playerId : players)
				m_aAllPlayersInfo.Insert(playerId, m_ScoringSystem.GetPlayerScoreInfo(playerId));
		}

		FactionManager fm = GetGame().GetFactionManager();
		if (fm)
		{
			fm.GetFactionsList(m_aFactions);
		}

		m_wTable = GetRootWidget().FindAnyWidget("Table");

		// Create navigation buttons
		Widget footer = GetRootWidget().FindAnyWidget("FooterLeft");
		Widget footerBack = GetRootWidget().FindAnyWidget("Footer");
		SCR_InputButtonComponent back = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, footerBack);
		if (back)
			back.m_OnActivated.Insert(OnBack);
		
		// Setup all buttons, throw error in case of misconfigured layout.
		// Then do not check in other methods
		m_Mute = SCR_InputButtonComponent.GetInputButtonComponent("Mute", footer);
		if (m_Mute)
		{
			m_Mute.SetEnabled(false);
			m_Mute.m_OnActivated.Insert(OnMute);
		}
		else
		{
			Print("'SCR_PlayerListMenu' missing 'Mute' 'SCR_InputButtonComponent'", LogLevel.ERROR); 
		}

		m_Block = SCR_InputButtonComponent.GetInputButtonComponent("Block", footer);
		if (m_Block)
		{
			m_Block.SetEnabled(false);
			m_Block.m_OnActivated.Insert(OnBlock);
		}
		else
		{
			Print("'SCR_PlayerListMenu' missing 'Block' 'SCR_InputButtonComponent'", LogLevel.ERROR); 
		}
		
		m_Unblock = SCR_InputButtonComponent.GetInputButtonComponent("Unblock", footer);
		if (m_Unblock)
		{
			m_Unblock.m_OnActivated.Insert(OnUnblock);
			// Create this button hidden and disabled
			m_Unblock.SetVisible(false);
			m_Unblock.SetEnabled(false);
		}
		else
		{
			Print("'SCR_PlayerListMenu' missing 'Unblock' 'SCR_InputButtonComponent'", LogLevel.ERROR); 
		}
		
		m_Vote = SCR_InputButtonComponent.GetInputButtonComponent("Vote", footer);
		if (m_Vote)
		{
			if (m_VotingManager)
				m_Vote.m_OnActivated.Insert(OnVoting);
			else
				m_Vote.SetVisible(false,false);
			m_Vote.SetEnabled(false);
		}
		else
		{
			Print("'SCR_PlayerListMenu' missing 'Vote' 'SCR_InputButtonComponent'", LogLevel.ERROR); 
		}
		
		m_Invite = SCR_InputButtonComponent.GetInputButtonComponent("Invite", footer);
		if (m_Invite)
		{
			if (m_PlayerGroupController)
				m_Invite.m_OnActivated.Insert(OnInvite);
			else
				m_Invite.SetVisible(false, false);
			m_Invite.SetEnabled(false);
		}
		else
		{
			Print("'SCR_PlayerListMenu' missing 'Invite' 'SCR_InputButtonComponent'", LogLevel.ERROR); 
		}
		
		m_ViewProfile = SCR_InputButtonComponent.GetInputButtonComponent("ViewProfile", footer);
		if (m_ViewProfile)
		{
			UpdateViewProfileButton(0, true);
			m_ViewProfile.m_OnActivated.Insert(OnViewProfile);
		}
		else
		{
			Print("'SCR_PlayerListMenu' missing 'ViewProfile' 'SCR_InputButtonComponent'", LogLevel.ERROR); 
		}

		SCR_InputButtonComponent filter = SCR_InputButtonComponent.GetInputButtonComponent("Filter", footer);

		// Create table
		if (!m_wTable || m_sScoreboardRow == string.Empty)
			return;

		//Get editor Delegate manager to check if has editor rights
		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));


		array<int> ids = {};
		GetGame().GetPlayerManager().GetPlayers(ids);

			foreach (int id : ids)
			{
				CreateEntry(id, editorDelegateManager);
			}

		InitSorting();
		
		m_SuperMenuComponent.GetTabView().GetOnChanged().Insert(OnTabChanged);

		// Create new tabs
		SCR_Faction scrFaction;
		foreach (Faction faction : m_aFactions)
		{
			if (!faction)
				continue;

			scrFaction = SCR_Faction.Cast(faction);
			if (scrFaction && !scrFaction.IsPlayable())
				continue; //--- ToDo: Refresh dynamically when a new faction is added/removed

			string name = faction.GetFactionName();
			m_SuperMenuComponent.GetTabView().AddTab(ResourceName.Empty,name);
			
			AddFactionPlayerCounter(faction);
		}
		
		UpdateFrequencies();

		s_OnPlayerListMenu.Invoke(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		m_fTimeSkip = m_fTimeSkip + tDelta;

		if (m_fTimeSkip >= TIME_STEP)
		{
			DeleteDisconnectedEntries();
			m_fTimeSkip = 0.0;
		}

		GetGame().GetInputManager().ActivateContext("PlayerMenuContext");
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		GetGame().GetInputManager().AddActionListener("ShowScoreboard",	EActionTrigger.DOWN, Close);

		if (m_Header)
			m_Header.SetCurrentSortElement(DEFAULT_SORT_INDEX, ESortOrder.ASCENDING, useDefaultSortOrder: true);

		// TODO: Consider not changing focus if it is already set e.g. dialog opened over
		FocusFirstItem();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		GetGame().GetInputManager().RemoveActionListener("ShowScoreboard",	EActionTrigger.DOWN, Close);

		//--- Close when some other menu is opened on top
		// Update: We do not want to close menu under the dialog
		//Close();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		GetGame().GetGameBlocklist().OnBlockListUpdateInvoker.Insert(OnBlocklistUpdate);;

		SCR_HUDManagerComponent hudManager = SCR_HUDManagerComponent.Cast(m_PlayerController.FindComponent(SCR_HUDManagerComponent));
		if (hudManager)
			hudManager.SetVisibleLayers(hudManager.GetVisibleLayers() | EHudLayers.HIGH);

		SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));

		//Remove the subscriptions to player right changed
		if (editorDelegateManager)
		{
			foreach (SCR_PlayerListEntry entry : m_aEntries)
			{
				if (entry)
				{
					SCR_EditablePlayerDelegateComponent playerEditorDelegate = editorDelegateManager.GetDelegate(entry.m_iID);

					if (playerEditorDelegate)
					{
						playerEditorDelegate.GetOnLimitedEditorChanged().Remove(OnEditorRightsChanged);
					}
				}
			}
		}

		m_aAllPlayersInfo.Clear();
		m_aFactions.Clear();

		if (m_ScoringSystem)
		{
			m_ScoringSystem.GetOnPlayerAdded().Remove(OnPlayerAdded);
			m_ScoringSystem.GetOnPlayerRemoved().Remove(OnPlayerRemoved);
			m_ScoringSystem.GetOnPlayerScoreChanged().Remove(OnPlayerScoreChanged);
			m_ScoringSystem.GetOnFactionScoreChanged().Remove(OnFactionScoreChanged);
		}
		
		if (!m_mVotingTypesOnCooldown.IsEmpty())
			GetGame().GetCallqueue().Remove(UpdatePlayerActionList);

		s_OnPlayerListMenu.Invoke(false);
	}

	// Call updates with delay, so name is synced properly
	//------------------------------------------------------------------------------------------------
	void OnPlayerConnected(int id)
	{
		GetGame().GetCallqueue().CallLater(UpdatePlayerList, 1000, false, true, id);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionCommanderChanged(SCR_Faction faction, int commanderPlayerId)
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			ToggleCommanderIndicator(entry, commanderPlayerId == entry.m_iID);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdatePlayerList(bool addPlayer, int id)
	{
		if (addPlayer)
		{
			SCR_PlayerDelegateEditorComponent editorDelegateManager = SCR_PlayerDelegateEditorComponent.Cast(SCR_PlayerDelegateEditorComponent.GetInstance(SCR_PlayerDelegateEditorComponent));
			CreateEntry(id, editorDelegateManager);

			// Get current sort method and re-apply sorting
			if (!m_Header)
				return;

			OnHeaderChanged(m_Header);
		}
		else
		{
			// Delete entry from the list
			SCR_PlayerListEntry playerEntry;
			foreach (SCR_PlayerListEntry entry : m_aEntries)
			{
				if (entry.m_iID != id)
					continue;

				playerEntry = entry;
				break;
			}

			if (!playerEntry)
				return;

			playerEntry.m_wRow.RemoveFromHierarchy();
			m_aEntries.RemoveItem(playerEntry);
		}
	}

	//------------------------------------------------------------------------------------------------
	void UpdateFrequencies()
	{
		SCR_GadgetManagerComponent gadgetManager;
		IEntity localPlayer = SCR_PlayerController.GetLocalMainEntity();
		Faction localFaction;
		set<int> localFrequencies = new set<int>();
		if (localPlayer)
		{
			gadgetManager = SCR_GadgetManagerComponent.Cast(localPlayer.FindComponent(SCR_GadgetManagerComponent));
			SCR_Global.GetFrequencies(gadgetManager, localFrequencies);

			FactionAffiliationComponent factionAffiliation = FactionAffiliationComponent.Cast(localPlayer.FindComponent(FactionAffiliationComponent));
			if (factionAffiliation)
				localFaction = factionAffiliation.GetAffiliatedFaction();
		}

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_Faction == localFaction)
			{
				IEntity playerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(entry.m_iID);
				if (playerEntity)
				{
					//--- ToDo: Don't extract frequencies locally; do it on server and distribute values to all clients
					gadgetManager = SCR_GadgetManagerComponent.Cast(playerEntity.FindComponent(SCR_GadgetManagerComponent));
					set<int> frequencies = new set<int>();
					SCR_Global.GetFrequencies(gadgetManager, frequencies);
					if (!frequencies.IsEmpty())
					{
						entry.m_iSortFrequency = frequencies[0];
						entry.m_wFreq.SetText(SCR_FormatHelper.FormatFrequencies(frequencies, localFrequencies));
						continue;
					}
				}
			}
			entry.m_iSortFrequency = int.MAX;
			entry.m_wFreq.SetText("-");
		}
		GetGame().GetCallqueue().CallLater(UpdateFrequencies, 1000, false);
	}
	
	//------------------------------------------------------------------------------------------------	
	//! Updates selected player list entry and main actions/buttons
	void UpdateShownUIElements()
	{	
		if (!m_SelectedEntry || !m_SocialComponent)
			return;
		
		// Self player list record handling
		if (IsLocalPlayer(m_SelectedEntry.m_iID))
		{
			// Show disabled block button
			m_Block.SetEnabled(false, false);
			m_Block.SetVisible(true, false);
			
			// Hide unblock button
			m_Unblock.SetEnabled(false, false);
			m_Unblock.SetVisible(false, false);
			
			// Disable mute
			m_Mute.SetEnabled(false, false);
			
			GetRootWidget().Update();
			
			m_SelectedEntry.m_wBlockedIcon.SetOpacity(0);
			m_SelectedEntry.m_Mute.SetEnabled(false);
			return;
		}
		
		bool isSelectedMuted = m_SocialComponent.IsMuted(m_SelectedEntry.m_iID);
		bool isSelectedBlocked = m_SocialComponent.IsBlocked(m_SelectedEntry.m_iID);
		
		// Update elements in selected entry
		m_SelectedEntry.m_wBlockedIcon.SetOpacity(isSelectedBlocked);
		// Force muted for blocked ones
		m_SelectedEntry.m_Mute.SetToggled(isSelectedBlocked || isSelectedMuted, false, false, true);
		m_SelectedEntry.m_Mute.SetEnabled(!isSelectedBlocked);
		
		// Update buttons
		m_Block.SetEnabled(!isSelectedBlocked, false);
		m_Block.SetVisible(!isSelectedBlocked, false);
		// Unblock button is available only if not blocked outside of game
		bool canBeUnblocked = m_SocialComponent.CanUnblock(m_SelectedEntry.m_iID);
		m_Unblock.SetEnabled(isSelectedBlocked && canBeUnblocked, false);
		m_Unblock.SetVisible(isSelectedBlocked, false);
		// Enable muted only if not blocked
		m_Mute.SetEnabled(!isSelectedBlocked, false);

		if (isSelectedMuted || isSelectedBlocked)
		{
			m_Mute.SetLabel(UNMUTE);
		}
		else
		{
			m_Mute.SetLabel(MUTE);
		}
		
		GetRootWidget().Update();	
	}

	//------------------------------------------------------------------------------------------------
	void UpdateScore()
	{
		if (!m_aAllPlayersInfo || m_aAllPlayersInfo.Count() == 0 || !m_ScoringSystem)
			return;

		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			foreach (int playerId, SCR_ScoreInfo info : m_aAllPlayersInfo)
			{
				if (!info || playerId != entry.m_iID)
					continue;

				entry.m_Info = info;
				break;
			}

			if (!entry.m_Info)
				continue;

			if (entry.m_wKills)
				entry.m_wKills.SetText(entry.m_Info.m_iKills.ToString());
			if (entry.m_wDeaths)
				entry.m_wDeaths.SetText(entry.m_Info.m_iDeaths.ToString());
			if (entry.m_wScore)
			{
				// Use modifiers from scoring system where applicable!!!
				int score;
				if (m_ScoringSystem)
					score = m_ScoringSystem.GetPlayerScore(entry.m_iID);

				entry.m_wScore.SetText(score.ToString());
			}
		}

		// Get current sort method and re-apply sorting
		if (!m_Header)
			return;

		OnHeaderChanged(m_Header);
	}

	//------------------------------------------------------------------------------------------------
	void CloseAllVoting()
	{
		foreach (SCR_PlayerListEntry entry : m_aEntries)
		{
			if (entry.m_PlayerActionList && entry.m_PlayerActionList.IsOpened())
				entry.m_PlayerActionList.CloseList();
		}
	}

	//------------------------------------------------------------------------------------------------
	void DeleteDisconnectedEntries()
	{
		for (int i = m_aEntries.Count() - 1; i >= 0; i--)
		{
			if (!m_aEntries[i])
				continue;
			
			if (!m_aEntries[i] || GetGame().GetPlayerManager().IsPlayerConnected(m_aEntries[i].m_iID))
				continue;
			
			m_aEntries[i].m_wRow.RemoveFromHierarchy();			
			m_aEntries.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void AddFactionPlayerCounter(Faction faction)
	{
		SCR_Faction scriptedFaction = SCR_Faction.Cast(faction);
		if (!scriptedFaction)
			return;
		
		Widget contentLayout = GetRootWidget().FindAnyWidget("FactionPlayerNumbersLayout");
		if (!contentLayout)
			return;
		
		Widget factionTile = GetGame().GetWorkspace().CreateWidgets(FACTION_COUNTER_LAYOUT, contentLayout);
		if (!factionTile)
			return;
		
		RichTextWidget playerCount = RichTextWidget.Cast(factionTile.FindAnyWidget("PlayerCount"));
		if (!playerCount)
			return;
		
		ImageWidget factionFlag = ImageWidget.Cast(factionTile.FindAnyWidget("FactionFlag"));
		if (!factionFlag)
			return;

		int x, y;
		factionFlag.LoadImageTexture(0, scriptedFaction.GetFactionFlag());	
		factionFlag.GetImageSize(0, x, y);
		factionFlag.SetSize(x, y);
		
		string playerCountText = scriptedFaction.GetPlayerCount().ToString();
		int playerLimit = scriptedFaction.GetPlayerLimit();
		if (playerLimit >= 0)
			playerCount.SetTextFormat("#AR-SupportStation_ActionFormat_ItemAmount", playerCountText, playerLimit);
		else
			playerCount.SetText(playerCountText);
	}

	//------------------------------------------------------------------------------------------------
	protected void ToggleCommanderIndicator(SCR_PlayerListEntry entry, bool toggle)
	{
		ImageWidget commanderIndicator = ImageWidget.Cast(entry.m_wRow.FindAnyWidget(m_sCommanderIndicatorName));

		if (!commanderIndicator)
			return;

		if (toggle && SCR_FactionManager.SGetLocalPlayerFaction() != SCR_FactionManager.SGetPlayerFaction(entry.m_iID))
			toggle = false;

		if (toggle)
		{
			commanderIndicator.LoadImageFromSet(0, "{3262679C50EF4F01}UI/Textures/Icons/icons_wrapperUI.imageset", "emote_salute");
			commanderIndicator.SetColor(SCR_FactionManager.SGetLocalPlayerFaction().GetFactionColor());
		}

		commanderIndicator.SetVisible(toggle);
	}
	
	//------------------------------------------------------------------------------------------------
	
	//! Show game blocking error dialog with given tag
	private void ShowBlockErrorDialog(string dialogTag)
	{
		SCR_BlockedUsersDialogUI dialog = new SCR_BlockedUsersDialogUI();
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_AccountWidgetComponent.BLOCKED_USER_DIALOG_CONFIG, dialogTag, dialog);
	}

	//------------------------------------------------------------------------------------------------
	private void OnBlockSuccess(BackendCallback cb)
	{
		m_BlockCallback = null;
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnBlockError(BackendCallback cb)
	{
		if (cb.GetBackendError() == EBackendError.EBERR_STORAGE_IS_FULL)
			ShowBlockErrorDialog("block_failed_full");
		else
			ShowBlockErrorDialog("block_failed_general");
		m_BlockCallback = null;
	}
	
	//------------------------------------------------------------------------------------------------
	private void OnBlocklistUpdate(bool success)
	{
		if (!success)
		{
			ShowBlockErrorDialog("block_failed_general");
			return;
		}
		
		// Possible risk/todo: We may get more information in the blocklist update
		// thus we should iterate all the playerlist entries and update them properly
		// we are now updating only focused item
		UpdateShownUIElements();
	}
};

//------------------------------------------------------------------------------------------------
//~ Holds the data of the player list combo entry
class SCR_PlayerListComboEntryData : Managed
{
	protected SCR_EPlayerListComboType m_eComboEntryType;
	protected SCR_EPlayerListComboAction m_eComboEntryAction;
	
	void SCR_PlayerListComboEntryData(SCR_EPlayerListComboType comboEntryType, SCR_EPlayerListComboAction comboEntryAction)
	{
		m_eComboEntryType = comboEntryType;
		m_eComboEntryAction = comboEntryAction;
	}
	
	/*!
	\return Type of Player List Combo Entry Data
	*/
	SCR_EPlayerListComboType GetComboEntryType()
	{
		return m_eComboEntryType;
	}
	
	/*!
	\return Action for type of Player List Combo Entry Data
	*/
	SCR_EPlayerListComboAction GetComboEntryAction()
	{
		return m_eComboEntryAction;
	}
}

//------------------------------------------------------------------------------------------------
//~ Types of combo list data entry, inherents from Voting as voting is the main type used
enum SCR_EPlayerListComboType : EVotingType
{
	GROUP, //~ Makes sure System knows the combo data is regarding group
	REPORT,
	BLOCK
};

//------------------------------------------------------------------------------------------------
//~ Available actions valid for the player combo box data types
enum SCR_EPlayerListComboAction
{
	//~ Voting
	VOTE = 0,
	START_VOTE, 
	CANCEL_VOTE, 
	ABSTAIN_VOTE, 
	
	//Group
	INVITE_TO_GROUP,
	COMFIRM_JOIN_PRIVATE_GROUP,
	CANCEL_JOIN_PRIVATE_GROUP,
	
	//Report
	REPORT_PLAYER,
	
	//Blocking
	BLOCK_PLAYER,
	UNBLOCK_PLAYER
};
