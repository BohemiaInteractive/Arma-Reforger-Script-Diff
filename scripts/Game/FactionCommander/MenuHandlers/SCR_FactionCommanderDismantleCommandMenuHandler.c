[BaseContainerProps()]
class SCR_FactionCommanderDismantleCommandMenuHandler : SCR_FactionCommanderConflictBaseMenuHandler
{
	[Attribute("-10 0 -10", desc:"Task offset")]
	protected vector m_vPositionOffset;

	protected const string HQ_CANNOT_BE_DISMANTLED = "#AR-FactionCommander_HQCannotBeDismantled";
	protected const string BASE_CANNOT_BE_DISMANTLED = "#AR-FactionCommander_BaseCannotBeDismantled";

	//------------------------------------------------------------------------------------------------
	override bool CanCreateObjectiveEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanCreateObjectiveEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		SCR_Task task = GetTaskOnHoveredBase(commanderFaction, SCR_DismantleCampaignMilitaryBaseTaskEntity);
		if (task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanQuickEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		SCR_Task task = SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(hoveredEntity);
		if (!task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		if (!super.CanEntryBeSelected(commanderFaction, playerId, position, hoveredEntity))
		{
			disabledText = BASE_NOT_SELECTED_TEXT;
			return false;
		}

		SCR_CampaignFaction campaignFaction = SCR_CampaignFaction.Cast(commanderFaction);
		if (!campaignFaction)
			return false;

		SCR_CampaignMilitaryBaseComponent hoveredBase = GetHoveredBase();
		if (hoveredBase.GetFaction() != commanderFaction)
			return false;

		if (hoveredBase.IsHQ())
		{
			disabledText = HQ_CANNOT_BE_DISMANTLED;
			return false;
		}

		if (hoveredBase.IsControlPoint() || hoveredBase.GetType() == SCR_ECampaignBaseType.RELAY || hoveredBase.GetType() == SCR_ECampaignBaseType.SOURCE_BASE)
		{
			disabledText = BASE_CANNOT_BE_DISMANTLED;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		if (!super.CanGroupEntryBeSelected(commanderFaction, playerId, position, hoveredEntity, group))
			return false;

		return CanCampaignGroupEntryBeSelected(commanderFaction, hoveredEntity, group, SCR_DismantleCampaignMilitaryBaseTaskEntity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_CampaignFactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCampaignCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_Task task = SCR_DismantleCampaignMilitaryBaseTaskEntity.Cast(hoveredEntity);
		if (!task)
			task = GetTaskOnHoveredBase(commanderFaction, SCR_DismantleCampaignMilitaryBaseTaskEntity);

		if (!task)
			factionCommanderPlayerComponent.CreateCampaignMilitaryBaseTask(GetTaskPrefab(), group, playerId, this, m_vPositionOffset);
		else if (group)
			factionCommanderPlayerComponent.AssignGroupToTask(group.GetGroupID(), task.GetTaskID(), playerId);
	}
}
