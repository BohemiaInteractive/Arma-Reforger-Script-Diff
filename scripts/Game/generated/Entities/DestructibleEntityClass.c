/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class DestructibleEntityClass: StaticModelEntityClass
{
	proto external float GetMaxHealth();
	proto external BaseDestructionPhase GetDestructionPhase(int phaseID);
	proto external int GetNumDestructionPhases();
	proto external float GetDamageReduction();
	proto external float GetDamageThreshold();
	proto external float GetDamageMultiplier(EDamageType type);
	proto external float GetBaseDamageMultiplier();
}

/*!
\}
*/
