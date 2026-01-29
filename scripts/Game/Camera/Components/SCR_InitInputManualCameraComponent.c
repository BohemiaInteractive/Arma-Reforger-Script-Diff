//! @ingroup ManualCamera

//! Enable user input only after buttons pressed when the camera was initialzied were released.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_InitInputManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "0.1", desc: "How long it takes to check for initial input (it doesn't happen on the first frame)")]
	private float m_fMinTime;
	
	[Attribute(defvalue: "0.5", desc: "After which duration is the input accepted anyway.")]
	private float m_fMaxTime;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (m_fMaxTime > 0 && (m_fMinTime > 0 || GetInputManager().GetActionValue("ManualCameraAny")))
			param.isManualInputEnabled = false;
		else
			SetEnabled(false); //--- Disable the component after use

		m_fMinTime -= param.timeSlice;
		m_fMaxTime -= param.timeSlice;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
