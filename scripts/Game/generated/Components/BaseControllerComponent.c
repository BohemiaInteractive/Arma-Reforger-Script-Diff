/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseControllerComponentClass: GameComponentClass
{
}

class BaseControllerComponent: GameComponent
{
	//! Returns the entity owner of the component
	proto external IEntity GetOwner();
	//! Returns state of movement capacity.
	proto external bool CanMove();
	//! Changes state of movement capacity. Internally verifies if all necessary conditions are met.
	proto external void SetCanMove(bool canMove);

	// callbacks

	event bool ValidateCanMove() { return true; };
}

/*!
\}
*/
