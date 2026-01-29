//! Handles interactions for the faction Commander role. Should be attached to player controller.
[ComponentEditorProps(category: "GameScripted/Commander", description: "Handles interactions for the faction Commander role. Should be attached to player controller.")]
class SCR_FactionCommanderPlayerComponentClass : ScriptComponentClass
{
	[Attribute("squad", desc: "Default icon for a group order.", category: "Defaults")]
	protected string m_sGroupOrderIcon;

	[Attribute("VON_radio", desc: "Default icon for a request order.", category: "Defaults")]
	protected string m_sRequestOrderIcon;

	[Attribute("addCircle", desc: "Default icon for a create objective order.", category: "Defaults")]
	protected string m_sCreateObjectiveOrderIcon;

	[Attribute("#AR-Tasks_TitleRequest", desc: "'Request' default text.", category: "Defaults")]
	protected string m_sStringRequest;

	[Attribute("#AR-FactionCommander_TaskObjective", desc: "'Create objective' default text.", category: "Defaults")]
	protected string m_sStringCreateObjective;

	//------------------------------------------------------------------------------------------------
	string GetGroupOrderIcon()
	{
		return m_sGroupOrderIcon;
	}

	//------------------------------------------------------------------------------------------------
	string GetRequestOrderIcon()
	{
		return m_sRequestOrderIcon;
	}

	//------------------------------------------------------------------------------------------------
	string GetCreateObjectiveOrderIcon()
	{
		return m_sCreateObjectiveOrderIcon;
	}

	//------------------------------------------------------------------------------------------------
	string GetStringRequest()
	{
		return m_sStringRequest;
	}

	//------------------------------------------------------------------------------------------------
	string GetStringCreateObjective()
	{
		return m_sStringCreateObjective;
	}
}

class SCR_FactionCommanderPlayerComponent : ScriptComponent
{
	protected bool m_bMenuVisible;

	protected SCR_MapRadialUI m_MapContextualMenu;

	protected SCR_RadialMenu m_RadialMenu;

	protected SCR_PlayerController m_PlayerController;

	protected SCR_MapEntity m_MapEntity;

	protected IEntity m_HoveredEntity;
	protected SCR_Task m_HoveredTask;
	protected SCR_Task m_CurrentHoveredTask;
	protected IEntity m_CurrentHoveredEntity;

	protected ref map<string, SCR_SelectionMenuEntry> m_mEntryNames = new map<string, SCR_SelectionMenuEntry>();
	protected ref map<SCR_SelectionMenuEntry, SCR_SelectionMenuCategoryEntry> m_mEntryParents = new map<SCR_SelectionMenuEntry, SCR_SelectionMenuCategoryEntry>();
	protected ref map<SCR_SelectionMenuEntry, SCR_AIGroup> m_mEntryGroups = new map<SCR_SelectionMenuEntry, SCR_AIGroup>();
	protected ref map<SCR_SelectionMenuEntry, ref SCR_FactionCommanderBaseMenuHandler> m_mEntryHandlers = new map<SCR_SelectionMenuEntry, ref SCR_FactionCommanderBaseMenuHandler>();
	protected ref map<SCR_SelectionMenuEntry, ref SCR_FactionCommanderMenuEntry> m_mTaskRootMenuEntries = new map<SCR_SelectionMenuEntry, ref SCR_FactionCommanderMenuEntry>();

	[RplProp(condition: RplCondition.OwnerOnly)]
	protected WorldTimestamp m_fNextVolunteeringAvailableAt;

	protected SCR_MapCursorModule m_MapCursorModule;
	protected bool m_bShouldClearHoveredTask;

	protected WorldTimestamp m_ReplaceCommanderCooldown;
	protected SCR_GroupTaskManagerComponent m_GroupTaskManager;

	protected static int s_iLastGeneratedTaskId;

	static const string TASK_ID = "%1_Task_%2";
	static const string REQUESTED_TASK_ID = "%1_RequestedTask_%2";

