class SCR_FastTravelAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(pUserEntity);
		if (!gadgetManager)
			return;

		IEntity mapGadget = gadgetManager.GetGadgetByType(EGadgetType.MAP);
		if (!mapGadget)
			return;

		gadgetManager.SetGadgetMode(mapGadget, EGadgetMode.IN_HAND);
		SCR_FastTravelComponent.ToggleMapDestinationSelection(true);
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBePerformedScript(IEntity user)
	{
		if (SCR_PlayerController.GetLocalControlledEntity() != user)
			return false;

		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return false;

		Faction faction = factionManager.GetLocalPlayerFaction();
		if (!faction)
			return false;

		array<SCR_SpawnPoint> spawnPoints = SCR_SpawnPoint.GetSpawnPointsForFaction(faction.GetFactionKey());
		return (spawnPoints.Count() > 1);
	}

	//------------------------------------------------------------------------------------------------
	override bool HasLocalEffectOnlyScript()
	{
		return true;
	}
}
