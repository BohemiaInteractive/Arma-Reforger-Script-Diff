class SCR_AddonLinePresetComponent : SCR_WLibComponentBase
{
	[Attribute(UIColors.GetColorAttribute(UIColors.CONTRAST_COLOR))]
	protected ref Color m_cSelectedColor;

	[Attribute(UIColors.GetColorAttribute(UIColors.NEUTRAL_INFORMATION))]
	protected ref Color m_cDefaultColor;

	protected const string STR_DEFAULT_NAME = "Preset ";

	protected ref SCR_AddonLinePresetWidgets m_Widgets = new SCR_AddonLinePresetWidgets();
	protected SCR_WorkshopAddonPreset m_Preset;

//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
// Old untyped invokers. Lots of duplicate code that should be in a common parent class of UI components, both the events and the common interaction states caching
	
	protected bool m_bSelected;
	protected bool m_bIsEdit;
	protected bool m_bIsFocused;

	// Buttons
	protected SCR_ModularButtonComponent m_ButtonComponent;

	// Current input device
	protected EInputDeviceType m_eInputDeviceType;

	//------------------------------------------------------------------------------------------------
	// Invokers
	//------------------------------------------------------------------------------------------------
	
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent, string> Event_OnNameChanged;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnNameEditStart;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnNameEditLeave;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnFocus;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnFocusLost;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnLoad;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnOverride;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnDelete;
	protected ref ScriptInvoker<SCR_AddonLinePresetComponent> Event_OnButtonClick;
	
//---- REFACTOR NOTE END ----

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnButtonClick()
	{
		if (Event_OnButtonClick)
			Event_OnButtonClick.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnButtonClick()
	{
		if (!Event_OnButtonClick)
			Event_OnButtonClick = new ScriptInvoker();

		return Event_OnButtonClick;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnNameChanged(string arg0)
	{
		if (Event_OnNameChanged)
			Event_OnNameChanged.Invoke(this, arg0);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnNameChanged()
	{
		if (!Event_OnNameChanged)
			Event_OnNameChanged = new ScriptInvoker();

		return Event_OnNameChanged;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnNameEditStart()
	{
		if (Event_OnNameEditStart)
			Event_OnNameEditStart.Invoke(this);

		InvokeEventOnButtonClick();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnNameEditStart()
	{
		if (!Event_OnNameEditStart)
			Event_OnNameEditStart = new ScriptInvoker();

		return Event_OnNameEditStart;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnNameEditLeave()
	{
		if (Event_OnNameEditLeave)
			Event_OnNameEditLeave.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnNameEditLeave()
	{
		if (!Event_OnNameEditLeave)
			Event_OnNameEditLeave = new ScriptInvoker();

		return Event_OnNameEditLeave;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnFocus()
	{
		if (Event_OnFocus)
			Event_OnFocus.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnFocus()
	{
		if (!Event_OnFocus)
			Event_OnFocus = new ScriptInvoker();

		return Event_OnFocus;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnFocusLost()
	{
		if (Event_OnFocusLost)
			Event_OnFocusLost.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnFocusLost()
	{
		if (!Event_OnFocusLost)
			Event_OnFocusLost = new ScriptInvoker();

		return Event_OnFocusLost;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnLoad()
	{
		if (Event_OnLoad)
			Event_OnLoad.Invoke(this);

		InvokeEventOnButtonClick();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnLoad()
	{
		if (!Event_OnLoad)
			Event_OnLoad = new ScriptInvoker();

		return Event_OnLoad;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnOverride()
	{
		if (Event_OnOverride)
			Event_OnOverride.Invoke(this);

		InvokeEventOnButtonClick();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnOverride()
	{
		if (!Event_OnOverride)
			Event_OnOverride = new ScriptInvoker();

		return Event_OnOverride;
	}

	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnDelete()
	{
		if (Event_OnDelete)
			Event_OnDelete.Invoke(this);

		InvokeEventOnButtonClick();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetEventOnDelete()
	{
		if (!Event_OnDelete)
			Event_OnDelete = new ScriptInvoker();

		return Event_OnDelete;
	}

	//------------------------------------------------------------------------------------------------
	// Override widget functions
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_Widgets.Init(w);

		m_Widgets.m_EditNameButtonComponent.m_OnClicked.Insert(StartEditName);
		m_Widgets.m_LoadButtonComponent.m_OnClicked.Insert(InvokeEventOnLoad);
		m_Widgets.m_OverrideButtonComponent.m_OnClicked.Insert(InvokeEventOnOverride);
		m_Widgets.m_DeleteButtonComponent.m_OnClicked.Insert(InvokeEventOnDelete);
		m_Widgets.m_SelectedHighlightButtonComponent.m_OnClicked.Insert(ToggleSelected);
		

		m_ButtonComponent = SCR_ModularButtonComponent.FindComponent(w);

		ShowEditWidget(false);
		ShowButtons(false);

		//--- This will trigger everytime the input device change, so it controls the visibility of the navigation buttons, and on start
		OnInputDeviceChange(-1, GetGame().GetInputManager().GetLastUsedInputDevice());
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChange);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
	}

	//------------------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		super.OnDoubleClick(w, x, y, button);

		if (button == UIConstants.MOUSE_LEFT_CLICK)
			InvokeEventOnLoad();

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		m_bIsFocused = true;
		super.OnFocus(w, x, y);
		InvokeEventOnFocus();
		ShowButtons(m_eInputDeviceType == EInputDeviceType.MOUSE);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		m_bIsFocused = false;
		super.OnFocusLost(w, x, y);
		InvokeEventOnFocusLost();
		ShowButtons(m_bIsFocused);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	// Functions
	//------------------------------------------------------------------------------------------------
	
	
	//------------------------------------------------------------------------------------------------
	//! Toggles the current selection
	protected void ToggleSelected()
	{
		InvokeEventOnLoad();
	}

	//------------------------------------------------------------------------------------------------
	//! Show editbox instead of name to edit name string
	void StartEditName()
	{
		m_bIsEdit = true;
		if (!m_Widgets.m_EditName)
		{
			#ifdef DEBUG_WORKSHOP
			Print("Missing m_NameEdit!");
			#endif
			return;
		}

		// This will prevent the edit box from immediately closing due to the button refocusing
		if (m_ButtonComponent)
			m_ButtonComponent.SetIsFocusOnMouseEnter(false);

		//--- Handle visibilities and focus state
		ShowEditWidget(true);
		ShowButtons(false);
		GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_EditNameComponent.GetEditBoxWidget());
		m_Widgets.m_EditNameComponent.SetValue(m_Widgets.m_NameText.GetText());
		m_Widgets.m_EditNameComponent.ActivateWriteMode();

		//--- Handle event listeners
		m_Widgets.m_EditNameComponent.m_OnConfirm.Insert(OnEditNameConfirm); //TODO: this does not get called on gamepad
		m_Widgets.m_EditNameComponent.m_OnFocusChangedEditBox.Insert(OnEditNameCancel);
		m_Widgets.m_EditNameComponent.m_OnCancel.Insert(OnEditNameCancel);

		InvokeEventOnNameEditStart();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowEditWidget(bool edit)
	{
		if (m_Widgets.m_NameText)
			m_Widgets.m_NameText.SetVisible(!edit);

		if (m_Widgets.m_EditName)
			m_Widgets.m_EditName.SetVisible(edit);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveEditActions()
	{
		m_Widgets.m_EditNameComponent.m_OnConfirm.Remove(OnEditNameConfirm);
		m_Widgets.m_EditNameComponent.m_OnFocusChangedEditBox.Remove(OnEditNameCancel);
		m_Widgets.m_EditNameComponent.ClearInteractionState();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEditNameConfirm()
	{
		// Reactivate focus on mouse enter
		if (m_ButtonComponent)
			m_ButtonComponent.SetIsFocusOnMouseEnter(true);

		InvokeEventOnNameChanged(m_Widgets.m_EditNameComponent.GetValue());
		RemoveEditActions();

		if (m_bSelected)
			InvokeEventOnLoad();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditNameCancel()
	{
		m_bIsEdit = false;

		// If the focus change was a result of clicking the same line we're editing, then consider it a confirmation
		// This is also necessary to register confirmation with gamepad
		if (GetGame().GetWorkspace().GetFocusedWidget() == GetRootWidget())
		{
			OnEditNameConfirm();
			return;
		}

		// Reactivate focus on mouse enter
		if (m_ButtonComponent)
			m_ButtonComponent.SetIsFocusOnMouseEnter(true);

		ShowEditWidget(false);
		//ShowButtons(true);
		RemoveEditActions();

		InvokeEventOnNameEditLeave();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowButtons(bool show)
	{
		if (m_Widgets.m_OverrideButton)
			m_Widgets.m_OverrideButton.SetVisible(show && !m_bIsEdit);

		if (m_Widgets.m_EditNameButton)
			m_Widgets.m_EditNameButton.SetVisible(show && !m_bIsEdit);

		if (m_Widgets.m_DeleteButton)
			m_Widgets.m_DeleteButton.SetVisible(show && !m_bIsEdit);
	}

	//------------------------------------------------------------------------------------------------
	//! Type default prefab name based on count of current addons
	protected string DefaultName()
	{
		int count = SCR_AddonManager.GetInstance().GetPresetStorage().GetPresets().Count();
		string name = STR_DEFAULT_NAME + count;

		// Is name unique
		int fallback = 100;

		while (name || fallback > 0)
		{
			if (!SCR_AddonManager.GetInstance().GetPresetStorage().PresetExists(name))
			{
				return name;
			}
			else
			{
				count++;
				name = STR_DEFAULT_NAME + count;

				fallback++;
			}
		}

		return "";
	}

	//------------------------------------------------------------------------------------------------
	// Api
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopAddonPreset GetPreset()
	{
		return m_Preset;
	}

	//------------------------------------------------------------------------------------------------
	void SetPreset(SCR_WorkshopAddonPreset preset)
	{
		m_Preset = preset;
	}

	//------------------------------------------------------------------------------------------------
	void UpdateWidgets()
	{
		ShowModCount(m_Preset.GetAddonCount());
	}

	//------------------------------------------------------------------------------------------------
	void ShowWarning(bool show)
	{
		m_Widgets.m_SizeWarning.SetVisible(show);
	}

	//------------------------------------------------------------------------------------------------
	void ShowModCount(int count)
	{
		m_Widgets.m_TxtAddonsCount.SetText(count.ToString());
	}

	//------------------------------------------------------------------------------------------------
	void ShowDefaultName()
	{
		m_Widgets.m_NameText.SetText(DefaultName());
	}

	//------------------------------------------------------------------------------------------------
	string GetName()
	{
		return m_Widgets.m_NameText.GetText();
	}

	//------------------------------------------------------------------------------------------------
	void SetSelected(bool selected)
	{
		if (m_bSelected == selected)
			return;

		if (m_ButtonComponent)
			m_ButtonComponent.SetToggled(selected);

		if (m_Widgets.m_SelectedHighlightButtonComponent)
			m_Widgets.m_SelectedHighlightButtonComponent.SetToggled(selected);



		Color color = m_cDefaultColor;
		if (selected)
			color = m_cSelectedColor;

		m_Widgets.m_NameText.SetColor(color);
		m_Widgets.m_wImgAddons.SetColor(color);
		m_Widgets.m_TxtAddonsCount.SetColor(color);
		m_bSelected = selected;
	}

	//------------------------------------------------------------------------------------------------
	bool GetSelected()
	{
		return m_bSelected;
	}
	//------------------------------------------------------------------------------------------------
	// Sets the used input device type and toggles the buttons accordingly
	protected void OnInputDeviceChange(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		ShowButtons(newDevice == EInputDeviceType.MOUSE && m_bIsFocused);
		m_eInputDeviceType = newDevice;
	}
}
