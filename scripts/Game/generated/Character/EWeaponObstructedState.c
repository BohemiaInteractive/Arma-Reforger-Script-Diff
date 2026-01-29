/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

enum EWeaponObstructedState
{
	//! Missing components, or obstruction test was not possible.
	INVALID,
	//! Obstruction is at 0.0.
	UNOBSTRUCTED,
	//! Character is moving back to avoid obstruction and can still shoot and ADS
	SLIGHTLY_OBSTRUCTED,
	//! Character is moving their weapon back to avoid obstruction, can shoot but cannot ADS
	SIGNIFICANTLY_OBSTRUCTED,
	//! obstruction alpha higher then fully obstructed threshold
	FULLY_OBSTRUCTED_CANT_FIRE,
}

/*!
\}
*/
