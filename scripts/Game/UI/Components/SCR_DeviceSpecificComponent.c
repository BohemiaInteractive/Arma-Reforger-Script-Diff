class SCR_DeviceSpecificComponent: ScriptedWidgetComponent
{
	[Attribute("0", UIWidgets.Auto, "")]
	private bool m_bControllerOnly;
	
	private Widget m_Widget;

	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		bool enable = m_bControllerOnly == isGamepad;
		m_Widget.SetEnabled(enable);
		m_Widget.SetVisible(enable);
	}
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) return;
		
		m_Widget = w;
		
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
	}
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode()) return;
		
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
};