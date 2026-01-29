//! Handles interactions for the faction Commander role in Conflict. Should be attached to player controller.
[ComponentEditorProps(category: "GameScripted/Commander", description: "Handles interactions for the faction Commander role in Conflict. Should be attached to player controller.")]
class SCR_CampaignFactionCommanderPlayerComponentClass : SCR_FactionCommanderPlayerComponentClass
{
	[Attribute("{EA5C18CC750579E2}Configs/Campaign/CommanderGraphLinesConfig.conf", category: "Lines")]
	protected ref SCR_CommanderGraphLinesData m_GraphLinesData;

	//------------------------------------------------------------------------------------------------
	SCR_CommanderGraphLinesData GetGraphLinesData()
	{
		return m_GraphLinesData;
	}
}

class SCR_CampaignFactionCommanderPlayerComponent : SCR_FactionCommanderPlayerComponent
{
	protected SCR_CampaignMilitaryBaseComponent m_HoveredBase;
	protected SCR_CampaignMilitaryBaseComponent m_CurrentHoveredBase;

	protected bool m_bMustClearHoveredBase;

	protected ref array<MapLink> m_aMapLinks = {};

	static const string CAMPAIGN_MILITARY_BASE_TASK_ID = "%1_CampaignMilitaryBaseTask_%2_%3";

	//------------------------------------------------------------------------------------------------
	static SCR_CampaignFactionCommanderPlayerComponent GetLocalCampaignCommanderComponent()
	{
		return SCR_CampaignFactionCommanderPlayerComponent.Cast(GetGame().GetPlayerController().FindComponent(SCR_CampaignFactionCommanderPlayerComponent));
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_CommanderGraphLinesData GetGraphLinesData()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return null;

		SCR_CampaignFactionCommanderPlayerComponentClass componentData = SCR_CampaignFactionCommanderPlayerComponentClass.Cast(GetComponentData(owner));

		if (!componentData)
			return null;

		return componentData.GetGraphLinesData();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);

		m_HoveredBase = null;

		if (!m_MapEntity)
			return;

