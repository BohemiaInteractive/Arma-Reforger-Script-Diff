/**
List of entity catalog types
*/
enum EEntityCatalogType
{
	NONE = 0, ///< NONE should not be set as a data type
	
	//~ These are ordered similar to Editor Entity Types (x10) Keep as is in case of converting the EntityData to Editor ~\\
	OBJECT 						= 10, ///< Props
	CHARACTER 					= 20, ///< Characters
	VEHICLE 					= 30, ///< Vehicles
	GROUP 						= 40, ///< Groups
	COMPOSITION 				= 50, ///< Compositions
	ITEM 						= 60, ///< Inventory Items
	SYSTEM 						= 70, ///< Systems (Eg Respawn point, Objectives)
	//~
	
	WEAPONS_TRIPOD 				= 80, ///< Static Weapons on Tripods
	RESPAWN_LOADOUT 			= 90, ///< Used for player spawning
	SUPPLY_CONTAINER_ITEM		= 100, ///< Inventory Items that hold supplies
};