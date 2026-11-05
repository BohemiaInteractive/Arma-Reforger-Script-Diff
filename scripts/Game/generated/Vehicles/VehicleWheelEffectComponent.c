/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicles
\{
*/

class VehicleWheelEffectComponent: GameComponent
{
	proto external IEntity GetOwner();
	//! Returns the wheel surface temperature in degrees Celcius.
	//! The burnout effect starts at 150 degrees
	proto external float GetWheelTemperature(int wheelIdx);
	//! Returns the dust factor on the wheel, in range [0, 1]
	proto external float GetWheelDust(int wheelIdx);
	//! Returns the mud factor on the wheel, in range [0, 1]
	proto external float GetWheelMud(int wheelIdx);
	//! Returns the mud factor on the body, in range [0, 1]
	proto external float GetBodyDust();
	//! Returns the mud factor on the body, in range [0, 1]
	proto external float GetBodyMud();
	//! Set the wheel surface temperature in degrees Celcius
	//! The burnout effect starts at 150 degrees
	proto external void SetWheelTemperature(int wheelIdx, float value);
	//! Set the dust factor on the wheel, in range [0, 1]
	proto external void SetWheelDust(int wheelIdx, float value);
	//! Set the mud factor on the wheel, in range [0, 1]
	proto external void SetWheelMud(int wheelIdx, float value);
	//! Set the dust factor on the body, in range [0, 1]
	proto external void SetBodyDust(float value);
	//! Set the mud factor on the body, in range [0, 1]
	proto external void SetBodyMud(float value);
	//! Returns true if sliding track (skid marks) are enabled
	proto external bool IsSlidingTrackEnabled();
	proto external void EnableSlidingTrack(bool enabled);
	//! Returns true if static track (trails) are enabled
	proto external bool IsStaticTrackEnabled();
	proto external void EnableStaticTrack(bool enabled);
}

/*!
\}
*/
