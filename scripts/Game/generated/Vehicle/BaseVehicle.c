/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Vehicle
\{
*/

class BaseVehicle: GameEntity
{
	proto external SCR_DamageManagerComponent GetDamageManager();
	/*!
	Sets the wreck model of the vehicle. This will change the state of the vehicle to a wrecked vehicle which means some components will be deactivated.
	\param newModel New model which should be displayed
	*/
	proto external void SetWreckModel(ResourceName newModel);
	//! Returns true if the vehicle is not upside down
	proto external bool IsAccessible();
}

/*!
\}
*/
