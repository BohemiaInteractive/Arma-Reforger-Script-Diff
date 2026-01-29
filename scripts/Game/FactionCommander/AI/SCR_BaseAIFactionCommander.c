[BaseContainerProps()]
class SCR_BaseAIFactionCommander
{
	[Attribute()]
	protected FactionKey m_sFactionKey;

	protected SCR_FactionCommanderHandlerComponent m_FactionCommanderHandlerComponent;
	protected SCR_Faction m_Faction;
	protected bool m_bIsAICommanderActivated

	//------------------------------------------------------------------------------------------------
	protected void OnAICommanderActivated()
	{
		m_bIsAICommanderActivated = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAICommanderDeactivated()
	{
		m_bIsAICommanderActivated = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnFactionCommanderChanged(notnull SCR_Faction faction, int commanderPlayerId)
	{
		if (faction != m_Faction)
			return;

		UpdateAICommanderState();
	}

	//------------------------------------------------------------------------------------------------
	//! Init AI faction commander
	//! \param[in] factionCommanderHandlerComponent
	void Init(notnull SCR_FactionCommanderHandlerComponent factionCommanderHandlerComponent)
	{
		m_FactionCommanderHandlerComponent = factionCommanderHandlerComponent;

		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return;

		m_Faction = SCR_Faction.Cast(factionManager.GetFactionByKey(m_sFactionKey));
		if (!m_Faction)
			return;

		UpdateAICommanderState();

		m_FactionCommanderHandlerComponent.GetOnFactionCommanderChanged().Insert(OnFactionCommanderChanged);
	}

	//------------------------------------------------------------------------------------------------
	//! Deinit AI faction commander
	void Deinit()
	{
		if (!m_FactionCommanderHandlerComponent)
			return;

		m_FactionCommanderHandlerComponent.GetOnFactionCommanderChanged().Remove(OnFactionCommanderChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateAICommanderState()
	{
		if (!m_Faction)
			return;

		bool isAICommander = m_Faction.IsAICommander();
		if (isAICommander == m_bIsAICommanderActivated)
			return;

		if (isAICommander)
			OnAICommanderActivated();
		else
			OnAICommanderDeactivated();
	}
}
