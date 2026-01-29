/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class AISpawnerGroupClass: AIGroupClass
{
}

class AISpawnerGroup: AIGroup
{
	//! Spawns a new group entity, sets its transformation and then calls OnSpawn
	proto external bool Spawn();
	proto external bool IsSpawningFinished();
	proto external void SetMaxAgentsToSpawn(int pCount);
	proto external int GetMaxAgentsToSpawn();

	// callbacks

	event void OnSpawn(IEntity spawned);
}

/*!
\}
*/
