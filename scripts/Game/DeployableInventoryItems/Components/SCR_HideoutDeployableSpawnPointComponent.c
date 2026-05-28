[EntityEditorProps(category: "GameScripted/DeployableItems", description: "Deployable spawn point that ignores gamemode budget type override, keeping the prefab-configured budget type (e.g. SUPPLIES)")]
class SCR_HideoutDeployableSpawnPointComponentClass : SCR_RestrictedDeployableSpawnPointComponentClass
{
}

//------------------------------------------------------------------------------------------------
//! Deployable spawn point that preserves its prefab-configured budget type
//! instead of letting the gamemode override it.
//! Used for FIA hideout tents that should always consume supplies regardless of gamemode settings.
class SCR_HideoutDeployableSpawnPointComponent : SCR_RestrictedDeployableSpawnPointComponent
{
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		// Save the prefab-configured budget type before super overwrites it
		SCR_ESpawnPointBudgetType prefabBudgetType = m_eRespawnBudgetType;
		int prefabMaxRespawns = m_iMaxRespawns;

		super.EOnInit(owner);

		// Restore the prefab values that were overwritten by the gamemode
		m_eRespawnBudgetType = prefabBudgetType;
		m_iMaxRespawns = prefabMaxRespawns;
		Replication.BumpMe();

		// Unsubscribe from gamemode budget/ticket changes so they don't override us later
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;

		SCR_PlayerSpawnPointManagerComponent playerSpawnPointManager = SCR_PlayerSpawnPointManagerComponent.Cast(gameMode.FindComponent(SCR_PlayerSpawnPointManagerComponent));
		if (playerSpawnPointManager)
		{
			playerSpawnPointManager.GetOnSpawnPointBudgetTypeChanged().Remove(OnSpawnPointBudgetTypeChanged);
			playerSpawnPointManager.GetOnSpawnPointTicketAmountChanged().Remove(OnSpawnPointTicketAmountChanged);
		}
	}

}
