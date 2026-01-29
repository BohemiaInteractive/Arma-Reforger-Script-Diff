/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class PerceptionComponentClass: AIComponentEntityClass
{
}

class PerceptionComponent: AIComponentEntity
{
	proto external int GetSimulateCounter();
	proto external int GetTargetsList(out notnull array<BaseTarget> outTargets, ETargetCategory category);
	/*!
	Returns count of targets of given category
	*/
	proto external int GetTargetCount(ETargetCategory category);
	/*!
	Returns target with lowest LastSeen value, but it must be below lastSeenMax
	*/
	proto external BaseTarget GetLastSeenTarget(ETargetCategory category, float timeSinceSeenMax);
	proto external BaseTarget GetTargetPerceptionObject(IEntity entityToFind, ETargetCategory targetCategory);
	//! Same as GetTargetPerceptionObject, but searches in all categories
	proto external BaseTarget FindTargetPerceptionObject(IEntity entityToFind);
	proto external BaseTarget GetClosestTarget(ETargetCategory category, float timeSinceSeenMax, float timeSinceDetectedMax);
	//! Sets perception factor - how good this AI can recognize enemy, independent of enemy state. 1.0 is standard value. Bigger value means faster recognition.
	proto external void SetPerceptionFactor(float value);
	// Returns the desired update interval of Perception Component, as configured in PerceptionSystem. It depends on LOD level.
	proto external float GetUpdateInterval();
	proto external IEntity GetFriendlyInLineOfFire();
	proto external void SetFriendlyFireCheck(bool on);
}

/*!
\}
*/
