//------------------------------------------------------------------------------------------------
//! Component responsible for handling group requests and visualization in deploy menu.
class SCR_GroupRequestUIComponent : SCR_DeployRequestUIBaseComponent
{
	[Attribute("GroupList", desc: "Container for available groups' buttons")]
	protected string m_sGroupList;
	protected Widget m_wGroupList;

	[Attribute("{848C61DDE45501B2}UI/layouts/Menus/DeployMenu/GroupButton.layout", desc: "Layout for group button, has to have SCR_GroupButton attached to it.")]
	protected ResourceName m_sGroupButton;
	
	[Attribute("{59F46BD8645E8549}UI/layouts/Menus/DeployMenu/CreateGroupButton.layout")]
	protected ResourceName m_sNewGroupButton;
	protected Widget m_wNewGroupButton;

	[Attribute("1")]
	protected bool m_bCreateNewGroupButton;
	
	protected SCR_GroupsManagerComponent m_GroupManager;
	protected SCR_PlayerControllerGroupComponent m_PlyGroupComp;
	protected ref SCR_GroupFlagImageComponent m_GroupFlagImage;
	protected Widget m_wGroupFlagButton;
	protected ScriptedWidgetEventHandler m_GroupFlagHandler;
	protected Faction m_PlyFaction;
	protected string m_sFlagName;
	protected int m_iShownGroupId = -1;
	
