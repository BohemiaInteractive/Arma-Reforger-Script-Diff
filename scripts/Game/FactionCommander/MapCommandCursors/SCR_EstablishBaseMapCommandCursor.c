class SCR_EstablishBaseMapCommandCursor : SCR_MapCommandCursor
{
	protected Faction m_CommanderFaction;
	protected int m_iPlayerId;
	protected IEntity m_HoveredEntity;
	protected ENotification m_eBlockingNotification;
	protected BaseWorld m_World;
	protected float m_fOceanBaseHeight;
	protected bool m_bIsOcean;
	protected float m_fSignalRangeOffset;

	protected const int CONSTRUCTION_TRUCK_BUILDING_RADIUS = 35;

	//------------------------------------------------------------------------------------------------
	int GetPlayerId()
	{
		return m_iPlayerId;
	}

	//------------------------------------------------------------------------------------------------
	void SetData(notnull Faction commanderFaction, int playerId, IEntity hoveredEntity)
	{
		m_CommanderFaction = commanderFaction;
		m_iPlayerId = playerId;
		m_HoveredEntity = hoveredEntity;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool CanExecuteCommand(vector position)
	{
		m_eBlockingNotification = ENotification.UNKNOWN;

		SCR_GameModeCampaign gameModeCampaign = SCR_GameModeCampaign.Cast(GetGame().GetGameMode());
		if (!gameModeCampaign || !gameModeCampaign.GetEstablishingBasesEnabled())
			return false;

		SCR_EstablishBaseTaskEntity hoveredTask = SCR_EstablishBaseTaskEntity.Cast(m_HoveredEntity);
		if (hoveredTask)
			return true;

		if (!gameModeCampaign.GetBaseManager().CanFactionBuildNewBase(m_CommanderFaction))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_LIMIT_REACHED;
			return false;
		}

		if (m_World && m_bIsOcean && m_World.GetSurfaceY(position[0], position[2]) < m_fOceanBaseHeight)
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_INVALID_AREA;
			return false;
		}

		if (!gameModeCampaign.GetBaseManager().IsPositionInFactionRadioSignal(position, m_CommanderFaction, m_fSignalRangeOffset))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_OUTSIDE_RADIO_RANGE;
			return false;
		}

		if (SCR_FactionCommanderEstablishBaseMenuHandlerHelper.IsNearAnyBase(m_CommanderFaction, position))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_BASE;
			return false;
		}

		if (SCR_FactionCommanderEstablishBaseMenuHandlerHelper.IsNearAnyEstablishTask(m_CommanderFaction, position))
		{
			m_eBlockingNotification = ENotification.GROUP_TASK_ESTABLISH_BASE_TOO_CLOSE_ANOTHER_OBJECTIVE;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnCommandNotExecuted(vector position)
	{
		super.OnCommandNotExecuted(position);

		if (m_eBlockingNotification != ENotification.UNKNOWN)
			SCR_NotificationsComponent.SendLocal(m_eBlockingNotification);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_EstablishBaseMapCommandCursor()
	{
		// The maximum distance from the base at which an establish base objective can be placed must be reduced by establish base radius + construction truck building radius
		// It serves as a preventive measure so that a newly built base can always connect to the nearest base.
		m_fSignalRangeOffset = -SCR_CampaignFactionCommanderHandlerComponent.Cast(SCR_FactionCommanderHandlerComponent.GetInstance()).GetBaseEstablishingRadius() - CONSTRUCTION_TRUCK_BUILDING_RADIUS;

		m_World = GetGame().GetWorld();
		if (!m_World)
			return;

		if (!m_World.IsOcean())
			return;

		m_bIsOcean = true;
		m_fOceanBaseHeight = m_World.GetOceanBaseHeight();
	}
}
