/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleWheeledSimulation_SA_BClass: VehicleBaseSimulationClass
{
}

class VehicleWheeledSimulation_SA_B: VehicleBaseSimulation
{
	//! Returns current vehicle speed in km/h (kilometers per hour).
	proto external float GetSpeedKmh();
	//! Returns current steering input in range < -1, 1 >
	proto external float GetSteering();
	//! Sets steering input.
	//! \param in should be in range < -1, 1 >
	proto external void SetSteering(float in);
	//! Returns current clutch value in range < 0, 1 >
	//!  0 .. clutch disengaged
	//!  1 .. clutch fully engaged
	proto external float GetClutch();
	//! Sets the clutch input
	//! \param in should be in range < 0, 1 >
	proto external void SetClutch(float in);
	//! Returns current brake input in range < 0, 1 >
	proto external float GetBrake();
	//! Sets brake inputs
	//! \param in should be in range < 0, 1 >
	//! \param hb indicating whether handbrake should be applied
	proto external void SetBreak(float in, bool hb);
	//! returns current throttle input in range < 0, 1 >
	proto external float GetThrottle();
	//! Sets throttle input
	//! \param in should be in range < 0, 1 >
	proto external void SetThrottle(float in);
	//! Returns index of currently engaged gear
	proto external int GetGear();
	//! Switches to given gear
	proto external void SetGear(int in);
	//! Returns whether handbrake is currently applied
	proto external bool IsHandbrakeOn();
	//! Starts the engine
	proto external void EngineStart();
	//! Stops the engine
	proto external void EngineStop();
	//! Returns true if engine is running
	proto external bool EngineIsOn();
	//! Returns idle rpm of the engine
	proto external float EngineGetRPMIdle();
	//! Returns rpm at which engine provides maximal torque
	proto external float EngineGetRPMPeakTorque();
	//! Returns rpm at which engine provides maximal power
	proto external float EngineGetRPMPeakPower();
	//! Returns maximal working rpm before engine explodes
	proto external float EngineGetRPMMax();
	//! Returns current engine's RPM
	proto external float EngineGetRPM();
	//! Returns RPM that is feeded back from rest of the powertrain
	proto external float EngineGetRPMFeedback();
	//! Returns current engine load in range < 0, 1 >
	proto external float EngineGetLoad();
	//! Returns initial engine's peak torque in Nm (newton-metre).
	proto external float EngineGetPeakTorque();
	//! Returns current engine's peak torque in Nm (newton-metre).
	proto external float EngineGetPeakTorqueState();
	//! Sets current engine's peak torque.
	//! \param peakTorque must be in Nm (newton-metre).
	proto external void EngineSetPeakTorqueState(float peakTorque);
	//! Returns initial engine's peak power in kW (kilowatts).
	proto external float EngineGetPeakPower();
	//! Returns current engine's peak power in kW (kilowatts).
	proto external float EngineGetPeakPowerState();
	//! Sets current engine's peak power.
	//! \param peakTorque must be in kW (kilowatts).
	proto external void EngineSetPeakPowerState(float peakPower);
	//! Returns initial gearbox efficiency.
	proto external float GearboxGetEfficiency();
	//! Returns current gearbox efficiency.
	proto external float GearboxGetEfficiencyState();
	//! Sets current gearbox efficiency.
	proto external void GearboxSetEfficiencyState(float efficiency);
	//! Returns number of all gear ratios (neutral included).
	proto external int GearboxGearsCount();
	//! Returns number of forward gears.
	proto external int GearboxForwardGearsCount();
	//! Returns number of wheels
	proto external int WheelCount();
	//! Returns stored index of the wheel based on its name
	proto external int WheelGetIndex(string name);
	/*!
	Get the wheel position based on suspension.
	\param wheelIdx Index of the wheel
	\param displacement 0 means the position of the wheel when the suspension is fully extended ( zero g ).
	\return Returns wheel position in local space.
	*/
	proto external vector WheelGetPosition(int wheelIdx, float displacement = 0.0);
	//! Returns name of the wheel based on its index
	proto external string WheelGetName(int wheelIdx);
	//! Returns true if wheel has contact with ground or other object
	proto external bool WheelHasContact(int wheelIdx);
	//! Returns wheel contact material
	proto external GameMaterial WheelGetContactMaterial(int wheelIdx);
	//! Returns wheel contact position in world space
	proto external vector WheelGetContactPosition(int wheelIdx);
	//! Returns wheel contact normal
	proto external vector WheelGetContactNormal(int wheelIdx);
	//! Returns entity which is in contact with wheel
	proto external IEntity WheelGetContactEntity(int wheelIdx);
	//! Returns whether and how is wheel in contact with some liquid
	proto external EWheelContactLiquidState WheelGetContactLiquidState(int wheelIdx);
	//! Returns the material of the liquid which the wheel is in contact
	proto external GameMaterial WheelGetContactLiquidMaterial(int wheelIdx);
	//! Returns the position where the wheel touches a liquid surface
	proto external vector WheelGetContactLiquidPosition(int wheelIdx);
	//! Returns current wheel's RPM
	proto external float WheelGetRPM(int wheelIdx);
	//! Returns initial wheel's radius
	proto external float WheelGetRadius(int wheelIdx);
	//! Returns current wheel's radius
	proto external float WheelGetRadiusState(int wheelIdx);
	//! Sets current wheel's radius
	proto external float WheelSetRadiusState(int wheelIdx, float radius);
	//! Returns initial wheel's mass
	proto external float WheelGetMass(int wheelIdx);
	//! Returns current wheel's mass
	proto external float WheelGetMassState(int wheelIdx);
	//! Sets current wheel's mass
	proto external float WheelSetMassState(int wheelIdx, float mass);
	//! Returns longitudinal slip of tyre
	proto external float WheelGetLongitudinalSlip(int wheelIdx);
	//! Returns lateral slip of tyre
	proto external float WheelGetLateralSlip(int wheelIdx);
	//! Returns current wheel's rolling drag percentage
	proto external float WheelGetRollingDrag(int wheelIdx);
	//! Sets current rolling drag percentage
	proto external void WheelSetRollingDrag(int wheelIdx, float drag);
	//! Returns initial roughness of wheel's tyre surface
	proto external float WheelTyreGetRoughness(int wheelIdx);
	//! Returns current roughness of wheel's tyre surface
	proto external float WheelTyreGetRoughnessState(int wheelIdx);
	//! Sets current roughness of wheel's tyre surface
	proto external void WheelTyreSetRoughnessState(int wheelIdx, float roughness);
	//! Returns tyre's initial longitudinal friction
	proto external float WheelTyreGetLongitudinalFriction(int wheelIdx);
	//! Returns tyre's current longitudinal friction
	proto external float WheelTyreGetLongitudinalFrictionState(int wheelIdx);
	//! Sets tyre's current longitudinal friction
	proto external void WheelTyreSetLongitudinalFrictionState(int wheelIdx, float lngFriction);
	//! Returns tyre's initial lateral friction
	proto external float WheelTyreGetLateralFriction(int wheelIdx);
	//! Returns tyre's current lateral friction
	proto external float WheelTyreGetLateralFrictionState(int wheelIdx);
	//! Sets tyre's current lateral friction
	proto external void WheelTyreSetLateralFrictionState(int wheelIdx, float latFriction);
	//! Returns sensitivity against noise steer value of surface in range < 0, 1 >
	proto external float GetNoiseSteerSensitivity();
	//! Sets sensitivity on noise steer
	//! \param newValue should be in range < 0, 1 >
	proto external void SetNoiseSteerSensitivity(float newValue);
	//! Returns sensitivity against roughness value of surface in range < 0, 1 >
	proto external float GetRoughnessSensitivity();
	//! Sets sensitivity on roughness value of surface
	//! \param newValue should be in range < 0, 1 >
	proto external void SetRoughnessSensitivity(float newValue);
}

/*!
\}
*/
