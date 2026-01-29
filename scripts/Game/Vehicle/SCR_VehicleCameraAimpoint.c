//-----------------------------------------------------------------------------
/*!
	Defines a curve that represents different translations at different vehicle speeds.
	Allows the user to sample the given curve at provided speed and a value of local translation is returned.

	Can be added or removed to/from SCR_VehicleCameraDataComponent at will,
	to enable or disable this camera behaviour.
*/
[BaseContainerProps()]
class SCR_VehicleCameraAimpoint : Managed
{
	/*!
		Source curve data.
		Defines magnitude of offsets in local space of the camera based on vehicle speed.
		X (horizontal) axis represents the vehicle speed in kph, Y (vertical) axis represents the amount of translation.
	*/
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, desc: "Source data. X (horizontal) axis represents the vehicle speed in kph, Y (vertical) axis represents the amount of translation.")]
	private ref Curve m_pCurveData;
	
	/*!
		The scale of x-axis, i.e. the speed of vehicle in kph we are sampling at.
	*/
	[Attribute("60.0", UIWidgets.Slider, desc: "The scale of curve in X axis, i.e. the speed of vehicle in kph we are sampling at.")]
	private float m_fMaxSpeed;
	/*!
		The scale of x-axis, i.e. the speed of vehicle in kph we are sampling at (when in reverse).
	*/
	[Attribute("35.0", UIWidgets.Slider, desc: "The scale of curve in X axis, i.e. the speed of vehicle in kph we are sampling at (when in reverse).")]
	private float m_fReverseMaxSpeed;
	/*!
		The scale of y-axis, i.e. the translation of camera in meters based on the speed we've sampled at.
	*/
	[Attribute("1.2", UIWidgets.Slider, desc: "The scale of translation in local Z axis (forward), i.e. the forward translation of camera in meters based on the value we've sampled our speed at.", params: "-2 2 0.01")]
	private float m_fForwardTranslationScale;
	/*!
		The scale of y-axis, i.e. the translation of camera in meters based on the speed we've sampled at.
	*/
	[Attribute("-0.3", UIWidgets.Slider, desc: "The scale of translation in local Z axis (forward), i.e. the forward translation of camera in meters based on the value we've sampled our speed at (when in reverse).", params: "-2 2 0.01")]
	private float m_fReverseForwardTranslationScale;
	/*!
		The scale of y-axis, i.e. the amount of translation.
	*/
	[Attribute("-0.1", UIWidgets.Slider, "The scale of translation in local Y axis (up), i.e. the up translation of camera in meters based on the value we've sampled our speed at.", params: "-2 2 0.01")]
	private float m_fUpTranslationScale;
	/*!
		The scale of x-axis, i.e. maximum angular speed we're sampling at for the amount of aside translation.
	*/
	[Attribute("200", UIWidgets.Slider, "The scale of x-axis, i.e. maximum angular speed we're sampling at for the amount of aside translation.", params: "0.01 500 0.01")]
	private float m_fAsideMaxAngularSpeed;
	/*!
		The scale of y-axis, i.e. the maximum aside translation based on the amount of angular speed.
	*/
	[Attribute("0.1", UIWidgets.Slider, "The scale of translation in local X axis (aside), i.e. the up translation of camera based on the value we've sampled our speed at.", params: "-10 10 0.01")]
	private float m_fAsideTranslationScale;	
	
	/*
		Samples the curve for given speed (in kph).
		\return Returns local translation in X axis (aside) in local space.
	*/
	float SampleAside(float asideSpeed)
	{
		float key = 0.0;
		// Reverse
		if (asideSpeed < 0) 
		{
			key = Math.Clamp(-asideSpeed / m_fAsideMaxAngularSpeed, 0.0, 1.0);
		}
		else
			key = Math.Clamp(asideSpeed / m_fAsideMaxAngularSpeed, 0.0, 1.0);
		
		float sample = LegacyCurve.Curve(ECurveType.CatmullRom, key, m_pCurveData)[1] * m_fAsideTranslationScale;
		if (asideSpeed > 0)
			return sample;
		else
			return -sample;
	}
	
	/*
		Samples the curve for given speed (in kph).
		\return Returns the vector of translation. X = 0, Y = up, Z = forward in local space.
	*/
	vector Sample(float speed)
	{		
		if (m_fMaxSpeed == 0)
			return vector.Zero;
		
		float upScale = m_fUpTranslationScale;
		float fwdScale = m_fForwardTranslationScale;
		float key = 0.0;
		// Reverse
		if (speed < 0) 
		{
			key = Math.Clamp(-speed / m_fReverseMaxSpeed, 0.0, 1.0);
			fwdScale = m_fReverseForwardTranslationScale;	
		}
		else
			key = Math.Clamp(speed / m_fMaxSpeed, 0.0, 1.0);
		
		float sample = LegacyCurve.Curve(ECurveType.CatmullRom, key, m_pCurveData)[1];
		return Vector(0, sample * upScale, sample * fwdScale);
	}
};