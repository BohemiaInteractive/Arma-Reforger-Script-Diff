/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleFixedWingSimulationClass: VehicleBaseSimulationClass
{
}

class VehicleFixedWingSimulation: VehicleBaseSimulation
{
	//! Experimental code for testing

	proto external float GetElevator();
	proto external float GetAilerons();
	proto external float GetRudder();
	proto external float GetThrottle();
	proto external float GetFlaps();
	proto external bool EngineIsOn();
	proto external GenericEntity GetOwner();
	proto external void GetCommonState(out notnull CommonStateV1 state);
	proto external void ApplyForces(vector totalForce, vector totalTorque);

	// callbacks

	event protected void OnPostInit(IEntity owner);
	event protected void OnDelete(IEntity owner);
	event protected void OnSimulate(float dt);
	event protected void OnDebugDraw(IEntity owner, bool isPlayerVehicle, bool isEditorMode);
}

/*!
\}
*/
