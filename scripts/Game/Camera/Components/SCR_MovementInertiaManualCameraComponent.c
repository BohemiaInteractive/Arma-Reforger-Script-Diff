//! @ingroup ManualCamera

//! Inertia applied on camera movement
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_MovementInertiaManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected static const float INERTIA_THRESHOLD = 0.0002; //--- Squared value
	
	[Attribute(defvalue: "0.08", desc: "Inertia strength. Larger values mean more inertia.")]
	private float m_fInertiaStrength;
	
	private vector m_vVelocity;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		//--- Current velocity
		vector velocity = param.transform[3] - param.transformOriginal[3];
		
		//--- Terminate when target velocity was achieved
		if (!param.isManualInput && vector.DistanceSq(velocity, param.velocityOriginal) < INERTIA_THRESHOLD)
			return;
		
		//--- Apply
		float progress = Math.Min(param.timeSlice * m_fInertiaStrength, 1);
		velocity = vector.Lerp(param.velocityOriginal, velocity, progress);
		vector pos = param.transformOriginal[3] + velocity;
		
		param.transform[3] = pos;
		param.isDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_fInertiaStrength = 1 / Math.Max(m_fInertiaStrength, 0.001);
		return true;
	}
}
