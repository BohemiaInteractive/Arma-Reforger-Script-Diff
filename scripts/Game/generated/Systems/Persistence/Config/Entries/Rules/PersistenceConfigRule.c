/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Systems_Persistence_Config_Entries_Rules
\{
*/

class PersistenceConfigRule: ScriptAndConfig
{
	//! Only constructed through the persistence system
	private void PersistenceConfigRule();

	proto external int GetNativeTypePriority();
	proto external int NativeCompare(const PersistenceConfigRule other);
	proto external bool IsNativeMatch(const IEntity entity);

	// callbacks

	//! Fixed rule priority. Used to e.g. prioritze prefab over entity class if nothing else was configured. Should be unique across all rule types.
	event protected int GetTypePriority() { return GetNativeTypePriority(); };
	//! If the other rule has the same priority, then the details of the instance may be compared to break the tie. Expected return values: 1, 0, -1
	event protected int Compare(const PersistenceConfigRule other) { return NativeCompare(other); };
	event protected bool IsMatch(const IEntity entity) { return IsNativeMatch(entity); };
}

/*!
\}
*/
