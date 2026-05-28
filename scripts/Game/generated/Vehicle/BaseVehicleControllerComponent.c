/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class BaseVehicleControllerComponent: CompartmentControllerComponent
{
	//! Returns the base of simulation component associated with this controller.
	proto external VehicleBaseSimulation GetBaseSimulation();
	//! Returns the fuel manager associated with this controller.
	proto external FuelManagerComponent GetFuelManager();
	//! Returns the light manager associated with this controller.
	proto external BaseLightManagerComponent GetLightManager();
	//! Returns the pilot compartment slot associated with this controller.
	proto external PilotCompartmentSlot GetPilotCompartmentSlot();
	//! Sets the given compartment as the current pilot
	proto external void SetPilotCompartmentSlot(PilotCompartmentSlot pilot);
	//! Returns the signals manager associated with this controller.
	proto external SignalsManagerComponent GetSignalsManager();
	//! Returns the weapon manager associated with this controller.
	proto external BaseWeaponManagerComponent GetWeaponManager();
	//! Performs actions associated with vehicle shutdown (such as input reset)
	proto external void Shutdown();
	//! Returns true if seat switching is not blocked by some action/state
	proto external bool CanSwitchSeat();
	//! Returns true if it is possible to switch controls. For user action script usage.
	proto external bool ArePilotControlsLocked();
	//! Allows/Disallows switching of controls. For user action script usage.
	proto external void LockPilotControls(bool bIsLocked);
	//! Returns the global driving assistance mode.
	static proto EVehicleDrivingAssistanceMode GetDrivingAssistanceMode();
	//! Sets the global driving assistance mode.
	static proto void SetDrivingAssistanceMode(EVehicleDrivingAssistanceMode mode);
	//! Cancels startup procedure
	proto external void CancelStart();
	//! Tries to start the engine with the chance of getting the engine not started based on engine startup chance.
	proto external void TryStartEngine();
	/*!
	Issues a start engine input request
	\return Returns true if the engine is started otherwise false.
	*/
	proto external bool StartEngine();
	/*!
	Forcibly starts the engine without any delay, only meant for cinematics, not to be used in any game logic!
	*/
	proto external void ForceStartEngine(ForceStartEngineParams params = null);
	/*!
	Forcibly stops the engine without any delay, only meant for cinematics, not to be used in any game logic!
	*/
	proto external void ForceStopEngine();
	/*!
	Stops the engine.
	\param playDriverAnimation The driver should play the animation or not.
	*/
	proto external void StopEngine(bool playDriverAnimation = true);
	//! Returns true if engine is running
	proto external bool IsEngineOn();
	//! Returns the engine startup chance in <0, 100>.
	proto external float GetEngineStartupChance();
	/*!
	Sets the engine startup chance.
	\param chance Startup chance in <0, 100>.
	*/
	proto external void SetEngineStartupChance(float chance);
	//! Returns true if the engine is drowned.
	proto external bool GetEngineDrowned();
	/*!
	Sets the engine drowned.
	\param drowned True to set the engine drowned, false otherwise.
	*/
	proto external void SetEngineDrowned(bool drowned);

	// callbacks

	//! Gets called after init when all components are initialized.
	event void OnPostInit(IEntity owner);
	//! Gets called when the engine start routine begins (animation event - NOT IMPLEMENTED IN BASE VEHICLE).
	event void OnEngineStartBegin();
	//! Gets called while engine starter is active (animation event - NOT IMPLEMENTED IN BASE VEHICLE).
	event void OnEngineStartProgress();
	//! Gets called when the engine start routine was interrupted.
	event void OnEngineStartInterrupt();
	//! Gets called when the engine start routine has successfully completed.
	event void OnEngineStartSuccess();
	//! Gets called when the engine start routine has failed.
	event void OnEngineStartFail(EVehicleEngineStartFailedReason reason);
	/*!
	Is called every time the controller wants to start the engine.
	\return true if the engine can start, false otherwise.
	*/
	event bool OnBeforeEngineStart();
	//! Is called every time the engine starts.
	event void OnEngineStart();
	//! Is called every time the engine stops.
	event void OnEngineStop();
}

/*!
\}
*/
