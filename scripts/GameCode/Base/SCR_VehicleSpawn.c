class SCR_VehicleSpawn : SCR_BasePrefabSpawner
{
	[Attribute("10", UIWidgets.EditBox, desc: "Delay for vehicle to spawn upon destruction (s)",  params: "0 10000")]
	protected float respawnDelay;
	[Attribute("0", UIWidgets.EditBox, desc: "Delay for first vehicle to be spawned after (s)",  params: "0 10000")]
	protected float firstSpawn;
	protected float timer = 10;


	protected Vehicle spawnedVehicle;

	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		timer = firstSpawn;
	}

	protected override bool CanSpawn()
	{
		return !spawnedVehicle && timer <= 0;
	}

	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		super.EOnFrame(owner, timeSlice);

		if (!spawnedVehicle)
			timer -= timeSlice;

	}

	protected override void OnSpawn(IEntity newEnt)
	{
		spawnedVehicle = Vehicle.Cast(newEnt);
		EventHandlerManagerComponent ehmc = EventHandlerManagerComponent.Cast (spawnedVehicle.FindComponent(EventHandlerManagerComponent));
		if (ehmc)
		{
			ehmc.RegisterScriptHandler("OnDestroyed", this, OnDestroyed);
		}
		timer = respawnDelay;
	}

	void OnDestroyed(IEntity newEnt)
	{
		spawnedVehicle = null;
	}
};

class SCR_VehicleSpawnClass : SCR_BasePrefabSpawnerClass
{
};