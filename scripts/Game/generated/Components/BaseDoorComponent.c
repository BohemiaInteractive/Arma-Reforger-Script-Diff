/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class BaseDoorComponent: GameComponent
{
	//! Returns the pivot around which the door open/close in world space
	proto external vector GetDoorPivotPointWS();
	//! Returns the current state of the door in [-1,1] range (negative if door is rotating/moving on the other side)
	proto external float GetNormalizedDoorState();
	//! Returns true if an object of a given size can pass through the door
	proto external bool CanCharacterPass(float characterWidth);
	//! Returns true if the door is open
	proto external bool IsOpen();
	//! Returns true if the door is opening
	proto external bool IsOpening();
	//! Returns true if the door is closing
	proto external bool IsClosing();
	//! Sets the control value of the door and updates the wanted state. Automatically clamped to <-1, 1>
	proto external void SetControlValue(float controlValue, RplId instigator);
	//! Returns the control value of the door ranging from <-1, 1>
	proto external float GetControlValue();
	//! Returns the current door state
	proto external float GetDoorState();
	//! Returns the normal of the door in it's current state
	proto external vector GetDoorNormal();
	//! Activate the door by the user
	proto external float UseDoorAction(IEntity user);
	//! Loads the door state from the control value.
	//! Unlike InitValue, this one is not meant to initialize the default door state,
	//! but a deviation from it (aka as if somebody opened/closed the door). Therefore,
	//! this function also instantiates simulation data.
	proto external void InitDoor(float controlValue);
}

/*!
\}
*/
