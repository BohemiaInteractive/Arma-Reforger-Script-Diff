/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Damage
\{
*/

sealed class Instigator: Managed
{
	//use CreateInstigator instead
	private void Instigator()
	{
	}

	//! Set instigator with automatic conversion from entity to player if necessary
	proto external void SetInstigator(IEntity entity);
	//!Will set the Instigator of type InstigatorType.Player
	proto external void SetInstigatorByPlayerID(int playerID);
	//!Sets the instigator type to GM. PlayerID will be 0 and entity will be null.
	proto external void SetInstigatorTypeToGM(int playerID = 0);
	/*!
	Get the instigator entity, player ID will be automatically converted to current entity. Can return null if the player is dead.
	e.g.: Players can wait on the respawn screen, and this will return null, don't rely on IEntity, use PlayerID instead
	*/
	proto external IEntity GetInstigatorEntity();
	//!Gets instigator player ID if the instigator is a player. Returns 0 otherwise
	proto external int GetInstigatorPlayerID();
	//!Returns instigator type
	proto external InstigatorType GetInstigatorType();
	//! Proper way to create an instigator.
	static proto ref Instigator CreateInstigator(IEntity instigator);
	// Special case for create instigator. It creates an instigator with InstigatorType: INSTIGATOR_GM
	static proto ref Instigator CreateInstigatorGM(int playerID = 0);
}

/*!
\}
*/
