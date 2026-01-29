enum EEditableEntityBudget
{
	PROPS, //!< All objects without special functionality, e.g., chairs, rocks, houses, etc.
	AI, //!< Entityes controlled by AI agent
	VEHICLES, //!< Vehicles and static guns
	WAYPOINTS, //!< AI waypoints
	SYSTEMS, //!< Objects that provide special functionality that's anticipated to be resource intensive, e.g., spawn points, artillery strikes, etc.
	
	CAMPAIGN, //!< Entities that can be constructed in Conflict game mode
	COOLDOWN, //!< Free Roam Building cooldown timer
	AI_SERVER, //!< Server side AI budget.
	
	//~ Character ranks. Take note to update any changes in SCR_EntityCatalogSpawnerData and SCR_ECharacterRank
	RANK_RENEGADE = 100,
	RANK_PRIVATE = 110,
	RANK_CORPORAL = 120,
	RANK_SERGEANT = 130,
	RANK_LIEUTENANT = 140,
	RANK_CAPTAIN = 150,
	RANK_MAJOR = 160,
	RANK_COLONEL = 170,
	RANK_GENERAL = 180,

	ESTABLISH_BASE = 190,
};
