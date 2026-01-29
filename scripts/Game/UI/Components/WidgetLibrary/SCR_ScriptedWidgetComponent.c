//! parent of all Scripted Widgets, even non-interactive ones
void ScriptInvokerScriptedWidgetComponentMethod(SCR_ScriptedWidgetComponent component);
typedef func ScriptInvokerScriptedWidgetComponentMethod;
typedef ScriptInvokerBase<ScriptInvokerScriptedWidgetComponentMethod> ScriptInvokerScriptedWidgetComponent;

//------------------------------------------------------------------------------------------------
class SCR_ScriptedWidgetComponent : ScriptedWidgetComponent
{
	protected Widget m_wRoot;

	//TODO: typed invoker ScriptInvokerScriptedWidgetComponent
	ref ScriptInvoker m_OnClick = new ScriptInvoker;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		m_OnClick.Invoke(this);
		return super.OnClick(w, x, y, button);
	}

	//------------------------------------------------------------------------------------------------
	//! Base method for component lookup through the widget library
	static SCR_ScriptedWidgetComponent GetComponent(typename componentType, string name, Widget parent, bool searchAllChildren = true)
	{
		if (!parent || name == string.Empty)
			return null;

		Widget w;
		if (searchAllChildren)
			w = parent.FindAnyWidget(name);
		else
			w = parent.FindWidget(name);

		if (!w)
		{
			Print(string.Format("SCR_ScriptedWidgetComponent.GetComponent: widget not found: %1 %2", componentType, name), LogLevel.WARNING);
			Debug.DumpStack();
			return null;
		}

		return SCR_ScriptedWidgetComponent.Cast(w.FindHandler(componentType));
	}

	//------------------------------------------------------------------------------------------------
	Widget GetRootWidget()
	{
		return m_wRoot;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsVisible()
	{
		if (!m_wRoot)
			return false;
		
		return m_wRoot.IsVisible();
	}
};
