[BaseContainerProps()]
class SCR_AnalyticsDataCollectionPlayerFactionModule : SCR_AnalyticsDataCollectionModule
{
	protected ref map<int, Faction> m_mPlayerFactions = new map<int, Faction>();

	//------------------------------------------------------------------------------------------------
	protected override void Enable()
	{
		SCR_FactionManager fm = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!fm)
			return;

		fm.GetOnPlayerFactionChanged_S().Insert(OnPlayerFactionChanged);

		super.Enable();
	}

	//------------------------------------------------------------------------------------------------
	protected override void Disable()
	{
		SCR_FactionManager fm = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!fm)
			return;

		fm.GetOnPlayerFactionChanged_S().Remove(OnPlayerFactionChanged);

		super.Disable();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPlayerFactionChanged(int playerId, SCR_PlayerFactionAffiliationComponent playerFactionAffiliationComponent, Faction faction)
	{
		if (!faction)
			return;

		m_mPlayerFactions.Insert(playerId, faction);

		SCR_PlayerData playerData = SCR_PlayerData.GetPlayerData(playerId);
		if (!playerData)
			return;

		playerData.dataEvent.name_faction_player = faction.GetFactionKey();
	}

	#ifdef ENABLE_DIAG
	//------------------------------------------------------------------------------------------------
	override void DrawContent()
	{
		if (!m_bIsEnabled)
		{
			DbgUI.Text("Module disabled!");
			return;
		}

		foreach (int playerID, Faction playerFaction : m_mPlayerFactions)
		{
			DbgUI.Text(string.Format("Player %1 - Faction %2", GetGame().GetPlayerManager().GetPlayerName(playerID), playerFaction.GetFactionKey()));
		}
	}
	#endif

	//------------------------------------------------------------------------------------------------
	protected void ~SCR_AnalyticsDataCollectionPlayerFactionModule()
	{
		Disable();
	}
}
