/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup System
\{
*/

sealed class ObserversSystem: GameSystem
{
	//! Insert or update a temporary local observer that can follow an entity.
	//! Use a stable key for the feature that owns the observer, then remove it when it is no longer needed.
	//! When pEntity is null, x/z is a world position. Otherwise it is a local X/Z offset from the entity.
	//! \warning Temporary observers can keep distant entities simulated, so be mindful of their lifetime.
	//! \param key Local observer key.
	//! \param x World X position, or local X offset when pEntity is set.
	//! \param z World Z position, or local Z offset when pEntity is set.
	//! \param pEntity Entity used as the origin, or null for a fixed observer point.
	proto external void InsertObserverSP(int key, float x, float z, IEntity pEntity);
	//! Remove a temporary local observer.
	//! \param key Local observer key.
	proto external void RemoveObserverSP(int key);
	//! Adds or updates the active far observer
	//! \param pObserverEntity Root observer entity. This is the character.
	//! \param pEyeEntity The entity observed through. This is your weapon's scope, binoculars, or similar.
	//! \param eyePositionFrom Position where the eye is looking from. E.g., the position where the scope is mounted on your weapon.
	//! \param direction Direction in which the eye is looking. E.g. the direction of the scope mounted on your weapon.
	//! \param range Maximum distance we are looking at through the eye. E.g. the range of your weapon's scope.
	//! \param timeSlice Elapsed time since the last call to this function.
	//! \note There can only ever be one far observer defined. It is bound to the player character.
	proto external void AddFarObserver(IEntity pObserverEntity, IEntity pEyeEntity, vector eyePositionFrom, vector direction, float range, float timeSlice);
	//! Removes any active far observer.
	proto external void DelFarObserver();
	//! Insert or update a temporary MP observer that can follow an entity.
	//! This is used for MP to track remote observers without relying on cameras or players.
	//! E.g., this can be used to streams in / preloads replicated entities in a given place
	//! for a given player.
	//! When pEntity is null, x/z is a world position. Otherwise it is a local X/Z offset from the entity.
	//! \warning Temporary observers can keep distant entities simulated, so be mindful of their lifetime.
	//! \param identity Identity of the observer.
	//! \param x World X position, or local X offset when pEntity is set.
	//! \param z World Z position, or local Z offset when pEntity is set.
	//! \param pEntity Entity used as the origin, or null for a fixed observer point.
	proto external void InsertObserverMP(RplIdentity identity, float x, float z, IEntity pEntity);
	//! Remove a temporary observer for the given identity.
	//! \param identity Identity of the observer to remove.
	proto external void RemoveObserverMP(RplIdentity identity);
	//! Get 2D positions of all local observers (cameras).
	//! \param outPositions Output array to fill with observer 2D positions.
	proto external void GetObserversSP(out notnull array<vector> observers);
	//! Get 2D positions of all MP observers.
	//! \param outPositions Output array to fill with MP observer 2D positions.
	proto external void GetObserversMP(out notnull array<vector> observers);
	//! Get 2D positions of all observers.
	//! \param outPositions Output array to fill with observer 2D positions.
	proto external void GetObservers(out notnull array<vector> observers);
	//! Get 2D positions of observers within the given squared distance from the position.
	//! \param x X coordinate to check against.
	//! \param z Z coordinate to check against.
	//! \param rangeSq Squared distance to check.
	//! \param outPositions Output array to fill with observer 2D positions within range.
	proto external void GetObserverWithinRangeSq(float x, float z, float rangeSq, out notnull array<vector> observers);
	//! Check if there any observer within the given squared distance from the position.
	//! \param x X coordinate to check against.
	//! \param z Z coordinate to check against.
	//! \param rangeSq Squared distance to check.
	proto external bool HasObserverWithinRangeSq(float x, float z, float rangeSq);
}

/*!
\}
*/
