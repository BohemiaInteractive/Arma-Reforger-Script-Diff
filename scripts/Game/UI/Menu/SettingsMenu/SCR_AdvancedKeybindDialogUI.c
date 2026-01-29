//------------------------------------------------------------------------------------------------
class SCR_AdvancedKeybindDialogUI: SCR_KeybindDialogBase
{
	protected SCR_InputButtonComponent m_Select;
	
	protected string m_sActionPreset;
	protected SCR_EActionPrefixType m_eActionPrefixType;
	protected int m_iSelectedKeybind;
	
	protected SCR_ModularButtonComponent m_FocusedBindRowButton;
	protected ref array<SCR_ModularButtonComponent> m_aBindRowButtons = {};
	
	protected Widget m_wPrefixesCombo;
	
	protected const ResourceName KEYBIND_ROW_LAYOUT_PATH = "{25888355688EE402}UI/layouts/Menus/SettingsMenu/BindingMenu/AdvancedKeybindActionRow.layout";
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);
		
		SetTitle(m_sDisplayName);
		
		m_Select = FindButton("Select");
		if (m_Select)
		{
			m_Select.m_OnActivated.Insert(OnSelectButton);
			m_Select.SetVisible(false, false);
		}
		
		SCR_InputButtonComponent button = FindButton("Reset");
		if (button)
			button.m_OnActivated.Insert(ResetBindings);
		
		button = FindButton("DeleteBind");
		if (button)
			button.m_OnActivated.Insert(DeleteSelectedBind);
		
		button = FindButton("UnbindAll");
		if (button)
			button.m_OnActivated.Insert(UnbindAction);
		
		SetupCaptureButton();
		SetupPrefixesCombo();
		SetupBindsCombo();
		SetupCombinationCombo();
		
		ShowBindsForAction();
		ReselectRow();
	}
	
	//------------------------------------------------------------------------------------------------
	// Reselect correct keybind row
	protected void ReselectRow()
	{
		if (!m_aBindRowButtons.IsIndexValid(m_iSelectedKeybind))
			return;
		
		m_FocusedBindRowButton = m_aBindRowButtons[m_iSelectedKeybind];
		GetGame().GetWorkspace().SetFocusedWidget(m_FocusedBindRowButton.GetRootWidget());
		SetSelectedKeybind(m_aBindRowButtons[m_iSelectedKeybind]);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void ShowBindsForAction()
	{
		Widget contentWidget = m_wRoot.FindAnyWidget("ScrollContent");
		if (!contentWidget)
			return;
		
		//clear the content so we prevent duplicates
		Widget children = contentWidget.GetChildren();
		while (children)
		{
			contentWidget.RemoveChild(children);
			children = contentWidget.GetChildren();
		}
		
		int bindCount = m_SettingsKeybindModule.GetActionBindCount(m_sActionName, m_sActionPreset, m_eDevice);
		/* TODO: no keybind is sometimes considered a binding and shows the confilcting bindings message. FIX 
		// Fallback to force showing the No Keybind message if there are no bindings
		if (bindCount == 0)
			UnbindAction();
		*/
		m_aBindRowButtons.Clear();	
		SCR_AdvancedActionRowComponent rowComponent;
		SCR_ModularButtonComponent rowModularButtonComponent;
		Widget rowWidget;
		for (int i = 0; i < bindCount; i++)
		{
			rowWidget = GetGame().GetWorkspace().CreateWidgets(KEYBIND_ROW_LAYOUT_PATH, contentWidget);
			if (!rowWidget)
				continue;
			
			rowComponent = SCR_AdvancedActionRowComponent.FindComponentInHierarchy(rowWidget);
			if (!rowComponent)
				continue;
			
			rowComponent.Init(m_sActionName, m_sActionPreset, i, m_SettingsKeybindModule, m_eDevice);
			
			rowModularButtonComponent = SCR_ModularButtonComponent.FindComponent(rowWidget);
			if (!rowModularButtonComponent)
				continue;
			
			m_aBindRowButtons.Insert(rowModularButtonComponent);
			rowModularButtonComponent.m_OnClicked.Insert(OnKeybindRowClick);
			rowModularButtonComponent.m_OnFocus.Insert(OnKeybindRowFocus);
			rowModularButtonComponent.m_OnFocusLost.Insert(OnKeybindRowFocusLost);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void ShowBindsForCurrentAction()
	{
		ShowBindsForAction();
		
		// Select the last row
		m_iSelectedKeybind = m_aBindRowButtons.Count() - 1;
		ReselectRow();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void SetSelectedKeybind(notnull SCR_ModularButtonComponent modularButtonComp)
	{
		Widget root = modularButtonComp.GetRootWidget();
		if (!root)
			return;
		
		SCR_AdvancedActionRowComponent advancedKeybindComponent = SCR_AdvancedActionRowComponent.FindComponentInHierarchy(root);
		if (!advancedKeybindComponent)
			return;

		m_iSelectedKeybind = advancedKeybindComponent.GetKeybindIndex();
		
		// Visual update, show border on selected and remove from others
		foreach (SCR_ModularButtonComponent comp : m_aBindRowButtons)
		{
			comp.SetToggled(comp == modularButtonComp, false);
		}
	}

	//-----------------------------------------------------------------------------------------------
	protected void ResetBindings()
	{
		m_SettingsKeybindModule.ResetAction(m_sActionName, m_sActionPreset);

		ShowBindsForAction();
		
		m_iSelectedKeybind = 0;
		ReselectRow();
		
		SCR_AnalyticsApplication.GetInstance().ChangeKeybind(m_sActionName, m_sActionPreset);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void DeleteSelectedBind()
	{
		m_SettingsKeybindModule.DeleteActionBindByIndex(m_sActionName, m_iSelectedKeybind, m_sActionPreset);
		
		ShowBindsForAction();
		
		// Select the row immediately above
		m_iSelectedKeybind = Math.Clamp(m_iSelectedKeybind - 1, 0, m_aBindRowButtons.Count() - 1);
		ReselectRow();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void StartCapturingKeybind()
	{
		m_SettingsKeybindModule.StartCaptureForAction(m_sActionName, m_sActionPreset, EInputDeviceType.KEYBOARD, true);

		SCR_SimpleKeybindDialogUI dialog = SCR_KeybindDialogs.CreateSimpleKeybindDialog(
			m_Entry,
			m_sDisplayName,
			m_sActionName,
			m_SettingsKeybindModule.GetInputBindings(), 
			EInputDeviceType.KEYBOARD,
			false
		);
		
		if (!dialog)
			return;

		dialog.GetOnKeyCaptured().Insert(ShowBindsForCurrentAction);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void UnbindAction()
	{
		m_SettingsKeybindModule.UnbindAction(m_sActionName, m_sActionPreset);
		
		ShowBindsForAction();
		
		m_iSelectedKeybind = 0;
		ReselectRow();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void SetupCaptureButton()
	{
		ButtonWidget buttonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("AddKeybind"));
		if (!buttonWidget)
			return;
		
		SCR_ModularButtonComponent buttonComp = SCR_ModularButtonComponent.Cast(buttonWidget.FindHandler(SCR_ModularButtonComponent));
		if (!buttonComp)
			return;
		
		buttonComp.SetEnabled(!m_sActionPreset.IsEmpty());
		buttonComp.m_OnClicked.Insert(StartCapturingKeybind);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void SetupPrefixesCombo()
	{
		m_wPrefixesCombo = m_wRoot.FindAnyWidget("ActionPrefixCombo");
		if (!m_wPrefixesCombo)
		{
			Print("SCR_AdvancedKeybindDialogUI: Action prefix combo widget was not found", LogLevel.WARNING);
			return;
		}
		
		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(m_wPrefixesCombo.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;
		
		array<ref SCR_KeyBindingFilter> filters = m_SettingsKeybindModule.GetFilters(m_eActionPrefixType);		
		
		foreach (SCR_KeyBindingFilter filter : filters)
		{
			comboBoxComponent.AddItem(filter.m_sFilterDisplayName);
		}
		
		comboBoxComponent.m_OnChanged.Insert(OnPrefixesComboChanged);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void SetupBindsCombo()
	{
		Widget systemBindCombo = m_wRoot.FindAnyWidget("SystemKeyCombo");
		if (!systemBindCombo)
		{
			Print("SCR_AdvancedKeybindDialogUI: System key combo widget was not found", LogLevel.WARNING);
		}
		
		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(systemBindCombo.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;
		
		array<ref SCR_KeyBindingBind> binds = m_SettingsKeybindModule.GetCustomBinds();		
		foreach (SCR_KeyBindingBind bind : binds)
		{
			comboBoxComponent.AddItem(bind.m_sBindDisplayName);
		}
		
		comboBoxComponent.m_OnChanged.Insert(OnBindsComboChanged);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void SetupCombinationCombo()
	{
		Widget combinationBindCombo = m_wRoot.FindAnyWidget("ComboKeyCombo");
		if (!combinationBindCombo)
		{
			Print("SCR_AdvancedKeybindDialogUI: Combination key combo widget was not found", LogLevel.WARNING);
			return;
		}
		
		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(combinationBindCombo.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;
		
		array<ref SCR_KeyBindingCombo> combos = m_SettingsKeybindModule.GetCustomComboKeys();
		foreach (SCR_KeyBindingCombo combo : combos)
		{
			comboBoxComponent.AddItem(combo.m_sComboDisplayName);
		}
		
		comboBoxComponent.m_OnChanged.Insert(OnCombinationComboChanged);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPrefixesComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.SetFilterForActionByIndex(m_sActionName, m_sActionPreset, index, m_iSelectedKeybind, m_eActionPrefixType);
		
		ShowBindsForAction();
		
		ReselectRow();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnBindsComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.AddKeybindToActionByIndex(m_sActionName, m_sActionPreset, index);
		
		ShowBindsForAction();
		
		// Select the last row
		m_iSelectedKeybind = m_aBindRowButtons.Count() - 1;
		ReselectRow();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCombinationComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.AddComboToActionByIndex(m_sActionName, m_sActionPreset, index, m_iSelectedKeybind);
		
		ShowBindsForAction();
		
		ReselectRow();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSelectButton(SCR_InputButtonComponent button, string action)
	{
		if (m_FocusedBindRowButton)
			SetSelectedKeybind(m_FocusedBindRowButton);
		
		m_Select.SetVisible(false, false);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnKeybindRowClick(notnull SCR_ModularButtonComponent modularButtonComp)
	{
		if (m_FocusedBindRowButton && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE)
			SetSelectedKeybind(m_FocusedBindRowButton);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnKeybindRowFocus(SCR_ModularButtonComponent comp)
	{
		m_FocusedBindRowButton = comp;
		
		Widget root = comp.GetRootWidget();
		if (!root)
			return;
		
		SCR_AdvancedActionRowComponent advancedKeybindComponent = SCR_AdvancedActionRowComponent.FindComponentInHierarchy(root);
		if (advancedKeybindComponent)
			advancedKeybindComponent.StartScrolling();
		
		if (m_Select && m_iSelectedKeybind != advancedKeybindComponent.GetKeybindIndex())
			m_Select.SetVisible(true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnKeybindRowFocusLost(SCR_ModularButtonComponent comp)
	{
		m_FocusedBindRowButton = null;
		
		if (m_Select)
			m_Select.SetVisible(false, false);
		
		Widget root = comp.GetRootWidget();
		if (!root)
			return;
		
		SCR_AdvancedActionRowComponent advancedKeybindComponent = SCR_AdvancedActionRowComponent.FindComponentInHierarchy(root);
		if (advancedKeybindComponent)
			advancedKeybindComponent.StopScrolling();
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_AdvancedKeybindDialogUI(SCR_KeyBindingEntry entry, string displayName, string actionName, string actionPreset, SCR_EActionPrefixType prefixType)
	{
		Setup(entry, displayName, actionName, EInputDeviceType.KEYBOARD);
		m_sActionPreset = actionPreset;
		m_eActionPrefixType = prefixType;
	}
}