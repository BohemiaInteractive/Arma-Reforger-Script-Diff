/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseCollimatorSightsComponent: SightsComponent
{
	/*!
	Set virtual reticle size.
	\param angularSize apparent angular size of the reticle, in degrees
	\param reticlePortion the portion of the texture (in percent) that covers the angular size, i.e. the reticle "diameter"
	*/
	proto external void SetReticleSize(float angularSize, float reticlePortion);
	//! Get the default angular size of the reticle, in degrees
	proto external float GetReticleAngularSize();
	//! Get the default portion of the reticle that covers the given angular size
	proto external float GetReticlePortion();
	//! Get the number of reticle infos
	proto external int GetNumReticles();
	//! Is reticle index valud
	proto external bool IsReticleValid(int index);
	//! Get a reticle by index
	proto external BaseCollimatorReticleInfo GetReticleByIndex(int index);
	//! Get current reticle shape
	proto external int GetCurrentReticleShape();
	//! Set the next reticle shape. This always works, but might not do anything if only one reticle is defined
	proto external void ReticleNextShape();
	//! Set the previous reticle shape. This always works, but might not do anything if only one reticle is defined
	proto external void ReticlePreviousShape();
	//! Set reticle shape by index. Returns true if successful
	proto external bool SetReticleShapeByIndex(int iIndex);
	//! Get the number of reticle colors
	proto external int GetNumColors();
	//! Is color index valid
	proto external int IsColorValid(int index);
	//! Get a color record by index
	proto external BaseCollimatorReticleColor GetColorByIndex(int index);
	//! Get current reticle color index
	proto external int GetCurrentColor();
	//! Set the next reticle color. This always works, but might not do anything if only one or no color is defined
	proto external void ReticleNextColor();
	//! Set the previous reticle color. This always works, but might not do anything if only one or no color is defined
	proto external void ReticlePreviousColor();
	//! Get the normalized light intensity at the sight
	proto external float GetNormalizedLightIntensity();
	//! Set reticle color by index. Return true if successful
	proto external bool SetReticleColorByIndex(int iIndex);
	//! Set the angular correction (vertical) (in mils)
	proto external void SetVerticalAngularCorrection(float fAngle);
	//! Get the angular correction (vertical) in mils
	proto external float GetVerticalAngularCorrection();
	//!Set the angular correction (widage) (in mils)
	proto external void SetHorizontalAngularCorrection(float fAngle);
	//! Get the angular correction (windage) in mils
	proto external float GetHorizontalAngularCorrection();
	/*!
	Returns true if the sight is active, false if it isn't
	THis doesn't necessarily mean that the sight is visible, it can be active but
	the reticle might be hidden because it is only visible during ADS.
	*/
	proto external bool IsSightActive();
	/*!
	Set the sight on or off.
	Note that the sight can only be forced off. Whether it is on/visible or not
	depends on other factors. By default, the sight is on if and only if
	- ADS is active for a weapon sight
	- There is an occupant in the seat the sight is attached to for vehicle sights
	Calling this function with true as a parameter will force the sight off.
	*/
	proto external void SetSightForcedOff(bool forceOff);
	/*!
	Enable manual brightness setting. If false, brightness is either automatic or fixed to Day/Night mode
	If true, manual brightness can be set
	*/
	proto external void EnableManualBrightness(bool bEnable);
	//! Returns true if manual brightness is enabled
	proto external bool IsManualBrightnessEnabled();
	/*!
	Enable and set manaul brightness control.
	fBrightness is a factor greater or equal to zero. If bClamp is true,
	then the brightess is set between Night intensity (0.0f) and day intensity (1.0f).
	If bClamp is false, then the brightness is set between off (0.0f) and day intensity (1.0f).

	fBrightness might go over 1 for extra brightness, but never below zero.
	*/
	proto external void SetManualBrightness(float fBrightness, bool bClamp);
	//! Return the current reticle manual brightness.
	proto external float GetManualBrightness();
	//! Return true if manual brighness is clamped between day and night, or false if between off and day
	proto external bool GetManualBrightnessClamp();

	// callbacks

	//! Called to update the sight position U/V
	event void UpdateReticlePosition(float u, float v, float uScale, float vScale);
	//! Called on PostInit when all components are added
	event void OnPostInit(IEntity owner);
	//! Called to update the Reticle shape
	event void UpdateReticleShapeIndex(int index);
	//! Called to update reticle color
	event void UpdateReticleColor(vector inner, vector glow);
	//! Called to set auto brightness factor
	event void UpdateReticleBrightnessScale(float scale);
	//! Called to set glow brightness
	event void UpdateReticleBrightness(float lvFactor,  bool useOwn);
	//! Get the Brightness of the reticle glow
	event float GetReticleBrightnessDay();
	event float GetReticleBrightnessNight();
	event void OnSightADSActivate();
	event void OnSightADSDeactivated();
}

/*!
\}
*/
