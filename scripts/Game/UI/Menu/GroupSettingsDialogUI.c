//------------------------------------------------------------------------------------------------
class GroupSettingsDialogUI: DialogUI
{
	protected SCR_ComboBoxComponent m_GroupLeader;
	protected SCR_EditBoxComponent m_Description;
	protected SCR_ComboBoxComponent m_GroupStatus;
	protected SCR_EditBoxComponent m_GroupName;
	protected SCR_EditBoxComponent m_GroupDescription;
	protected SCR_PlayerControllerGroupComponent m_PlayerComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_AIGroup m_PlayerGroup;
	protected bool m_bHasPrivilege;
	
	protected static ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_PlayerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerComponent)
			return;
		
		if (m_PlayerComponent.GetSelectedGroupID() == -1)
		{
			Close();
			return;
		}
		if (!m_CallbackGetPrivilege)
			m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();
		
		m_CallbackGetPrivilege.m_OnResult.Insert(OnPrivilegeCallback);
		SocialComponent.RequestSocialPrivilege(EUserInteraction.UserGeneratedContent, m_CallbackGetPrivilege);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		if (m_CallbackGetPrivilege)
			m_CallbackGetPrivilege.m_OnResult.Remove(OnPrivilegeCallback);
		
		if (m_PlayerGroup)
		{
			m_PlayerGroup.GetOnPlayerAdded().Remove(SetupGroupLeaderCombo);
			m_PlayerGroup.GetOnPlayerRemoved().Remove(SetupGroupLeaderCombo);
			m_PlayerGroup.GetOnPlayerLeaderChanged().Remove(OnGroupLeaderChanged);
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnPrivilegeCallback(UserPrivilege privilege, UserPrivilegeResult result)
	{
		
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupsManager)
			return;
		
		m_PlayerGroup = m_GroupsManager.FindGroup(m_PlayerComponent.GetGroupID());
		if (!m_PlayerGroup)
			return;
		
		m_PlayerGroup.GetOnPlayerAdded().Insert(SetupGroupLeaderCombo);
		m_PlayerGroup.GetOnPlayerRemoved().Insert(SetupGroupLeaderCombo);
		m_PlayerGroup.GetOnPlayerLeaderChanged().Insert(OnGroupLeaderChanged);
		
		Widget w = GetRootWidget();
		if (!w)
			return;
		
		m_GroupLeader = SCR_ComboBoxComponent.GetComboBoxComponent("GroupLeader", w);
		if (!m_GroupLeader)
			return;
		
		m_GroupStatus = SCR_ComboBoxComponent.GetComboBoxComponent("Type", w);
		if (!m_GroupStatus)
			return;
		
		m_GroupName = SCR_EditBoxComponent.GetEditBoxComponent("Name", w);
		if (!m_GroupName)
			return;
		
		m_GroupDescription = SCR_EditBoxComponent.GetEditBoxComponent("Description", w);
		if (!m_GroupDescription)
			return;
		
		if (result == UserPrivilegeResult.ALLOWED)
		{
			m_GroupName.SetEnabled(true);
			m_GroupName.SetValue(m_PlayerGroup.GetCustomName());
			
			m_GroupDescription.SetEnabled(true);
			m_GroupDescription.SetValue(m_PlayerGroup.GetCustomDescription());
			m_bHasPrivilege = true;
		}
		else
		{
			m_GroupName.SetEnabled(false);
			m_GroupName.SetValue(WidgetManager.Translate("#AR-UserActionUnavailable"));
			m_GroupDescription.SetEnabled(false);
			m_GroupDescription.SetValue(WidgetManager.Translate("#AR-UserActionUnavailable"));
			m_bHasPrivilege = false;
		}
		
		SetupGroupStatusCombo();
		SetupGroupLeaderCombo();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		GetGame().GetInputManager().ActivateContext("InteractableDialogContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm()
	{
		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return;
		
		if (m_bHasPrivilege)
		{
			int newGroupLeaderId = GetPlayerIdFromName(m_GroupLeader.GetCurrentItem());
			if (newGroupLeaderId > 0 && m_PlayerGroup.GetLeaderID() != newGroupLeaderId)
			{
				// Appoint a new group leader
				groupController.SetGroupLeader(newGroupLeaderId);
			}
			else
			{
				// Change name and description
				int groupID = m_PlayerGroup.GetGroupID();

				groupController.RequestSetCustomGroupDescription(groupID, m_GroupDescription.GetValue());
				groupController.RequestSetCustomGroupName(groupID, m_GroupName.GetValue());
			}
		}
		
		Close();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupGroupStatusCombo()
	{
		m_GroupStatus.AddItem("#AR-Player_Groups_Public");
		m_GroupStatus.AddItem("#AR-Player_Groups_Private");
		
		m_GroupStatus.SetCurrentItem(m_PlayerGroup.IsPrivate());
		m_GroupStatus.m_OnChanged.Insert(OnStatusChanged);
		
		if (m_PlayerGroup)
			m_GroupStatus.SetEnabled(m_PlayerGroup.IsPrivacyChangeable());
	}
	
	//------------------------------------------------------------------------------------------------
	void OnStatusChanged(SCR_ComboBoxComponent combo, int index)
	{
		if (!m_PlayerComponent)
			return;
		
		m_PlayerComponent.RequestPrivateGroupChange(m_PlayerComponent.GetPlayerID() , index);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupGroupLeaderCombo()
	{
		if (!m_PlayerGroup)
			return;

		int localPlayerId = SCR_PlayerController.GetLocalPlayerId();
		if (m_PlayerGroup.GetLeaderID() != localPlayerId)
		{
			m_GroupLeader.SetEnabled(false);
			return;
		}

		int selectedIndex;
		m_GroupLeader.ClearAll();

		PlayerManager playerManager = GetGame().GetPlayerManager();
		array<int> playerIds = m_PlayerGroup.GetPlayerIDs();
		
		// Populate the combo box with names of players in group
		foreach (int index, int playerId : playerIds)
		{
			m_GroupLeader.AddItem(playerManager.GetPlayerName(playerId));

			if (playerId == localPlayerId)
				selectedIndex = index;
		}

		// Select current leader by default
		m_GroupLeader.SetCurrentItem(selectedIndex);
		m_GroupLeader.SetEnabled(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Group leader changed, if local player is not group leader anymore, the group settings dialog should close
	//! \param[in] groupId
	//! \param[in] playerId
	protected void OnGroupLeaderChanged(int groupId, int playerId)
	{
		if (groupId != m_PlayerGroup.GetGroupID())
			return;

		if (m_PlayerComponent.GetPlayerID() == playerId)
			return;

		m_PlayerGroup.GetOnPlayerAdded().Remove(SetupGroupLeaderCombo);
		m_PlayerGroup.GetOnPlayerRemoved().Remove(SetupGroupLeaderCombo);
		m_PlayerGroup.GetOnPlayerLeaderChanged().Remove(OnGroupLeaderChanged);

		Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Finds a player id from the player's name
	//! \param[in] name
	//! \return playerId
	protected int GetPlayerIdFromName(string name)
	{
		array<int> playerIds = m_PlayerGroup.GetPlayerIDs();
		foreach (int playerId : playerIds)
		{
			if (GetGame().GetPlayerManager().GetPlayerName(playerId) == name)
				return playerId;
		}

		return 0;
	}

};
