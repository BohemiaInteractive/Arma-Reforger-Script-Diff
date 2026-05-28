/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class HelicopterControllerComponent: VehicleControllerComponent
{
	//! Returns the simulation component associated with this controller.
	proto external VehicleHelicopterSimulation GetSimulation();
	//! Gets the current collective mode
	proto external CollectiveMode GetCollectiveMode();
	//! Sets the current collective mode
	proto external void SetCollectiveMode(CollectiveMode collectiveMode);
	proto external EInputDeviceType GetCollectiveInputType();
	//! Returns true if wheel brake is active
	proto external bool GetWheelBrake();
	//! Returns true if persistent wheel brake is active
	proto external bool GetPersistentWheelBrake();
	//! Sets persistent handbrake state
	proto external void SetPersistentWheelBrake(bool newValue);
	//! Returns true if autohover system is enabled
	proto external bool GetAutohoverEnabled();
	//! Sets autohover system state
	proto external void SetAutohoverEnabled(bool enabled);

	// callbacks

	//! Gets called before inputs are collected. Used for collective mode manipulations.
	event void OnPrepareControls();
}

/*!
\}
*/
