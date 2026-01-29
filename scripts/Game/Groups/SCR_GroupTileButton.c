class SCR_PlayerTileButtonComponent : SCR_ScriptedWidgetComponent
{
	protected SCR_ComboBoxComponent m_OptionsCombo;
	protected int m_iPlayerId;
	protected SCR_ChimeraCharacter m_AICharacter;

	protected ref ScriptInvokerInt m_OnTileFocus;
	protected ref ScriptInvokerInt m_OnTileFocusLost;
	
	
	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		// TODO: on self, this combo box is empty and invisible. On others allows inviting to your group
		m_OptionsCombo.OpenList();
		return super.OnClick(w, x, y, button);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{	
		if (m_OnTileFocus)
			m_OnTileFocus.Invoke(m_iPlayerId);
		
		return super.OnFocus(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_OnTileFocusLost)
			m_OnTileFocusLost.Invoke(m_iPlayerId);
		
		return super.OnFocusLost(w, x, y);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnTileFocus()
	{
		if (!m_OnTileFocus)
			m_OnTileFocus = new ScriptInvokerInt();
		
		return m_OnTileFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerInt GetOnTileFocusLost()
	{
		if (!m_OnTileFocusLost)
			m_OnTileFocusLost = new ScriptInvokerInt();
		
		return m_OnTileFocusLost;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetTilePlayerID()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	void SetTilePlayerID(int playerID)
	{
		m_iPlayerId = playerID;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ChimeraCharacter GetCharacter()
	{
		return m_AICharacter;
	}

	//------------------------------------------------------------------------------------------------
	void SetCharacter(SCR_ChimeraCharacter character)
	{
		m_AICharacter = character;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ComboBoxComponent GetOptionsComboComponent()
	{
		return m_OptionsCombo;
	}

	//------------------------------------------------------------------------------------------------
	void SetOptionsComboBoxComponent(SCR_ComboBoxComponent optionsCombo)
	{
		m_OptionsCombo = optionsCombo;
	}

	//------------------------------------------------------------------------------------------------
	void SetOpacity(float opacity)
	{
		Widget comboBox = m_OptionsCombo.GetRootWidget();
		if (!comboBox)
			return;

		ImageWidget comboImage = ImageWidget.Cast(comboBox.FindAnyWidget("Image0"));
		if (!comboImage)
			return;

		comboImage.SetOpacity(opacity);
	}
};

//------------------------------------------------------------------------------------------------
class SCR_GroupTileButton : SCR_ButtonBaseComponent
{
	protected int m_iGroupID;
	protected Faction m_GroupFaction;

	[Attribute()]
	protected ResourceName m_textLayout;

	[Attribute()]
	protected ref SCR_UIInfo m_LeaderInfo;

	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_TaskSystem m_TaskSystem;
	protected SCR_InputButtonComponent m_JoinGroupButton;
	protected SCR_InputButtonComponent m_RemoveGroupButton;
	protected SCR_PlayerControllerGroupComponent m_GroupComponent;
	protected SCR_PlayerFactionAffiliationComponent m_sPlyFactionAffilComp;
	
	protected SCR_HorizontalScrollAnimationComponent m_ScrollComponent;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_PlayerNameSelfColor;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_PlayerNameDeadColor;

	[Attribute("0.898 0.541 0.184 1", UIWidgets.ColorPicker)]
	protected ref Color m_GroupFullColor;

	protected RichTextWidget wGroupTaskName;
	protected string m_sGroupImageWidgetName = "GroupImage";
	protected string m_sGroupInnerImageWidgetName = "GroupIconInner";

	protected const string OPTIONS_COMBO_INVITE = "#AR-PlayerList_Invite";
	protected const string OPTIONS_COMBO_KICK = "#AR-DeployMenu_Groups_Kick";
	protected const string OPTIONS_COMBO_PROMOTE = "#AR-DeployMenu_Groups_Promote";
	protected const string LOCK_GROUP = "#AR-Player_Groups_Lock";
	protected const string UNLOCK_GROUP = "#AR-Player_Groups_Unlock";
	protected const string PRIVATE_GROUP = "#AR-Player_Groups_Private";
	protected const string PUBLIC_GROUP = "#AR-Player_Groups_Public";
	protected const string JOIN_GROUP = "#AR-DeployMenu_JoinGroup";
	protected const string REQUEST_JOIN_GROUP = "#AR-DeployMenu_RequestJoinGroup";
	protected const string CUSTOM_GROUP_NAME_FORMAT = "#AR-Player_Groups_CustomName_Format";

	protected const ResourceName GROUP_FLAG_SELECTION = "{7340FE3C6872C6D3}UI/layouts/Menus/GroupSlection/GroupFlagSelection.layout";

	protected ref array<SCR_PlayerTileButtonComponent> m_aPlayerComponentsList = {};
	protected SCR_PlayerTileButtonComponent m_PlayerTileComponent;
	protected static ref ScriptInvoker s_OnGroupButtonClicked = new ScriptInvoker();
	
	protected static ref ScriptInvokerInt m_OnPlayerTileFocus;
	protected static ref ScriptInvokerInt m_OnPlayerTileFocusLost;
	
	protected bool m_bIsPriority;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		m_ScrollComponent = SCR_HorizontalScrollAnimationComponent.Cast(w.FindAnyWidget("CallsignFrame").FindHandler(SCR_HorizontalScrollAnimationComponent));
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		super.OnClick(w, x, y, button);

#ifdef DEBUG_GROUPS
		SCR_GroupsManagerComponent.GetInstance().UpdateDebugUI();
#endif
		m_GroupComponent.SetSelectedGroupID(m_iGroupID);
		s_OnGroupButtonClicked.Invoke();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void InitiateGroupTile()
	{
		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupManager)
			return;

		m_GroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_GroupComponent)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;
		
		SocialComponent  socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return;


		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		group.GetOnPlayerLeaderChanged().Insert(SetupJoinGroupButton);
		group.GetOnJoinPrivateGroupRequest().Insert(SetupJoinGroupButton);
		
		//workaround for issues with playerControllerGroupsComponent, needs to be reworked with this whole script mess
		SCR_GroupSubMenu.Init();
		SCR_GroupSubMenu.GetOnJoingGroupRequestSent().Insert(SetupJoinGroupButton);

		m_TaskSystem = SCR_TaskSystem.GetInstance();
		if (m_TaskSystem)
		{
			m_TaskSystem.GetOnTaskAdded().Insert(OnTaskAdded);
			m_TaskSystem.GetOnTaskRemoved().Insert(OnTaskRemoved);
		}

		group.GetOnMemberStateChange().Insert(RefreshPlayers);

		RichTextWidget squadName = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Callsign"));
		if (squadName)
		{
			// This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
			squadName.SetText(SCR_GroupHelperUI.GetTranslatedGroupName(group));
		}

		RichTextWidget squadType = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Type"));
		Widget squadFrameType = GetRootWidget().FindAnyWidget("FrameType");
		if (squadType && squadFrameType)
		{
			string roleName = SCR_GroupHelperUI.GetRoleName(group);
			squadFrameType.SetVisible(!roleName.IsEmpty());
			squadType.SetText(roleName);
		}

		RichTextWidget frequency = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Frequency"));
		if (frequency)
			frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

		RichTextWidget playerCount = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		ImageWidget playerImageWidget = ImageWidget.Cast(GetRootWidget().FindAnyWidget("PlayerIcon"));

		if (playerCount)
			playerCount.SetText(group.GetPlayerCount().ToString() + "/" + group.GetMaxMembers());
		if (group.IsFull())
		{
			SetGroupInfoColor(m_GroupFullColor);
			playerCount.SetColor(UIColors.WARNING);
			playerImageWidget.SetColor(UIColors.WARNING);
		}
		else
		{
			playerCount.SetColor(Color.White);
			playerImageWidget.SetColor(Color.White);
		}

		SizeLayoutWidget groupImage = SizeLayoutWidget.Cast(GetRootWidget().FindAnyWidget("GroupImageMenu"));

		if (groupImage)
			SetGroupFlag(group, groupImage);

		ImageWidget background = ImageWidget.Cast(GetRootWidget().FindAnyWidget("Background"));

		if (background)
			background.SetVisible(m_GroupComponent.GetSelectedGroupID() == m_iGroupID);

		if (group.IsPlayerInGroup(playerController.GetPlayerId()))
		{
			squadName.SetColor(m_PlayerNameSelfColor);
			ImageWidget m_wPrivateIconImage = ImageWidget.Cast(GetRootWidget().FindAnyWidget("PrivateImage"));
			SetGroupInfoColor(m_GroupFullColor);
			if (m_wPrivateIconImage)
				m_wPrivateIconImage.SetColor(m_PlayerNameSelfColor);
		}
		
		SizeLayoutWidget privateIcon = SizeLayoutWidget.Cast(GetRootWidget().FindAnyWidget("PrivateSize"));
		if (privateIcon)
			privateIcon.SetVisible(group.IsPrivate());
		
		if (!m_ParentSubMenu)
			FindParentMenu();
		
		SetupSelectGroupFlagButton(group);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	//! \param[in] groupImage
	void SetGroupFlag(SCR_AIGroup group, Widget groupImage)
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(group.GetFaction());
		if (!scrFaction)
			return;

		SCR_GroupFlagImageComponent groupIcon = SCR_GroupFlagImageComponent.Cast(groupImage.FindHandler(SCR_GroupFlagImageComponent));
		if (!groupIcon)
			return;

		ResourceName flag = group.GetGroupFlag();

		if (flag.IsEmpty())
		{
			array<ResourceName> textures = {};
			scrFaction.GetGroupFlagTextures(textures);
			if (!scrFaction.GetGroupFlagImageSet().IsEmpty())
			{
				array<string> names = {};
				scrFaction.GetFlagNames(names);
				groupIcon.SetFlagButtonFromImageSet(names[0]);
			}
			else if (!textures.IsEmpty())
				groupIcon.SetImage(textures[0]);
			else
			{
				array<ResourceName> images = {};
				scrFaction.GetGroupFlagTextures(images);

				if (!scrFaction.GetGroupFlagImageSet().IsEmpty())
				{
					array<string> names = {};
					scrFaction.GetFlagNames(names);
					if (!names.IsEmpty())
						groupIcon.SetFlagButtonFromImageSet(names[0]);
				}
				else if (!images.IsEmpty())
					groupIcon.SetImage(images[0]);
				else
					groupIcon.SetImage(scrFaction.GetFactionFlag());
			}
		}
		else
		{
			if (group.GetFlagIsFromImageSet())
				groupIcon.SetFlagButtonFromImageSet(flag);
			else
				groupIcon.SetImage(flag);
		}

	}
	//------------------------------------------------------------------------------------------------
	//! \param[in] group
	void SetupSelectGroupFlagButton(SCR_AIGroup group)
	{
		if (!m_ParentSubMenu)
			return;

		Widget groupImage = m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupImage");
		if (!groupImage)
		return;

		SCR_GroupFlagImageComponent imageButton = SCR_GroupFlagImageComponent.Cast(groupImage.FindHandler(SCR_GroupFlagImageComponent));
		if (!imageButton)
			return;

		imageButton.SetVisible(true);
		imageButton.m_OnClicked.Insert(OnSelectGroupFlagButtonClicked);

		// disable group flag image editing when groups use the group role
		if (group.GetGroupRole() != SCR_EGroupRole.NONE)
			imageButton.SetEnabled(false);

		SCR_Faction scrFaction = SCR_Faction.Cast(m_GroupFaction);
		if (!scrFaction)
			return;
		
		ResourceName flag = group.GetGroupFlag();
		
		if (flag.IsEmpty())
		{
			array<ResourceName> textures = {};
			
			scrFaction.GetGroupFlagTextures(textures);
			
			if (!scrFaction.GetGroupFlagImageSet().IsEmpty())
			{
				array<string> names = {};
			
				scrFaction.GetFlagNames(names);
								
				imageButton.SetFlagButtonFromImageSet(names[0]);
			}
			else if (!textures.IsEmpty())
			{
				imageButton.SetImage(textures[0]);
			}
			else
			{
				array<ResourceName> images = {};
			
				scrFaction.GetGroupFlagTextures(images);
			
				if (!scrFaction.GetGroupFlagImageSet().IsEmpty())
					{
					array<string> names = {};
			
					scrFaction.GetFlagNames(names);
						
					if (!names.IsEmpty())
						imageButton.SetFlagButtonFromImageSet(names[0]);
				}
				else if (!images.IsEmpty())
				{
					imageButton.SetImage(images[0]);
				}
				else
				{
					imageButton.SetImage(scrFaction.GetFactionFlag());			
				}
			}
		}
		else 
		{
			if (group.GetFlagIsFromImageSet())
			{
				imageButton.SetFlagButtonFromImageSet(flag);
			}
			else
			{
				imageButton.SetImage(flag);				
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnSelectGroupFlagButtonClicked()
	{
		SCR_PlayerControllerGroupComponent playerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerComponent)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		if (!group.IsPlayerLeader(playerComponent.GetPlayerID()))
		{
			SCR_NotificationsComponent.SendLocal(ENotification.GROUPS_PLAYER_IS_NOT_LEADER);
			return;
		}
		
		SCR_Faction playerFaction = SCR_Faction.Cast(group.GetFaction());
		
		array<string> flagNames = {};
		array<ResourceName> flagTextures = {};
		playerFaction.GetFlagNames(flagNames);
		playerFaction.GetGroupFlagTextures(flagTextures);

		if (!flagNames.IsEmpty() || !flagTextures.IsEmpty())
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.GroupFlagDialog, DialogPriority.CRITICAL, 0, true);
		else
			SCR_NotificationsComponent.SendLocal(ENotification.GROUPS_NO_FLAGS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTaskAdded(notnull SCR_Task task)
	{
		RefreshPlayers();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTaskRemoved(notnull SCR_Task task)
	{
		RefreshPlayers();
	}

	//------------------------------------------------------------------------------------------------
	void RefreshPlayers()
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (!playerController)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SetupSelectGroupFlagButton(group);

		if (m_ParentSubMenu == null)
			FindParentMenu();

		m_aPlayerComponentsList.Clear();

		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;

		VerticalLayoutWidget leaderList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Leader"));
		if (!leaderList)
			return;

		RichTextWidget squadName = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailCallsign"));
		if (!squadName)
			return;

		RichTextWidget squadType = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Type"));
		if (!squadType)
			return;

		RichTextWidget description = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("Description"));
		if (!description)
			return;

		RichTextWidget frequency = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailFrequency"));
		if (!frequency)
			return;
		
		RichTextWidget groupDetailType = RichTextWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupDetailType"));
		if (!groupDetailType)
			return;

		SCR_PlayerControllerGroupComponent s_PlayerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!s_PlayerGroupController)
			return;
		
		SocialComponent socialComp = SocialComponent.Cast(playerController.FindComponent(SocialComponent));
		if (!socialComp)
			return;
		
		if (!s_PlayerGroupController.CanPlayerJoinGroup(playerController.GetPlayerId(), m_GroupManager.FindGroup(m_iGroupID)))
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);
		
		// This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
		squadName.SetText(SCR_GroupHelperUI.GetTranslatedGroupName(group));

		Widget groupTypeOverlay = m_ParentSubMenu.GetRootWidget().FindAnyWidget("GroupTypeOverlay");
		if (groupTypeOverlay)
		{
			string roleName = SCR_GroupHelperUI.GetRoleName(group);
			groupTypeOverlay.SetVisible(!roleName.IsEmpty());
			groupDetailType.SetText(roleName);
		}

		if (!group.GetCustomDescription().IsEmpty())
			description.SetText(group.GetCustomDescription());
		else
			description.SetText(string.Empty);

		CheckLeaderOptions();

		frequency.SetText(""+group.GetRadioFrequency()*0.001 + " #AR-VON_FrequencyUnits_MHz");

		Widget children = playerList.GetChildren();
		while (children)
		{
			playerList.RemoveChild(children);
			children = playerList.GetChildren();
		}

		children = leaderList.GetChildren();
		while (children)
		{
			leaderList.RemoveChild(children);
			children = leaderList.GetChildren();
		}

		array<int> playerIDs = group.GetPlayerIDs();
		Widget playerTile;

		int leaderID = group.GetLeaderID();
		if (leaderID >= 0)
		{
			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, leaderList);
			SetupPlayerTile(playerTile, leaderID);
		}

		foreach (int playerID : playerIDs)
		{
			if (playerID == leaderID)
				continue;

			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList);
			SetupPlayerTile(playerTile, playerID);
		}

		//disable this for now as it has issues with replication of aigroups
		//ShowAIsInGroup();

		SetupJoinGroupButton();
		SetupRemoveGroupButton();
		GetGame().GetWorkspace().SetFocusedWidget(GetRootWidget());
	}

	//------------------------------------------------------------------------------------------------
	void SetupJoinGroupButton()
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		SCR_AIGroup group;
		if (playerGroupController.GetSelectedGroupID() == -1)
		{
			group = m_GroupManager.FindGroup(m_iGroupID);
		}
		else
		{
			group = m_GroupManager.FindGroup(playerGroupController.GetSelectedGroupID());
		}

		if (!group)
			return;

		m_JoinGroupButton.m_OnActivated.Clear();

		if (group.IsPrivate())
		{
			m_JoinGroupButton.SetLabel(REQUEST_JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.RequestJoinPrivateGroup);
		}
		else
		{
			m_JoinGroupButton.SetLabel(JOIN_GROUP);
			m_JoinGroupButton.m_OnActivated.Insert(SCR_GroupSubMenu.JoinSelectedGroup);
		}

		array<int> denied = {};
		group.GetDeniedRequesters(denied);
		array<int> requesters = {};
		group.GetRequesterIDs(requesters);

		bool requesterDenied = denied.Contains(playerGroupController.GetPlayerID());
		bool requestSent = requesters.Contains(playerGroupController.GetPlayerID());

		if (
				requesterDenied ||
				requestSent ||
				SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander() || // block joining to other group when player is commander
				!m_GroupManager.FindGroup(m_iGroupID) ||
				!playerGroupController.CanPlayerJoinGroup( SCR_PlayerController.GetLocalPlayerId() ,m_GroupManager.FindGroup(m_iGroupID)))
			m_JoinGroupButton.SetEnabled(false);
		else
			m_JoinGroupButton.SetEnabled(true);

		SCR_Faction scrFaction = SCR_Faction.Cast(m_GroupFaction);
		if (!scrFaction)
			return;

		array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
		scrFaction.GetGroupRolePresetConfigs(groupRolePresetConfigs);

		// disable join group button by group role preset
		foreach (SCR_GroupRolePresetConfig preset : groupRolePresetConfigs)
		{
			if (preset.GetGroupRole() == group.GetGroupRole() && !preset.CanPlayerJoin())
			{
				m_JoinGroupButton.SetEnabled(false);
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Setup remove group button, set visibility by selected group
	void SetupRemoveGroupButton()
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupController)
			return;

		SCR_AIGroup group;
		if (playerGroupController.GetSelectedGroupID() == -1)
		{
			group = m_GroupManager.FindGroup(m_iGroupID);
		}
		else
		{
			group = m_GroupManager.FindGroup(playerGroupController.GetSelectedGroupID());
		}

		if (!group)
			return;

		int playerId = SCR_PlayerController.GetLocalPlayerId();

		if (playerGroupController.CanPlayerRemoveGroup(playerId, group))
			m_RemoveGroupButton.SetEnabled(true);
		else
			m_RemoveGroupButton.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	//TODO: setup should be taken care of by the player tile component
	void SetupPlayerTile(Widget playerTile, int playerID)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!m_GroupFaction || !m_GroupManager || !playerManager)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		SCR_GadgetManagerComponent gadgetManager;
		TextWidget playerName, playerFrequency;
		ImageWidget taskIcon, muteIcon, background, loadoutIcon;
		//string m_sTaskText;
		SizeLayoutWidget m_wTaskLayout;
		ButtonWidget playerButton;
		SCR_TaskExecutor taskExecutor;
		SCR_BasePlayerLoadout playerLoadout;
		Resource res;
		IEntityComponentSource source;
		BaseContainer container;
		SCR_EditableEntityUIInfo info;
		set<int> frequencies = new set<int>();
		
		playerName = TextWidget.Cast(playerTile.FindAnyWidget("PlayerName"));
		playerFrequency = TextWidget.Cast(playerTile.FindAnyWidget("Frequency"));
		m_wTaskLayout = SizeLayoutWidget.Cast(playerTile.FindAnyWidget("TaskLayout"));
		Widget wBackground = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconBackground"));
		Widget m_wOutline = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconOutline"));
		Widget m_wSymbol = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconSymbol"));
		muteIcon = ImageWidget.Cast(playerTile.FindAnyWidget("MuteIcon"));
		background = ImageWidget.Cast(playerTile.FindAnyWidget("Background"));
		loadoutIcon = ImageWidget.Cast(playerTile.FindAnyWidget("LoadoutIcon"));
		ImageWidget m_wIconSymbol = ImageWidget.Cast(playerTile.FindAnyWidget("TaskIconSymbol")); 
		ImageWidget platformIcon = ImageWidget.Cast(playerTile.FindAnyWidget("PlatformIcon"));

		playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
		if (!playerButton)
			return;

		m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
		m_PlayerTileComponent.SetTilePlayerID(playerID);

		SetupOptionsCombo(playerTile);

		playerName.SetText(SCR_PlayerNamesFilterCache.GetInstance().GetPlayerDisplayName(playerID));
		
		SCR_PlayerController playerCtrl = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		if (playerCtrl)
			playerCtrl.SetPlatformImageTo(playerID, platformIcon);

		ChimeraCharacter controlledEntity = ChimeraCharacter.Cast(playerManager.GetPlayerControlledEntity(playerID));
		if (controlledEntity)
		{
			gadgetManager = SCR_GadgetManagerComponent.Cast(controlledEntity.FindComponent(SCR_GadgetManagerComponent));
			SCR_Global.GetFrequencies(gadgetManager, frequencies);
			if (!frequencies.IsEmpty())
			{
				playerFrequency.SetText(SCR_FormatHelper.FormatFrequencies(frequencies));
				background.SetOpacity(0.85);
			}
		}

		if (playerID == GetGame().GetPlayerController().GetPlayerId())
			playerName.SetColor(m_PlayerNameSelfColor);
		
//		if (m_TaskSystem)
//		{
//			PlayerController m_PlayerController = GetGame().GetPlayerController();
//			if (!m_PlayerController) 
//				return;
//			
//			// TODO: check for good const usage
//			const ResourceName TaskIconImageset = "{10C0A9A305E8B3A4}UI/Imagesets/Tasks/Task_Icons.imageset";	
//			SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
//			if (!factionManager) 
//				return;
//			
//			Faction playerFaction = factionManager.GetPlayerFaction(m_PlayerController.GetPlayerId());
//			if (!playerFaction) 
//				return;
//			
//			Faction localPlayerFaction = factionManager.GetLocalPlayerFaction();
//			if (!localPlayerFaction) 
//				return;
//			
//			PlayerController playerController = GetGame().GetPlayerController();	
//			if (!playerController)
//				return;
//			
//			m_sPlyFactionAffilComp = SCR_PlayerFactionAffiliationComponent.Cast(playerController.FindComponent(SCR_PlayerFactionAffiliationComponent));
//			if (!m_sPlyFactionAffilComp)
//				return;
//
//			SCR_Faction m_sfactionCol = SCR_Faction.Cast(m_sPlyFactionAffilComp.GetAffiliatedFaction());
//			if (!m_sfactionCol)
//				return;
//
//			//check for task and adjust the icon visibility
//			taskExecutor = SCR_TaskExecutor.FromPlayerID(playerID);
//			if(!taskExecutor)
//				return;	
//						
//			SCR_EditorTask gmTask = SCR_EditorTask.Cast(m_TaskSystem.GetTaskAssignedTo(taskExecutor));
//			SCR_Task baseTask = m_TaskSystem.GetTaskAssignedTo(taskExecutor);
//			SCR_CampaignMilitaryBaseTaskEntity task = SCR_CampaignMilitaryBaseTaskEntity.Cast(m_TaskSystem.GetTaskAssignedTo(taskExecutor));
//			
//			if(gmTask || baseTask || task && playerFaction == localPlayerFaction)
//			{ 
//				m_wTaskLayout.SetVisible(true);
//				m_wTaskLayout.SetColor(localPlayerFaction.GetFactionColor());
//				wGroupTaskName = RichTextWidget.Cast(playerTile.FindAnyWidget("TaskDescription"));
//		
//				if (task) // When task is assigned and when player has the same faction with entry faction for campaign.
//				{	
//					m_wIconSymbol.LoadImageFromSet(0, TaskIconImageset, task.GetTaskIconSetName());
//					m_sTaskText = task.GetTaskName();
//				}
//
//				if (gmTask) // When task is assigned and when player has the same faction with entry faction for Game Master.
//				{
//					m_wIconSymbol.LoadImageFromSet(0, TaskIconImageset, gmTask.GetTaskIconSetName());
//					m_sTaskText = gmTask.GetTaskName();
//					wGroupTaskName.SetTextFormat(gmTask.GetTaskName(), gmTask.GetLocationName());
//				}
//			}
//		}
		
		
		//set the state of mute
		PlayerController pc = GetGame().GetPlayerController();
		// Using another PlayerController variable to not alter rest of code
		if (muteIcon && pc)
		{
			SocialComponent sc = SocialComponent.Cast(pc.FindComponent(SocialComponent));
			if (sc && sc.IsRestricted(playerID, EUserInteraction.VoiceChat))
			{
				muteIcon.SetColor(m_PlayerNameSelfColor);
				muteIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, "sound-off");
			}
		} 
		

		//look for loadout and set the appropriate icon
		if (loadoutIcon)
		{
			SCR_LoadoutManager loadoutManager = GetGame().GetLoadoutManager();
			if (loadoutManager)
				playerLoadout = loadoutManager.GetPlayerLoadout(playerID);			
			
			if (playerLoadout)
			{
				res = Resource.Load(playerLoadout.GetLoadoutResource());
				source = SCR_BaseContainerTools.FindComponentSource(res, "SCR_EditableCharacterComponent");
				if (source)
				{
					container = source.GetObject("m_UIInfo");
					info = SCR_EditableEntityUIInfo.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
					info.SetIconTo(loadoutIcon);
				}
			}

			//set leader icon if given player is a leader
			if (group.IsPlayerLeader(playerID))
			{
				m_LeaderInfo.SetIconTo(loadoutIcon);
				loadoutIcon.SetColor(m_PlayerNameSelfColor);
			}

			//set badge color
			Widget badge = playerTile.FindAnyWidget("PlayerBadge");
			if (badge)
			{
				Color badgeColor = m_GroupFaction.GetFactionColor();
				if (group.IsPlayerLeader(playerID))
					badgeColor = (m_PlayerNameSelfColor);

				SetBadgeColor(badge, badgeColor);
			}
		}

		m_PlayerTileComponent.GetOnTileFocus().Insert(OnPlayerTileFocus);
		m_PlayerTileComponent.GetOnTileFocusLost().Insert(OnPlayerTileFocusLost);

		m_aPlayerComponentsList.Insert(m_PlayerTileComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsPriority(bool isPriority)
	{
		m_bIsPriority = isPriority;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsPriority()
	{
		return m_bIsPriority;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocus(int id)
	{
		if (m_OnPlayerTileFocus)
			m_OnPlayerTileFocus.Invoke(id);
		
		 DisableConfirmButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnPlayerTileFocusLost(int id)
	{
		if (m_OnPlayerTileFocusLost)
			m_OnPlayerTileFocusLost.Invoke(id);
		
		EnableConfirmButton();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupInfoColor(Color groupColor)
	{
		ImageWidget frequencyImage = ImageWidget.Cast(GetRootWidget().FindAnyWidget("FrequencyImage"));
		ImageWidget playerIcon = ImageWidget.Cast(GetRootWidget().FindAnyWidget("PlayerIcon"));
		TextWidget callsign = TextWidget.Cast(GetRootWidget().FindAnyWidget("Callsign"));
		TextWidget frequency = TextWidget.Cast(GetRootWidget().FindAnyWidget("Frequency"));
		TextWidget playerCount = TextWidget.Cast(GetRootWidget().FindAnyWidget("PlayerCount"));
		
		if (!frequencyImage || !playerIcon || !callsign || !frequency || !playerCount)
			return;
		
		frequencyImage.SetColor(groupColor);
		playerIcon.SetColor(groupColor);
		callsign.SetColor(groupColor);
		frequency.SetColor(groupColor);
		playerCount.SetColor(groupColor);
	}

	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return m_iGroupID;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);

		if (m_GroupComponent.GetSelectedGroupID() == -1)
			m_GroupComponent.SetSelectedGroupID(m_iGroupID);
		
		EInputDeviceType deviceType = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (deviceType == EInputDeviceType.GAMEPAD || deviceType == EInputDeviceType.KEYBOARD)
		{
			m_GroupComponent.SetSelectedGroupID(m_iGroupID);
			RefreshPlayers();
		}
		
		if (!m_ScrollComponent.GetContentFitX())
			m_ScrollComponent.AnimationStart();
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_ScrollComponent.IsAnimating())
		{
			m_ScrollComponent.AnimationStop();
			m_ScrollComponent.ResetPosition();
		}
		
		return super.OnFocusLost(w, x, y);
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupID(int id)
	{
		m_iGroupID = id;
	}

	//------------------------------------------------------------------------------------------------
	Faction GetGroupFaction()
	{
		return m_GroupFaction;
	}

	//------------------------------------------------------------------------------------------------
	void SetGroupFaction(Faction groupFaction)
	{
		m_GroupFaction = groupFaction;
	}

	//------------------------------------------------------------------------------------------------
	void SetJoinGroupButton(SCR_InputButtonComponent joinGroupButton)
	{
		m_JoinGroupButton = joinGroupButton;
	}

	//------------------------------------------------------------------------------------------------
	void SetRemoveGroupButton(SCR_InputButtonComponent removeGroupButton)
	{
		m_RemoveGroupButton = removeGroupButton;
	}

	//------------------------------------------------------------------------------------------------
	void SetupOptionsCombo(Widget playerTile)
	{
		ButtonWidget playerOptionsButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerOptions"));
		if (!playerOptionsButton)
			return;

		SCR_ComboBoxComponent playerOptionsCombo = SCR_ComboBoxComponent.Cast(playerOptionsButton.FindHandler(SCR_ComboBoxComponent));
		if (!playerOptionsCombo)
			return;

		m_PlayerTileComponent.SetOptionsComboBoxComponent(playerOptionsCombo);

		int playerID = GetGame().GetPlayerController().GetPlayerId();
		SCR_AIGroup group = m_GroupManager.GetPlayerGroup(playerID);

		if (group && m_iGroupID != group.GetGroupID() && m_PlayerTileComponent.GetTilePlayerID() != -1)
			m_PlayerTileComponent.GetOptionsComboComponent().AddItem(OPTIONS_COMBO_INVITE);


		group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		if (group.IsPlayerLeader(playerID) && playerID != m_PlayerTileComponent.GetTilePlayerID() && m_PlayerTileComponent.GetTilePlayerID() != -1)
		{
			m_PlayerTileComponent.GetOptionsComboComponent().AddItem(OPTIONS_COMBO_KICK);			
			m_PlayerTileComponent.GetOptionsComboComponent().AddItem(OPTIONS_COMBO_PROMOTE);
		}

		if (m_PlayerTileComponent.GetOptionsComboComponent().GetNumItems() == 0 || group.GetGroupRole() != SCR_EGroupRole.NONE)
		{
			m_PlayerTileComponent.SetOpacity(0.3);
			m_PlayerTileComponent.GetOptionsComboComponent().SetVisible(false);
			m_PlayerTileComponent.GetOptionsComboComponent().SetEnabled(false);
		}

		m_PlayerTileComponent.GetOptionsComboComponent().m_OnOpened.Insert(DisableConfirmButton);
		m_PlayerTileComponent.GetOptionsComboComponent().m_OnClosed.Insert(EnableConfirmButton);

		m_PlayerTileComponent.GetOptionsComboComponent().m_OnChanged.Insert(OnComboBoxConfirm);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxConfirm(SCR_ComboBoxComponent combo, int index)
	{
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();

		if (!playerGroupController)
			return;

		int playerID;
		for (int i = m_aPlayerComponentsList.Count()-1; i >= 0; i--)
		{
			if (m_aPlayerComponentsList[i].GetOptionsComboComponent() == combo)
				playerID = m_aPlayerComponentsList[i].GetTilePlayerID();
		}

		switch (combo.GetItemName(index))
		{
			case OPTIONS_COMBO_INVITE:
			{
				playerGroupController.InvitePlayer(playerID);
				break;
			}
			case OPTIONS_COMBO_KICK:
			{
				playerGroupController.RequestKickPlayer(playerID);
				break;
			}
			case OPTIONS_COMBO_PROMOTE:
			{
				playerGroupController.RequestPromoteLeader(playerID);
				break;
			}
		}

		combo.SetCurrentItem(-1, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckLeaderOptions()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		Widget privateChecker = m_ParentSubMenu.GetRootWidget().FindAnyWidget("PrivateCheckLayout");
		if (!privateChecker)
			return;

		Widget privateCheckerButton = m_ParentSubMenu.GetRootWidget().FindAnyWidget("PrivateChecker");
		if (!privateCheckerButton)
			return;

		SCR_ButtonCheckerComponent checkerComp = SCR_ButtonCheckerComponent.Cast(privateCheckerButton.FindHandler(SCR_ButtonCheckerComponent));
		if (!checkerComp)
			return;

		ButtonWidget nameChangeButton = ButtonWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("ChangeNameButton"));
		if (!nameChangeButton)
			return;

		if (!group.IsPlayerLeader(GetGame().GetPlayerController().GetPlayerId()) || !m_GroupManager.CanPlayersChangeAttributes() ||
			!group.IsPrivacyChangeable())
		{
			privateChecker.SetOpacity(0);
			privateChecker.SetEnabled(false);
			nameChangeButton.SetOpacity(0);
			nameChangeButton.SetEnabled(false);
			return;
		}

		checkerComp.SetToggled(group.IsPrivate(), instant: true);
		if (group.IsPrivate())
			checkerComp.SetText(PRIVATE_GROUP);
		else
			checkerComp.SetText(PUBLIC_GROUP);

		privateChecker.SetOpacity(1);
		privateChecker.SetEnabled(true);
		nameChangeButton.SetOpacity(1);
		nameChangeButton.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvoker GetOnGroupTileClicked()
	{
		return s_OnGroupButtonClicked;
	}

	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnPlayerTileFocus()
	{
		if (!m_OnPlayerTileFocus)
			m_OnPlayerTileFocus = new ScriptInvokerInt();
		
		return m_OnPlayerTileFocus;
	}
	
	//------------------------------------------------------------------------------------------------
	static ScriptInvokerInt GetOnPlayerTileFocusLost()
	{
		if (!m_OnPlayerTileFocusLost)
			m_OnPlayerTileFocusLost = new ScriptInvokerInt();
		
		return m_OnPlayerTileFocusLost;
	}
	
	//------------------------------------------------------------------------------------------------
	void EnableConfirmButton()
	{
		SetConfirmButtonStatus(true);
	}

	//------------------------------------------------------------------------------------------------
	void DisableConfirmButton()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager && inputManager.GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
			SetConfirmButtonStatus(false);
	}

	//------------------------------------------------------------------------------------------------
	void SetConfirmButtonStatus(bool status)
	{
//		SCR_RespawnSubMenuBase subMenu = SCR_RespawnSubMenuBase.Cast(m_ParentSubMenu);
//		if (!subMenu)
//			return;

//		subMenu.SetConfirmButtonEnabled(status);
	}

	//------------------------------------------------------------------------------------------------
	void SetBadgeColor(Widget badge, Color color)
	{
		if (!badge)
			return;

		ImageWidget badgeTop, badgeMiddle, badgeBottom;

		badgeTop = ImageWidget.Cast(badge.FindAnyWidget("BadgeTop"));
		badgeMiddle = ImageWidget.Cast(badge.FindAnyWidget("BadgeMiddle"));
		badgeBottom = ImageWidget.Cast(badge.FindAnyWidget("BadgeBottom"));

		if (!badgeMiddle || !badgeTop || !badgeBottom)
			return;

		badgeTop.SetColor(color);
		badgeMiddle.SetColor(color);
		badgeBottom.SetColor(color);
	}

	//------------------------------------------------------------------------------------------------
	void ShowAIsInGroup()
	{
		SCR_AIGroup group = m_GroupManager.FindGroup(m_iGroupID);
		if (!group)
			return;

		VerticalLayoutWidget playerList = VerticalLayoutWidget.Cast(m_ParentSubMenu.GetRootWidget().FindAnyWidget("PlayerList"));
		if (!playerList)
			return;

		Widget playerTile;
		TextWidget playerName;
		ButtonWidget playerButton;

		SCR_AIGroup slaveGroup = group.GetSlave();
		if (!slaveGroup)
			return;

		array<SCR_ChimeraCharacter> aiMembers = slaveGroup.GetAIMembers();
		SCR_CharacterIdentityComponent identityComponent;

		foreach (SCR_ChimeraCharacter AIcharacter : aiMembers)
		{
			playerTile = GetGame().GetWorkspace().CreateWidgets(m_textLayout, playerList);
			playerName = TextWidget.Cast(playerTile.FindAnyWidget("PlayerName"));

			identityComponent = SCR_CharacterIdentityComponent .Cast(AIcharacter.FindComponent(SCR_CharacterIdentityComponent));
			if (!identityComponent)
				return;
			string name;
			array<string> nameParams = {};
			identityComponent.GetFormattedFullName(name, nameParams);
			playerName.SetTextFormat(name, nameParams[0], nameParams[1], nameParams[2]);

			playerButton = ButtonWidget.Cast(playerTile.FindAnyWidget("PlayerButton"));
			if (!playerButton)
				return;

			m_PlayerTileComponent = SCR_PlayerTileButtonComponent.Cast(playerButton.FindHandler(SCR_PlayerTileButtonComponent));
			if (!m_PlayerTileComponent)
				return;

			m_PlayerTileComponent.SetTilePlayerID(-1);

			m_PlayerTileComponent.SetCharacter(AIcharacter);

			//set badge color
			Widget badge = playerTile.FindAnyWidget("PlayerBadge");

			if (badge)
				SetBadgeColor(badge, m_GroupFaction.GetFactionColor());
			
			SetupOptionsCombo(playerTile);
		}
	}
}
