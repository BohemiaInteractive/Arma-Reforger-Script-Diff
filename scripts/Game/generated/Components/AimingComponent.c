/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Components
\{
*/

class AimingComponent: BaseAimingComponent
{
	//! Get the init rotation defined in the prefab in degrees
	proto external vector GetInitAiming();
	/*!
	Set the desired aiming rotation. It will move the aiming rotation smoothly to your desired aiming rotation.
	\param inRotation The rotation wanted in radians
	*/
	proto external void SetAimingRotationWanted(vector inRotation);
	/*!
	Set the current aiming rotation
	\param inRotation The rotation wanted in radians
	*/
	proto external void SetAimingRotation(vector inRotation);
	//! Get the current desired aiming rotation in degrees.
	proto external vector GetAimingRotationWanted();
	//! Get the current aiming rotation in degress.
	proto external vector GetAimingRotation();
	//! Get the current aiming direction in local space.
	proto external vector GetAimingDirection();
	//! Get the current aiming direction in world space.
	proto external vector GetAimingDirectionWorld();
	proto external vector GetAimingRotationModification();
	proto external vector GetModifiedAimingTranslation();
	proto external vector GetCurrentRecoilTranslation();
	/*!
	Get the current aiming limits
	\param limitsHoriz Return the horizontal limits
	\param limitsVert Return the vertical limits
	*/
	proto void GetAimingLimits(out vector limitsHoriz, out vector limitsVert);
	/*!
	Returns the aiming translation prior to any transformations in weapon model space (1 = 1 m).

	x: +right, -left
	y: +up, -down
	z: +front, -back
	*/
	proto external vector GetRawAimingTranslation();
	/*!
	Returns the aiming rotation in weapon model space. (1 = 1 rad).

	x: +yaw, -yaw
	y: +pitch, -pitch
	z: +roll, -roll
	*/
	proto external vector GetRawAimingRotation();
}

/*!
\}
*/
