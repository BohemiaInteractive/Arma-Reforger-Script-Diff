/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleBaseSimulation: GameComponent
{
	//! Returns true if this component was properly initialized.
	proto external bool IsValid();
	//! Returns the world transformation of a vehicle (internal physics space)
	proto external void GetWorldTransform(out vector mat[4]);
	//! Returns the linear velocity of a vehicle [m/s]
	proto external vector GetVelocity();
	//! Returns the linear velocity in a vehicle local space [m/s]
	proto external vector GetVelocityLocal();
	//! Returns the angular velocity of a vehicle [rad/s]
	proto external vector GetAngularVelocity();
	//! Returns the angular velocity in a vehicle local space [rad/s]
	proto external vector GetAngularVelocityLocal();
	//! Returns the linear acceleration of a vehicle [m/s^2]
	proto external vector GetAcceleration();
	//! Returns the linear acceleration in a vehicle local space [m/s^2]
	proto external vector GetAccelerationLocal();
	//! Returns current vehicle speed [km/h]
	proto external float GetSpeedKmh();
	//! Returns the g-force acting on a vehicle [g(force)]
	proto external vector GetGForce();
	//! Returns the total amount of g-force acting on a vehicle [g(force)]
	proto external float GetGForceMagnitude();
	//! Returns true if any wheel/landing gear has contact with ground or other object
	proto external bool HasAnyGroundContact();
	//! Internal use only! Overrides surface material of wheel/track casts
	proto external void SetSurfaceOverride(ResourceName surfaceMat);

	// callbacks

	/*!
	Called during EOnInit.
	\param owner Entity this component is attached to.
	*/
	event protected void OnInit(IEntity owner);
}

/*!
\}
*/