		SCR_MapCampaignUI mapCampaignUI = SCR_MapCampaignUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapCampaignUI));
		if (mapCampaignUI)
			mapCampaignUI.GetOnBaseHovered().Insert(OnBaseHovered);

	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		super.OnMapClose(config);

		m_HoveredBase = null;
		if (m_MapEntity)
		{
			SCR_MapCampaignUI mapCampaignUI = SCR_MapCampaignUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapCampaignUI));
			if (mapCampaignUI)
				mapCampaignUI.GetOnBaseHovered().Remove(OnBaseHovered);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBaseHovered(SCR_CampaignMilitaryBaseComponent base)
	{
		m_bMustClearHoveredBase = base == null;

		m_CurrentHoveredBase = base;

		if (base)
			m_CurrentHoveredEntity = base.GetOwner();
		else
			m_CurrentHoveredEntity = null;
	}

	//------------------------------------------------------------------------------------------------
	SCR_CampaignMilitaryBaseComponent GetHoveredBase()
	{
		return m_HoveredBase;
	}

	//------------------------------------------------------------------------------------------------
	//! Called each time the radial menu is before open in the map
	override protected void OnRadialMenuBeforeOpen()
	{
		super.OnRadialMenuBeforeOpen();

		m_HoveredBase = m_CurrentHoveredBase;
		m_HoveredEntity = m_CurrentHoveredEntity;
	}

	//------------------------------------------------------------------------------------------------
	override void ClearRadialMenu()
	{
		super.ClearRadialMenu();

		ClearAllLinks();

		if (m_bMustClearHoveredBase)
			m_HoveredBase = null;

		m_bMustClearHoveredBase = false;

	}

	//------------------------------------------------------------------------------------------------
	//! Triggered when a radial menu entry is hovered on
	override void OnEntryHovered(SCR_SelectionMenuEntry entry, int id)
	{
		super.OnEntryHovered(entry, id);

		ClearAllLinks();

		if (!m_HoveredBase || !entry)
			return;

		Faction faction = SCR_FactionManager.SGetLocalPlayerFaction();

		if (!faction)
			return;

		SCR_GroupsManagerComponent groupsManager = SCR_GroupsManagerComponent.Cast(GetGame().GetGameMode().FindComponent(SCR_GroupsManagerComponent));

		if (!groupsManager)
			return;

		SCR_AIGroup selectedGroup = m_mEntryGroups.Get(entry);

		if (selectedGroup)
		{
			// Draw a link to selected group
			LinkBaseWithGroup(selectedGroup);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Draw a map link from a base to a group leader
	protected void LinkBaseWithGroup(notnull SCR_AIGroup group)
	{
		IEntity leader = GetGame().GetPlayerManager().GetPlayerControlledEntity(group.GetLeaderID());

		if (!leader || leader == SCR_PlayerController.GetLocalControlledEntity())
			return;

		MapDescriptorComponent desc = MapDescriptorComponent.Cast(leader.FindComponent(MapDescriptorComponent));

		if (!desc)
			return;

		MapItem item = desc.Item();

		if (!item)
			return;

		MapDescriptorComponent baseDesc = MapDescriptorComponent.Cast(m_HoveredBase.GetOwner().FindComponent(MapDescriptorComponent));

		if (!baseDesc)
			return;

		MapItem baseItem = baseDesc.Item();

		if (!baseItem)
			return;

		MapLink link = baseItem.LinkTo(item);
		m_aMapLinks.Insert(link);
		MapLinkProps props = link.GetMapLinkProps();

		if (!props)
			return;

		SCR_CommanderGraphLinesData linesData = GetGraphLinesData();

		if (!linesData)
			return;

		props.SetLineWidth(linesData.GetLineWidth());
		props.SetLineColor(linesData.GetColor());
	}

	//------------------------------------------------------------------------------------------------
	//! Remove all existing map links
	protected void ClearAllLinks()
	{
		if (!m_HoveredBase)
			return;

		MapDescriptorComponent baseDesc = MapDescriptorComponent.Cast(m_HoveredBase.GetOwner().FindComponent(MapDescriptorComponent));

		if (!baseDesc)
			return;

		MapItem baseItem = baseDesc.Item();

		if (!baseItem)
			return;

		foreach (MapLink link : m_aMapLinks)
		{
			baseItem.UnLink(link.Target());
		}

		m_aMapLinks.Clear();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsValidRequest(SCR_CampaignMilitaryBaseComponent base)
	{
		if (!base)
			return false;

		SCR_Faction faction = SCR_Faction.Cast(base.GetFaction(true));
		if (!faction)
			return false;

		PlayerController sender = PlayerController.Cast(GetOwner());
		return sender && faction.IsPlayerCommander(sender.GetPlayerId());
	}

	//------------------------------------------------------------------------------------------------
	//! Create campaign military base task
	//! \param[in] taskPrefab
	//! \param[in] group
	//! \param[in] playerId
	//! \param[in] handler
	//! \param[in] positionOffset
	void CreateCampaignMilitaryBaseTask(ResourceName taskPrefab, SCR_AIGroup group, int playerId, SCR_FactionCommanderBaseMenuHandler handler = null, vector positionOffset = vector.Zero)
	{
		int groupId = -1;

		if (group)
			groupId = group.GetGroupID();

		int data;

		if (handler)
		{
			SCR_SelectionMenuEntry entry = m_mEntryHandlers.GetKeyByValue(handler);

			if (entry && entry.GetId())
				data = entry.GetId().ToInt();
		}

		int baseCallsign = SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN;

		if (m_HoveredBase)
			baseCallsign = m_HoveredBase.GetCallsign();

		if (baseCallsign == SCR_MilitaryBaseComponent.INVALID_BASE_CALLSIGN)
			return;

		Rpc(RpcAsk_CreateCampaignMilitaryBaseTask, taskPrefab, groupId, GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetLocalPlayerFaction()), playerId, data, baseCallsign, positionOffset);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CreateCampaignMilitaryBaseTask(ResourceName taskPrefab, int assigneeId, int factionIndex, int playerId, int data, int baseCallsign, vector positionOffset)
	{
		PlayerController sender = PlayerController.Cast(GetOwner());
		if (!sender)
			return;

		SCR_Faction faction = SCR_Faction.Cast(GetGame().GetFactionManager().GetFactionByIndex(factionIndex));
		if (!faction || !faction.IsPlayerCommander(sender.GetPlayerId()))
			return;

		if (!m_GroupTaskManager.CanCreateNewTaskWithResourceName(taskPrefab, faction))
			return;

		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseCallsign);
		if (!base)
			return;

		SCR_TaskSystem taskSystem = SCR_TaskSystem.GetInstance();
		if (!taskSystem)
			return;

		string taskID = string.Format(CAMPAIGN_MILITARY_BASE_TASK_ID, faction.GetFactionKey(), base.GetCallsign(), GenerateTaskID());

		if (SCR_TaskSystem.GetTaskFromTaskID(taskID, false))
			return;

		SCR_CampaignMilitaryBaseTaskEntity task = SCR_CampaignMilitaryBaseTaskEntity.Cast(taskSystem.CreateTask(
			taskPrefab,
			taskID,
			"",
			"",
			base.GetOwner().GetOrigin() + positionOffset
		));

		if (!task)
		{
			Print("Task was not created", LogLevel.ERROR);
			return;
		}

		string title;
		string description;

		if (SCR_SeizeCampaignMilitaryBaseTaskEntity.Cast(task))
		{
			if (base.GetType() == SCR_ECampaignBaseType.RELAY)
			{
				title = SCR_CampaignFactionCommanderHandlerComponent.SEIZE_TASK_RELAY_NAME;
				description = SCR_CampaignFactionCommanderHandlerComponent.SEIZE_TASK_RELAY_DESC;
			}
			else
			{
				title = SCR_CampaignFactionCommanderHandlerComponent.SEIZE_TASK_NAME;
				description = SCR_CampaignFactionCommanderHandlerComponent.SEIZE_TASK_DESC;
			}

			task.SetTaskName(title, {base.GetFormattedBaseNameWithCallsign(faction)});
			task.SetTaskDescription(description);
		}
		else
		{
			SCR_TaskUIInfo taskUIInfo = task.GetTaskUIInfo();
			if (!taskUIInfo)
				return;

			task.SetTaskName(taskUIInfo.GetName(), {base.GetFormattedBaseNameWithCallsign(faction)});
		}

		task.SetMilitaryBaseCallSign(baseCallsign);
		taskSystem.SetTaskOwnership(task, SCR_ETaskOwnership.EXECUTOR);
		taskSystem.SetTaskVisibility(task, SCR_ETaskVisibility.GROUP);
		taskSystem.AddTaskFaction(task, faction.GetFactionKey());

		if (m_GroupTaskManager)
			m_GroupTaskManager.SetGroupTask(task, playerId);

		if (assigneeId > -1)
			taskSystem.AssignTask(task, SCR_TaskExecutorGroup.FromGroup(assigneeId), true, playerId); // force assign
	}

#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	//! Cheat method to set a local player as commander
	void CheatCommander()
	{
		int playerId = m_PlayerController.GetPlayerId();
		if (playerId == 0)
			return;

		Rpc(RpcAsk_CheatCommander, playerId);
	}
#endif

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_CheatCommander(int playerId)
	{
		if (!GetGame().IsDev())
			return;

		SCR_Faction playerFaction = SCR_Faction.Cast(SCR_FactionManager.SGetPlayerFaction(playerId));
		if (!playerFaction)
			return;

		SCR_CampaignFactionCommanderHandlerComponent handler = SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_CampaignFactionCommanderHandlerComponent.GetInstance());
		if (!handler)
			return;

		handler.SetFactionCommander(playerFaction, playerId);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the supply limit for a given campaign military base.
	//! \param[in] base Reference to the campaign military base whose supply limit will be set.
	//! \param[in] value The new supply limit to assign to the base.
	void SetSuppliesLimit(notnull SCR_CampaignMilitaryBaseComponent base, float value)
	{
		Rpc(RpcAsk_SetSuppliesLimit, base.GetCallsign(), value);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetSuppliesLimit(int baseCallsign, float value)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseCallsign);
		if (!IsValidRequest(base))
			return;

		base.SetSupplyLimit(value);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the reserved supply amount for a specified campaign military base.
	//! \param[in] base Reference to the campaign military base for which the reserved supply amount is set.
	//! \param[in] value The new value to assign as the reserved supply amount for the base.
	void SetReservedSupplyAmount(notnull SCR_CampaignMilitaryBaseComponent base, float value)
	{
		Rpc(RpcAsk_SetReservedSupplyAmount, base.GetCallsign(), value);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetReservedSupplyAmount(int baseCallsign, float value)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseCallsign);
		if (!IsValidRequest(base))
			return;

		base.SetReservedSupplyAmount(value);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the supply request execution priority for a specified campaign military base.
	//! \param[in] base Reference to the campaign military base whose supply request priority will be set.
	//! \param[in] priority The new execution priority to assign to the base.
	void SetSupplyRequestExecutionPriority(notnull SCR_CampaignMilitaryBaseComponent base, int priority)
	{
		Rpc(RpcAsk_SetSupplyRequestExecutionPriority, base.GetCallsign(), priority);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_SetSupplyRequestExecutionPriority(int baseCallsign, int priority)
	{
		SCR_CampaignMilitaryBaseComponent base = SCR_GameModeCampaign.GetInstance().GetBaseManager().FindBaseByCallsign(baseCallsign);
		if (!IsValidRequest(base))
			return;

		base.SetSupplyRequestExecutionPriority(priority);
	}
}
