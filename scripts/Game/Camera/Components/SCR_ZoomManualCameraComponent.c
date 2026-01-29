//! @ingroup ManualCamera

//! Adjust camera field of view.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_ZoomManualCameraComponent : SCR_BaseManualCameraComponent
{
	protected static const float INERTIA_THRESHOLD = 0.001;
	
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Minimum field of view", params: "0 180 1")]
	protected float m_fMinFOV;
	
	[Attribute(defvalue: "120", uiwidget: UIWidgets.Slider, desc: "Maximum field of view", params: "0 180 1")]
	protected float m_fMaxFOV;
	
	[Attribute(defvalue: "0.1", desc: "Zoom value coefficient")]
	protected float m_fCoef;
	
	[Attribute(defvalue: "0.08", desc: "Indertia strength. Larger values mean more inertia.")]
	protected float m_fInertiaStrength;
	
	protected float m_fTargetFOV;
	protected bool m_bIsAnimating;
	protected bool m_bIsInstant;
	protected ref ScriptInvoker m_OnZoomChange = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvoker GetOnZoomChange()
	{
		return m_OnZoomChange;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		if (m_fTargetFOV != 0)
			data.m_aValues = {m_fTargetFOV};
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		m_fTargetFOV = data.m_aValues[0];
		m_bIsInstant = true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraReset()
	{
		m_fTargetFOV = GetCameraEntity().GetDefaultFOV();
		m_bIsAnimating = true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled)
			return;
		
		if (GetInputManager().GetActionValue("ManualCameraZoomReset"))
		{
			m_fTargetFOV = GetCameraEntity().GetDefaultFOV();
			m_bIsAnimating = true;
		}
		
		float input = 0;
		if (GetInputManager().IsUsingMouseAndKeyboard() || GetInputManager().GetActionValue("ManualCameraModifier"))
			input = GetInputManager().GetActionValue("ManualCameraZoom");
		
		if (input != 0)
		{
			m_fTargetFOV = Math.Clamp(param.fov * (1 - input * m_fCoef), m_fMinFOV, m_fMaxFOV);
			m_bIsAnimating= true;
		}
		
		if (m_bIsInstant)
		{
			m_bIsInstant = false;
			param.fov = m_fTargetFOV;
			param.isDirty = true;
			m_OnZoomChange.Invoke(param.fov);
		}
		else if (m_bIsAnimating)
		{
			param.fov = Math.Lerp(param.fov, m_fTargetFOV, Math.Min(param.timeSlice / m_fInertiaStrength, 1));
			param.isDirty = true;
			if (Math.AbsFloat(param.fov - m_fTargetFOV) < INERTIA_THRESHOLD)
				m_bIsAnimating = false;
			
			m_OnZoomChange.Invoke(param.fov);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
