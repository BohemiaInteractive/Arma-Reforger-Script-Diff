//! @ingroup Editor_UI Editor_UI_Components

class SCR_DialogEditorUIComponent : SCR_BaseEditorUIComponent
{
	[Attribute(desc: "Pressing this action toggles the dialog.")]
	protected string m_sToggleActionName;
	
	[Attribute(desc: "Pressing this action activates previously activated item.")]
	protected string m_sRepeatActionName;
	
	[Attribute(desc: "Dialog opened on action press.", uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(ChimeraMenuPreset))]
	protected ChimeraMenuPreset m_DialogPreset;
	
	[Attribute("-1", desc: "When >= 0, it will hide parent of main screen widget when the dialog version is opened.\n0 means this widget will be hidden, 1 means its parent will, etc.")]
	protected int m_iHideLinkedIndex;
	
	protected bool m_bIsInDialog;
	protected bool m_CanCloseWithToggleAction = true;
	protected EditorMenuBase m_EditorMenu;
	protected SCR_DialogEditorUIComponent m_LinkedComponent;
	
	//------------------------------------------------------------------------------------------------
	protected void AllowFocus(Widget w)
	{
		if (m_bIsInDialog)
			w.ClearFlags(WidgetFlags.NOFOCUS);
		else
			w.SetFlags(WidgetFlags.NOFOCUS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetLinkedWidgetVisibility(bool show)
	{
		Widget parent = GetWidget();
		for (int i = 0; i < m_iHideLinkedIndex; i++)
		{
			parent = parent.GetParent();
		}
		parent.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInput(float value, EActionTrigger reason)
	{
		MenuManager menuManager = GetGame().GetMenuManager();
		if (!menuManager)
			return;
		
		if (m_bIsInDialog)
		{
			/*
			//--- Close the dialog by pressing the same key which opened it
			//--- Disabled, not compatible with grids where all directions are needed
			if (m_CanCloseWithToggleAction)
				CloseDialog();
			*/
		}
		else if (m_DialogPreset != 0 && !menuManager.IsAnyDialogOpen() && CanOpenDialog())
		{
			menuManager.OpenDialog(m_DialogPreset);
			SetLinkedWidgetVisibility(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnRepeat()
	{
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanOpenDialog()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseDialog()
	{
		if (m_bIsInDialog)
		{
			EditorMenuBase menu = EditorMenuBase.Cast(GetMenu());
			if (menu)
				menu.CloseSelf();
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		CloseDialog();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float timeSlice)
	{
		m_EditorMenu.GetOnMenuUpdate().Invoke(timeSlice); //--- ToDo: Better solution to update entity icons?
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDialogOpened(SCR_DialogEditorUIComponent linkedComponent);

	//------------------------------------------------------------------------------------------------
	protected void OnDialogClosed(SCR_DialogEditorUIComponent linkedComponent);
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		MenuRootBase menu = GetMenu();
		m_bIsInDialog = menu.Type() != EditorMenuUI;
		
		if (m_bIsInDialog)
		{
			m_EditorMenu = EditorMenuBase.Cast(GetGame().GetMenuManager().FindMenuByPreset(ChimeraMenuPreset.EditorMenu));
			m_LinkedComponent = SCR_DialogEditorUIComponent.Cast(m_EditorMenu.GetRootComponent().FindComponent(Type(), true));
			if (m_LinkedComponent)
				GetGame().GetCallqueue().Call(OnDialogOpened, m_LinkedComponent); //--- Call later to make sure it's executed after HandlerAttachedScripted of inherited classes
			
			SCR_MenuEditorComponent menuComponent = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
			if (menuComponent)
				menuComponent.SetVisible(true);
			
			SCR_StatesEditorComponent statesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
			if (statesManager)
				statesManager.SetSafeDialog(true);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			if (!m_sToggleActionName.IsEmpty())
				inputManager.AddActionListener(m_sToggleActionName, EActionTrigger.DOWN, OnInput);
			
			if (!m_sRepeatActionName.IsEmpty())
				inputManager.AddActionListener(m_sRepeatActionName, EActionTrigger.DOWN, OnRepeat);
			
			if (m_bIsInDialog)
				inputManager.AddActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, CloseDialog);
		}
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		if (m_bIsInDialog && m_EditorMenu)
			menu.GetOnMenuUpdate().Insert(OnMenuUpdate);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (GetGame().OnInputDeviceIsGamepadInvoker())
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		if (m_bIsInDialog)
		{
			if (m_EditorMenu)
			{
				SCR_DialogEditorUIComponent linkedComponent = SCR_DialogEditorUIComponent.Cast(m_EditorMenu.GetRootComponent().FindComponent(Type(), true));
				if (linkedComponent)
				{
					linkedComponent.SetLinkedWidgetVisibility(true);
					linkedComponent.OnDialogClosed(this);
				}
			}
			
			SCR_StatesEditorComponent statesManager = SCR_StatesEditorComponent.Cast(SCR_StatesEditorComponent.GetInstance(SCR_StatesEditorComponent));
			if (statesManager)
				statesManager.SetSafeDialog(false);
		}
		
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			if (!m_sToggleActionName.IsEmpty())
				inputManager.RemoveActionListener(m_sToggleActionName, EActionTrigger.DOWN, OnInput);
			
			if (!m_sRepeatActionName.IsEmpty())
				inputManager.RemoveActionListener(m_sRepeatActionName, EActionTrigger.DOWN, OnRepeat);
			
			if (m_bIsInDialog)
				inputManager.RemoveActionListener(UIConstants.MENU_ACTION_BACK, EActionTrigger.DOWN, CloseDialog);
		}
		
		MenuRootBase menu = GetMenu();
		if (menu && m_bIsInDialog)
			menu.GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
}
