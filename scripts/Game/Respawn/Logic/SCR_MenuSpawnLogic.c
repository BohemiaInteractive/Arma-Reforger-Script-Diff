[BaseContainerProps(category: "Respawn")]
class SCR_MenuSpawnLogic : SCR_SpawnLogic
{
	[Attribute("", uiwidget: UIWidgets.EditBox, category: "Respawn", desc: "Default faction for players to spawn with or empty if none.")]
	protected FactionKey m_sForcedFaction;

	[Attribute("4", desc: "Delay (in seconds) for opening deploy menu after death.")]
	protected float m_fDeployMenuOpenDelay;

	[Attribute("0", desc: "Delay opening deploy menu until player has a spawn point available")]
	protected bool m_bWaitForSpawnPoints;

	[Attribute("0", desc: "If true, use a fade effect when deploy map is open")]
	protected bool m_bUseFadeEffect;

	//------------------------------------------------------------------------------------------------
	override void OnPlayerAuditSuccess_S(int playerId)
	{
		super.OnPlayerAuditSuccess_S(playerId);
		ExcuteInitialLoadOrSpawn_S(playerId);
	}

	//------------------------------------------------------------------------------------------------
	override protected void DoSpawn_S(int playerId)
	{
		Faction forcedFaction;
		if (GetForcedFaction(forcedFaction))
		{
			PlayerController pc = GetGame().GetPlayerManager().GetPlayerController(playerId);
			SCR_PlayerFactionAffiliationComponent playerFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
			if (playerFactionComp)
				playerFactionComp.RequestFaction(forcedFaction);
		}

		// Send a notification to registered client:
		// Always ensure to hook OnLocalPlayer callbacks prior to sending such notification,
		// otherwise the notification will be disregarded
		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPlayerEntityLost_S(int playerId)
	{
		super.OnPlayerEntityLost_S(playerId);
		GetPlayerRespawnComponent_S(playerId).NotifyReadyForSpawn_S();
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetForcedFaction(out Faction faction)
	{
		if (m_sForcedFaction.IsEmpty())
			return false;

		// Resolve Alias
		SCR_FactionAliasComponent factionAliasComponent = SCR_FactionAliasComponent.Cast(GetGame().GetFactionManager().FindComponent(SCR_FactionAliasComponent));
		if (factionAliasComponent)
			m_sForcedFaction = factionAliasComponent.ResolveFactionAlias(m_sForcedFaction);

		faction = GetGame().GetFactionManager().GetFactionByKey(m_sForcedFaction);
		if (!faction)
		{
			Print(string.Format("Auto spawn logic did not find faction by name: %1", m_sForcedFaction), LogLevel.WARNING);
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Delay in seconds
	float GetDeployMenuOpenDelay()
	{
		return m_fDeployMenuOpenDelay;
	}

	//------------------------------------------------------------------------------------------------
	bool GetWaitForSpawnPoints()
	{
		return m_bWaitForSpawnPoints;
	}

	//------------------------------------------------------------------------------------------------
	bool GetUseFadeEffect()
	{
		return m_bUseFadeEffect;
	}
}
