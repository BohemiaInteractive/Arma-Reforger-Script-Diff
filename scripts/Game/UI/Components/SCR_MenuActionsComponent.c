/*!
Handles activating action listeners when the menu is focused. Requires the menu to be handled by a child of MenuRootBase, and the actions to be in an active context
*/
void ScriptInvokerActionMethod(string name, float multiplier);
typedef func ScriptInvokerActionMethod;
typedef ScriptInvokerBase<ScriptInvokerActionMethod> ScriptInvokerAction;

//------------------------------------------------------------------------------------------------
class SCR_MenuActionsComponent : SCR_ScriptedWidgetComponent
{
	[Attribute(desc: "Actions to be shown in the Tooltip")]
	protected ref array<ref SCR_MenuActionPreset> m_aActions;

	[Attribute("400", desc: "Action listeners activation delay")]
	protected int m_iDelay;

	protected bool m_bHasActionListeners;

	protected ref array<SCR_MenuActionPreset> m_aPressActions = {};
	protected ref array<SCR_MenuActionPreset> m_aValueActions = {};
	
	protected ref ScriptInvokerAction m_OnAction;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		// Setup actions
		foreach (SCR_MenuActionPreset action : m_aActions)
		{
			switch (action.m_eActionTrigger)
			{
				case EActionTrigger.UP:
				case EActionTrigger.DOWN:
				case EActionTrigger.PRESSED:
				{
					m_aPressActions.Insert(action);
					break;
				}
				
				case EActionTrigger.VALUE:
				{
					m_aValueActions.Insert(action);
					break;
				}
			}
		}
		
		AddActionListenersDelayed(m_iDelay);
		
		SCR_MenuHelper().GetOnMenuFocusLost().Insert(OnMenuDeactivated);
		SCR_MenuHelper().GetOnMenuHide().Insert(OnMenuDeactivated);
		SCR_MenuHelper().GetOnMenuClose().Insert(OnMenuDeactivated);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		RemoveActionListeners();
		
		SCR_MenuHelper().GetOnMenuFocusLost().Remove(OnMenuDeactivated);
		SCR_MenuHelper().GetOnMenuHide().Remove(OnMenuDeactivated);
		SCR_MenuHelper().GetOnMenuClose().Remove(OnMenuDeactivated);
	}

	//------------------------------------------------------------------------------------------------
	// Owner Menu Events
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	protected void OnMenuDeactivated(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			RemoveActionListeners();
	}
	
	//------------------------------------------------------------------------------------------------
	// Events
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Bind this in your menu
	protected void OnAction(float multiplier, EActionTrigger reason)
	{
		//! We might be in a hidden tab or menu, in which case we don't want the invoker to trigger
		if (!m_wRoot.IsVisible() || !m_OnAction)
			return;

		InputManager inputManager = GetGame().GetInputManager();
		
		switch (reason)
		{
			case EActionTrigger.UP:
			case EActionTrigger.DOWN:
			case EActionTrigger.PRESSED:
			{
				foreach (SCR_MenuActionPreset action : m_aPressActions)
				{
					if (inputManager.GetActionTriggered(action.m_sActionName) && action.m_eInputMode == inputManager.GetLastUsedInputDevice())
					{
						m_OnAction.Invoke(action.m_sActionName, multiplier);
						
						if (!action.m_sActivationSound.IsEmpty())
							SCR_UISoundEntity.SoundEvent(action.m_sActivationSound);
					}
				}
				
				break;
			}
			
			case EActionTrigger.VALUE:
			{
				foreach (SCR_MenuActionPreset action : m_aValueActions)
				{
					if (inputManager.GetActionValue(action.m_sActionName) != 0 && action.m_eInputMode == inputManager.GetLastUsedInputDevice())
						m_OnAction.Invoke(action.m_sActionName, multiplier);
				}
				
				break;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// Action Listeners
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	//! Activate / deactivate in your menu. Tried calling this on focus gained / lost, but if the menu is not focused the component gets deactivated
	//! As of now, if a button press closes a menu and that button's action is active in the menu below, it may get triggered immediatly, thus the delayed activation. This is already being investigated by the Enfusion guys
	protected void AddActionListenersDelayed(int delay)
	{
		if (m_bHasActionListeners)
			return;

		GetGame().GetCallqueue().CallLater(AddActionListeners, delay);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddActionListeners()
	{
		GetGame().GetCallqueue().Remove(AddActionListeners);

		if (m_bHasActionListeners)
			return;

		m_bHasActionListeners = true;

		InputManager inputManager = GetGame().GetInputManager();
		string name;
		EActionTrigger trigger;
		foreach (SCR_MenuActionPreset action : m_aActions)
		{
			name = action.m_sActionName;
			trigger = action.m_eActionTrigger;
			inputManager.AddActionListener(name, trigger, OnAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveActionListeners()
	{
		GetGame().GetCallqueue().Remove(AddActionListeners);

		if (!m_bHasActionListeners)
			return;

		m_bHasActionListeners = false;

		InputManager inputManager = GetGame().GetInputManager();
		string name;
		EActionTrigger trigger;
		foreach (SCR_MenuActionPreset action : m_aActions)
		{
			name = action.m_sActionName;
			trigger = action.m_eActionTrigger;
			inputManager.RemoveActionListener(name, trigger, OnAction);
		}
	}

	//------------------------------------------------------------------------------------------------
	void ActivateActions()
	{
		AddActionListenersDelayed(m_iDelay);
	}

	//------------------------------------------------------------------------------------------------
	void DeactivateActions()
	{
		RemoveActionListeners();
	}

	//------------------------------------------------------------------------------------------------
	// Utility
	//------------------------------------------------------------------------------------------------
	//------------------------------------------------------------------------------------------------
	array<ref SCR_MenuActionPreset> GetActions()
	{
		return m_aActions;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerAction GetOnAction()
	{
		if (!m_OnAction)
			m_OnAction = new ScriptInvokerAction();

		return m_OnAction;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_MenuActionsComponent FindComponent(Widget w)
	{
		return SCR_MenuActionsComponent.Cast(w.FindHandler(SCR_MenuActionsComponent));
	}
}

//------------------------------------------------------------------------------------------------
//! Configuration for an action
[BaseContainerProps(), SCR_BaseContainerCustomTitleField("m_sActionName")]
class SCR_MenuActionPreset
{
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// Overreliance on strings: UIs have hardcoded strings everywhere, often hidden in huge layouts and a myriad of components, making them hard to mantain. Furthermore, changing the hierarchy of a parent prefab will destroy of all these settings in children
	
	[Attribute()]
	string m_sActionName;
	
	[Attribute(typename.EnumToString(EActionTrigger, EActionTrigger.PRESSED), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EActionTrigger))]
	EActionTrigger m_eActionTrigger;
	
	[Attribute(SCR_SoundEvent.CLICK, UIWidgets.EditBox)]
	string m_sActivationSound;

	//---- REFACTOR NOTE END ----
	
	//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
	// These should be flags: as of now, having the same action active on different input devices means adding multiple entries to the array of presets, making the component's setup needlessly bloated
		
	[Attribute(typename.EnumToString(EInputDeviceType, EInputDeviceType.MOUSE), UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EInputDeviceType))]
	EInputDeviceType m_eInputMode;
	
	//---- REFACTOR NOTE END ----
}

