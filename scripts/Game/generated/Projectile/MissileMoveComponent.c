/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Projectile
\{
*/

class MissileMoveComponent: ProjectileMoveComponent
{
	proto external void AddForce(vector force);
	proto external void AddTorque(vector torque);
	proto external void SetVelocity(vector velocity);
	proto external void SetAngularVelocity(vector velocity);
	proto external vector GetAngularVelocity();
	proto external EngineState GetCurrentEngineState();
	proto external FlightMode GetCurrentFlightMode();
	proto external float GetCurrentThrustTime();
	proto external float GetCurrentThrustForce();
}

/*!
\}
*/
