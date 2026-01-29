/*
	This object defines the shape and default behaviour of camera shake and convenience API.
*/
[BaseContainerProps()]
class SCR_RecoilCameraShakeParams : BaseItemAttributeData
{
	[Attribute("0.01", UIWidgets.Slider, params: "0.001 1.0 0.001", precision: 3, category: "Camera Shake", desc: "Time rate of shake blending in.")]
	float m_fBlendInTime; //!< The duration (or sharpness) of shake blend in transition
	[Attribute("0.175", UIWidgets.Slider, params: "0.001 1.0 0.001", precision: 3, category: "Camera Shake", desc: "Time rate of shake blending out.")]
	float m_fBlendOutTime; //!< The duration (or sharpness) of shake blend out transition
	[Attribute("15.0", UIWidgets.Slider, params: "0.0 60.0 0.001", precision: 3, category: "Camera Shake", desc: "Maximum smoothing velocity.")]
	float m_fMaxVelocity; //!< Maximum velocity of smoothing
	
	/*!
		The target value at which "shake" is considered to be at its peak.
	*/
	[Attribute("0.0375", UIWidgets.Slider, params: "0.0 1.0 0.001", precision: 4, category: "Camera Shake", desc: "Distance of translation at which recoil is deemed at maximum scale.")]
	float m_fRecoilTarget; //!< The maximum z translation that recoil is scaled against.
	[Attribute("1.25", UIWidgets.Slider, params: "0.0 3.0 0.001", precision: 3, category: "Camera Shake", desc: "Maximum percentage of recoil applied based on current vs target recoil translation.")]
	float m_fMaximumPercentage; //!< The final allowed scale of the recoil. Clamps recoil after scaled by m_fRecoilTarget.
	
	
	/* 
		The minimum translation range to pick a random value from.
		This is the raw value per axis (+right, +up, +forward in meters).
		Keep positive only, sign is generated randomly.
	 */
	[Attribute("0.0005 0.0005 0", UIWidgets.EditBox, precision: 4, category: "Camera Shake", desc: "Minimum range to pick translation values from. +right +up +forward in meters.")]
	protected vector m_vTranslationMin;
	/* 
		The maximum translation range to pick a random value from.
		This is the raw value per axis (+right, +up, +forward in meters).
		Keep positive only, sign is generated randomly.
	 */
	[Attribute("0.006 0.006 0", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "Maximum range to pick translation values from. +right +up +forward in meters.")]
	protected vector m_vTranslationMax;
	
	/*!
		The percentage of FOV used when shake is at max value.
	*/
	[Attribute("0.975", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "Percentage of field of view when shake is at peak value.")]
	protected float m_fFOVModifier;
	
	/* 
		On top of the continuous shake there is an impulse which is stronger and has single direction
		for the entirety or most of the underlying shake duration. This value defines the minimum
		rotation range of this impulse.
	 */
	[Attribute("0.005 0.005 1", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "On top of the continuous shake there is an impulse which is stronger and has single direction for the entirety or most of the underlying shake duration. This value defines the minimum rotation range of this impulse. +pitch +yaw +roll in in degrees.")]
	protected vector m_vRotationImpulseMin;
	
	/* 
		On top of the continuous shake there is an impulse which is stronger and has single direction
		for the entirety or most of the underlying shake duration. This value defines the maximum
		rotation range of this impulse.
	 */
	[Attribute("0.012 0.012 1.5", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "On top of the continuous shake there is an impulse which is stronger and has single direction for the entirety or most of the underlying shake duration. This value defines the maximum rotation range of this impulse. +pitch +yaw +roll in degrees.")]
	protected vector m_vRotationImpulseMax;
	
	/* 
		The minimum recoil delta that can trigger large impulses.
	 */
	[Attribute("0.04", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "The minimum recoil delta that can trigger impulses.")]
	float m_fMinImpulseThreshold;
	
	/* 
		The minimum rate at which impulses can be added. (0.1 == up to 0.1 impulses per second, 1 == up to 1 impulses per second)
	*/
	[Attribute("0.333", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "The minimum rate at which impulses can be added. (0.1 == up to 0.1 impulses per second, 1 == up to 1 impulses per second.")]
	float m_fMinImpulseRate;
	
	/* 
		Impulse magnitude multiplier for when character is crouching.
	*/
	[Attribute("0.8", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "Impulse magnitude multiplier for when character is crouching.", params: "0 1 0.01")]
	float m_fImpulseCrouchMagnitudeModifier;
	
	/* 
		Impulse magnitude multiplier for when character is crouching.
	*/
	[Attribute("0.6", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "Impulse magnitude multiplier for when character is crouching.", params: "0 1 0.01")]
	float m_fImpulseProneMagnitudeModifier;
	
