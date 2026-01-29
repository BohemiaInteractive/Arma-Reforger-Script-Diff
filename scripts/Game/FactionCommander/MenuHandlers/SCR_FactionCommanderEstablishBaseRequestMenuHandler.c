[BaseContainerProps()]
class SCR_FactionCommanderEstablishBaseRequestMenuHandler : SCR_FactionCommanderBaseRequestMenuHandler
{
	protected int m_iRequesterGroup;
	protected SCR_AIGroup m_Group;
	protected ref SCR_EstablishBaseMapCommandCursor m_MapCommandCursor;

	protected const string ESTABLISH_BASE_LIMIT_REACHED = "#AR-Notification_GROUP_TASK_ESTABLISH_BASE_TOO_MANY_BASES";

	//------------------------------------------------------------------------------------------------
	override bool CanQuickEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		if (!super.CanQuickEntryBeShown(commanderFaction, playerId, position, hoveredEntity))
			return false;

		SCR_EstablishBaseRequestedTaskEntity task = SCR_EstablishBaseRequestedTaskEntity.Cast(hoveredEntity);
		if (!task)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, out string disabledText = "")
	{
		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (gameModeCampaign && !gameModeCampaign.GetBaseManager().CanFactionBuildNewBase(commanderFaction))
		{
			disabledText = ESTABLISH_BASE_LIMIT_REACHED;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanGroupEntryBeSelected(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group, out string disabledText = "")
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnCommandIssued(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity, SCR_AIGroup group)
	{
		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		SCR_GroupsManagerComponent groupComp = SCR_GroupsManagerComponent.GetInstance();
		if (!groupComp)
			return;

		SCR_AIGroup requesterGroup = groupComp.GetPlayerGroup(playerId);
		if (!requesterGroup)
			return;

		SCR_EstablishBaseRequestedTaskEntity task = SCR_EstablishBaseRequestedTaskEntity.Cast(hoveredEntity);
		if (!task)
		{
			m_MapCommandCursor = new SCR_EstablishBaseMapCommandCursor();
			m_MapCommandCursor.ShowCursor(position);
			m_MapCommandCursor.SetData(commanderFaction, playerId, hoveredEntity);
			m_MapCommandCursor.GetOnCommandExecuted().Insert(OnCommandExecuted);
			m_Group = group;
			m_iRequesterGroup = requesterGroup.GetGroupID();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCommandExecuted(vector position)
	{
		if (!m_MapCommandCursor)
			return;

		SCR_FactionCommanderPlayerComponent factionCommanderPlayerComponent = GetLocalCommanderComponent();
		if (!factionCommanderPlayerComponent)
			return;

		factionCommanderPlayerComponent.CreateRequestedTask(GetTaskPrefab(), position, m_iRequesterGroup, m_Group, m_MapCommandCursor.GetPlayerId(), this);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanEntryBeShown(notnull Faction commanderFaction, int playerId, vector position, IEntity hoveredEntity)
	{
		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (gameModeCampaign && !gameModeCampaign.GetEstablishingBasesEnabled())
			return false;

		return super.CanEntryBeShown(commanderFaction, playerId, position, hoveredEntity);
	}
}
