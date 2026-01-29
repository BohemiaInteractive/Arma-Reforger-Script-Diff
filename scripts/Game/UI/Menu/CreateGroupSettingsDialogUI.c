class CreateGroupSettingsDialogUI : DialogUI
{
	protected Widget m_RulesDescriptionSize;
	protected SCR_EditBoxComponent m_Description;
	protected SCR_ComboBoxComponent m_GroupStatus;
	protected SCR_ComboBoxComponent m_GroupRole;
	protected SCR_EditBoxComponent m_GroupName;
	protected SCR_EditBoxComponent m_GroupDescription;

	protected ref SCR_ScriptPlatformRequestCallback m_CallbackGetPrivilege;

	protected SCR_PlayerControllerGroupComponent m_PlayerComponent;
	protected SCR_GroupsManagerComponent m_GroupsManager;
	protected SCR_AIGroup m_PlayerGroup;

	protected bool m_bHasPrivilege;
	protected SCR_Faction m_LocalFaction;

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();

		m_LocalFaction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		m_PlayerComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerComponent)
			return;

		m_CallbackGetPrivilege = new SCR_ScriptPlatformRequestCallback();
		m_CallbackGetPrivilege.m_OnResult.Insert(OnPrivilegeCallback);
		SocialComponent.RequestSocialPrivilege(EUserInteraction.UserGeneratedContent, m_CallbackGetPrivilege);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrivilegeCallback(UserPrivilege privilege, UserPrivilegeResult result)
	{
		m_GroupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!m_GroupsManager)
			return;

		Widget w = GetRootWidget();
		if (!w)
			return;

		m_RulesDescriptionSize = w.FindAnyWidget("RulesDescriptionSize");
		if (!m_RulesDescriptionSize)
			return;

		m_RulesDescriptionSize.SetVisible(!SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander());

		m_GroupStatus = SCR_ComboBoxComponent.GetComboBoxComponent("Type", w);
		if (!m_GroupStatus)
			return;

		m_GroupRole = SCR_ComboBoxComponent.GetComboBoxComponent("SquadRole", w);
		if (!m_GroupRole)
			return;

		m_GroupName = SCR_EditBoxComponent.GetEditBoxComponent("Name", w);
		if (!m_GroupName)
			return;

		m_GroupDescription = SCR_EditBoxComponent.GetEditBoxComponent("Description", w);
		if (!m_GroupDescription)
			return;

		m_bHasPrivilege = result == UserPrivilegeResult.ALLOWED;
		bool canEdit = m_bHasPrivilege && !SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander();
		string unavailable = WidgetManager.Translate("#AR-UserActionUnavailable"); // Translate is used because to change language the CreateGroupSettingsDialogUI has to be closed and then opened so it will renew.

		m_GroupName.SetEnabled(canEdit);
		m_GroupDescription.SetEnabled(canEdit);

		if (canEdit)
		{
			m_GroupName.SetValue("");
			m_GroupDescription.SetValue("");
		}
		else
		{
			m_GroupName.SetValue(unavailable);
			m_GroupDescription.SetValue(unavailable);
		}

		SetupGroupStatusCombo();
		SetupGroupRoleCombo();
		UpdateConfirmButton();
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
		array<SCR_EGroupRole> availableGroupRoles = m_GroupsManager.GetAvailableGroupRoles(m_LocalFaction);
		if (availableGroupRoles.IsEmpty())
			return;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return;

		if (m_bHasPrivilege)
		{
			bool isPrivate = m_GroupStatus.GetCurrentIndex() == SCR_EGroupPrivacy.PRIVATE;

			array<SCR_EGroupRole> configuredGroupRoles = m_GroupsManager.GetConfiguredGroupRoles(m_LocalFaction, true);

			int currentIndex = m_GroupRole.GetCurrentIndex();
			if (configuredGroupRoles.IsIndexValid(currentIndex) && availableGroupRoles.Contains(configuredGroupRoles[currentIndex]))
			{
				bool joinGroup = !SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander();
				
				string groupName = string.Empty;
				if (m_GroupName.IsEnabled())
					groupName = m_GroupName.GetValue();
				
				string groupDescription = string.Empty;
				if (m_GroupDescription.IsEnabled())
					groupDescription = m_GroupDescription.GetValue();
				
				groupController.RequestCreateGroupWithData(configuredGroupRoles[currentIndex], isPrivate, groupName, groupDescription, joinGroup, true);
			}
			else
			{
				Print("Cannot create group because is not available", LogLevel.WARNING);
			}
		}

		Close();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupGroupStatusCombo()
	{
		m_GroupStatus.AddItem("#AR-Player_Groups_Public");
		m_GroupStatus.AddItem("#AR-Player_Groups_Private");

		m_GroupStatus.SetCurrentItem(SCR_EGroupPrivacy.PUBLIC); // default value

		if (SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			m_GroupStatus.SetEnabled(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupGroupRoleCombo()
	{
		m_GroupRole.m_OnOpened.Insert(OnGroupRoleComboOpened);

		// set available role to combobox
		array<SCR_EGroupRole> availableGroupRoles = m_GroupsManager.GetAvailableGroupRoles(m_LocalFaction);
		bool selectAvailableGroupRole = !availableGroupRoles.IsEmpty();

		array<SCR_GroupRolePresetConfig> groupRolePresetConfigs = {};
		m_LocalFaction.GetGroupRolePresetConfigs(groupRolePresetConfigs);

		SCR_GroupRolePresetConfig preset;
		string groupRoleName;
		int count = groupRolePresetConfigs.Count();
		for (int i = 0; i < count; i++)
		{
			preset = groupRolePresetConfigs[i];

			if (!preset.CanBeCreatedByPlayer())
				continue;

			if (preset.GetGroupRoleName().IsEmpty())
				groupRoleName = SCR_Enum.GetEnumName(SCR_EGroupRole, preset.GetGroupRole());
			else
				groupRoleName = preset.GetGroupRoleName();

			m_GroupRole.AddItem(groupRoleName, i == count-1);

			if (selectAvailableGroupRole && preset.GetGroupRole() == availableGroupRoles[0])
			{
				m_GroupRole.SetCurrentItem(i);
				selectAvailableGroupRole = false;
			}
		}
	}
	//------------------------------------------------------------------------------------------------
	protected void OnGroupRoleComboOpened()
	{
		array<SCR_EGroupRole> configuredGroupRoles = m_GroupsManager.GetConfiguredGroupRoles(m_LocalFaction, true);
		array<SCR_EGroupRole> availableGroupRoles = m_GroupsManager.GetAvailableGroupRoles(m_LocalFaction);

		array<Widget> comboBoxWidgets = {};
		int itemsCount = m_GroupRole.GetElementWidgets(comboBoxWidgets);

		SCR_SquadRoleComboBoxElement squadRoleComboBoxElement;
		bool isAvailable = false;
		bool isPlayerCommander = SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander();

		// set element availability in combobox
		for (int i = 0; i < itemsCount; i++)
		{
			SCR_EGroupRole configuredGroupRole = configuredGroupRoles[i];
			if (configuredGroupRoles.IsIndexValid(i))
				isAvailable = availableGroupRoles.Contains(configuredGroupRole);
			else
				isAvailable = false;

			m_GroupRole.SetElementWidgetEnabled(i, isAvailable, false);

			squadRoleComboBoxElement = SCR_SquadRoleComboBoxElement.Cast(comboBoxWidgets[i].FindHandler(SCR_SquadRoleComboBoxElement));
			if (!squadRoleComboBoxElement)
				continue;

			bool isRequiredRankVisible = !isPlayerCommander && !m_GroupsManager.CanCreateGroupWithLocalPlayerRank(configuredGroupRole, m_LocalFaction);
			squadRoleComboBoxElement.SetVisibleInsufficientRank(isRequiredRankVisible);
			if (isRequiredRankVisible)
			{
				SCR_ECharacterRank requiredRank = m_GroupsManager.GetRequiredRank(configuredGroupRole, m_LocalFaction);
				squadRoleComboBoxElement.SetRankImage(m_LocalFaction.GetRankInsignia(requiredRank));
			}

			bool isNotEnoughFullGroupVisible = !isPlayerCommander && !m_GroupsManager.AreAllGroupsMajorityFull(configuredGroupRole, m_LocalFaction);
			squadRoleComboBoxElement.SetVisibleNotEnoughFullGroup(isNotEnoughFullGroupVisible);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateConfirmButton()
	{
		array<SCR_EGroupRole> availableGroupRoles = m_GroupsManager.GetAvailableGroupRoles(m_LocalFaction);
		bool isAnyAvailable = availableGroupRoles.Count() > 0;
		m_Confirm.SetVisible(isAnyAvailable);
	}
}
