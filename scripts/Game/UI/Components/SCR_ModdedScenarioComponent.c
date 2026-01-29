//------------------------------------------------------------------------------------------------
class SCR_ModdedScenarioComponent : SCR_ButtonComponent 
{
	protected Widget m_wPlayablePC;
	protected ref SCR_ButtonSplitComponent m_pButton;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;
		m_wPlayablePC = m_wRoot.FindAnyWidget("PlayablePC");
		m_pButton = SCR_ButtonSplitComponent.Cast(w.FindHandler(SCR_ButtonSplitComponent));

		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_wPlayablePC.SetVisible(!isGamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_OnFocus.Invoke(w);
		super.OnFocus(w, x, y);
		ColorizeWidgets(COLOR_BACKGROUND_HOVERED, COLOR_CONTENT_HOVERED);
		return false;
	}
	
};
