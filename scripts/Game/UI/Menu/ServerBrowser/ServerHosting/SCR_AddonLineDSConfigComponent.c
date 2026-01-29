//----------------------------------------------------------------------------------------------
class SCR_AddonLineDSConfigComponent : SCR_AddonLineBaseComponent
{
	protected const string BUTTON_UP = "m_UpButton";
	protected const string BUTTON_DOWN = "m_DownButton";
	protected const string BUTTON_SORT_CONFIRM = "m_SortConfirmButton";
	protected const string BUTTON_DISABLE_REQUIRED = "m_DisableRequiredButton";
	
	// Marks the addon as enabled in the hosting menu, but does NOT actually enable the addon. It's a flag for the creation of the server config
	protected bool m_bWidgetEnabled;
	
	// Marks which addons must be added to the server config to play a specific scenario
	protected bool m_bRequired;
	
	// Sort buttons
	protected SCR_ModularButtonComponent m_ButtonUp;
	protected SCR_ModularButtonComponent m_ButtonDown;
	protected SCR_ModularButtonComponent m_ButtonSortConfirm;
	protected SCR_ModularButtonComponent m_ButtonDisableRequired;
	
	protected bool m_bOnBottom;
	protected bool m_bIsSorting;
	
	protected ref ScriptInvokerScriptedWidgetComponent Event_OnButtonUp;
	protected ref ScriptInvokerScriptedWidgetComponent Event_OnButtonDown;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnSortConfirm;
	protected ref ScriptInvokerScriptedWidgetComponent m_OnRequiredDisabled;

	//----------------------------------------------------------------------------------------------
	override void Init(SCR_WorkshopItem item)
	{
		// Sorting buttons
		// Up
		Widget wButtonUp = m_wRoot.FindAnyWidget(BUTTON_UP);
		if (wButtonUp)
			m_ButtonUp = SCR_ModularButtonComponent.FindComponent(wButtonUp);
		
		if (m_ButtonUp)
		{
			m_ButtonUp.m_OnClicked.Insert(InvokeEventOnButtonUp);
			m_aMouseButtons.Insert(m_ButtonUp);
		}

		// Down
		Widget wButtonDown = m_wRoot.FindAnyWidget(BUTTON_DOWN);
		if (wButtonDown)
			m_ButtonDown = SCR_ModularButtonComponent.FindComponent(wButtonDown);
		
		if (m_ButtonDown)
		{
			m_ButtonDown.m_OnClicked.Insert(InvokeEventOnButtonDown);
			m_aMouseButtons.Insert(m_ButtonDown);
		}

		// Confirm
		Widget sortConfirm = m_wRoot.FindAnyWidget(BUTTON_SORT_CONFIRM);
		if (sortConfirm)
			m_ButtonSortConfirm = SCR_ModularButtonComponent.FindComponent(sortConfirm);
			
		if (m_ButtonSortConfirm)
		{
			m_ButtonSortConfirm.m_OnClicked.Insert(OnSortConfirm);
			m_aMouseButtons.Insert(m_ButtonSortConfirm);
		}
		
		// Disable required
		Widget disableRequired = m_wRoot.FindAnyWidget(BUTTON_DISABLE_REQUIRED);
		if (disableRequired)
			m_ButtonDisableRequired = SCR_ModularButtonComponent.FindComponent(disableRequired);
		
		if (m_ButtonDisableRequired)
			m_ButtonDisableRequired.m_OnClicked.Insert(OnDisableButton);
		
		// Setup rest 
		super.Init(item);
		
		HandleEnableButtons(m_bWidgetEnabled, m_bIsSorting);
	}
	
