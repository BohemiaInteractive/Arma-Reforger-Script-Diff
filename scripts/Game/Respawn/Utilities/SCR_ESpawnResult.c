/*
	This enum represents possible results/responses sent from the authority after a SCR_SpawnRequestComponent
	issues a request to respawn to a SCR_SpawnHandlerComponent.

	When the spawning process is successfull, the return value is SCR_ESpawnResult.OK,
	anything else is a request which has failed and resulted in the player not spawning.
*/
enum SCR_ESpawnResult
{
	/*!
		Spawn successfull!
	*/
	OK = 0,

	/*!
		Request was invalid and caught before it was sent further.
	*/
	BAD_REQUEST = 1000,

	/*!
		Response sent when a user attempts to request a respawn through a method which is
		disallowed through the game mode.
	*/
	UNSUPPORTED_SPAWN_METHOD = 2000,
	
	/*!
		Bad data setup. Each request component should always have a request handler.
	*/
	MISSING_HANDLER,

	/*!
		Response sent when a user attempts to request a respawn on a spawn point for
		which the authority deems the user is not eligible to spawn on. Usually a game
		specific implementation, e.g. respawn timer and similar.
		See SCR_BaseRespawnHandler.CanRequestSpawn.
	*/
	SPAWN_NOT_ALLOWED = 3000,
	
	NOT_ALLOWED_TIMER,
	NOT_ALLOWED_SPAWNPOINT_DISABLED,
	NOT_ALLOWED_SPAWNING_DISABLED,
	NOT_ALLOWED_VEHICLE_FULL,
	NOT_ALLOWED_VEHICLE_MOVING,
	NOT_ALLOWED_RADIO_VEHICLE_SPAWNING_DISABLED,
	NOT_ALLOWED_SPAWNPOINT_OCCUPIED_BY_HOSTILE,
	NOT_ALLOWED_SPAWNING_DISABLED_ENEMIES_NEARBY,
	NOT_ALLOWED_SPAWNPOINT_DISABLED_OUT_OF_RESPAWNS,
	NOT_ALLOWED_NOT_ENOUGH_SUPPLIES,
	NOT_ALLOWED_NO_ARSENAL,
	NOT_ALLOWED_CUSTOM_LOADOUT,
	NOT_ALLOWED_BASE_UNDER_ATTACK,
	NOT_ALLOWED_NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES,
	NOT_ALLOWED_RANK_TOO_LOW,

	/*!
		Prefab needs to meet certain conditions before it is spawned.
		This error is returned when it does not meet them.
		See SCR_BaseRespawnHandler.ValidatePrefab.
	*/
	INVALID_PREFAB = 5000,

	/*!
		To spawn prefab a handler needs certain data (position, rotation, ...)
		Data can vary based on individual handlers, but each handler can
		verify its data and react accordingly. This error is returned
		when verification of input data fails.
		See SCR_BaseRespawnHandler.ValidateData
	*/
	CANNOT_VALIDATE,

	/*!
		And intenal error occured while spawning prefab. Entity was not spawned
		and therefore whole request is denied by the authority.
	*/
	CANNOT_SPAWN,

	/*!
		After an entity is spawned, it can be further prepared by the server.
		That can be used for e.g. seating a character or spawning additional
		items to their inventory. This error is returned when preparation
		fails and the spawn is therefore deemed failure.
	*/
	CANNOT_PREPARE,

	/*!
		If attempting to posses an entity that is already possessed by a player,
		this result is returned.
	*/
	CANNOT_POSSES,

	/*!
		After an entity is prepared, the server attempts to pass the ownership
		to the requesting player, including setting it as the controlled entity
		in the PlayerController. This error is returned when assignment of
		the newly spawned entity fails and therefore spawn is deemed failure.
	*/
	CANNOT_ASSIGN,

	/*!
		Some error occured that should have been handled in implementation!
	*/
	INTERNAL_ERROR = 666666,

	/*!
		This error should never occur. If it occurs, it most likely means that
		a fatal/unrecoverable issue has occured, but is not specified.
	*/
	UNKNOWN_ERROR = 999999,
};
