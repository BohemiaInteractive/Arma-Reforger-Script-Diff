//! Component to execute action when the button or its shortcut is pressed.
class ButtonActionComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: "0", uiwidget: UIWidgets.ComboBox, enums: { ParamEnum("LMB", "0", ""), ParamEnum("RMB", "1", ""), ParamEnum("MMB", "2", ""), ParamEnum("ALL", "3", "") }, desc: "ID of mouse button which activates the action upon click.")]
	private int m_iMouseButton;
	
	[Attribute(desc: "Name of input action which activates the action.")]
	private string m_bActionName;
	
	[Attribute(defvalue: "2", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EActionTrigger))]
	private EActionTrigger m_ActionTrigger;
	
	[Attribute(desc: "Name of the widget which shows key assigned to the input action.")]
	private string m_bWidgetHintName;
	
	private Widget m_wWidget;
	private ref ScriptInvoker m_OnAction = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	//! Get invoker called when the button or its shortcut is pressed.
	//! \return Script invoker
	ScriptInvoker GetOnAction()
	{
		return m_OnAction;
	}

	//------------------------------------------------------------------------------------------------
	//! Get mouse button this component is tracking.
	//! \return Mouse button ID
	int GetMouseButton()
	{
		return m_iMouseButton;
	}

	//------------------------------------------------------------------------------------------------
	//! Get invoker called when the button or its shortcut is pressed.
	//! Example of use:
	//! \code
	//! ScriptInvoker onButtonPressed = ButtonActionComponent.GetOnAction(w, "MyButton");
	//! if (onButtonPressed)
	//! 	onButtonPressed.Insert(OnButtonPressed);
	//! \endcode
	//! \param[in] w Parent widget
	//! \param[in] buttonName Button whose invoker is looked for
	//! \param[in] canCreateComponent when true, new ButtonActionComponent will be created for the widget when none exists
	//! \param[in] mouseButton
	//! \return Script invoker
	static ScriptInvoker GetOnAction(Widget w, string buttonName, bool canCreateComponent = false, int mouseButton = 0)
	{
		Widget button = w.FindAnyWidget(buttonName);
		if (!button)
		{
			Print(string.Format("Widget '%1' not found in widget '%2'!", buttonName, w.GetName()), LogLevel.ERROR);
			return new ScriptInvoker();
		}
		
		return GetOnAction(button, canCreateComponent, mouseButton);
	}

	//------------------------------------------------------------------------------------------------
	//! Get invoker called when the button or its shortcut is pressed.
	//! Example of use:
	//! \code
	//! ScriptInvoker onButtonPressed = ButtonActionComponent.GetOnAction(buttonWidget);
	//! if (onButtonPressed)
	//! 	onButtonPressed.Insert(OnButtonPressed);
	//! \endcode
	//! \param[in] button button widget
	//! \param[in] canCreateComponent When true, new ButtonActionComponent will be created for the widget when none exists
	//! \param[in] mouseButton ID of mouse button which the component should track
	//! \return Script invoker
	static ScriptInvoker GetOnAction(Widget button, bool canCreateComponent = false, int mouseButton = 0)
	{
		ButtonActionComponent component;
		int countHandlers = button.GetNumHandlers();
		for (int i = 0; i < countHandlers; i++)
		{
			ButtonActionComponent componentCandidate = ButtonActionComponent.Cast(button.GetHandler(i));
			if (componentCandidate && componentCandidate.GetMouseButton() == mouseButton)
			{
				component = componentCandidate;
				break;
			}
		}

		if (!component)
		{
			if (canCreateComponent)
			{
				component = new ButtonActionComponent();
				component.m_iMouseButton = mouseButton;
				button.AddHandler(component);
			}
			else
			{
				Print(string.Format("Widget '%1' is missing ButtonActionComponent component!", button.GetName()), LogLevel.ERROR);
				return new ScriptInvoker();
			}
		}
		
		return component.GetOnAction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Execute(float value, EActionTrigger reason)
	{
		if (!m_wWidget.IsVisible() || !m_wWidget.IsEnabledInHierarchy())
			return;

		m_OnAction.Invoke(m_wWidget, value, reason);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnAction(float value, EActionTrigger reason)
	{
		Execute(value, reason);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (button == m_iMouseButton || m_iMouseButton == 3)
			Execute(1, EActionTrigger.DOWN);

		return false;
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wWidget = w;
		
		//--- Get hint widget
		RichTextWidget widgetIcon;
		if (m_bWidgetHintName.IsEmpty())
			widgetIcon = RichTextWidget.Cast(w);
		else
			widgetIcon = RichTextWidget.Cast(w.FindAnyWidget(m_bWidgetHintName));
		
		if (widgetIcon)
		{
			if (m_bActionName != "")
				widgetIcon.SetText(string.Format("<action name=\"%1\"/>", m_bActionName));
			else
				widgetIcon.SetText("");
		}
		else
		{
			if (m_bWidgetHintName != "")
				Print(string.Format("ButtonActionComponent: RichTextWidget widget '%1' not found on widget '%2'!", m_bWidgetHintName, w.GetName()), LogLevel.ERROR);
		}
		
		//--- Add action listener
		if (m_bActionName == "")
			return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		InputManager inputManager = game.GetInputManager();
		if (inputManager)
			inputManager.AddActionListener(m_bActionName, m_ActionTrigger, OnAction);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_bActionName == "")
			return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		InputManager inputManager = game.GetInputManager();
		if (inputManager)
			inputManager.RemoveActionListener(m_bActionName, m_ActionTrigger, OnAction);
	}
}
