//~ When no support station found it needs to know the reason.
//~ It will always use the highest enum reason why the support station was not found
enum ESupportStationReasonInvalid 
{
	NOT_IN_RANGE = 100,				//!< No support station in range
	
	DISABLED = 150,					//!< Station is in range but disabled
	
	DESTROYED_STATION = 200,		//!< Station is in range but is destroyed
	
	INVALID_FACTION = 300,			//!< Support stations has invalid faction for user.
	
	NO_SUPPLIES = 400,				//!< There are support stations in the area but they don't have supplies
	NO_SUPPLY_STORAGE_SPACE = 410,	//!< Cannot execute support station as the supplies it gains have no storage space
	
	NO_FUEL_TO_GIVE = 500,			//!< There are support stations in range for refueling but non have fuel to give
	FUEL_CANISTER_EMPTY,			//!< The player is holder a fuel canister but the canister is empty
	FUEL_TANK_FULL,					//!< Fuel tank is full so cannot refuel
	
	HEAL_ENTITY_UNDAMAGED = 600,	//!< Cannot Heal entity as it is undamaged
	HEAL_CHARACTER_IS_BLEEDING,		//!< Cannot heal character as character is activily bleeding
	HEAL_MAX_HEALABLE_HEALTH_REACHED,			//!< Cannot heal the entity has the max health has been reached. This is generic reasoning compared to those below
	HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD,		//!< Cannot heal the entity has the max health has been reached, will need a static support station. e.g: Repair truck can only heal a vehicle up to x%
	HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY,	//!< Cannot heal the entity has the max health has been reached, will need a static or mobile support station e.g: Repair wrench can only heal a vehicle up to x%
	LOAD_CASUALTY_NO_SPACE,			//!< Cannot load casualty as there is no space
	HEAL_ENTITY_ONFIRE,				//!< Cannot Heal entity as it is on fire
	
	RESUPPLY_ENOUGH_ITEMS = 700,	//!< Resupply as already has enough magazines
	RESUPPLY_NOT_IN_STORAGE,		//!< Cannot resupply as magazine are not in the storage
	RESUPPLY_INVENTORY_FULL,		//!< Cannot Resupply as Inventory is full
	RESUPPLY_NO_VALID_WEAPON,		//!< Cannot Resupply as player is not holding any valid weapons
	
	IS_MOVING = 800,				//!< If not static and moving
	RANK_TOO_LOW = 850, 			//!< If player ranks are enabled and player has too low of a rank
	NOT_ENOUGH_AVAILABLE_ALLOCATED_SUPPLIES = 900,		//!< If Military Supply Allocation is enabled and player does not have enough Available Allocated Supplies
	
	IN_USE = 9999,					// !< The User action is being used by another player already
}
