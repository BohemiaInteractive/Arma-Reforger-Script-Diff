class SCR_CampaignSupplyRestrictedSpawnPointClass : SCR_SpawnPointClass
{

}

class SCR_CampaignSupplyRestrictedSpawnPoint : SCR_SpawnPoint
{
	//------------------------------------------------------------------------------------------------
	//! Returns whether this point can be reserved for provided player.
	//! Returns false when the player has a custom loadout or there is not enough resources for spawning
	override bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanReserveFor_S(playerId, result))
			return false;

		IEntity playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
		if (!playerController)
			return false;

		SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
		if (!loadoutComp)
			return false;

		SCR_PlayerArsenalLoadout loadout = SCR_PlayerArsenalLoadout.Cast(loadoutComp.GetLoadout());
		if (loadout)
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_CUSTOM_LOADOUT;
			return false;
		}

		SCR_ResourceComponent resourceComp;
		SCR_CampaignMilitaryBaseComponent base;
		float spawnSupplyCost = SCR_ArsenalManagerComponent.GetLoadoutCalculatedSupplyCost(loadoutComp.GetLoadout(), false, playerId, null, this, base, resourceComp);

		if (!resourceComp || base)
			return false;

		SCR_ResourceConsumer resourceConsumer = resourceComp.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
		if (!resourceConsumer)
			return false;

		SCR_ResourceConsumtionResponse consumerResponse = resourceConsumer.RequestAvailability(spawnSupplyCost);

		if (consumerResponse.GetReason() == EResourceReason.SUFFICIENT)
		{
			result = SCR_ESpawnResult.OK;
			return true;
		}

		result = SCR_ESpawnResult.NOT_ALLOWED_NOT_ENOUGH_SUPPLIES;
		return false;

	}

}
