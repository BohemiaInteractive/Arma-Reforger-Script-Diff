/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Camera
\{
*/

class PlayerCameraClass: CameraBaseClass
{
}

class PlayerCamera: CameraBase
{
	//! Returns the current value of focus mode in the <0,1> range.
	proto external float GetFocusMode();
	//! Returns the focus field of view in degrees.
	proto external float GetFocusFOV();
	//! Returns interpolated value from baseFOV to focusFOV based on the input scalar value.
	proto external float GetFocusFOVScale(float baseFOV, float scalar);
}

/*!
\}
*/
