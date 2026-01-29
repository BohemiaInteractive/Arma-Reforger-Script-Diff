/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI
\{
*/

sealed class BlurWidget: Widget
{
	//! Returns intensity of the blur effect
	proto external float GetIntensity();
	//! Sets intensity of the blur effect
	proto external void SetIntensity(float intensity);
	//! Returns true if tint blend mode is set to Add, returns false if it is set to Multiply
	proto external bool IsTintBlendModeAdd();
	//! If isAdd is true sets tint blend mode to Add, if false the mode is set to Multiply
	proto external void SetTintBlendModeAdd(bool isAdd);
	//! Loads a texture from given resource and uses it as a mask, pass an empty string to disable masking
	proto external void SetMask(ResourceName maskResource);
	//! Sets smooth border width in pixels in reference resolution
	proto external void SetSmoothBorder(float left, float top, float right, float bottom);
}

/*!
\}
*/
