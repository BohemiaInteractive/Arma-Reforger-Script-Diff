/**
Deprecated component - for this functionality, use eventHandlerComponent
If focused will send out event
*/
class SCR_OnFocusUIComponent: ScriptedWidgetComponent
{	
	protected ref ScriptInvoker Event_OnFocusChanged = new ScriptInvoker;
	
	ScriptInvoker GetOnFocusChanged()
	{
		return Event_OnFocusChanged;
	}
	
	override bool OnFocus(Widget w, int x, int y)
	{
		Event_OnFocusChanged.Invoke(true);
		return true;
	}
	
	override bool OnFocusLost(Widget w, int x, int y)
	{
		Event_OnFocusChanged.Invoke(false);
		return false;
	}
	
};