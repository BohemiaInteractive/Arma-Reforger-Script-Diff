//! @ingroup ManualCamera

//! Adjusting speed at small increments for manual camera.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_AdjustSpeedManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("0.1")]
	protected float m_fMinMultiplier;	
	
	[Attribute("16")]
	protected float m_fMaxMultiplier;
	
	[Attribute(params: "layout")]
	protected ResourceName m_Layout;
	
	[Attribute("3")]
	protected float m_fLayoutDuration;
	
	protected float m_fMultiplier = 1;
	protected TextWidget m_Widget;
	protected float m_fWidgetAlpha;
	protected ref ScriptInvoker m_OnSpeedChange = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! Get event called every time the speed is adjusted.
	//! \return Script invoker
	ScriptInvoker GetOnSpeedChange()
	{
		return m_OnSpeedChange;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_fMultiplier = 1;
		m_OnSpeedChange.Invoke(m_fMultiplier, false);
		m_Widget.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void ManualCameraSpeedReset(float value, EActionTrigger trigger)
	{
		if (!IsEnabled() || !(GetCameraEntity().GetCameraParam().flag & EManualCameraFlag.ROTATE)) return;
		
		m_fMultiplier = 1;
		m_OnSpeedChange.Invoke(m_fMultiplier, true);
		UpdateWidget();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateWidget()
	{
		if (!m_Widget) return;
		
		
		// if min value is very low, e.g. 0.001, then the hardcoded decimal value will not display all data,
		// so we dynamically adjust the displayed precision as we go below 1.0, based on min value
		int dec = 10;
		if (m_fMinMultiplier > 0 && m_fMultiplier < 1.0)
			dec = Math.Round(1.0 / Math.Clamp(m_fMinMultiplier, 0.0, 1.0));
		
		m_Widget.SetTextFormat("#AR-ValueUnit_Short_Multiplier", Math.Round(m_fMultiplier * dec) / dec);
		m_Widget.SetVisible(true);
		m_fWidgetAlpha = 1 + m_fLayoutDuration;
	}

	//------------------------------------------------------------------------------------------------
	protected void FadeOutWidget(float timeSlice)
	{
		if (!m_Widget || !m_Widget.IsVisible()) return;
		
		m_fWidgetAlpha -= timeSlice;
		if (m_fWidgetAlpha > 0)
		{
			m_Widget.SetOpacity(Math.Clamp(m_fWidgetAlpha, 0, 1));
		}
		else
		{
			m_Widget.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		data.m_aValues = {m_fMultiplier};
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		if (data.m_aValues && !data.m_aValues.IsEmpty())
		{
			m_fMultiplier = data.m_aValues[0];
			m_OnSpeedChange.Invoke(m_fMultiplier, true);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled)
		{
			if (m_Widget)
				m_Widget.SetVisible(false);
			
			return;
		}
		
		//--- Adjust
		if (param.flag & EManualCameraFlag.ROTATE)
		{
			float inputValue = GetInputManager().GetActionValue("ManualCameraSpeedAdjust");
			if (inputValue != 0)
			{
				inputValue = Math.Clamp(1 + inputValue * param.timeSlice, 0.5, 2);
				m_fMultiplier = Math.Clamp(m_fMultiplier * inputValue, m_fMinMultiplier, m_fMaxMultiplier);
				m_OnSpeedChange.Invoke(m_fMultiplier, true);
				UpdateWidget();
			}
		}
		
		//--- Apply
		param.multiplier *= m_fMultiplier;
		
		//--- Visualize
		FadeOutWidget(param.timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{	
		if (m_fMinMultiplier > 1 || m_fMaxMultiplier < 1)
		{
			Print("Value range in SCR_AdjustSpeedManualCameraComponent must contain 1!", LogLevel.ERROR);
		}
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		GetInputManager().AddActionListener("ManualCameraSpeedReset", EActionTrigger.DOWN, ManualCameraSpeedReset);
		
		m_Widget = TextWidget.Cast(GetCameraEntity().CreateCameraWidget(m_Layout, false));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		GetInputManager().RemoveActionListener("ManualCameraSpeedReset", EActionTrigger.DOWN, ManualCameraSpeedReset);
		
		if (m_Widget) m_Widget.RemoveFromHierarchy();
	}
}