	//----------------------------------------------------------------------------------------------
	override void UpdateAllWidgets()
	{
		super.UpdateAllWidgets();
		
		HandleEnableButtons(m_bWidgetEnabled, m_bIsSorting);
		HandleSortingButtons();
		
		// Hide action buttons
		m_Widgets.m_DeleteButtonComponent0.SetVisible(false);
		
		m_Widgets.m_wHorizontalState.SetVisible(false);
		m_Widgets.m_UpdateButtonComponent0.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandleEnableButtons(bool addonEnabled, bool forceHidden = false)
	{
		super.HandleEnableButtons(addonEnabled, forceHidden);
		
		if (m_bRequired && m_bWidgetEnabled)
			m_Widgets.m_wSizeMoveLeft.SetVisible(false);
		
		if (m_ButtonDisableRequired)
			m_ButtonDisableRequired.SetVisible(m_bRequired && m_bWidgetEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDisableButton()
	{
		if (!m_bRequired)
		{
			super.OnDisableButton();
			return;
		}
		
		SCR_ConfigurableDialogUi dialog = SCR_ServerHostingDialogs.CreateRequiredDisableDialog();
		if (dialog)
			dialog.m_OnConfirm.Insert(OnRequiredDisableDialogConfirm);
	}

	//----------------------------------------------------------------------------------------------
	override bool IsEnabled()
	{
		return m_bWidgetEnabled;
	}
	
	// Display oredering buttons if enabled
	//------------------------------------------------------------------------------------------------
	protected void HandleSortingButtons()
	{
		bool onTop = m_wRoot.GetZOrder() == 0;
		bool visible = m_bIsSorting || (m_bFocused && GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE);
		
		if (m_ButtonUp)
			m_ButtonUp.SetVisible(!onTop && m_bWidgetEnabled && visible);
		
		if (m_ButtonDown)
			m_ButtonDown.SetVisible(!m_bOnBottom && m_bWidgetEnabled && visible);
		
		if (m_ButtonSortConfirm)
			m_ButtonSortConfirm.SetVisible(m_bIsSorting);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRequiredDisableDialogConfirm()
	{
		if (m_OnRequiredDisabled)
			m_OnRequiredDisabled.Invoke(this);
		
		super.OnDisableButton();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSortConfirm()
	{
		if (m_OnSortConfirm)
			m_OnSortConfirm.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonUp()
	{
		if (Event_OnButtonUp)
			Event_OnButtonUp.Invoke(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonDown()
	{
		if (Event_OnButtonDown)
			Event_OnButtonDown.Invoke(this);
	}
	
	//----------------------------------------------------------------------------------------------
	// API
	//----------------------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------------------
	void SetWidgetEnabled(bool enabled)
	{
		if (m_bRequired)
			enabled = true;
		
		m_bWidgetEnabled = enabled;	
		UpdateAllWidgets();
	}
	
	//----------------------------------------------------------------------------------------------
	void SetRequired(bool required)
	{
		m_bRequired = required;
		HandleEnableButtons(m_bWidgetEnabled, m_bIsSorting);
	}
	
	//----------------------------------------------------------------------------------------------
	bool IsItemRequired()
	{
		return m_bRequired;
	}
	
	//----------------------------------------------------------------------------------------------
	void SetOnBottom(bool onBottom)
	{
		m_bOnBottom = onBottom;
	}
	
	//----------------------------------------------------------------------------------------------
	void NotifySorting(bool sorting)
	{
		m_bIsSorting = sorting;
		HandleSortingButtons();
		HandleEnableButtons(m_bWidgetEnabled, sorting);
	}
		
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetEventOnButtonUp()
	{
		if (!Event_OnButtonUp)
			Event_OnButtonUp = new ScriptInvokerScriptedWidgetComponent();

		return Event_OnButtonUp;
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetEventOnButtonDown()
	{
		if (!Event_OnButtonDown)
			Event_OnButtonDown = new ScriptInvokerScriptedWidgetComponent();

		return Event_OnButtonDown;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnSortConfirm()
	{
		if (!m_OnSortConfirm)
			m_OnSortConfirm = new ScriptInvokerScriptedWidgetComponent();

		return m_OnSortConfirm;
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerScriptedWidgetComponent GetOnRequiredDisabled()
	{
		if (!m_OnRequiredDisabled)
			m_OnRequiredDisabled = new ScriptInvokerScriptedWidgetComponent();

		return m_OnRequiredDisabled;
	}
}