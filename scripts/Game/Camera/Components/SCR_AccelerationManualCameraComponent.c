//! @ingroup ManualCamera

//! Basic camera speed acceleration
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_AccelerationManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "0.11", desc: "How long it takes to accelerate to full speed.")]
	private float m_fDuration;	
	
	private float m_fProgress;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (param.isManualInput)
		{
			if (m_fProgress < 1)
			{
				m_fProgress = Math.Min(m_fProgress + param.timeSlice / m_fDuration, 1);
				param.transform[3] = vector.Lerp(param.transformOriginal[3], param.transform[3], m_fProgress);
			}
		}
		else
		{
			m_fProgress = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
