//! @ingroup ManualCamera

//! Instantly set the speed to a value within a sequence
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_SetSpeedManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("2.5")]
	protected float m_fGamepadCameraSpeed;
	
	[Attribute("12")]
	protected float m_fGamepadCameraSpeedBoosted;
	
	protected bool m_bIsBoosted;
	protected bool m_bIsUsingGamepad;
	
//	[Attribute("3")]
//	float m_fLayoutDuration;
//
//	[Attribute(params: "layout")]
//	private ResourceName m_Layout;
//	private TextWidget m_Widget;
//	private float m_fWidgetAlpha;

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_bIsUsingGamepad = isGamepad;
		
		if (!m_bIsUsingGamepad)
			m_bIsBoosted = false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ManualCameraSpeedToggle()
	{
		if (!IsEnabled())
			return;
		
		m_bIsBoosted = !m_bIsBoosted;
		
		//UpdateWidget();
	}
	
//	//------------------------------------------------------------------------------------------------
//	protected void UpdateWidget()
//	{
//		if (!m_Widget) return;
//
//		if (!m_bIsBoosted)
//			m_Widget.SetTextFormat("#AR-ValueUnit_Short_Multiplier", m_fGamepadCameraSpeed);
//		else
//			m_Widget.SetTextFormat("#AR-ValueUnit_Short_Multiplier", m_fGamepadCameraSpeedBoosted);
//
//		m_Widget.SetVisible(true);
//		m_fWidgetAlpha = 1 + m_fLayoutDuration;
//	}

//	//------------------------------------------------------------------------------------------------
//	protected void FadeOutWidget(float timeSlice)
//	{
//		if (!m_Widget || !m_Widget.IsVisible()) return;
//
//		m_fWidgetAlpha -= timeSlice;
//		if (m_fWidgetAlpha > 0)
//		{
//			m_Widget.SetOpacity(Math.Clamp(m_fWidgetAlpha, 0, 1));
//		}
//		else
//		{
//			m_Widget.SetVisible(false);
//		}
//	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled || !m_bIsUsingGamepad)
			return;

		if (!m_bIsBoosted)
		{
			param.multiplier *= m_fGamepadCameraSpeed;
		}
		else
		{
			param.multiplier *= m_fGamepadCameraSpeedBoosted;
			
			float lateral = GetInputManager().GetActionValue("ManualCameraMoveLateral");
			float vertical = GetInputManager().GetActionValue("ManualCameraMoveVertical");
			float longitudinal = GetInputManager().GetActionValue("ManualCameraMoveLongitudinal");
			
			if (lateral == 0 && vertical == 0 && longitudinal == 0)
			{
				m_bIsBoosted = false;
				//UpdateWidget()
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		GetInputManager().AddActionListener("ManualCameraSpeedToggle", EActionTrigger.DOWN, ManualCameraSpeedToggle);
		
		//m_Widget = TextWidget.Cast(GetCameraEntity().CreateCameraWidget(m_Layout, false));
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		GetInputManager().RemoveActionListener("ManualCameraSpeedToggle", EActionTrigger.DOWN, ManualCameraSpeedToggle);
		
		//if (m_Widget) m_Widget.RemoveFromHierarchy();
	}
}
