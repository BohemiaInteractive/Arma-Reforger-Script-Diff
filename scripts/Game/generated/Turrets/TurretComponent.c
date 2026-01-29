/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Turrets
\{
*/

class TurretComponent: AimingComponent
{
	proto external BaseSightsComponent GetSights();
	//! Returns true if the turret entity itself is physically rotated rather than just one of its bones.
	//! If it's just bones rotating, the turret entity transform would remain unchanged.
	//! This happens e.g. when the turret is rotated via ProceduralAnimationComponent that rotates
	//! the turret via the "Scene_Root" bone.
	proto external bool HasRootRotation();
	proto external bool HasMoveableBase();
	proto external PointInfo GetCameraAttachmentSlot();
	proto external bool IsVehicleMounted();
	proto external float GetBaseRotation();
	proto external void SetBaseRotation(float fRotation);
	/*!
	Calculates aiming angle to target and returns horizontal and vertical excess of target angle compared to aiming limits, in degrees.
		outExcess[0] - horizontal excess
		outExcess[1] - vertical excess
	Example:
		Limits = -10...10. TgtAngle = 12. Excess = 2.
		Limits = -10...10. TgtAngle = -13. Excess = -3.
		Limits = -10...10. TgtAngle = 9. Excess = 0.
	*/
	proto external vector GetAimingAngleExcess(vector tgtPosWorld);
	proto external void SetSights(int index);
	proto external int NextSights();
	proto external void SwitchNextSights();
	proto external void SwitchPrevSights();
	proto external bool IsUsingWeaponSights();
}

/*!
\}
*/