	protected ref ScriptInvoker<SCR_GroupButton> m_OnPlayerGroupJoined;
	protected ref ScriptInvoker<int> m_OnLocalPlayerGroupJoined;	
	protected ImageWidget m_wGroupIcon;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_GroupManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupManager || !m_GroupManager.IsActive())
		{
			m_bEnabled = false;
			return;
		}

		m_wExpandButtonName = TextWidget.Cast(w.FindAnyWidget(m_sExpandButtonName));
		m_wExpandButtonNameFreq = TextWidget.Cast(w.FindAnyWidget("freq"));
		m_wExpandButtonIcon = ImageWidget.Cast(w.FindAnyWidget(m_sExpandButtonIcon));
		m_wGroupFlagButton = w.FindAnyWidget("GroupFlagImage");

		m_PlyGroupComp = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlyGroupComp)
			return;
		
		SetGroupFlags();
		
		m_GroupManager.GetOnPlayableGroupCreated().Insert(AddGroup);		
		m_GroupManager.GetOnPlayableGroupRemoved().Insert(RemoveGroup);

		SCR_AIGroup.GetOnPlayerRemoved().Insert(UpdateGroupPlayers);		
		SCR_AIGroup.GetOnPlayerAdded().Insert(UpdateGroupPlayers);
		SCR_AIGroup.GetOnPrivateGroupChanged().Insert(UpdateGroupPrivacy);
		SCR_AIGroup.GetOnCustomNameChanged().Insert(UpdateGroupNames);
		SCR_AIGroup.GetOnFrequencyChanged().Insert(UpdateGroupFrequency);
		SCR_AIGroup.GetOnFlagSelected().Insert(UpdateGroupFlag);
		
		m_PlyGroupComp.GetOnGroupChanged().Insert(UpdateLocalPlayerGroup);

		m_wGroupList = w.FindAnyWidget(m_sGroupList);

		m_wExpandButton = w.FindAnyWidget(m_sExpandButton);
		if (m_wExpandButton && m_wExpandButton.IsVisible())
		{
			SCR_ButtonBaseComponent expandBtn = SCR_ButtonBaseComponent.Cast(m_wExpandButton.FindHandler(SCR_ButtonBaseComponent));
			expandBtn.m_OnClicked.Insert(ToggleCollapsed);
			SetPlayerGroup(m_PlyGroupComp.GetPlayersGroup());
			GetOnListCollapse().Insert(OnListExpand);
		}
	}

	protected override void ToggleCollapsed()
	{
		if (m_wExpandButton && m_wExpandButton.IsVisible())
		{
			bool visible = !IsExpanded();
			SetExpanded(visible);
			GetOnListCollapse().Invoke(this, visible);
		}
	}
	
	override void SetExpanded(bool expanded)
	{
		m_wGroupList.SetVisible(expanded);
	}	
	
	protected override bool IsExpanded()
	{
		return m_wGroupList.IsVisible();
	}

	override void HandlerDeattached(Widget w)
	{
		if (m_GroupManager)
		{
			m_GroupManager.GetOnPlayableGroupCreated().Remove(AddGroup);
			m_GroupManager.GetOnPlayableGroupRemoved().Remove(RemoveGroup);
		}

		SCR_AIGroup.GetOnPlayerRemoved().Remove(UpdateGroupPlayers);
		SCR_AIGroup.GetOnPlayerAdded().Remove(UpdateGroupPlayers);
		SCR_AIGroup.GetOnPrivateGroupChanged().Remove(UpdateGroupPrivacy);
		SCR_AIGroup.GetOnCustomNameChanged().Remove(UpdateGroupNames);
		SCR_AIGroup.GetOnFrequencyChanged().Remove(UpdateGroupFrequency);		
		SCR_AIGroup.GetOnFlagSelected().Remove(UpdateGroupFlag);
		
		if (m_PlyGroupComp)
			m_PlyGroupComp.GetOnGroupChanged().Remove(UpdateLocalPlayerGroup);
	}
	
	// Function that manages group flags, setting the correct icon to player local group
	protected void SetGroupFlags()
	{
		if (!m_wGroupFlagButton || !m_PlyGroupComp)
			return;

		SCR_AIGroup group = m_PlyGroupComp.GetPlayersGroup();
		if (!group)
			return;

		ResourceName flag = group.GetGroupFlag();
		if (!flag)
			return;

		m_GroupFlagImage = SCR_GroupFlagImageComponent.Cast(m_wGroupFlagButton.FindHandler(SCR_GroupFlagImageComponent));
		if (!m_GroupFlagImage)
			return;

		m_GroupFlagImage.SetFlagButtonFromImageSet(flag);
	}
	
	//! Update the group widget when player joins/leaves group.
	protected void UpdateGroupPlayers(SCR_AIGroup group, int pid)
	{
		// go through shown groups and update the relevant one
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
			if (!groupBtn)
				continue;

			bool canJoinGroup = m_PlyGroupComp.CanPlayerJoinGroup(GetGame().GetPlayerController().GetPlayerId(), group);
			if (!canJoinGroup && group == GetPlayerGroup())
				canJoinGroup = true;

			if (groupBtn && groupBtn.GetGroupId() == group.GetGroupID())
				groupBtn.UpdateGroup(canJoinGroup);

			groupBtn.UpdateButtonAvailability(m_PlyGroupComp);
		}

		GetOnPlayerGroupJoined().Invoke(group, pid);
		UpdateNewGroupButton();
	}
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//! Update the group widget relevant to local player.
	protected void UpdateLocalPlayerGroup(int groupId)
	{
		m_iShownGroupId = groupId;
		SCR_AIGroup group = m_GroupManager.FindGroup(groupId);
		GetOnLocalPlayerGroupJoined().Invoke(group);

		bool isPlayerGroup = (group == GetPlayerGroup());
		if (isPlayerGroup)
		{
			foreach (SCR_DeployButtonBase btn : m_aButtons)
			{
				SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
				if (groupBtn)
				{
					if (groupBtn.GetGroupId() == groupId)
					{
						groupBtn.HideTooltip();
						groupBtn.UpdateGroup();
						groupBtn.UpdateButtonAvailability(m_PlyGroupComp);
					}

					groupBtn.SetSelected(groupBtn.GetGroupId() == groupId);
				}
			}

			UpdateNewGroupButton();
			SetGroupFlags();
			GetGame().GetCallqueue().CallLater(SetPlayerGroup, 100, false, group); // call later because of group name initialization
		}
	}
	//---- REFACTOR NOTE END ----
	//! Set group private.
	protected void UpdateGroupPrivacy(int groupId, bool isPrivate)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
			if (!groupBtn || (groupBtn.GetGroupId() != groupId))
				continue;

			groupBtn.UpdateGroupPrivacy(isPrivate);
			groupBtn.UpdateButtonAvailability(m_PlyGroupComp);
			break;
		}

		UpdateNewGroupButton();
	}

	//! Set group name.
	protected void UpdateGroupNames()
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
			if (groupBtn)
			{
				groupBtn.UpdateGroupName();
				if (m_wExpandButtonName && groupBtn.GetGroup() == GetPlayerGroup())
					SCR_GroupButton.SGetGroupName(groupBtn.GetGroup(), m_wExpandButtonName,m_wExpandButtonNameFreq); // update group name in map's group selector
			}
		}
	}

	//! Set group frequency.
	protected void UpdateGroupFrequency()
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
			if (groupBtn)
				groupBtn.UpdateGroupFrequency();
		}		
	}

	//! Set group flag.
	protected void UpdateGroupFlag()
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
			if (groupBtn)
				groupBtn.UpdateGroupFlag();
		}
	}

	//! Update the expandable button name.
	void SetPlayerGroup(SCR_AIGroup group)
	{
		if (m_wExpandButtonName && group)
			SCR_GroupButton.SGetGroupName(group, m_wExpandButtonName, m_wExpandButtonNameFreq);
	}

	//! Show groups available for given faction.
	void ShowAvailableGroups(notnull Faction faction)
	{
		if (!m_wGroupList || !m_GroupManager)
			return;
		
		if (!m_wExpandButtonName) // todo@lk: crate IsExpandable property or something like that
			m_wGroupList.SetVisible(true);
		m_PlyFaction = faction;

		ClearGroupList();

		array<SCR_AIGroup> playableGroups = m_GroupManager.GetSortedPlayableGroupsByFaction(faction);
		if (!playableGroups)
			return;
#ifdef DEPLOY_MENU_DEBUG
		PrintFormat("ShowAvailableGroups() for %1", faction.GetFactionKey());
#endif
		int groupCount = playableGroups.Count();

		for (int i = 0; i < groupCount; ++i)
		{
			AddGroup(playableGroups[i])
		}
		
		CreateNewGroupButton();
	}

	//------------------------------------------------------------------------------------------------
	void ShowGroupSelector(bool show)
	{
		m_wRoot.SetVisible(show);
	}

	//! Joins an automatically selected group
	void JoinGroupAutomatically()
	{
		if (!m_GroupManager)
			return;

		SCR_AIGroup playerGroup = m_GroupManager.GetFirstNotFullForFaction(m_PlyFaction, respectPrivate: true);
		if (!playerGroup)
		{
			RequestNewGroup();
		}
		else
		{
			m_PlyGroupComp.SetSelectedGroupID(playerGroup.GetGroupID());
			m_PlyGroupComp.RequestJoinGroup(m_PlyGroupComp.GetSelectedGroupID());		
		}
	}

	//! Removes the group button from the list.
	protected void RemoveGroup(SCR_AIGroup group)
	{
		if (!m_wGroupList)
			return;

		Widget child = m_wGroupList.GetChildren();
		while (child)
		{
			SCR_GroupButton comp = SCR_GroupButton.Cast(child.FindHandler(SCR_GroupButton));
			if (!comp)
				comp = SCR_GroupButton.Cast(child.FindAnyWidget("Button").FindHandler(SCR_GroupButton));

			if (comp && comp.GetGroupId() == group.GetGroupID())
			{
				child.RemoveFromHierarchy();
				break;
			}

			child = child.GetSibling();
		}

		UpdateNewGroupButton();	
	}

	//! Creates a button which handles creating of a new group.
	protected void CreateNewGroupButton()
	{
		if (!m_bCreateNewGroupButton)
			return;

		if (m_wNewGroupButton)
			m_wNewGroupButton.RemoveFromHierarchy();

		m_wNewGroupButton = GetGame().GetWorkspace().CreateWidgets(m_sNewGroupButton, m_wGroupList);
		SCR_DeployButtonBase handler = SCR_DeployButtonBase.Cast(m_wNewGroupButton.FindAnyWidget("Button").FindHandler(SCR_DeployButtonBase));
		handler.m_OnClicked.Insert(OnNewGroupButtonClicked);
		UpdateNewGroupButton();
	}

	//! Updates visibility of the button for creating a new group.
	protected void UpdateNewGroupButton()
	{
		if (m_wNewGroupButton)
			m_wNewGroupButton.SetVisible(m_GroupManager.CanCreateNewGroup(m_PlyFaction));	
	}

	//! Requests creation of a new group.
	protected void RequestNewGroup()
	{
		// copypasta from marian's code
		m_PlyGroupComp.SetSelectedGroupID(-1);
		m_PlyGroupComp.RequestCreateGroup();
	}

	//! Adds a new group button into the list.
	protected void AddGroup(SCR_AIGroup group)
	{
		if (group.GetFaction() != m_PlyFaction)
			return;

		UpdateNewGroupButton();

		bool create = true;
		Widget child = m_wGroupList.GetChildren();
		while (child)
		{
			SCR_GroupButton comp = SCR_GroupButton.Cast(child.FindHandler(SCR_GroupButton));
			if (comp && comp.GetGroupId() == group.GetGroupID())
			{
				create = false;
				break;
			}

			child = child.GetSibling();
		}

		if (!create)
			return;

		Widget btnW = GetGame().GetWorkspace().CreateWidgets(m_sGroupButton, m_wGroupList);

		SCR_GroupButton btnComp;
		if (m_bUseListFromButton)
			btnComp = SCR_GroupButton.Cast(btnW.FindAnyWidget("Button").FindHandler(SCR_GroupButton));
		else
			btnComp = SCR_GroupButton.Cast(btnW.FindHandler(SCR_GroupButton));
		btnComp.SetGroup(group);
		btnComp.SetSelected(m_PlyGroupComp.GetGroupID() == group.GetGroupID());
		bool canJoinGroup = m_PlyGroupComp.CanPlayerJoinGroup(GetGame().GetPlayerController().GetPlayerId(), group);
		// Have to use CallLater for group attributes to be correctly synced on client before updating the button
		GetGame().GetCallqueue().CallLater(UpdateGroupButton, 1, false, btnComp, canJoinGroup);
		btnComp.m_OnClicked.Insert(RequestJoinGroup);

		btnComp.m_OnFocus.Insert(OnButtonFocused);
		btnComp.m_OnMouseEnter.Insert(OnButtonFocused);	

		btnComp.m_OnMouseLeave.Insert(OnMouseLeft);

		m_aButtons.Insert(btnComp);
		UpdateNewGroupButton();
	}

	//------------------------------------------------------------------------------------------------
	private void UpdateGroupButton(SCR_GroupButton groupButton, bool canJoinGroup)
	{
		groupButton.UpdateGroup(canJoinGroup);
	}

	//! Called when the group button is focused.
	protected void OnButtonFocused(Widget w)
	{
		SCR_GroupButton btn = SCR_GroupButton.Cast(w.FindHandler(SCR_GroupButton));
		if (!btn)
			return;

		SCR_AIGroup group = m_GroupManager.FindGroup(btn.GetGroupId());
		btn.SetTooltipAvailable(group != GetPlayerGroup());
		m_OnButtonFocused.Invoke(group);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNewGroupButtonClicked()
	{
		if (!m_PlyFaction)
			return;

		SCR_Faction scrFaction = SCR_Faction.Cast(m_PlyFaction);
		if (!scrFaction)
			return;

		// checks if the group role config is set, if it's set it will open the CreateGroupSettingsDialog
		// otherwise it will create a new group without a role
		if (scrFaction.IsGroupRolesConfigured())
			GetGame().GetMenuManager().OpenDialog(ChimeraMenuPreset.CreateGroupSettingsDialog);
		else
			RequestNewGroup();
	}

	//! Sends a request for joining a group.
	protected void RequestJoinGroup(notnull SCR_GroupButton groupBtn)
	{
		// block joining to other group when player is commander
		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			return;

		m_PlyGroupComp.SetSelectedGroupID(groupBtn.GetGroupId());
		m_PlyGroupComp.RequestJoinGroup(m_PlyGroupComp.GetSelectedGroupID());

		groupBtn.SetSelected(true);
		
		Widget child = m_wGroupList.GetChildren();
		while (child)
		{
			SCR_GroupButton comp = SCR_GroupButton.Cast(child.FindHandler(SCR_GroupButton));
			if (comp && comp != groupBtn)
			{
				comp.SetSelected(false);
			}

			child = child.GetSibling();
		}
	}

	//! Gets relevant group button.
	SCR_GroupButton GetGroupButton(SCR_AIGroup group)
	{
		foreach (SCR_DeployButtonBase btn : m_aButtons)
		{
			SCR_GroupButton groupBtn = SCR_GroupButton.Cast(btn);
			if (groupBtn && groupBtn.GetGroup() == group)
				return groupBtn;
		}

		return null;
	}

	//! Return current group list.
	Widget GetGroupList()
	{
		return m_wGroupList;
	}

	//! Return local player's group.
	SCR_AIGroup GetPlayerGroup()
	{
		return m_PlyGroupComp.GetPlayersGroup();
	}

	//! Set current group list.
	override void SetListWidget(Widget list)
	{
		if (m_wGroupList)
			ClearGroupList();
		m_wGroupList = list;
	}

	//! Clear the group list.
	protected void ClearGroupList()
	{
		Widget child = m_wGroupList.GetChildren();
		while (child)
		{
			Widget sibling = child.GetSibling();
			delete child;
			child = sibling;
		}
		
		m_aButtons.Clear();
	}

	//! Gets id of a currently displayed group.
	int GetShownGroupId()
	{
		return m_iShownGroupId;
	}

	//! Set id of a currently displayed group.
	void SetShownGroupId(int groupId)
	{
		m_iShownGroupId = groupId;
	}

	ScriptInvoker GetOnPlayerGroupJoined()
	{
		if (!m_OnPlayerGroupJoined)
			m_OnPlayerGroupJoined = new ScriptInvoker();
		
		return m_OnPlayerGroupJoined;
	}
	
	ScriptInvoker GetOnLocalPlayerGroupJoined()
	{
		if (!m_OnLocalPlayerGroupJoined)
			m_OnLocalPlayerGroupJoined = new ScriptInvoker();
		
		return m_OnLocalPlayerGroupJoined;
	}
};

