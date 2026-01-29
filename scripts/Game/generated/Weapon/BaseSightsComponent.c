/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseSightsComponent: GameComponent
{
	proto external IEntity GetOwner();
	//! Allow to override Zeroing via aim modifier (only works in Workbench)
	proto external void ForceSightsZeroValue(vector offset, vector angles, vector turnOffset);
	//! Disable forced zeroing data
	proto external void ForceSightsZeroValueEnablel(bool bOnOff);
	proto external bool IsSightADSActive();
	proto external float GetADSActivationPercentage();
	proto external float GetADSDeactivationPercentage();
	proto external bool AreSightsValid(bool front = true, bool rear = true);
	/*!
	Returns additional sights position used for calculating zeroing
	A vector is calculated from start position to end position and used as direction for calculating zeroing offsets
	Start position should be the one closer to eye position than end position
	\param localSpace Whether returned position should be in local space or world space
	*/
	proto external vector GetSightsRearPosition(bool localSpace = false);
	/*!
	Returns additional sights position used for calculating zeroing
	A vector is calculated from start position to end position and used as direction for calculating zeroing offsets
	End position should be the one further from eye position than start position
	\param localSpace Whether returned position should be in local space or world space
	*/
	proto external vector GetSightsFrontPosition(bool localSpace = false);
	/*!
	Returns a direction vector from the rear sight position to the front sight position
	A vector is calculated from the rear sight to the front sight in such a way that
	GetSightsRearPosition() + GetSightsDirection() == GetSightsFrontPosition()
	\param localSpce Wether returned vector is in local or world space
	\param normalize Wether returned vector is normalized or not, independent of the weapon sight geometry
	*/
	proto external vector GetSightsDirection(bool localSpace = false, bool normalize = true);
	/*!
	Like GetSightsDirection, but ignores and bones associated with the sight points. This is used to get an
	untransformed vector which might be needed if camera/optic and turret are rotating together.
	*/
	proto external vector GetSightsDirectionUntransformed(bool localSpace = false, bool normalize = true);
	/*!
	Returns the local sights reference point offset.
	This corresponds to the set point info in base sights component.
	Returns zero vector if no point info is present.
	*/
	proto external vector GetSightsOffset();
	/*!
	Returns the transform matrix of the local sights.
	This corresponds to the set point info in the base sights component.
	\param transform output Transformation matrix for the sight point
	\param localSpace if true, return the local space transformation of the sights. If false, returns the transformation in world space
	\return true if the point is defined, false otherwise. If this function returns false, the output matrix is undefined
	*/
	proto external bool GetSightsTransform(out vector transform[4], bool localSpace = false);
	/*!
	Returns current value of field of view.
	\return Current field of view value in degrees.
	*/
	proto external float GetFOV();
	/*!
	Returns current range info value where
		x: animation value
		y: distance
		z: unused
	\return Returns sights info values or empty vector if none.
	*/
	proto external vector GetCurrentSightsRange();
	/*!
	Returns currently selected sights FOV info or null if none.
	\return Current SightsFOVInfo or null if none.
	*/
	proto external SightsFOVInfo GetFOVInfo();
	//! Percentage 0...1 of recoil that should be applied to camera when using this sights component.
	proto external float GetCameraRecoilAmount();
	//! Boolean indicating whether this sight component is prioritized
	proto external bool GetSightsPriority();
	//! Return true if the sights can be switched to.
	proto external bool GetSightsSkipSwitch();
	//! Hide parts of the sight on ADS. 0 means completely visible, 1 means completely hidden
	proto external void SetSightsVisibility(float visFactor);
	proto external PointInfo GetPositionPointInfo();
	proto external PointInfo GetRearPositionPointInfo();
	proto external PointInfo GetFrontPositionPointInfo();

	// callbacks

	//! Positive weapon angle tilts weapon upwards (muzzle goes up, stock goes down)
	//! Negative weapon angle tilts weapon downwards (muzzle goes down, stock goes up)
	//! Called from GameCode, do not remove!
	event protected bool WB_GetZeroingData(IEntity owner, BaseSightsComponent sights, float weaponAngle, out vector offset, out vector angles);
}

/*!
\}
*/
