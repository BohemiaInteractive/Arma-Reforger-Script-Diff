[BaseContainerProps(), SCR_BaseGroupCommandTitleField("m_sCommandName")]
class SCR_SetRallyPointCommand : SCR_BaseRadialCommand
{
	[Attribute("Assign Rally Point", desc: "Text displayed for set rally point command.")]
	protected string m_sAssignRallyPointCommandString;

	[Attribute("Unassign Rally Point", desc: "Text displayed for remove really point command.")]
	protected string m_sUnassignRallyPointCommandString;

	[Attribute("No Base Selected", desc: "Text display for disabled command due to no base selected.")]
	protected string m_sNoBaseSelectedString;

	protected SCR_CampaignMilitaryBaseComponent m_HoveredBase;
	protected SCR_CampaignMilitaryBaseComponent m_CurrentHoveredBase;
	protected ref SCR_SelectionMenuEntry m_RadialMenuEntry;
	protected SCR_PlayerControllerGroupComponent m_PlayerControllerGroupComponent;
	protected MapConfiguration m_MapConfig;

	protected bool m_bAssign;

	//------------------------------------------------------------------------------------------------
	override bool CanRoleShow()
	{
		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerControllerGroupComponent)
			return false;

		return m_PlayerControllerGroupComponent.IsPlayerLeaderOwnGroup();
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanShowOnMap()
	{
		if (m_MapConfig.MapEntityMode != EMapEntityMode.FULLSCREEN)
			return false;

		if (!SCR_GameModeCampaign.GetInstance())
			return false;

		m_PlayerControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!m_PlayerControllerGroupComponent)
			return false;

