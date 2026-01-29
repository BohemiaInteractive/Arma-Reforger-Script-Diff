class Persistence_TestWeatherRandomizerAction : ScriptedUserAction
{
	//------------------------------------------------------------------------------------------------
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		RplComponent replication = RplComponent.Cast(pOwnerEntity.FindComponent(RplComponent));
		if (replication && !replication.IsOwner()) return;

		ChimeraWorld world = pOwnerEntity.GetWorld();
		TimeAndWeatherManagerEntity weatherManager = world.GetTimeAndWeatherManager();
		if (!weatherManager)
			return;

		array<ref WeatherState> weatherStates();
		weatherManager.GetWeatherStatesList(weatherStates);
		weatherManager.ForceWeatherTo(true, weatherStates.GetRandomElement().GetStateName());
	}
}
