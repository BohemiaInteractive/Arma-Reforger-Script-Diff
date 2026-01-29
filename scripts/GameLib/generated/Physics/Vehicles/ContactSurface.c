/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Physics_Vehicles
\{
*/

//! surface properties that influence wheel traction etc.
sealed class ContactSurface: BaseContainer
{
	proto external float GetNoiseSteer();
	proto external float GetNoiseFrequency();
	proto external float GetRoughness();
	proto external float GetFrictionNonTread();
	proto external float GetFrictionTread();
	proto external float GetRollingResistance();
	proto external float GetRollingDrag();
}

/*!
\}
*/
