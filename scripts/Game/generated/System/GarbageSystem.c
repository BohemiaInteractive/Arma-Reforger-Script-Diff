/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

class GarbageSystem: GameSystem
{
	/*!
	Requests the garbage handling of the provided entity.
	\param lifeTime Life time of instance in seconds. If <= 0, the config rule values are used.
	\return Returns true on success, false if instance is already or can not be inserted.
	*/
	proto external bool Insert(notnull IEntity entity, float lifetime = -1, bool ignoreBlacklist = false);
	/*!
	Check if the entity is tracked.
	\return True if instance is tracked, false otherwise.
	*/
	proto external bool IsInserted(notnull IEntity entity);
	/*!
	Removes the provided entity from the garbage system.
	\return True if successfull, false otherwise.
	*/
	proto external bool Withdraw(notnull IEntity entity);
	/*!
	Adds additional lifetime to the provided entity.
	\param additionalLifetime Lifetime to add to current lifetime (in seconds)
	\return Returns true on success, false if instance is not tracked.
	*/
	proto external bool Bump(notnull IEntity entity, float additionalLifetime);
	/*!
	Returns the original lifetime the provided entity was inserted with.
	\return Lifetime duration in seconds or -1 if not tracked..
	*/
	proto external float GetLifetime(notnull IEntity entity);
	/*!
	Returns the remaining lifetime of the provided entity.
	Note: Nearby players and their lifetime effect are only processed in the general system interval defined in the config.
		  Thus the return value might briefly be 0 and jump back to a higher number in a later frame.
	\return Remaining lifetime in seconds or -1 if not tracked.
	*/
	proto external float GetRemainingLifetime(notnull IEntity entity);
	//! Forcefully disposes all entities that are tracked at least for x seconds.
	proto external void Flush(float minTrackedSeconds = 30);
	/*!
	Determine if the provided entity instance is blacklisted from being inserted.
	Will withdraw the entity if it is currently tracked and blacklist was enabled for it.
	Can by bypassed via Insert(..., ignoreBlacklist: true)
	\return True if blacklist update was successful, false otherwise.
	*/
	proto external bool UpdateBlacklist(notnull IEntity entity, bool blacklisted);
	//! Fetch all entities tracked by the garbage system. Expensive operation. Use with care!
	proto external int FetchTrackedEntities(notnull array<IEntity> entities);

	// callbacks

	/*
	React to or change the behavior of the garbage insertion.
	\param ent Entity instance to check logic for.
	\param lifetime Original lifetime the insertion was requested with.
	\return Returns a lifetime >0 for insertion and <= 0 to prevent it.

	Example override to use in modded classes:
	@code
	override protected float OnInsertRequested(IEntity entity, float lifetime)
	{
		if (someCondition)
			return -1; // Prevent insertion by returning negative values

		if (someOtherCondition)
			return lifetime * 0.5; // Cut lifetime in half based on specific needs.

		return lifetime; // Return unmodified value
	}
	@endcode
	*/
	event protected float OnInsertRequested(IEntity entity, float lifetime) {return lifetime;};
	/*
	React to or skip the deletion of a tracked entity of which the lifetime has run out.
	\param ent Entity instance to check logic for.
	\return Return true to proceed with deletion, false to skip it (tracking for the item will end regardless).
	*/
	event protected bool OnBeforeDelete(IEntity entity) {return true;};
}

/*!
\}
*/
