/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class BaseCompartmentManagerComponent: GameComponent
{
	//! Returns true if doorIndex is valid and the assocciated door is open. False otherwise.
	//! Fake door is always considered open.
	proto external bool IsDoorOpen(int doorIndex);
	//! Returns true if doorIndex is valid and the assocciated door is fake. False otherwise.
	proto external bool IsDoorFake(int doorIndex);
	proto external IEntity GetDoorUser(int doorIndex);
	proto external bool IsGetInAndOutBlockedByDoorUser(int doorIndex);
	proto external IEntity GetOwner();
	proto external BaseCompartmentSlot FindCompartmentByName(string compartmentName, bool bPreferUnique = true);
	//! Searches for compartment by ID of slot and compartment manager.
	proto external BaseCompartmentSlot FindCompartment(int compartmentID, int mgrId = -1);
	//! Returns a list and count of all compartments
	proto external int GetCompartments(out notnull array<BaseCompartmentSlot> outCompartments);
	proto external BaseCompartmentDoor GetDoor(int doorIndex);
	//! Get door referencing this door index
	proto external CompartmentDoorReference GetDoorReference(int referenceDoorIndex);
	proto external CompartmentDoorInfo GetDoorInfo(int doorIndex);
}

/*!
\}
*/
