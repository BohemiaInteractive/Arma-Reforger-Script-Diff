/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup AI
\{
*/

class ChimeraAIGroupClass: AIGroupClass
{
}

class ChimeraAIGroup: AIGroup
{
	proto external bool IsDormant();
	proto external int GetDormantAliveCount();
	proto external int GetDormantDeadCount();
	proto external void SetDormantCounts(int alive, int dead);
	proto external void SetImportance(int importance);
	// local cameras, connected players, temp MP overrides. 2D XZ check.
	proto external bool HasObserverInRange(float range);

	// callbacks

	// for behaviour; defaults here are the no-op return for non-SCR groups.
	event bool ExpandOneMember() { return false; };
	event bool IsExpandComplete() { return true; };
	event void SpawnMembers();
	event void DespawnMembersFromC();
}

/*!
\}
*/
