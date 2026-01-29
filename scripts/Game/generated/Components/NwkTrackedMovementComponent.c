/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class NwkTrackedMovementComponentClass: NwkMovementComponentClass
{
}

class NwkTrackedMovementComponent: NwkMovementComponent
{
	proto external void SetAllowance(bool speedScaledDistance, float distance, float angle, float linearVelocity, float angularVelocity);
	proto external void SetPrediction(bool turnOn);
}

/*!
\}
*/