	/* 
		Continuous magnitude multiplier for when character is crouching.
	*/
	[Attribute("0.8", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "Continuous magnitude multiplier for when character is crouching.", params: "0 1 0.01")]
	float m_fCrouchMagnitudeModifier;
	
	/* 
		Continuous magnitude multiplier for when character is crouching.
	*/
	[Attribute("0.6", UIWidgets.EditBox, precision: 3, category: "Camera Shake", desc: "Continuous magnitude multiplier for when character is crouching.", params: "0 1 0.01")]
	float m_fProneMagnitudeModifier;
	
	/*!
		Generator used for generating points and ranges of values fed into the shake function.
	*/
	protected ref RandomGenerator m_pRandomGenerator = new RandomGenerator();
	
	/*!
		Returns impulse magnitude for dynamic character stance.
		\param dynamicStance Dynamic stance value of character (0=prone, 0.5=crouch, 1=erect)
	*/	
	float GetStanceImpulseMagnitude(float dynamicStance)
	{
		return GetMagnitude(dynamicStance, m_fImpulseCrouchMagnitudeModifier, m_fImpulseProneMagnitudeModifier, 1);
	}
	
	/*!
		Returns continuous magnitude for dynamic character stance.
		\param dynamicStance Dynamic stance value of character (0=prone, 0.5=crouch, 1=erect)
	*/	
	float GetStanceMagnitude(float dynamicStance)
	{
		return GetMagnitude(dynamicStance, m_fCrouchMagnitudeModifier, m_fProneMagnitudeModifier, 1);
	}
	
	protected float GetMagnitude(float stance, float crouch, float prone, float erect)
	{
		// Interpolate between crouch and erect
		if (stance >= 0.5)
		{
			float t = (stance * 2) - 1;
			return Math.Lerp(crouch, erect, t);			
		}
		
		// Interpolate between prone and crouch
		float t = (stance * 2);
		return Math.Lerp(prone, crouch, t);
	}
	
	/*!
		Generates a random float in both the positive and negative interval based on provided parameters.
	*/
	protected float RandomFloat(float min, float max)
	{
		if (min < 0.00001 || max < 0.00001)
			return 0;
		
		float sign = m_pRandomGenerator.RandFloatXY(-1337, 1337).Sign();
		return sign * m_pRandomGenerator.RandFloatXY(min, max);
	} 
	
	/*!
		Generates random vector.
	*/
	protected vector RandomVector(vector min, vector max)
	{
		vector result;
		result[0] = RandomFloat(min[0], max[0]);
		result[1] = RandomFloat(min[1], max[1]);
		result[2] = RandomFloat(min[2], max[2]);
		return result;
	}
	
	/*!
		Generates random vector using a random point on a sphere from provided parameters.
	*/
	protected vector RandomVectorSphere(vector min, vector max, bool uniform = true)
	{
		float sign = m_pRandomGenerator.RandFloatXY(-1337, 1337).Sign();
		vector res = m_pRandomGenerator.GenerateRandomPointInRadius(min.Length(), max.Length(), vector.Zero, uniform);
		res[0] = Math.Clamp(res[0], min[0], max[0]);
		res[1] = Math.Clamp(res[1], min[1], max[1]);
		res[2] = Math.Clamp(res[2], min[2], max[2]);
		return sign * res;
	}
	
	/*!
		Returns field of view scale <0, 1> based on input parameters.
	*/
	float GetFovScale(float progress01)
	{
		progress01 = Math.Clamp(progress01, 0, 1);
		return Math.Lerp(1, m_fFOVModifier, progress01);
	}
	
	/*!
		Returns random translation in the right, up, forward format.
	*/
	vector GetRandomTranslation(bool onSphere = false)
	{
		if (onSphere)
			return RandomVectorSphere(m_vTranslationMin, m_vTranslationMax);
		
		return RandomVector(m_vTranslationMin, m_vTranslationMax);
	}
	
	/*
		Returns random angle impulse in the pitch yaw roll format.
	*/
	vector GetRandomAnglesImpulse(bool onSphere = false)
	{
		if (onSphere)
			return RandomVectorSphere(m_vRotationImpulseMin, m_vRotationImpulseMax);
		
		return RandomVector(m_vRotationImpulseMin, m_vRotationImpulseMax);
	}
	
	/*
		Returns random angle impulse in the yaw pitch roll format.
	*/
	vector GetRandomYawPitchRollImpulse(bool onSphere = false)
	{
		vector angles = GetRandomAnglesImpulse(onSphere);
		return Vector(angles[1], angles[0], angles[2]);
	}
};