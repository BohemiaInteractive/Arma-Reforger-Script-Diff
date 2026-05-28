/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class CarControllerComponent: VehicleControllerComponent
{
	//! Returns the simulation component associated with this controller.
	proto external VehicleWheeledSimulation GetSimulation();
	//! Returns the simulation component associated with this controller.
	//! \deprecated Use GetSimulation() instead.
	[Obsolete("Use GetSimulation() instead.")]
	proto external VehicleWheeledSimulation GetWheeledSimulation();
	//! Returns true if throttle "turbo" modifier is active
	proto external bool IsThrottleTurbo();
	//! Returns true if handbrake is active
	proto external bool GetHandBrake();
	//! Returns true if persistent handbrake is active
	proto external bool GetPersistentHandBrake();
	//! Sets persistent handbrake state
	proto external void SetPersistentHandBrake(bool newValue);
	//! Returns true if RND transmission is used
	proto external bool HasAutomaticGearbox();
	//! Returns gear being shifted to
	proto external int GetFutureGear();
	//! Returns current gearbox gear
	proto external int GetCurrentGear();
}

/*!
\}
*/
