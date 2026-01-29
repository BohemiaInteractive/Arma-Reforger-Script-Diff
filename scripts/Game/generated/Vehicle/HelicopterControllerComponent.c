/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class HelicopterControllerComponentClass: VehicleControllerComponentClass
{
}

class HelicopterControllerComponent: VehicleControllerComponent
{
	//! Set the current collective mode
	proto external void SetCollectiveMode(CollectiveMode collectiveMode);
	//! return true if autohover system is enabled
	proto external bool GetAutohoverEnabled();
	//! enables autohover system
	proto external void SetAutohoverEnabled(bool enabled);
	//! returns true if wheel brake is active
	proto external bool GetWheelBrake();
	//! returns true if persistent wheel brake is active
	proto external bool GetPersistentWheelBrake();
	//! sets persistent handbrake state
	proto external void SetPersistentWheelBrake(bool newValue);

	// callbacks

	//! Called on prepare controls
	event void OnPrepareControls();
}

/*!
\}
*/
