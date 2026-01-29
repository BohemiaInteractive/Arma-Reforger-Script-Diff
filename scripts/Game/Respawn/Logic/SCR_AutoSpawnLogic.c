//------------------------------------------------------------------------------------------------
/*
	Object responsible for handling respawn logic on the authority side.
*/
[BaseContainerProps(category: "Respawn")]
class SCR_AutoSpawnLogic : SCR_SpawnLogic
{
	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default faction for players to spawn with or empty if none.")]
	protected FactionKey m_sForcedFaction;

	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default loadout for players to spawn with or empty if none")]
	protected string m_sForcedLoadout;

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess_S(int playerId)
	{
		super.OnPlayerAuditSuccess_S(playerId);
		ExcuteInitialLoadOrSpawn_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityLost_S(int playerId)
	{
		super.OnPlayerEntityLost_S(playerId);
		DoSpawn_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerSpawnFailed_S(int playerId)
	{
		super.OnPlayerSpawnFailed_S(playerId);

		const int delay = Math.RandomFloat(900, 1100);
		GetGame().GetCallqueue().CallLater(DoSpawn_S, delay, false, playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DoSpawn_S(int playerId)
	{
		array<Faction> factions = {};
		GetGame().GetFactionManager().GetFactionsList(factions);

		Faction targetFaction;
		if (!GetForcedFaction(targetFaction))
			targetFaction = factions.GetRandomElement();

		GetPlayerFactionComponent_S(playerId).RequestFaction(targetFaction);

		SCR_BasePlayerLoadout targetLoadout;
		if (!GetForcedLoadout(targetLoadout))
			targetLoadout = GetGame().GetLoadoutManager().GetRandomFactionLoadout(targetFaction);

		GetPlayerLoadoutComponent_S(playerId).RequestLoadout(targetLoadout);

		Faction faction = GetPlayerFactionComponent_S(playerId).GetAffiliatedFaction();
		if (!faction)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_BasePlayerLoadout loadout = GetPlayerLoadoutComponent_S(playerId).GetLoadout();
		if (!loadout)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPoint point = SCR_SpawnPoint.GetRandomSpawnPointForFaction(faction.GetFactionKey());
		if (!point)
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		SCR_SpawnPointSpawnData data = new SCR_SpawnPointSpawnData(loadout.GetLoadoutResource(), point.GetRplId());
		if (!GetPlayerRespawnComponent_S(playerId).CanSpawn(data))
		{
			OnPlayerSpawnFailed_S(playerId);
			return;
		}

		GetPlayerRespawnComponent_S(playerId).RequestSpawn(data);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetForcedFaction(out Faction faction)
	{
		if (m_sForcedFaction.IsEmpty() && 
			!System.GetCLIParam("autodeployFaction", m_sForcedFaction) && 
			!System.GetCLIParam("tdmf", m_sForcedFaction))
		{
			return false;
		}

		faction = GetGame().GetFactionManager().GetFactionByKey(m_sForcedFaction);
		if (!faction)
		{
			Print(string.Format("Auto spawn logic did not find faction by name: %1", m_sForcedFaction), LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetForcedLoadout(out SCR_BasePlayerLoadout loadout)
	{
		if (m_sForcedLoadout.IsEmpty())
			return false;

		loadout = GetGame().GetLoadoutManager().GetLoadoutByName(m_sForcedLoadout, m_sForcedFaction);
		if (!loadout)
		{
			Print(string.Format("Auto spawn logic did not find loadout by name: %1", m_sForcedLoadout), LogLevel.WARNING);
			return false;
		}
		return true;
	}
}
