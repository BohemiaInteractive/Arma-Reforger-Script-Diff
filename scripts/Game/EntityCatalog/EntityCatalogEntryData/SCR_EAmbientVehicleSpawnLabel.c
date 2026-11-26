//! Labels deciding which vehicle pool an SCR_AmbientVehicleSpawnPointComponent spawns from.
//! Catalog entries list their labels in SCR_EntityCatalogAmbientVehicleData.
//! Plain (non-flag) enum so addons can extend it with "modded enum" without value collisions.
enum SCR_EAmbientVehicleSpawnLabel
{
	NONE = 0,		//!< Spawn point: no label filtering. Not meant to be assigned to catalog entries.
	BASE_GAME,		//!< Base game vehicles. Catalog entries without any labels (or without ambient data) are treated as BASE_GAME.
	RALLY_RACING,	//!< Rally DLC vehicles rally variants of civilian vehicles
	RALLY_FARMING,	//!< Rally DLC vehicles tozers
}