		if (!m_PlayerControllerGroupComponent.IsPlayerLeaderOwnGroup())
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void OnMapOpen(MapConfiguration config)
	{
		m_MapConfig = config;
		if (m_MapConfig.MapEntityMode != EMapEntityMode.FULLSCREEN)
			return;

		SCR_MapRadialUI mapRadialUI = SCR_MapRadialUI.GetInstance();
		if (mapRadialUI)
			mapRadialUI.GetOnMenuInitInvoker().Insert(OnRadialMenuInit);

		SCR_MapCampaignUI mapCampaignUI = SCR_MapCampaignUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapCampaignUI));
		if (mapCampaignUI)
			mapCampaignUI.GetOnBaseHovered().Insert(OnBaseHovered);
	}

	//------------------------------------------------------------------------------------------------
	void OnMapClose(MapConfiguration config)
	{
		m_MapConfig = null;
		if (config.MapEntityMode != EMapEntityMode.FULLSCREEN)
			return;

		SCR_MapCampaignUI mapCampaignUI = SCR_MapCampaignUI.Cast(SCR_MapEntity.GetMapInstance().GetMapUIComponent(SCR_MapCampaignUI));
		if (mapCampaignUI)
			mapCampaignUI.GetOnBaseHovered().Remove(OnBaseHovered);

		SCR_MapRadialUI mapRadialUI = SCR_MapRadialUI.GetInstance();
		if (mapRadialUI)
		{
			mapRadialUI.GetOnMenuInitInvoker().Remove(OnRadialMenuInit);

			SCR_RadialMenu mapRadialMenu = mapRadialUI.GetRadialController().GetRadialMenu();
			if (mapRadialMenu)
			{
 				mapRadialMenu.GetOnOpen().Remove(OnMapRadialMenuOpen);
				mapRadialMenu.GetOnAddEntry().Remove(OnRadialMenuEntryAdded);
			}	
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnRadialMenuInit()
	{
		SCR_MapRadialUI mapRadialUI = SCR_MapRadialUI.GetInstance();
		if (mapRadialUI)
		{
			SCR_RadialMenu mapRadialMenu = mapRadialUI.GetRadialController().GetRadialMenu();
			if (mapRadialMenu)
			{
				mapRadialMenu.GetOnOpen().Insert(OnMapRadialMenuOpen);
				mapRadialMenu.GetOnAddEntry().Insert(OnRadialMenuEntryAdded);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnRadialMenuEntryAdded(SCR_SelectionMenu menu, SCR_SelectionMenuEntry entry)
	{
		SCR_RadialMenu radialMenu = SCR_RadialMenu.Cast(menu);
		if (!radialMenu)
			return;

		m_RadialMenuEntry = FindCommandEntry(radialMenu);
		if (!m_RadialMenuEntry)
			return;

		m_RadialMenuEntry.Enable(SetRadialMenuEntryEnabled());
		menu.GetOnAddEntry().Remove(OnRadialMenuEntryAdded);
	}

	//------------------------------------------------------------------------------------------------
	void OnMapRadialMenuOpen()
	{
		m_HoveredBase = m_CurrentHoveredBase;

		if (!m_RadialMenuEntry)
			return;

		m_RadialMenuEntry.Enable(SetRadialMenuEntryEnabled());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseHovered(SCR_CampaignMilitaryBaseComponent base)
	{
		m_CurrentHoveredBase = base;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SelectionMenuEntry FindCommandEntry(SCR_RadialMenu mapRadialMenu)
	{
		SCR_SelectionMenuEntry foundEntry;
		SCR_SelectionMenuCategoryEntry categoryEntry;
		string commandName;

		foreach (SCR_SelectionMenuEntry entry : mapRadialMenu.GetEntries())
		{
			categoryEntry = SCR_SelectionMenuCategoryEntry.Cast(entry);
			if (categoryEntry)
				foundEntry = FindCommandEntryInCategory(categoryEntry);

			if (foundEntry)
				return foundEntry;

			commandName = entry.GetId();
			SCR_SetRallyPointCommand command = SCR_SetRallyPointCommand.Cast(SCR_CommandingManagerComponent.GetInstance().FindCommand(commandName));
			if (!command)
				continue;

			foundEntry = entry;
			break;
		}

		return foundEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_SelectionMenuEntry FindCommandEntryInCategory(SCR_SelectionMenuCategoryEntry category)
	{
		SCR_SelectionMenuEntry foundEntry;
		SCR_SelectionMenuCategoryEntry categoryEntry;
		string commandName;

		foreach (SCR_SelectionMenuEntry element : category.GetEntries())
		{
			categoryEntry = SCR_SelectionMenuCategoryEntry.Cast(element);
			if (categoryEntry)
				foundEntry = FindCommandEntryInCategory(categoryEntry);

			if (foundEntry)
				return foundEntry;

			SCR_MapMenuCommandingEntry commandingEntry = SCR_MapMenuCommandingEntry.Cast(element);
			if (!commandingEntry)
				continue;

			commandName = commandingEntry.GetEntryIdentifier();
			SCR_SetRallyPointCommand command = SCR_SetRallyPointCommand.Cast(SCR_CommandingManagerComponent.GetInstance().FindCommand(commandName));
			if (!command)
				continue;

			foundEntry = element;
			break;
		}

		return foundEntry;
	}

	//------------------------------------------------------------------------------------------------
	protected bool SetRadialMenuEntryEnabled()
	{
		if (!m_PlayerControllerGroupComponent)
			return false;

		Faction playerFaction = SCR_FactionManager.SGetPlayerFaction(m_PlayerControllerGroupComponent.GetPlayerID());
		if (!playerFaction)
			return false;

		if (!m_HoveredBase)
		{
			m_RadialMenuEntry.SetName(m_sNoBaseSelectedString);
			return false;
		}

		if (m_HoveredBase.GetType() != SCR_ECampaignBaseType.BASE || m_HoveredBase.GetFaction() != playerFaction)
		{
			m_RadialMenuEntry.SetName(m_sAssignRallyPointCommandString);
			return false;
		}

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		SCR_AIGroup group = groupsManager.FindGroup(m_PlayerControllerGroupComponent.GetGroupID());
		if (!group)
			return false;

		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!campaign)
			return false;

		SCR_CampaignMilitaryBaseManager baseManager = campaign.GetBaseManager();
		if (!baseManager)
			return false;

		int groupRallyPointId = group.GetRallyPointId();
		SCR_CampaignMilitaryBaseComponent groupRallyPoint = baseManager.FindBaseByCallsign(groupRallyPointId);
		if (!groupRallyPoint)
		{
			m_RadialMenuEntry.SetName(m_sAssignRallyPointCommandString);
			m_bAssign = true;

			return true;
		}

		if (groupRallyPoint == m_HoveredBase)
		{
			m_RadialMenuEntry.SetName(m_sUnassignRallyPointCommandString);
			m_bAssign = false;
		}
		else
		{
			m_RadialMenuEntry.SetName(m_sAssignRallyPointCommandString);
			m_bAssign = true;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformed(notnull SCR_ChimeraCharacter user)
	{
		if (!m_PlayerControllerGroupComponent || !m_PlayerControllerGroupComponent.IsPlayerLeaderOwnGroup())
			return false;

		return super.CanBePerformed(user);
	}

	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity target, vector targetPosition, int playerID, bool isClient)
	{
		if (!m_HoveredBase)
			return false;

		SCR_PlayerControllerGroupComponent groupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!groupController)
			return false;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManager)
			return false;

		SCR_AIGroup group = groupsManager.GetPlayerGroup(playerID);
		if (!group || !group.IsPlayerLeader(playerID))
			return false;

		if (m_bAssign)
			groupController.SetRallyPoint(m_HoveredBase, true);
		else
			groupController.RemoveRallyPoint();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! constructor
	void SCR_SetRallyPointCommand()
	{
		SCR_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);
	}

	//------------------------------------------------------------------------------------------------
	//! destructor
	void ~SCR_SetRallyPointCommand()
	{
		SCR_MapEntity.GetOnMapOpen().Remove(OnMapOpen);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);
	}
}
