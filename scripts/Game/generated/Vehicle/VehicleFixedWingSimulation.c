/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleFixedWingSimulation: VehicleBaseSimulation
{
	//! Experimental code for testing

	proto external GenericEntity GetOwner();
	proto external void GetCommonState(out notnull CommonStateV1 state);
	proto external void ApplyForces(vector totalForce, vector totalTorque);
	proto external float GetElevator();
	proto external float GetAilerons();
	proto external float GetRudder();
	proto external float GetThrottle();
	proto external float GetFlaps();
	proto external bool EngineIsOn();

	// callbacks

	event protected void OnPostInit(IEntity owner);
	event protected void OnSimulate(float dt);
	event protected void OnDebugDraw(IEntity owner, bool isPlayerVehicle, bool isEditorMode);
}

/*!
\}
*/
