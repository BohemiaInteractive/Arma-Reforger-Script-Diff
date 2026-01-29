//! @ingroup ManualCamera

//! Basic camera rotation with modifier button held (RMB by default)
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_RotateModifierManualCameraComponent : SCR_BaseManualCameraComponent
{
	private bool m_bRotate;
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		//--- Always rotate with controller, cancel rotation when switching to mouse and keyboard
		m_bRotate = isGamepad;
	}

	//------------------------------------------------------------------------------------------------
	protected void ManualCameraRotateDown()
	{
		if (!IsEnabled())
			return;
		
		if (GetCameraEntity().GetCameraParam().isCursorEnabled)
			m_bRotate = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void ManualCameraRotateUp()
	{
		if (!IsEnabled())
			return;
		
		m_bRotate = false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled) return;

		if (m_bRotate)
			param.flag = param.flag | EManualCameraFlag.ROTATE;
		else
			param.flag = param.flag & ~EManualCameraFlag.ROTATE;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		GetInputManager().AddActionListener("ManualCameraRotate", EActionTrigger.DOWN, ManualCameraRotateDown);
		GetInputManager().AddActionListener("ManualCameraRotate", EActionTrigger.UP, ManualCameraRotateUp);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		GetInputManager().RemoveActionListener("ManualCameraRotate", EActionTrigger.DOWN, ManualCameraRotateDown);
		GetInputManager().RemoveActionListener("ManualCameraRotate", EActionTrigger.UP, ManualCameraRotateUp);
	}
}
