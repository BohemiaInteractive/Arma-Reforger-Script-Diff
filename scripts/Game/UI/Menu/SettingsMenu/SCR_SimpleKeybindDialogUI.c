//---------------------------------------------------------------------------------------------
class SCR_SimpleKeybindDialogUI : SCR_KeybindDialogBase
{
	protected ref InputBinding m_Binding;
	protected SCR_KeybindActionDisplayRowComponent m_ActionDisplayRow;
	protected RichTextWidget m_wActionName;
	protected SizeLayoutWidget m_wActionRowSize;
	protected Widget m_wActionRowWrapper;
	protected RichTextWidget m_wCloseHint;
		
	protected bool m_bShowOverrideWarning;
	
	protected ref ScriptInvokerVoid m_OnKeyCaptured;

	protected const string CLOSE_HINT = "#AR-Settings_KeybindSimple_CloseHint";
	
	protected const string ACTION_MENU_BACK_KEYBIND = "MenuBackKeybind";
	protected const string ACTION_DISPLAY_ROOT = "KeybindActionDisplayRow";
	
	protected bool m_WasCanceled = false;
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		super.OnMenuOpen(preset);

		if (m_Binding)
			GetGame().GetInputManager().AddActionListener(ACTION_MENU_BACK_KEYBIND, EActionTrigger.DOWN, CancelCapture);
		
		m_ActionDisplayRow = SCR_KeybindActionDisplayRowComponent.FindComponentInHierarchy(m_wRoot);
		
		m_wActionName = RichTextWidget.Cast(m_wRoot.FindAnyWidget("ActionName"));
		if (m_wActionName)
			m_wActionName.SetText(m_sDisplayName);
		
		m_wActionRowSize = SizeLayoutWidget.Cast(m_wRoot.FindAnyWidget("ActionDisplayRowSize"));
		m_wActionRowWrapper = m_wRoot.FindAnyWidget("ActionMainHorizontalLayout");
		m_wCloseHint = RichTextWidget.Cast(m_wRoot.FindAnyWidget("CloseHintText"));
		if (m_wCloseHint)
		{
			string device = UIConstants.DEVICE_KEYBOARD;
			if (m_eDevice == EInputDeviceType.GAMEPAD)
				device = UIConstants.DEVICE_GAMEPAD;
			
			string actionText = string.Format(
				"<action name='%1' scale='%2' state='%3'/>", 
				ACTION_MENU_BACK_KEYBIND, 
				UIConstants.ACTION_DISPLAY_ICON_SCALE_VERY_BIG, 
				UIConstants.GetActionDisplayStateAttribute(SCR_EActionDisplayState.NON_INTERACTABLE_HINT)
			);
			
			m_wCloseHint.SetText(WidgetManager.Translate(CLOSE_HINT, actionText));
		}
		
		SetMessageColor(Color.FromInt(UIColors.NEUTRAL_ACTIVE_STANDBY.PackToInt()));
		GetMessageWidget().SetVisible(m_bShowOverrideWarning);
		
		// Wait a frame for the action name text to be initialized, we need it's screen size
		GetGame().GetCallqueue().Call(SetupActionDisplayFrameSkip);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		if (!m_Binding)
			return;

		if (m_Binding.GetCaptureState() == EInputBindingCaptureState.IDLE)
		{
			m_Binding.Save();

			if (!m_WasCanceled)
				SCR_AnalyticsApplication.GetInstance().ChangeKeybind(m_sActionName, m_Entry.m_sPreset);
			
			if (m_OnKeyCaptured)
				m_OnKeyCaptured.Invoke(); //TODO: there is no "capturing cancelled" state nor check. This will be called upon closing the dialog as well. FIX
			
			Close();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuClose()
	{
		super.OnMenuClose();
		
		if (m_Binding && m_Binding.GetCaptureState() == EInputBindingCaptureState.CAPTURING)
			m_Binding.CancelCapture();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupActionDisplayFrameSkip()
	{
		GetGame().GetCallqueue().Call(SetupActionDisplay);
	}
	
	//------------------------------------------------------------------------------------------------
	// Calculate Size allotted to action displays
	protected void SetupActionDisplay()
	{
		if (!m_wActionRowSize || !m_wActionName || !m_ActionDisplayRow || !m_wActionRowWrapper || !m_SettingsKeybindModule)
			return;

		float textX, textY, wrapperX, wrapperY;
		m_wActionName.GetScreenSize(textX, textY);
		m_wActionRowWrapper.GetScreenSize(wrapperX, wrapperY);

		m_wActionRowSize.SetWidthOverride(GetGame().GetWorkspace().DPIUnscale(wrapperX - textX));
		m_ActionDisplayRow.CreateActionDisplays(m_Entry, m_SettingsKeybindModule, m_eDevice);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CancelCapture()
	{
		GetGame().GetInputManager().RemoveActionListener(ACTION_MENU_BACK_KEYBIND, EActionTrigger.DOWN, CancelCapture);
		m_Binding.CancelCapture(); //TODO: capture is started by the row component but ended by the dialog. FIX
		m_WasCanceled = true;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnKeyCaptured()
	{
		if (!m_OnKeyCaptured)
			m_OnKeyCaptured = new ScriptInvokerVoid();
		
		return m_OnKeyCaptured;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_SimpleKeybindDialogUI(SCR_KeyBindingEntry entry, string displayName, string actionName, InputBinding binding, EInputDeviceType device, bool showOverrideWarning)
	{
		Setup(entry, displayName, actionName, device);
		m_Binding = binding;
		m_bShowOverrideWarning = showOverrideWarning;
	}
}