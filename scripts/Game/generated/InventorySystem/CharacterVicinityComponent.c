/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup InventorySystem
\{
*/

class CharacterVicinityComponent: GameComponent
{
	/*!
	*** IMPORTANT ***
	Call ManipulationComplete method as soon as you are done working with items from the vicinity
	\param items Array which need to be filled.
	*/
	proto external void GetAvailableItems(inout notnull array<IEntity> items);
	proto external void SetItemOfInterest(IEntity item);
	proto external void ManipulationComplete();
	proto external IEntity GetItemOfInterest();
	proto external float GetDiscoveryRadius();

	// callbacks

	event protected void OnUpdate();
}

/*!
\}
*/
