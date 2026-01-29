//! @ingroup ManualCamera

//! Manual control of camera GUI visibility.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_ToggleUIManualCameraComponent : SCR_BaseManualCameraComponent
{
	//------------------------------------------------------------------------------------------------
	protected void ToggleUI()
	{
		if (!IsEnabled())
			return;
		
		Widget widget = GetCameraEntity().GetWidget();
		if (!widget)
			return;
		
		widget.SetVisible(!widget.IsVisible());
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		InputManager inputManager = GetInputManager();
		if (!inputManager)
			return false;
		
		inputManager.AddActionListener("ManualCameraToggleUI", EActionTrigger.DOWN, ToggleUI);
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		InputManager inputManager = GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.RemoveActionListener("ManualCameraToggleUI", EActionTrigger.DOWN, ToggleUI);
	}
}