//------------------------------------------------------------------------------------------------
//! Component attached to the group button.
class SCR_GroupButton : SCR_DeployButtonBase
{
	[Attribute("GroupName")]
	protected string m_sGroupName;
	protected TextWidget m_wGroupName;

	[Attribute("FrequencyText")]
	protected string m_sFreq;
	protected TextWidget m_wFreq;

	[Attribute("PlayerCountText")]
	protected string m_sPlayerCount;
	protected TextWidget m_wPlayerCount;
	
	[Attribute("PlayerIcon")]
	
	protected string m_sPlayerIcon;
	protected ImageWidget m_wPlayerIcon;
	
	[Attribute("PrivateGroupWrapper")]
	protected string m_sPrivateGroup;
	protected Widget m_wPrivateGroup;

	[Attribute("FullGroup")]
	protected string m_sFullGroup;
	protected Widget m_wFullGroup;
	
	[Attribute("Arrow")]
	protected string m_sArrowIcon;
	protected ImageWidget m_wArrowIcon;	
	
	[Attribute("GroupFlag")]
	protected string m_sGroupFlag;
	protected ButtonWidget m_wGroupFlag;
	
	protected int m_iGroupId;
	protected SCR_AIGroup m_Group;
	
	protected ImageWidget m_wGroupIcon;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wGroupName = TextWidget.Cast(w.FindAnyWidget(m_sGroupName));
		m_wFreq = TextWidget.Cast(w.FindAnyWidget(m_sFreq));
		m_wPlayerCount = TextWidget.Cast(w.FindAnyWidget(m_sPlayerCount));
		m_wPlayerIcon = ImageWidget.Cast(w.FindAnyWidget(m_sPlayerIcon));
		m_wPrivateGroup = w.FindAnyWidget(m_sPrivateGroup);
		m_wElements = w.FindAnyWidget(m_sElements);
		m_wArrowIcon = ImageWidget.Cast(w.FindAnyWidget(m_sArrowIcon));
		m_wFullGroup = w.FindAnyWidget(m_sFullGroup);
		m_wGroupFlag = ButtonWidget.Cast(w.FindAnyWidget(m_sGroupFlag));
	}

	//------------------------------------------------------------------------------------------------
	void SetGroup(notnull SCR_AIGroup group)
	{
		m_iGroupId = group.GetGroupID();
		m_Group = group;
	}
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	void UpdateGroup(bool canJoin = true)
	{
		if (!m_Group)
			return;
#ifdef DEPLOY_MENU_DEBUG
		PrintFormat("UpdateGroup() name: %1, group id: %2", m_Group.GetCustomNameWithOriginal(), m_Group.GetGroupID());
#endif
		UpdateGroupName();
		UpdateGroupFrequency();
		UpdateGroupPrivacy(m_Group.IsPrivate());
		UpdateGroupFlag();
		SetGroupFull(m_Group.IsFull());

		bool buttonEnabled = canJoin && !m_Group.IsFull() && !m_Group.IsPrivate();
		SetEnabled(buttonEnabled);
#ifdef DEPLOY_MENU_DEBUG
		PrintFormat("%1::UpdateGroup() name: %2, group id: %7, Is full: %3, IsEnabled(): %4, canJoin: %5, isPrivate: %6", this, m_Group.GetCustomNameWithOriginal(), m_Group.IsFull().ToString(), IsEnabled().ToString(), canJoin.ToString(), m_Group.IsPrivate().ToString(), m_Group.GetGroupID());
#endif
		if (m_wPlayerCount)
			m_wPlayerCount.SetTextFormat("%1/%2", m_Group.GetPlayerCount(), m_Group.GetMaxMembers());		
	}
	//---- REFACTOR NOTE END ----
	//------------------------------------------------------------------------------------------------
	void UpdateGroupName()
	{
		// This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
		if (m_Group)
			SetText(SCR_GroupHelperUI.GetTranslatedGroupNameAndRoleName(m_Group));
	}

	//------------------------------------------------------------------------------------------------
	string GetGroupName()
	{
		return m_wGroupName.GetText();
	}

	//! Set the group name into a TextWidget widget.
	static void SGetGroupName(notnull SCR_AIGroup group, notnull TextWidget widget, notnull TextWidget freqWidget)
	{
		string freq = string.Format("%1 #AR-VON_FrequencyUnits_MHz", group.GetRadioFrequency() * 0.001);
		if (group.GetCustomName().IsEmpty())
		{
			string company, platoon, squad, character, format;	
			group.GetCallsigns(company, platoon, squad, character, format);
			widget.SetTextFormat(format + " %5", company, platoon, squad, character);
			freqWidget.SetTextFormat(format + freq);
		}
		else
		{
			widget.SetTextFormat("%1 %2", group.GetCustomName());
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupId()
	{
		return m_iGroupId;
	}

	//------------------------------------------------------------------------------------------------
	SCR_AIGroup GetGroup()
	{
		return m_Group;
	}
	
	//------------------------------------------------------------------------------------------------
	void UpdateGroupPrivacy(bool isPrivate)
	{
		if (m_wPrivateGroup)
			m_wPrivateGroup.SetVisible(isPrivate);

#ifdef DEPLOY_MENU_DEBUG
		PrintFormat("%1::UpdateGroupPrivacy() %2, group id: %5, Is private: %3, IsEnabled(): %4",
			this, m_Group.GetCustomNameWithOriginal(), isPrivate.ToString(), IsEnabled().ToString(), m_Group.GetGroupID());
#endif

	}

	void UpdateButtonAvailability(SCR_PlayerControllerGroupComponent groupCtrl)
	{
		if (!m_Group)
			return;

		bool canJoinGroup = groupCtrl.CanPlayerJoinGroup(GetGame().GetPlayerController().GetPlayerId(), m_Group);
		if (!canJoinGroup && m_Group == groupCtrl.GetPlayersGroup())
		{
			SetEnabled(true);
			return;
		}

		SetEnabled(canJoinGroup && !m_Group.IsFull() && !m_Group.IsPrivate());
	}
	
	protected void SetGroupFull(bool full)
	{
		if (m_wPlayerCount)
		{
			if (full)
			{
				m_wPlayerCount.SetColor(m_ColorWarning);
				m_wPlayerIcon.SetColor(m_ColorWarning);
			}
			else
			{
				m_wPlayerCount.SetColor(Color.FromInt(Color.WHITE));
				m_wPlayerIcon.SetColor(Color.FromInt(Color.WHITE));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetText(string text)
	{
		if (m_wGroupName)
			m_wGroupName.SetText(text);
	}

	//------------------------------------------------------------------------------------------------
	void UpdateGroupFrequency()
	{
		if (m_wFreq && m_Group)
			m_wFreq.SetTextFormat("%1 #AR-VON_FrequencyUnits_MHz", (m_Group.GetRadioFrequency() * 0.001).ToString());
	}

	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	//------------------------------------------------------------------------------------------------
	void UpdateGroupFlag()
	{
		SCR_Faction scrFaction = SCR_Faction.Cast(m_Group.GetFaction());
		if (!scrFaction)
			return;
		
		array<ResourceName> textures = {};		
		array<string> names = {};		
		ResourceName imageSet = scrFaction.GetGroupFlagImageSet();
		
		scrFaction.GetGroupFlagTextures(textures);		
		scrFaction.GetFlagNames(names);
		
		ResourceName flag = m_Group.GetGroupFlag();
		
		if (flag.IsEmpty())
		{
			if (!imageSet.IsEmpty() && !names.IsEmpty())
			{	
				SetImage(imageSet, names[0]);			
			}
			else if (!textures.IsEmpty())
			{
				SetImage(textures[0]);
			}
			else
			{				
				SetImage(scrFaction.GetFactionFlag());							
			}
		}
		else
		{
			if (m_Group.GetFlagIsFromImageSet())
			{
				SetImage(imageSet, flag);				
			}
			else
			{
				SetImage(flag);
			}
		}		
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetSelected(bool selected)
	{
		super.SetSelected(selected);
		if (!m_wArrowIcon)
			return;
		
		if (selected)
			m_wArrowIcon.SetRotation(90);
			else
			m_wArrowIcon.SetRotation(270);
	}	
	//---- REFACTOR NOTE END ----
};

class SCR_NewGroupButton : SCR_DeployButtonBase
{	
	
};
