[BaseContainerProps()]
class SCR_FactionCommanderCommandConflictBaseMenuHandler : SCR_FactionCommanderConflictBaseMenuHandler
{
	[Attribute(desc:"If checked, player commander can only place this command on enemy base")]
	protected bool m_bMustPlaceOnEnemyBase;

	[Attribute("0 0 0", desc:"Task offset")]
	protected vector m_vPositionOffset;

	//------------------------------------------------------------------------------------------------
	override bool CanCreateObjectiveEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanCreateObjectiveEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		SCR_Task task = GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
		if (task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanQuickEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		return IsTaskValid(hoveredEntity);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		SCR_CampaignMilitaryBaseComponent hoveredBase = GetHoveredBase();
		if (!hoveredBase)
		{
			disabledText = BASE_NOT_SELECTED_TEXT;
			return false;
		}

		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(commanderFaction);
		if (!campaignFaction)
			return false;

		SCR_Task task = GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
		if (!m_bIsObjectiveCreationEnabled && !task)
			return false;

		if (m_bMustPlaceOnEnemyBase)
			return hoveredBase.IsValidTarget(campaignFaction);

		return hoveredBase.GetFaction() == commanderFaction;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_Task task = GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
		if (!m_bIsObjectiveCreationEnabled && !task)
			return false;

		if (!super.CanGroupEntryBeShown(commanderFaction, playerId, position, hoveredEntity, group))
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		if (!super.CanGroupEntryBeSelected(commanderFaction, playerId, position, hoveredEntity, group))
			return false;

		return CanCampaignGroupEntryBeSelected(commanderFaction, hoveredEntity, group, GetTaskPrefabClassTypename());
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_CampaignFactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCampaignCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_Task task = GetTask(hoveredEntity, commanderFaction, GetTaskPrefabClassTypename());
		if (!task)
		{
			factionCommanderPlayerComponent.CreateCampaignMilitaryBaseTask(GetTaskPrefab(), group, playerId, this, m_vPositionOffset);
			return;
		}

		if (group)
			factionCommanderPlayerComponent.AssignGroupToTask(group.GetGroupID(), task.GetTaskID(), playerId);
	}
}
