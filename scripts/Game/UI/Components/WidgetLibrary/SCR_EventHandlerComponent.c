//! Component for handling common events other scripts need to listen
//! Feel free to add any event in the list
class SCR_EventHandlerComponent : ScriptedWidgetComponent
{
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Old, unmaintained and barely used. Should be merged with SCR_ScriptedWidgetComponent as the base of the widget library, given the need for such events on all UI components

	protected ref ScriptInvokerVoid m_OnHandlerAttached;
	protected ref ScriptInvokerWidget m_OnHandlerDetached;
	protected ref ScriptInvoker m_OnChange;		// should be ScriptInvokerWidget but cannot due to wrong usage elsewhere (SCR_ChatPanel)
	protected ref ScriptInvokerWidget m_OnChangeFinal;
	protected ref ScriptInvoker m_OnFocus;		// should be ScriptInvokerWidget but cannot due to wrong usage elsewhere (SCR_ComboBoxComponent)
	protected ref ScriptInvoker m_OnFocusLost;	// should be ScriptInvokerWidget but cannot due to wrong usage elsewhere
	protected ref ScriptInvoker m_OnMouseEnter;	// should be ScriptInvokerWidget but cannot due to wrong usage elsewhere
	protected ref ScriptInvoker m_OnMouseLeave;	// should be ScriptInvokerWidget but cannot due to wrong usage elsewhere
	protected ref ScriptInvoker m_OnClick;		// should be ScriptInvokerWidget but cannot due to wrong usage elsewhere
	protected ref ScriptInvokerWidget m_OnDoubleClick;
	protected ref ScriptInvokerWidget m_OnMouseButtonDown;
	protected ref ScriptInvokerWidget m_OnMouseButtonUp;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (m_OnHandlerAttached)
			m_OnHandlerAttached.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_OnHandlerDetached)
			m_OnHandlerDetached.Invoke(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (m_OnClick)
			m_OnClick.Invoke(w);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnChange(Widget w, bool finished)
	{
		if (m_OnChange)
			m_OnChange.Invoke(w);

		if (finished && m_OnChangeFinal)
			m_OnChangeFinal.Invoke(w);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseEnter(Widget w, int x, int y)
	{
		if (m_OnMouseEnter)
			m_OnMouseEnter.Invoke(w);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseLeave(Widget w, Widget enterW, int x, int y)
	{
		if (m_OnMouseLeave)
			m_OnMouseLeave.Invoke(w);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		if (m_OnFocus)
			m_OnFocus.Invoke(w);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		if (m_OnFocusLost)
			m_OnFocusLost.Invoke(w);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (m_OnMouseButtonDown)
			m_OnMouseButtonDown.Invoke(w);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (m_OnMouseButtonUp)
			m_OnMouseButtonUp.Invoke(w, button);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (m_OnDoubleClick)
			m_OnDoubleClick.Invoke(w);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnHandlerAttached()
	{
		if (!m_OnHandlerAttached)
			m_OnHandlerAttached = new ScriptInvokerVoid();

		return m_OnHandlerAttached;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget m_OnHandlerDetached()
	{
		if (!m_OnHandlerDetached)
			m_OnHandlerDetached = new ScriptInvokerWidget();

		return m_OnHandlerDetached;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnChange()
	{
		if (!m_OnChange)
			m_OnChange = new ScriptInvoker();

		return m_OnChange;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnChangeFinal()
	{
		if (!m_OnChangeFinal)
			m_OnChangeFinal = new ScriptInvokerWidget();

		return m_OnChangeFinal;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFocus()
	{
		if (!m_OnFocus)
			m_OnFocus = new ScriptInvoker();

		return m_OnFocus;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnFocusLost()
	{
		if (!m_OnFocusLost)
			m_OnFocusLost = new ScriptInvoker();

		return m_OnFocusLost;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseEnter()
	{
		if (!m_OnMouseEnter)
			m_OnMouseEnter = new ScriptInvoker();

		return m_OnMouseEnter;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnMouseLeave()
	{
		if (!m_OnMouseLeave)
			m_OnMouseLeave = new ScriptInvoker();

		return m_OnMouseLeave;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnClick()
	{
		if (!m_OnClick)
			m_OnClick = new ScriptInvoker();

		return m_OnClick;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnMouseButtonDown()
	{
		if (!m_OnMouseButtonDown)
			m_OnMouseButtonDown = new ScriptInvokerWidget();

		return m_OnMouseButtonDown;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnMouseButtonUp()
	{
		if (!m_OnMouseButtonUp)
			m_OnMouseButtonUp = new ScriptInvokerWidget();

		return m_OnMouseButtonUp;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerWidget GetOnDoubleClick()
	{
		if (!m_OnDoubleClick)
			m_OnDoubleClick = new ScriptInvokerWidget();

		return m_OnDoubleClick;
	}

	//---- REFACTOR NOTE END ----
}
