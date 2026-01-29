/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AIWeaponTargetSelector: ScriptAndConfig
{
	//! Some functions accept minimal magazine count specification array: array<int> minMagCountSpec.
	//! This array lets you exclude weapons which have lower amount of magazines than specified.
	//! The array is of following format: {DefaultValue, WeaponType0, MagCount0, WeaponType1, MagCount1, ...}.
	//! 1. Element 0 is default mag. count if weapon type is not found in the array.
	//! 2. Other elements are pairs of EWeaponType, MagCount.
	//! !!! When magazines are counted, the current magazine in weapon is counted as well.

	proto external void Init(notnull IEntity owner);
	/*!
	Initializes properties for target/weapon selection
	*/
	proto external void SetSelectionProperties(float maxLastSeenDirect, float maxLastSeenIndirect, float maxLastSeen, float minTraceFractionIndirect, float maxDistanceInfantry, float maxDistanceVehicles, float maxTimeSinceEndangered, float maxDistanceDisarmed);
	/*!
	Sets constants for target score calculation. Formula is: score = offset + slope*distance
	*/
	proto external void SetTargetScoreConstants(EAIUnitType targetUnitType, float offset, float slope);
	/*!
	Selects weapon and target. Returns true when a valid option is selected. Call GetSelectedTarget, GetSelectedWeapon, GetSelectedWeaponProperties to get result.
	*/
	proto external bool SelectWeaponAndTarget(notnull array<IEntity> assignedTargets, float assignedTargetsScoreIncrement, float dangerTargetsScoreIncrement, bool useCompartmentWeapons, array<int> weaponTypesWhitelist = null, array<int> weaponTypesBlacklist = null);
	/*!
	Returns selected target
	*/
	proto BaseTarget GetSelectedTarget();
	/*!
	Returns selected weapon, muzzle, magazine
	*/
	proto void GetSelectedWeapon(out BaseWeaponComponent outWeapon, out int outMuzzleId, out BaseMagazineComponent outMagazine);
	/*!
	Return unit types which we can attack based on available weapons. Uses cached value from last SelectWeaponAndTarget call.
	*/
	proto external EAIUnitType GetUnitTypesCanAttack();
	/*!
	Returns properties of selected weapon/magazine
	*/
	proto void GetSelectedWeaponProperties(out float outMinDistance, out float outMaxDistance, out bool outDirectDamage);
	/*!
	Returns most relevant target which we can't attack.
	*/
	proto void GetMostRelevantTargetCantAttack(out BaseTarget target, out float targetScore);
	/*!
	Selects weapon against abstract target of given type. Call GetSelectedWeapon, GetSelectedWeaponProperties to get result.
	*/
	proto external bool SelectWeaponAgainstUnitType(EAIUnitType targetUnitType, bool useCompartmentWeapons, array<int> minMagCountSpec = null);
	/*!
	Selects weapon against abstract target of given type, but takes distance into account. Call GetSelectedWeapon, GetSelectedWeaponProperties to get result.
	*/
	proto external bool SelectWeaponAgainstUnitTypeAndDistance(EAIUnitType targetUnitType, float distance, bool preferDirectAttackWeapon, bool useCompartmentWeapons, array<int> weaponTypesWhitelist = null, array<int> weaponTypesBlacklist = null, array<int> minMagCountSpec = null);
	/*!
	Returns score of the target.
	*/
	proto external float CalculateTargetScore(BaseTarget target);
	/*!
	Methods below perform fast lookup from inventory caches. They should be prefered for weapon or magazine searches for AI instead of inventory queries.
	searchCompartments - if true, the inventory cache for current compartment is used. Otherwise character's cache is used.
	*/
	proto external bool HasWeaponOfType(EWeaponType weaponType, bool searchCompartment);
	proto external BaseWeaponComponent FindWeaponOfType(EWeaponType weaponType, bool searchCompartment);
	//! Returns amount of magazines for magazine well.
	proto external int GetMagazineCount(typename magazineWellType, bool searchCompartment);
	//! Finds magazine with given traits. Traits value is treated as flags.
	proto external BaseMagazineComponent FindMagazineWithTraits(EAICombatPropertiesComponentTraits traits, bool searchCompartment);
	//! Finds a weapon and muzzle compatible with given magazine well
	proto BaseWeaponComponent FindWeaponAndMuzzleForMagazineWell(typename magazineWellType, bool searchCompartment, out int muzzleId);
}

/*!
\}
*/
