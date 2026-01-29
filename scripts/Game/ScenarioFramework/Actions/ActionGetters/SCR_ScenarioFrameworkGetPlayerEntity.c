[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetPlayerEntity : SCR_ScenarioFrameworkGet
{
	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		array<int> playerIDs = {};
		GetGame().GetPlayerManager().GetPlayers(playerIDs);
		IEntity entity;
		foreach (int playerID : playerIDs)
		{
			entity = GetGame().GetPlayerManager().GetPlayerControlledEntity(playerID);
			if (!entity)
				continue;

			return new SCR_ScenarioFrameworkParam<IEntity>(entity);
		}

		return null;
	}
}