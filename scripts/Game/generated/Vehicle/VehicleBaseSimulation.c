/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class VehicleBaseSimulationClass: GameComponentClass
{
}

class VehicleBaseSimulation: GameComponent
{
	//! Returns true if any wheel/landing gear has contact with ground or other object
	proto external bool HasAnyGroundContact();
	//! Returns the acceleration of the vehicle produced by mechanical forces.
	proto external vector GetGForce();
	//! Returns the total amount of acceleration of the vehicle produced by mechanical forces.
	proto external float GetGForceMagnitude();
	//! Returns true if this component was properly initialized.
	proto external bool IsValid();
	//! Internal use only! Overrides surface material of wheel/track casts
	static proto void SetSurfaceOverride(ResourceName surfaceMat);

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