	//------------------------------------------------------------------------------------------------
	SCR_FactionCommanderMenuEntry GetConfigForEntry(notnull SCR_SelectionMenuEntry entry)
	{
		if (!m_mTaskRootMenuEntries)
			return null;

		SCR_FactionCommanderMenuEntry configEntry = m_mTaskRootMenuEntries.Get(entry);
		if (configEntry)
			return configEntry;

		if (m_mEntryNames)
		{
			SCR_SelectionMenuEntry key;
			if (m_mEntryNames && m_mTaskRootMenuEntries && m_mEntryNames.Find(entry.GetId(), key))
				return m_mTaskRootMenuEntries.Get(key);
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	void SetReplaceCommanderCooldownTimestamp(WorldTimestamp timeStamp)
	{
		m_ReplaceCommanderCooldown = timeStamp;
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetReplaceCommanderCooldownTimestamp()
	{
		return m_ReplaceCommanderCooldown;
	}

	//------------------------------------------------------------------------------------------------
	static int GenerateTaskID()
	{
		return s_iLastGeneratedTaskId++;
	}

	//------------------------------------------------------------------------------------------------
	static bool IsLocalPlayerCommander()
	{
		PlayerController pc = GetGame().GetPlayerController();

		if (!pc)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(pc.GetPlayerId()));

		if (!faction)
			return false;

		return (pc.GetPlayerId() == faction.GetCommanderId());
	}

	//------------------------------------------------------------------------------------------------
	void OnCommanderRightsGained();

	//------------------------------------------------------------------------------------------------
	void OnCommanderRightsLost();

	//------------------------------------------------------------------------------------------------
	void SetNextVolunteeringTimestamp(WorldTimestamp timestamp)
	{
		m_fNextVolunteeringAvailableAt = timestamp;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	WorldTimestamp GetNextVolunteeringTimestamp()
	{
		return m_fNextVolunteeringAvailableAt;
	}

	//------------------------------------------------------------------------------------------------
	//! \param name Entry name as defined in the config (SCR_FactionCommanderMenuHierarchy)
	protected SCR_SelectionMenuEntry GetEntryByName(string name)
	{
		return m_mEntryNames.Get(name);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a group associated with given entry
	protected SCR_AIGroup GetAssignedGroup(SCR_SelectionMenuEntry entry)
	{
		return m_mEntryGroups.Get(entry);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddRequestOrder(SCR_SelectionMenuCategoryEntry category, notnull SCR_FactionCommanderBaseMenuHandler handler)
	{
		string name;
		if (category)
			name = category.GetId();

		SCR_SelectionMenuEntry entry = m_MapContextualMenu.AddRadialEntry(GetRequestOrderEntryDisplayName(), category);
		entry.SetId(name);
		string disabledText;

		SCR_FactionCommanderBaseRequestMenuHandler requestHandler = SCR_FactionCommanderBaseRequestMenuHandler.Cast(handler);
		entry.Enable(requestHandler.CanRequestEntryBeSelected(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, disabledText));
		m_mEntryParents.Set(entry, category);
		m_mEntryHandlers.Set(entry, handler);
		m_mTaskRootMenuEntries.Set(entry, handler.GetMenuEntry());
		requestHandler.SetMenuEntry(handler.GetMenuEntry());

		if (!disabledText.IsEmpty())
			entry.SetName(disabledText);

		ResourceName imagesetName;
		string iconName;
		GetRequestOrderEntryIconData(imagesetName, iconName);

		if (!iconName.IsEmpty())
		{
			if (imagesetName.IsEmpty())
				entry.SetIconFromDeafaultImageSet(iconName);
			else
				entry.SetIcon(imagesetName, iconName);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void AddCreateObjectiveOrder(SCR_SelectionMenuCategoryEntry category, notnull SCR_FactionCommanderBaseMenuHandler handler)
	{
		string name;
		if (category)
			name = category.GetId();

		SCR_SelectionMenuEntry entry = m_MapContextualMenu.AddRadialEntry(GetCreateObjectiveOrderEntryDisplayName(), category);
		entry.SetId(name);
		string disabledText;

		SCR_FactionCommanderConflictBaseMenuHandler conflictBaseMenuhandler = SCR_FactionCommanderConflictBaseMenuHandler.Cast(handler);
		entry.Enable(conflictBaseMenuhandler.CanCreateObjectiveEntryBeSelected(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, disabledText));
		m_mEntryParents.Set(entry, category);
		m_mEntryHandlers.Set(entry, handler);
		m_mTaskRootMenuEntries.Set(entry, handler.GetMenuEntry());

		if (!disabledText.IsEmpty())
			entry.SetName(disabledText);

		ResourceName imagesetName;
		string iconName;
		GetCreateObjectiveOrderEntryIconData(imagesetName, iconName);

		if (!iconName.IsEmpty())
		{
			if (imagesetName.IsEmpty())
				entry.SetIconFromDeafaultImageSet(iconName);
			else
				entry.SetIcon(imagesetName, iconName);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Adds all groups of Commander's faction under a given category as menu entries
	protected void FillMenuWithGroups(SCR_SelectionMenuCategoryEntry category, notnull SCR_FactionCommanderBaseMenuHandler handler)
	{
		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!faction)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));

		if (!groupsManager)
			return;

		string name;
		if (category)
			name = category.GetId();

		array<SCR_AIGroup> playableGroups = groupsManager.GetPlayableGroupsByFaction(faction);
		SCR_SelectionMenuEntry entry;

		foreach (SCR_AIGroup group : playableGroups)
		{
			if (!group || !handler.CanGroupEntryBeShown(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, group))
				continue;

			string disabledText;
			entry = m_MapContextualMenu.AddRadialEntry(GetGroupEntryDisplayName(group), category);
			entry.SetId(name);
			entry.Enable(handler.CanGroupEntryBeSelected(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, group, disabledText));
			m_mEntryParents.Set(entry, category);
			m_mEntryHandlers.Set(entry, handler);
			m_mTaskRootMenuEntries.Set(entry, handler.GetMenuEntry());

			if (!disabledText.IsEmpty())
				entry.SetName(disabledText);

			ResourceName imagesetName;
			string iconName;
			GetGroupEntryIconData(group, imagesetName, iconName);

			if (!iconName.IsEmpty())
			{
				if (imagesetName.IsEmpty())
					entry.SetIconFromDeafaultImageSet(iconName);
				else
					entry.SetIcon(imagesetName, iconName);
			}

			m_mEntryGroups.Set(entry, group);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the display name to be shown in radial menu for the given group
	protected string GetGroupEntryDisplayName(SCR_AIGroup group)
	{
		// This function can only be used in UI menu, where to change the language you need to close and open the shown menu, so it will be renewed.
		return SCR_GroupHelperUI.GetTranslatedGroupNameAndRoleName(group);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the imageset and icon name to be shown in radial menu for the given group and category
	protected void GetGroupEntryIconData(SCR_AIGroup group, out ResourceName imagesetName, out string iconName)
	{
		iconName = GetDefaultGroupOrderIcon();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDefaultGroupOrderIcon()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return string.Empty;

		SCR_FactionCommanderPlayerComponentClass componentData = SCR_FactionCommanderPlayerComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return string.Empty;

		return componentData.GetGroupOrderIcon();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the display name to be shown in radial menu for the Request order category
	protected string GetRequestOrderEntryDisplayName()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return string.Empty;

		SCR_FactionCommanderPlayerComponentClass componentData = SCR_FactionCommanderPlayerComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return string.Empty;

		return componentData.GetStringRequest();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the imageset and icon name to be shown in radial menu for the Faction order category
	protected void GetRequestOrderEntryIconData(out ResourceName imagesetName, out string iconName)
	{
		iconName = GetDefaultRequestOrderIcon();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDefaultRequestOrderIcon()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return string.Empty;

		SCR_FactionCommanderPlayerComponentClass componentData = SCR_FactionCommanderPlayerComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return string.Empty;

		return componentData.GetRequestOrderIcon();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the display name to be shown in radial menu for the create objective order
	protected string GetCreateObjectiveOrderEntryDisplayName()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return string.Empty;

		SCR_FactionCommanderPlayerComponentClass componentData = SCR_FactionCommanderPlayerComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return string.Empty;

		return componentData.GetStringCreateObjective();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns the imageset and icon name to be shown in radial menu for the create objective order
	protected void GetCreateObjectiveOrderEntryIconData(out ResourceName imagesetName, out string iconName)
	{
		iconName = GetDefaultCreateObjectiveOrderIcon();
	}

	//------------------------------------------------------------------------------------------------
	protected string GetDefaultCreateObjectiveOrderIcon()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return string.Empty;

		SCR_FactionCommanderPlayerComponentClass componentData = SCR_FactionCommanderPlayerComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return string.Empty;

		return componentData.GetCreateObjectiveOrderIcon();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration config)
	{
		m_MapCursorModule = SCR_MapCursorModule.Cast(SCR_MapEntity.GetMapInstance().GetMapModule(SCR_MapCursorModule));

		m_HoveredEntity = null;

		if (SCR_TaskManagerUIComponent.GetInstance())
			SCR_TaskManagerUIComponent.GetInstance().GetOnTaskHovered().Insert(OnTaskHovered);

		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);

		if (!m_MapContextualMenu)
			m_MapContextualMenu = SCR_MapRadialUI.GetInstance();

		if (m_MapContextualMenu)
		{
			m_MapContextualMenu.GetOnMenuInitInvoker().Insert(OnContextualMenuInit);
			m_MapContextualMenu.GetOnEntryPerformedInvoker().Insert(OnCommandPerformed);
			m_MapContextualMenu.GetOnEntrySelectedInvoker().Insert(OnEntryHovered);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		m_bMenuVisible = false;
		m_HoveredEntity = null;

		if (SCR_TaskManagerUIComponent.GetInstance())
			SCR_TaskManagerUIComponent.GetInstance().GetOnTaskHovered().Remove(OnTaskHovered);

		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);

		if (!m_MapContextualMenu)
			m_MapContextualMenu = SCR_MapRadialUI.GetInstance();

		if (m_MapContextualMenu)
		{
			m_MapContextualMenu.GetOnMenuInitInvoker().Remove(OnContextualMenuInit);
			m_MapContextualMenu.GetOnEntryPerformedInvoker().Remove(OnCommandPerformed);
			m_MapContextualMenu.GetOnEntrySelectedInvoker().Remove(OnEntryHovered);
		}

		if (!m_RadialMenu)
			return;

		m_RadialMenu.GetOnOpen().Remove(OnRadialMenuOpen);
		m_RadialMenu.GetOnBeforeOpen().Remove(OnRadialMenuBeforeOpen);
		m_RadialMenu.GetOnClose().Remove(OnRadialMenuClose);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when opening the radial menu for the first time after opening the map
	protected void OnContextualMenuInit()
	{
		if (!m_MapContextualMenu || !m_PlayerController)
			return;

		m_RadialMenu = m_MapContextualMenu.GetRadialController().GetRadialMenu();

		if (!m_RadialMenu)
			return;

		m_RadialMenu.GetOnOpen().Insert(OnRadialMenuOpen);
		m_RadialMenu.GetOnBeforeOpen().Insert(OnRadialMenuBeforeOpen);
		m_RadialMenu.GetOnClose().Insert(OnRadialMenuClose);
	}

	//------------------------------------------------------------------------------------------------
	//! Adds entries as defined in the hierarchy config (stored in SCR_Faction)
	protected void CreateConfigEntries()
	{
		m_mEntryNames.Clear();
		m_mEntryGroups.Clear();
		m_mEntryParents.Clear();
		m_mEntryHandlers.Clear();
		m_mTaskRootMenuEntries.Clear();

		SCR_Faction faction = SCR_Faction.Cast(SCR_FactionManager.SGetLocalPlayerFaction());

		if (!faction)
			return;

		Resource container;

		// set proper commander menu config by faction, this config can by overrided by another one from SCR_MapFactionCommanderRadialMenu
		SCR_MapFactionCommanderRadialMenu mapFactionCommanderRadialMenu = SCR_MapFactionCommanderRadialMenu.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapFactionCommanderRadialMenu));
		if (mapFactionCommanderRadialMenu && !mapFactionCommanderRadialMenu.GetCommanderMenuConfig(faction).IsEmpty())
			container = BaseContainerTools.LoadContainer(mapFactionCommanderRadialMenu.GetCommanderMenuConfig(faction));
		else
			container = BaseContainerTools.LoadContainer(faction.GetCommanderMenuConfig());

		if (!container)
			return;

		SCR_FactionCommanderMenuHierarchy menuHierarchy = SCR_FactionCommanderMenuHierarchy.Cast(BaseContainerTools.CreateInstanceFromContainer(container.GetResource().ToBaseContainer()));

		if (!menuHierarchy)
			return;

		array<ref SCR_FactionCommanderMenuEntry> entries = {};
		menuHierarchy.GetEntries(entries);

		foreach (int index, SCR_FactionCommanderMenuEntry entry : entries)
		{
			if (entry.IsEnabled())
				CreateConfigEntry(entry, null, null);
		}

		// remove mainCommandMenuHandler icon from the radial menu if no commands were added.
		if (m_mEntryHandlers.Count() != 1)
			return;

		SCR_SelectionMenuEntry entry = m_mEntryHandlers.GetKey(0);
		SCR_FactionCommanderMainCommandMenuHandler mainCommandMenuHandler = SCR_FactionCommanderMainCommandMenuHandler.Cast(m_mEntryHandlers.GetElement(0));
		if (!mainCommandMenuHandler || !entry)
			return;

		m_MapContextualMenu.RemoveRadialEntry(entry);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a radial menu entry from config data SCR_FactionCommanderMenuEntry
	protected void CreateConfigEntry(notnull SCR_FactionCommanderMenuEntry configEntry, SCR_SelectionMenuCategoryEntry parentCategory, SCR_FactionCommanderMenuEntry rootConfigEntry)
	{
		SCR_FactionCommanderBaseMenuHandler handler = configEntry.GetMenuHandler();

		if (!handler)
		{
			PrintFormat("Faction Commander menu entry config '%1' is missing its handler class. Skipping...", configEntry.GetName(), level: LogLevel.WARNING);
			return;
		}

		if (!rootConfigEntry && SCR_FactionCommanderTaskRootMenuEntry.Cast(configEntry))
		{
			rootConfigEntry = configEntry;
		}

		handler.SetMenuEntry(rootConfigEntry);

		if (!handler.CanEntryBeShown(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity))
			return;

		string name = configEntry.GetName();

		array<ref SCR_FactionCommanderMenuEntry> childEntries = {};
		configEntry.GetEntries(childEntries);

		if (!childEntries.IsEmpty() || configEntry.IsCategory() || configEntry.IsGroupOrder())
		{
			// This entry's values indicate it should be in fact a category
			if (!childEntries.IsEmpty() || !configEntry.IsSupportRequest() || SCR_FactionCommanderPlayerComponent.IsLocalPlayerCommander())
			{
				CreateConfigCategory(configEntry, parentCategory, childEntries, rootConfigEntry);
				return;
			}
		}

		SCR_SelectionMenuEntry entry = m_MapContextualMenu.AddRadialEntry(configEntry.GetDisplayName(), parentCategory);

		m_mEntryParents.Set(entry, parentCategory);
		m_mEntryNames.Set(name, entry);
		m_mEntryHandlers.Set(entry, handler);
		m_mTaskRootMenuEntries.Set(entry, rootConfigEntry);

		entry.SetId(name);
		string disabledText;
		entry.Enable(handler.CanEntryBeSelected(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, disabledText));

		if (!disabledText.IsEmpty())
			entry.SetName(disabledText);

		string iconName = configEntry.GetIconName();

		if (!iconName.IsEmpty())
			entry.SetIcon(configEntry.GetIconImageset(), iconName);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates a radial menu category from config data (SCR_FactionCommanderMenuEntry)
	protected void CreateConfigCategory(notnull SCR_FactionCommanderMenuEntry configEntry, SCR_SelectionMenuCategoryEntry parentCategory, notnull array<ref SCR_FactionCommanderMenuEntry> childEntries, SCR_FactionCommanderMenuEntry rootConfigEntry)
	{
		SCR_FactionCommanderBaseMenuHandler handler = configEntry.GetMenuHandler();

		if (!handler)
			return;

		handler.SetMenuEntry(rootConfigEntry);

		// if hovered entity is a task, shows quick menu
		bool isQuickMenu = m_HoveredEntity && SCR_Task.Cast(m_HoveredEntity);

		// quick menu shows entries on the first level
		if (isQuickMenu)
			parentCategory = null;

		string name = configEntry.GetName();
		SCR_SelectionMenuCategoryEntry category;

		if (!isQuickMenu)
			category = m_MapContextualMenu.AddRadialCategory(configEntry.GetDisplayName(), parentCategory);

		m_mEntryNames.Set(name, category);
		m_mEntryParents.Set(category, parentCategory);
		m_mEntryHandlers.Set(category, handler);
		m_mTaskRootMenuEntries.Set(category, rootConfigEntry);

		if (category)
		{
			category.SetId(name);
			string disabledText;
			category.Enable(handler.CanEntryBeSelected(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, disabledText));

			if (!disabledText.IsEmpty())
				category.SetName(disabledText);

			string iconName = configEntry.GetIconName();

			if (!iconName.IsEmpty())
				category.SetIcon(configEntry.GetIconImageset(), iconName);
		}

		// skip other task categories
		if (isQuickMenu && !handler.CanQuickEntryBeShown(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity))
			return;

		if (configEntry.IsSupportRequest())
		{
			SCR_FactionCommanderBaseRequestMenuHandler requestHandler = SCR_FactionCommanderBaseRequestMenuHandler.Cast(handler);
			if (requestHandler && requestHandler.CanRequestEntryBeShown(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity))
				AddRequestOrder(category, handler);
		}
		else
		{
			SCR_FactionCommanderConflictBaseMenuHandler requestHandler = SCR_FactionCommanderConflictBaseMenuHandler.Cast(handler);
			if (!isQuickMenu && requestHandler && requestHandler.CanCreateObjectiveEntryBeShown(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity))
				AddCreateObjectiveOrder(category, handler);
		}

		if (configEntry.IsGroupOrder())
			FillMenuWithGroups(category, handler);

		foreach (SCR_FactionCommanderMenuEntry childEntry : childEntries)
		{
			if (childEntry.IsEnabled())
				CreateConfigEntry(childEntry, category, rootConfigEntry);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called each time the radial menu is before open in the map
	protected void OnRadialMenuBeforeOpen()
	{
		m_HoveredTask = m_CurrentHoveredTask;
		m_HoveredEntity = m_CurrentHoveredEntity;
	}

	//------------------------------------------------------------------------------------------------
	//! Called each time the radial menu is open in the map
	protected void OnRadialMenuOpen()
	{
		m_bMenuVisible = true;

		CreateConfigEntries();
	}

	//------------------------------------------------------------------------------------------------
	//! Called each time the radial menu is closed in the map
	protected void OnRadialMenuClose()
	{
		// Remove entries the next frame so callbacks still work properly
		GetGame().GetCallqueue().CallLater(ClearRadialMenu);
	}

	//------------------------------------------------------------------------------------------------
	protected void ClearRadialMenu()
	{
		SCR_SelectionMenuCategoryEntry category;

		foreach (SCR_SelectionMenuEntry entry, SCR_SelectionMenuCategoryEntry parent : m_mEntryParents)
		{
			if (!entry)
				continue;

			if (category)
				category.RemoveEntry(entry);
			else
				m_RadialMenu.RemoveEntry(entry);
		}

		m_bMenuVisible = false;

		if (m_bShouldClearHoveredTask)
		{
			m_HoveredTask = null;
			m_HoveredEntity = null;
		}

		m_bShouldClearHoveredTask = false;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when a radial menu action is triggered
	protected void OnCommandPerformed(SCR_SelectionMenuEntry element, float[] worldPos)
	{
		if (!element)
			return;

		vector position;
		position[0] = worldPos[0];
		position[1] = GetGame().GetWorld().GetSurfaceY(worldPos[0], worldPos[1]);
		position[2] = worldPos[1];

		SCR_FactionCommanderBaseMenuHandler menuHandler = m_mEntryHandlers.Get(element);
		if (menuHandler)
		{
			menuHandler.SetMenuEntry(m_mTaskRootMenuEntries.Get(element));
			menuHandler.OnCommandIssued(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity, GetAssignedGroup(element));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create requested task
	//! \param[in] taskPrefab
	//! \param[in] position
	//! \param[in] requesterGroupId
	//! \param[in] group
	//! \param[in] playerId
	//! \param[in] handler
	void CreateRequestedTask(string taskPrefab, vector position, int requesterGroupId, SCR_AIGroup group, int playerId, SCR_FactionCommanderBaseMenuHandler handler = null)
	{
		int groupId = -1;

		if (group)
			groupId = group.GetGroupID();

		if (requesterGroupId == -1)
			return;

		Rpc(RpcAsk_CreateRequestedTask, taskPrefab, position, requesterGroupId, groupId, playerId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CreateRequestedTask(string taskPrefab, vector destination, int requesterGroupId, int assigneeId, int playerId)
	{
		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return;

		SCR_AIGroup requesterGroup = groupsManager.FindGroup(requesterGroupId);
		if (!requesterGroup)
			return;

		Faction faction = requesterGroup.GetFaction();
		if (!faction)
			return;

		if (!m_GroupTaskManager.CanCreateNewTaskWithResourceName(taskPrefab, faction))
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		string taskID = string.Format(REQUESTED_TASK_ID, faction.GetFactionKey(), GenerateTaskID());

		SCR_BaseRequestedTaskEntity task = SCR_BaseRequestedTaskEntity.Cast(taskSystem.CreateTask(taskPrefab, taskID, "", "", destination, playerId));
		if (!task)
		{
			Print("Task was not created", LogLevel.ERROR);
			return;
		}

		taskSystem.SetTaskOwnership(task, SCR_ETaskOwnership.EXECUTOR);
		taskSystem.SetTaskVisibility(task, SCR_ETaskVisibility.GROUP);
		task.SetTaskRequesterId(requesterGroupId);
		taskSystem.AddTaskFaction(task, faction.GetFactionKey());

		SCR_TaskUIInfo taskUIInfo = task.GetTaskUIInfo();
		if (!taskUIInfo)
			return;

		task.SetTaskName(taskUIInfo.GetName(), {SCR_MapEntity.GetGridLabel(destination)});

		// set task description
		string company, platoon, squad, character, format;
		requesterGroup.GetCallsigns(company, platoon, squad, character, format);
		task.SetTaskDescription(taskUIInfo.GetDescription(), {string.Format(format, company, platoon, squad)});

		m_GroupTaskManager.SetRequestedGroupTask(task, playerId, requesterGroupId);

		if (assigneeId > -1)
			taskSystem.AssignTask(task, SCR_TaskExecutorGroup.FromGroup(assigneeId), true, playerId); // force assign
	}

	//------------------------------------------------------------------------------------------------
	//! Create task
	//! \param[in] taskPrefab
	//! \param[in] position
	//! \param[in] group
	//! \param[in] playerId
	//! \param[in] handler
	void CreateTask(ResourceName taskPrefab, vector position, SCR_AIGroup group, int playerId, SCR_FactionCommanderBaseMenuHandler handler = null)
	{
		int groupId = -1;

		if (group)
			groupId = group.GetGroupID();

		int data;

		if (handler)
		{
			SCR_SelectionMenuEntry entry = m_mEntryHandlers.GetKeyByValue(handler);

			if (entry)
				data = entry.GetId().ToInt();
		}

		Rpc(RpcAsk_CreateTask, taskPrefab, groupId, playerId, position, GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetLocalPlayerFaction()), data);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CreateTask(string taskPrefab, int assigneeId, int playerId, vector destination, int factionIndex, int data)
	{
		Faction faction = GetGame().GetFactionManager().GetFactionByIndex(factionIndex);
		if (!faction)
			return;

		if (!m_GroupTaskManager || !m_GroupTaskManager.CanCreateNewTaskWithResourceName(taskPrefab, faction))
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		string taskID = string.Format(TASK_ID, faction.GetFactionKey(), GenerateTaskID());

		SCR_Task task = taskSystem.CreateTask(taskPrefab, taskID, "", "", destination, playerId);
		if (!task)
		{
			Print("Task was not created", LogLevel.ERROR);
			return;
		}

		taskSystem.SetTaskOwnership(task, SCR_ETaskOwnership.EXECUTOR);
		taskSystem.SetTaskVisibility(task, SCR_ETaskVisibility.GROUP);
		taskSystem.AddTaskFaction(task, faction.GetFactionKey());

		SCR_TaskUIInfo taskUIInfo = task.GetTaskUIInfo();
		if (!taskUIInfo)
			return;

		task.SetTaskName(taskUIInfo.GetName(), {SCR_MapEntity.GetGridLabel(destination)});

		m_GroupTaskManager.SetGroupTask(task, playerId);

		if (assigneeId > -1)
			taskSystem.AssignTask(task, SCR_TaskExecutorGroup.FromGroup(assigneeId), true, playerId); // force assign
	}

	//------------------------------------------------------------------------------------------------
	void AssignGroupToTask(int groupId, string taskId, int requesterID)
	{
		Rpc(RpcAsk_AssignGroupToTask, groupId, taskId, requesterID);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_AssignGroupToTask(int groupId, string taskId, int requesterID)
	{
		SCR_Task task = SCR_TaskSystem.GetInstance().GetTaskFromTaskID(taskId);
		if (!task)
			return;

		SCR_TaskSystem.GetInstance().AssignTask(task, SCR_TaskExecutorGroup.FromGroup(groupId), true, requesterID); // force assign
	}

	//------------------------------------------------------------------------------------------------
	void CancelTask(int playerId, string taskId)
	{
		Rpc(RpcAsk_CancelTask, playerId, taskId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CancelTask(int playerId, string taskId)
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		SCR_Task task = taskSystem.GetTaskFromTaskID(taskId);
		if (!task)
			return;

		taskSystem.SetTaskState(task, SCR_ETaskState.CANCELLED);
		taskSystem.DeleteTask(task);
	}

	//------------------------------------------------------------------------------------------------
	//! Finish task
	//! \param[in] playerId
	//! \param[in] taskId
	void FinishTask(int playerId, string taskId)
	{
		Rpc(RpcAsk_FinishTask, playerId, taskId);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_FinishTask(int playerId, string taskId)
	{
		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		SCR_Task task = taskSystem.GetTaskFromTaskID(taskId);
		if (!task)
			return;

		taskSystem.SetTaskState(task, SCR_ETaskState.COMPLETED);
		taskSystem.DeleteTask(task);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryHovered(SCR_SelectionMenuEntry entry, int id)
	{
		m_mEntryHandlers.Get(entry).OnEntryHovered(SCR_FactionManager.SGetLocalPlayerFaction(), m_PlayerController.GetPlayerId(), m_MapContextualMenu.GetMenuWorldPosition(), m_HoveredEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTaskHovered(SCR_Task task)
	{
		m_bShouldClearHoveredTask = task == null;

		if (IsTaskValid(task))
		{
			m_CurrentHoveredTask = m_CurrentHoveredTask;
			m_CurrentHoveredEntity = task;
		}
		else
		{
			m_CurrentHoveredTask = null;
			m_CurrentHoveredEntity = null;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsTaskValid(SCR_Task task)
	{
		if (!task)
			return false;

		// the tasks still exist, even after they are finished, we don't need this
		if (task.GetTaskState() == SCR_ETaskState.COMPLETED)
			return false;

		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();
		if (!faction)
			return false;

		if (!task.GetOwnerFactionKeys() || !task.GetOwnerFactionKeys().Contains(faction.GetFactionKey()))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnOwnershipChanged(bool changing, bool becameOwner)
	{
		if (changing)
			return;

		ProcessEvents(becameOwner);
	}

	//------------------------------------------------------------------------------------------------
	protected void ProcessEvents(bool activate)
	{
		if (RplSession.Mode() == RplMode.Dedicated)
			return;

		if (activate)
			SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		else
			SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_MapEntity = SCR_MapEntity.GetMapInstance();

		m_PlayerController = SCR_PlayerController.Cast(owner);

		m_PlayerController.GetOnOwnershipChangedInvoker().Insert(OnOwnershipChanged);

		RplComponent rpl = RplComponent.Cast(m_PlayerController.FindComponent(RplComponent));

		m_GroupTaskManager = SCR_GroupTaskManagerComponent.GetInstance();

		if (rpl && rpl.IsOwner())
			ProcessEvents(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!GetGame().InPlayMode())
			return;

		SetEventMask(owner, EntityEvent.INIT);
	}
}
