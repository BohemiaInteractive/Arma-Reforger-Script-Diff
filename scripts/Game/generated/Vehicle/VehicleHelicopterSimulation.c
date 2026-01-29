/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleHelicopterSimulationClass: VehicleBaseSimulationClass
{
}

class VehicleHelicopterSimulation: VehicleBaseSimulation
{
	//! Sets throttle input
	//! \param in should be in range < 0, 1 >
	proto external void SetThrottle(float in);
	//! Starts the engine
	proto external void EngineStart();
	//! Stops the engine
	proto external void EngineStop();
	//! Returns true if engine is running
	proto external bool EngineIsOn();
	//! Returns number of rotors
	proto external int RotorCount();
	//! Sets scaling factor of rotor force
	proto external void RotorSetForceScaleState(int rotorIdx, float forceScale);
	//! Sets scaling factor of rotor torque
	proto external void RotorSetTorqueScaleState(int rotorIdx, float torqueScale);
	//! Returns rotor transform in local space
	proto external void RotorGetTransform(int rotorIdx, out vector transform[4]);
	//! Returns rotor axis in local space
	proto external void RotorGetAxis(int rotorIdx, out vector axis);
	//! Returns rotor radius
	proto external float RotorGetRadius(int rotorIdx);
	//! Returns rotor spin direction
	proto external SpinDirection RotorGetSpinDirection(int rotorIdx);
	//! Returns rotor target RPM
	proto external float RotorGetRPMTarget(int rotorIdx);
	//! Returns rotor actual RPM
	proto external float RotorGetRPM(int rotorIdx);
	//! Returns rotor state (static, spinning or destroyed)
	proto external RotorState RotorGetState(int rotorIdx);
	//! Returns type of landing gear available
	proto external LandingGearType LandingGearGetType();
	//! Returns altitude above ground level (represents terrain detection)
	proto external float GetAltitudeAGL();
}

/*!
\}
*/
