//#define INERTIA_DEBUG

//! @ingroup ManualCamera

//! Inertia applied on camera rotation
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_RotationInertiaManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected static const float INERTIA_THRESHOLD = 0.0002; //--- Squared value
	
	[Attribute(defvalue: "0.03", desc: "Inertia strength. Larger values mean more inertia.")]
	private float m_fInertiaStrength;
	
	private vector m_vRotDelta;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		//--- Terminate when target rotation was achieved
		if (!param.isManualInput && m_vRotDelta.LengthSq() < INERTIA_THRESHOLD) return;
		
		m_vRotDelta = vector.Lerp(m_vRotDelta, param.rotDelta, Math.Min(param.timeSlice * m_fInertiaStrength, 1));
		
		#ifdef INERTIA_DEBUG
			DbgUI.Begin("KEK", 0, 0);
			DbgUI.Text("Yaw Before");
			DbgUI.PlotLive("YawBefore", 500, 100, param.rotDelta[0], 0.01, 100, ARGBF(1, 0.25, 0.25, 0.25));
			DbgUI.Text("Yaw After");
			DbgUI.PlotLive("YawAfter", 500, 100, m_vRotDelta[0], 0.01, 100, ARGBF(1, 1, 1, 1));
			DbgUI.Text("Pitch Before");
			DbgUI.PlotLive("PitchBefore", 500, 100, param.rotDelta[1], 0.01, 100, ARGBF(1, 0.25, 0.25, 0.25));
			DbgUI.Text("Pitch After");
			DbgUI.PlotLive("PitchAfter", 500, 100, m_vRotDelta[1], 0.01, 100, ARGBF(1, 1, 1, 1));
			DbgUI.End();
		#endif
		
		param.rotDelta = m_vRotDelta;
		param.isDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_fInertiaStrength = 1 / Math.Max(m_fInertiaStrength, 0.001);
		return true;
	}
}
