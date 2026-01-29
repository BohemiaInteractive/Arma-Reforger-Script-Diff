/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

sealed class EntityUtils
{
	private void EntityUtils();
	private void ~EntityUtils();

	//! Returns true if \param pEntity is a player
	static proto bool IsPlayer(IEntity pEntity);
	//! Returns true if \param pEntity is a vehicle the local player is controlling
	static proto bool IsPlayerVehicle(IEntity pEntity);
	//! Returns the local player entity if any
	static proto IEntity GetPlayer();
	//! Returns true if \param pEntity can be treated as PROXY. Currently true for all non-STATIC and non-particle effect entities
	static proto bool CanTreatAsProxy(IEntity pEntity, IEntity pParent);
	/*!
	Returns a matrix that transforms a point in ancestor-space to child-space.
	\param child The Entity whose space is the target of the transformation. Can't be null.
	\param ancestor The Entity whose space is the source of the transformation. Can't be null.
	\param[out] outResult The output matrix transformation.
	\return true, if ancestor-descendant relationship was valid. Else false, in which case outResult is undefined.
	*/
	static proto bool GetAncestorToChildTransform(IEntity child, IEntity ancestor, out vector result[4]);
	/*!
	Returns a matrix that transforms a point in child-space to ancestor-space.
	\param child The Entity whose space is the source of the transformation. Can't be null.
	\param ancestor The Entity whose space is the target of the transformation. Can't be null.
	\param[out] outResult The output matrix transformation.
	\return true, if ancestor-descendant relationship was valid. Else false, in which case outResult is undefined.
	*/
	static proto bool GetChildToAncestorTransform(IEntity child, IEntity ancestor, out vector result[4]);
}

/*!
\}
*/
