/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Trigger
\{
*/

class BaseGameTriggerEntityClass: GenericEntityClass
{
}

class BaseGameTriggerEntity: GenericEntity
{
	protected ref ScriptInvoker Event_OnQueryFinished = new ScriptInvoker();

	ScriptInvoker GetOnQueryFinished()
	{
		return Event_OnQueryFinished;
	}

	// Check if the given entity is inside the Trigger shape (bypasses the defined query and filter)
	proto external bool QueryEntityInside(notnull IEntity ent);
	// Queries entities which are inside the Trigger shape based on the defined filter
	proto external void QueryEntitiesInside();
	// Compares the given entity with the defined filters (Name and/or Class)
	proto external bool DefaultEntityFilterForQuery(IEntity ent);
	/*!
	Get the entities inside the triggers.
	*IMPORTANT* This doesn't force the searching of entities. If you want to search entities, use the QueryEntitiesInside function.
	*/
	proto external int GetEntitiesInside(out notnull array<IEntity> outEntities);
	// Sets the sphere radius of the Trigger
	proto external void SetSphereRadius(float radius);
	// Gets the current sphere radius of the Trigger
	proto external float GetSphereRadius();
	// Sets the update rate of the trigger in seconds
	proto external void SetUpdateRate(float updateRate);
	// Gets the update rate of the trigger in seconds
	proto external float GetUpdateRate();
	// Set the periodic queries
	proto external void EnablePeriodicQueries(bool enable);
	// Is periodic queries enabled?
	proto external bool IsPeriodicQueriesEnabled();
	//! Add a name to the filter
	proto external void AddFilterName(string name);
	//! Remove a name from the filter
	proto external bool RemoveFilterName(string name);
	//! Add a class type to the filter
	proto external void AddClassType(typename classType);
	//! Remove a class type from the filter
	proto external bool RemoveClassType(typename classType);
	//! Add a prefab filter to the filters
	proto external void AddPrefabFilter(PrefabFilter prefabFilter);
	//! Remove a prefab filter from the filters
	proto external bool RemovePrefabFilter(PrefabFilter prefabFilter);
	//! Get flag(s) from the traces done by the the trigger
	proto external EQueryEntitiesFlags GetTraceMask();
	/*!
	Sets trace mask.
	Bits are or'ed with existing mask.
	*/
	proto external void SetTraceMask(EQueryEntitiesFlags flags);
	//! Clears trace mask.
	proto external void ClearTraceMask(EQueryEntitiesFlags flags);

	// callbacks

	//! callback - activation - occurs when and entity which fulfills the filter definitions enters the Trigger
	event protected void OnActivate(IEntity ent);
	//! callback - deactivation - occurs when and entity which was activated (OnActivate) leaves the Trigger
	event protected void OnDeactivate(IEntity ent);
	//! callback - query finished - occurs when the current query finished being processd and has updated results
	event protected void OnQueryFinished(bool bIsEmpty){ Event_OnQueryFinished.Invoke(this); Event_OnQueryFinished.Clear(); };
}

/*!
\}
*/
