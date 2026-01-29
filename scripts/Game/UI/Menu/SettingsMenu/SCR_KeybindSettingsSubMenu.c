class SCR_KeybindSetting : SCR_SettingsSubMenuBase
{
	protected ref SCR_KeyBindingMenuConfig m_KeybindConfig;

	// Resources
	protected static const string ACTION_ROW_LAYOUT_PATH =				"{75B1F7B766CA8C91}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindRow.layout";
	protected static const string SEPARATOR_LAYOUT_PATH =				"{01D9FD7791700ADA}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindSeparator.layout";
	protected static const string DESCRIPTION_ACTION_ROW_LAYOUT_PATH =	"{929E92CB72FBE8DD}UI/layouts/Menus/SettingsMenu/BindingMenu/KeybindsSettingsDescriptionActionRow.layout";
	protected static const string KEY_BINDING_CONFIG =					"{4EE7794C9A3F11EF}Configs/System/keyBindingMenu.conf";

	// Separator
	protected const float FIRST_SEPARATOR_PADDING = 20;
	
	// Widgets
	protected VerticalLayoutWidget m_wActionsLayout;
	protected ScrollLayoutWidget m_wActionsScrollLayout;

	// Bindings
	protected ref InputBinding m_Binding;
	protected static const string PRIMARY_PRESET_PREFIX = "";
	
	// Rows
	protected SCR_KeybindRowComponent m_SelectedRowComponent;
	protected int m_iLastSelectedRowIndex;
	protected float m_fLastSliderY;
	
	protected SCR_InputButtonComponent m_ResetSingleButton;
	protected SCR_InputButtonComponent m_UnbindSingleActionButton;
	protected SCR_InputButtonComponent m_SimpleBindingButton;
	protected SCR_InputButtonComponent m_AdvancedBindingButton;
	protected SCR_SettingsManagerKeybindModule m_SettingsKeybindModule;
	
	protected ref array<Widget> m_aRowWidgets = {};
	protected ref array<SCR_InputButtonComponent> m_aRowFooterButtons = {};

	// Description
	protected TextWidget m_wDescriptionHeader;
	protected RichTextWidget m_wDescription;
	protected RichTextWidget m_wDescriptionNL;
	protected Widget m_wDescriptionActionRowsContainer;
	protected ScrollLayoutWidget m_wDescriptionScroll;
	
	protected ref array<Widget> m_aDescriptionActionRows = {};
	
	// Footer
	protected SCR_InputButtonComponent m_ResetAllButton;
	protected SCR_InputButtonComponent m_CalibrateMCButton;
	
	protected SCR_SpinBoxComponent m_CategoriesSpinBox;

	// Actions	
	SCR_MenuActionsComponent m_ActionsComponent;
	
	protected const string DESCRIPTION = "#AR-Settings_Keybind_DetailsPanel_Description";
	protected const string DESCRIPTION_ADVANCED_BINDINGS = "#AR-Settings_Keybind_DetailsPanel_Description_AdvancedBindings";
	
	protected static const string ACTION_RESET_SINGLE = 	"MenuResetKeybind";
	protected static const string ACTION_RESET_ALL = 		"MenuResetAllKeybind";
	protected static const string ACTION_CALIBRATE_MC =		"MenuCalibrateMotionControl";
	protected static const string ACTION_UNBIND = 			"MenuUnbindKeybind";
	protected static const string ACTION_ADVANCED_KEYBIND = "MenuAdvancedKeybind";

	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);
		
		m_wActionsLayout = VerticalLayoutWidget.Cast(GetRootWidget().FindAnyWidget("ActionRowsContent"));
		if (!m_wActionsLayout)
			return;
		
		m_wActionsScrollLayout = ScrollLayoutWidget.Cast(GetRootWidget().FindAnyWidget("ActionRowScroll"));
		if (!m_wActionsScrollLayout)
			return;

		m_SettingsKeybindModule = SCR_SettingsManagerKeybindModule.Cast(GetGame().GetSettingsManager().GetModule(ESettingManagerModuleType.SETTINGS_MANAGER_KEYBINDING));
		if (!m_SettingsKeybindModule)
			return;
		
		m_Binding = m_SettingsKeybindModule.GetInputBindings();
		
		// Description
		m_wDescriptionHeader = TextWidget.Cast(GetRootWidget().FindAnyWidget("DescriptionHeader"));
		m_wDescription = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("Description"));
		m_wDescriptionNL = RichTextWidget.Cast(GetRootWidget().FindAnyWidget("DescriptionNL"));
		m_wDescriptionActionRowsContainer = GetRootWidget().FindAnyWidget("DescriptionActionRows");
		m_wDescriptionScroll = ScrollLayoutWidget.Cast(GetRootWidget().FindAnyWidget("DescriptionScroll"));
		
		if (m_wDescription && m_wDescriptionNL)
		{
			m_wDescription.SetVisible(false);
			m_wDescriptionNL.SetVisible(false);
			string action = string.Format(
				"<action name='%1', scale='%2', state = '%3'/>", 
				ACTION_ADVANCED_KEYBIND, 
				UIConstants.ACTION_DISPLAY_ICON_SCALE_BIG, 
				UIConstants.GetActionDisplayStateAttribute(SCR_EActionDisplayState.NON_INTERACTABLE_HINT)
			);
			
			m_wDescription.SetText(DESCRIPTION);
			m_wDescriptionNL.SetTextFormat(DESCRIPTION_ADVANCED_BINDINGS, action);
		}
		
		// Footer Buttons
		CreateResetAllKeybindsButton();
		CreateSimpleBindingButton();
		CreateUnbindSingleButton();
		CreateSingleKeybindResetButton();
		CreateAdvancedBindingButton();

		PlatformService ps = GetGame().GetPlatformService();
		if (ps && ps.GetLocalPlatformKind() == PlatformKind.PSN)
			CreateCalibrateGyroButton();
		
		// Actions
		m_ActionsComponent = SCR_MenuActionsComponent.FindComponent(GetRootWidget());
		if (m_ActionsComponent)
			m_ActionsComponent.GetOnAction().Insert(OnActionTriggered);

		// Read the categories and actions from KEY_BINDING_CONFIG
		Resource holder = BaseContainerTools.LoadContainer(KEY_BINDING_CONFIG);
		BaseContainer container = holder.GetResource().ToBaseContainer();
		m_KeybindConfig = SCR_KeyBindingMenuConfig.Cast(BaseContainerTools.CreateInstanceFromContainer(container));
		InsertCategoriesToComboBox();
		
		GetGame().OnInputDeviceUserChangedInvoker().Insert(OnInputDeviceChange);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuUpdate(float tDelta)
	{
		super.OnMenuUpdate(tDelta);
		
		//TODO: change this to use events insetad of tick
		UpdateButtons();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabShow()
	{
		super.OnTabShow();
		
		ListActionsFromCurrentCategory();
		
		m_SelectedRowComponent = null;
		
		if (m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
		
		UpdateDescription();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnTabHide()
	{
		super.OnTabHide();
		
		if (m_CategoriesSpinBox)
		{
			m_CategoriesSpinBox.SetKeepActionListeners(false);
			m_CategoriesSpinBox.RemoveActionListeners();
		}
		
		if (m_ActionsComponent)
			m_ActionsComponent.DeactivateActions();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusGained()
	{
		super.OnMenuFocusGained();
		
		if (m_CategoriesSpinBox)
		{
			m_CategoriesSpinBox.SetKeepActionListeners(true);
			m_CategoriesSpinBox.AddActionListeners();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMenuFocusLost()
	{
		super.OnMenuFocusLost();
		
		if (m_CategoriesSpinBox)
		{
			m_CategoriesSpinBox.SetKeepActionListeners(false);
			m_CategoriesSpinBox.RemoveActionListeners();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuShow()
	{
		super.OnMenuShow();
		
		if (m_bShown && m_ActionsComponent)
			m_ActionsComponent.ActivateActions();
	}
	
	// --- Protected ---
	//------------------------------------------------------------------------------------------------
	// Setup rows
	protected void ListActionsFromCurrentCategory()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("CategoriesBox");
		bool firstSeparator = true;
		if (!spinBox)
			return;

		m_CategoriesSpinBox = SCR_SpinBoxComponent.Cast(spinBox.GetHandler(0));
		if (!m_CategoriesSpinBox)
			return;

		// Allow category switching with gamepad from anywere in this menu. 
		// Otherwise the player will need to scroll all the way back to the spin box
		m_CategoriesSpinBox.SetKeepActionListeners(true);
		m_CategoriesSpinBox.AddActionListeners();
		
		SCR_KeyBindingCategory category = m_KeybindConfig.m_KeyBindingCategories.Get(m_CategoriesSpinBox.GetCurrentIndex());
		if (!category)
			return;

		// Delete rows
		foreach (Widget row : m_aRowWidgets)
		{
			row.RemoveFromHierarchy();
		}

		m_aRowWidgets.Clear();

		// Fill the menu
		Widget rowWidget;
		SCR_KeybindRowComponent component;
		SCR_LabelComponent separatorText;
		Widget keybindTitle;
		Widget gamepadTitle;
		EPlatform platform = System.GetPlatform();

		foreach (SCR_KeyBindingEntry entry : category.m_KeyBindingEntries)
		{
			// Platform specific actions
			if (!entry.m_aPlatforms.IsEmpty() && !entry.m_aPlatforms.Contains(platform))
				continue;

			// Action row
			if (entry.m_sActionName != "separator")
			{
				rowWidget = GetGame().GetWorkspace().CreateWidgets(ACTION_ROW_LAYOUT_PATH ,m_wActionsLayout);
				if (!rowWidget)
					continue;
				
				m_aRowWidgets.Insert(rowWidget);
				
				component = SCR_KeybindRowComponent.FindComponent(rowWidget);
				if (!component)
					continue;

				component.Init(m_Binding, entry);
				component.GetOnFocus().Insert(OnRowFocus);
				component.GetOnFocusLost().Insert(OnRowFocusLost);
				component.GetOnKeyCaptured().Insert(OnKeyCaptured);
				component.GetOnInlineButton().Insert(OnRowInlineButton);

				continue;
			}

			// Separator
			rowWidget = GetGame().GetWorkspace().CreateWidgets(SEPARATOR_LAYOUT_PATH ,m_wActionsLayout);
			if (!rowWidget)
				continue;
			
			m_aRowWidgets.Insert(rowWidget);
			
			separatorText = SCR_LabelComponent.GetComponent("SettingsTitle", rowWidget);
			if (!separatorText)
				continue;
			
			separatorText.SetVisible(!entry.m_sDisplayName.IsEmpty());
			separatorText.SetText(entry.m_sDisplayName);
			
			if (!firstSeparator)
			{
				separatorText.ResetTopPadding();
				continue;
			}
			
			// First separator
			keybindTitle = rowWidget.FindAnyWidget("Keybind");
			if (keybindTitle)
				keybindTitle.SetVisible(!GetGame().IsPlatformGameConsole() || GetGame().GetHasKeyboard());
			
			gamepadTitle = rowWidget.FindAnyWidget("Gamepad");
			if (gamepadTitle)
				gamepadTitle.SetVisible(true);
			
			separatorText.SetTopPadding(FIRST_SEPARATOR_PADDING);
			
			firstSeparator = false;
		}
		
		// Delayed position display update, to give enough time for all entries to be initialized
		if (m_iLastSelectedRowIndex > 0)
			GetGame().GetCallqueue().CallLater(UpdateListSelectionDisplay, 100, false, m_iLastSelectedRowIndex);
		else
			UpdateListSelectionDisplay(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateListSelectionDisplay(int index)
	{
		GetGame().GetCallqueue().Remove(UpdateListSelectionDisplay);
		
		if (!m_wActionsScrollLayout)
			return;
		
		float sliderTarget;
		if (index > 0)
			sliderTarget = m_fLastSliderY;
		
		Widget focusTarget;
		if (!m_aRowWidgets.IsEmpty() && m_aRowWidgets.IsIndexValid(index))
			focusTarget = m_aRowWidgets[index];

		m_wActionsScrollLayout.SetSliderPos(0, sliderTarget);
		
		if (GetGame().GetInputManager().GetLastUsedInputDevice() != EInputDeviceType.MOUSE || !focusTarget)
			UpdateFocusedWidget(focusTarget);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateFocusedWidget(Widget w)
	{
		if (!GetShown())
			return;
		
		if (w)
			GetGame().GetWorkspace().SetFocusedWidget(w);
		else if (m_CategoriesSpinBox)
			GetGame().GetWorkspace().SetFocusedWidget(m_CategoriesSpinBox.GetRootWidget());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InsertCategoriesToComboBox()
	{
		Widget spinBox = GetRootWidget().FindAnyWidget("CategoriesBox");
		if (!spinBox)
			return;

		SCR_SpinBoxComponent spinBoxComponent = SCR_SpinBoxComponent.Cast(spinBox.GetHandler(0));
		if (!spinBoxComponent)
			return;

		spinBoxComponent.ClearAll();

		int total = m_KeybindConfig.m_KeyBindingCategories.Count();
		foreach (int i, SCR_KeyBindingCategory category : m_KeybindConfig.m_KeyBindingCategories)
			spinBoxComponent.AddItem(category.m_sDisplayName, i == total - 1);

		spinBoxComponent.SetCurrentItem(0);
		
		spinBoxComponent.m_OnChanged.Insert(OnComboBoxChange);
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetKeybindsToDefault()
	{
		SCR_ConfigurableDialogUi dialog = SCR_KeybindDialogs.CreateKeybindsResetDialog();
		dialog.m_OnConfirm.Insert(ResetKeybindsToDefaultConfirm);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CalibrateMotionControl()
	{
		new SCR_MotionControlCalibrationPromptDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ResetSingleKeybindToDefault()
	{
		if (!m_SelectedRowComponent)
			return;
		
		m_SelectedRowComponent.ResetAction();
	}

	//------------------------------------------------------------------------------------------------
	protected void ResetKeybindsToDefaultConfirm()
	{
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		
		m_SettingsKeybindModule.ResetAllActions(device);
		ListActionsFromCurrentCategory();
	}

	// --- Footer buttons ---
	// -- Global --
	//------------------------------------------------------------------------------------------------
	protected void CreateResetAllKeybindsButton()
	{
		m_ResetAllButton = CreateNavigationButton(ACTION_RESET_ALL, "#AR-Settings_Keybind_ResetEveryKeybind", true);
		m_ResetAllButton.m_OnActivated.Insert(ResetKeybindsToDefault);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateCalibrateGyroButton()
	{
		m_CalibrateMCButton = CreateNavigationButton(ACTION_CALIBRATE_MC, "#AR-Settings_Calibrate_Motion_Control", true);
		m_CalibrateMCButton.m_OnActivated.Insert(CalibrateMotionControl);
	}
	
	// -- Row --
	//------------------------------------------------------------------------------------------------
	protected void CreateSingleKeybindResetButton()
	{		
		m_ResetSingleButton = CreateNavigationButton(ACTION_RESET_SINGLE, "#AR-Settings_Keybind_ResetAllKeybinds", true);
		m_ResetSingleButton.m_OnActivated.Insert(ResetSingleKeybindToDefault);

		m_aRowFooterButtons.Insert(m_ResetSingleButton);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateUnbindSingleButton()
	{
		m_UnbindSingleActionButton = CreateNavigationButton(ACTION_UNBIND, "#AR-Settings_Keybind_UnbindOne", true);
		m_UnbindSingleActionButton.m_OnActivated.Insert(UnbindSingleAction);
		m_aRowFooterButtons.Insert(m_UnbindSingleActionButton);
	}
	
	//------------------------------------------------------------------------------------------------
	// Visual only Enter button. Interaction is handled by the row buttons themselves
	protected void CreateSimpleBindingButton()
	{		
		m_SimpleBindingButton = CreateNavigationButton(UIConstants.MENU_ACTION_SELECT, "#AR-Settings_Keybind_Assign", true);
		m_aRowFooterButtons.Insert(m_SimpleBindingButton);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CreateAdvancedBindingButton()
	{		
		m_AdvancedBindingButton = CreateNavigationButton(ACTION_ADVANCED_KEYBIND, "#AR_Settings_KeybindAdvanced_Title", true);
		m_AdvancedBindingButton.m_OnActivated.Insert(AdvancedKeybindButtonClick);
		m_aRowFooterButtons.Insert(m_AdvancedBindingButton);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnbindSingleAction()
	{		
		if (!m_SelectedRowComponent)
			return;
		
		m_SelectedRowComponent.Unbind();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AdvancedKeybindButtonClick()
	{
		if (!m_SelectedRowComponent)
			return;
		
		SCR_AdvancedKeybindDialogUI keybindDialog = SCR_KeybindDialogs.CreateAdvancedKeybindDialog(
			m_SelectedRowComponent.GetEntry(),
			m_SelectedRowComponent.GetDisplayName(),
			m_SelectedRowComponent.GetActionName(), 
			PRIMARY_PRESET_PREFIX + m_SelectedRowComponent.GetActionPreset(),
			m_SelectedRowComponent.GetActionPrefixType()
		);

		if (keybindDialog)
			keybindDialog.m_OnCancel.Insert(ListActionsFromCurrentCategory);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		bool consoleController = GetGame().IsPlatformGameConsole() && !GetGame().GetHasKeyboard();
		bool usingMouse = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.MOUSE;
		bool usingKeyboard = GetGame().GetInputManager().GetLastUsedInputDevice() == EInputDeviceType.KEYBOARD;
		bool show = !usingMouse && m_SelectedRowComponent;
		
		if (m_ResetSingleButton)
			m_ResetSingleButton.SetVisible(m_bShown && show, false);
		
		if (m_UnbindSingleActionButton)
			m_UnbindSingleActionButton.SetVisible(m_bShown && show, false);
		
		if (m_SimpleBindingButton)
			m_SimpleBindingButton.SetVisible(m_bShown && show && !m_SelectedRowComponent.GetActionPreset().IsEmpty(), false);
		
		if (m_AdvancedBindingButton)
			m_AdvancedBindingButton.SetVisible(m_bShown && show && !consoleController && usingKeyboard, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateDescription()
	{
		if (m_wDescriptionScroll)
			m_wDescriptionScroll.SetSliderPos(0, 0);
		
		// --- Update Visibility ---
		string actionName;
		string actionPreset;
			
		if (m_SelectedRowComponent)
		{
			actionName = m_SelectedRowComponent.GetActionName();
			actionPreset = m_SelectedRowComponent.GetActionPreset();
		}
		
		EInputDeviceType device = GetGame().GetInputManager().GetLastUsedInputDevice();
		if (device != EInputDeviceType.GAMEPAD)
			device = EInputDeviceType.KEYBOARD;
		
		int bindCount = m_SettingsKeybindModule.GetActionBindCount(actionName, actionPreset, device);

		if (m_wDescriptionHeader)
			m_wDescriptionHeader.SetVisible(bindCount > 1);
		
		if (m_wDescription && m_wDescriptionNL)
		{
			m_wDescription.SetVisible(bindCount > 1);
			m_wDescriptionNL.SetVisible(bindCount > 1);
		}
		
		foreach (int i, Widget widget : m_aDescriptionActionRows)
		{
			m_aDescriptionActionRows[i].SetVisible(bindCount > 1 && i <= bindCount - 1 && m_SelectedRowComponent != null);
		}
		
		// --- Update contents ---
		if (!m_SelectedRowComponent || bindCount <= 1)
			return;
		
		if (m_wDescriptionHeader)
			m_wDescriptionHeader.SetText(m_SelectedRowComponent.GetDisplayName());

		// Create or update necessary rows (we don't want to keep destroying and recreating them, so we add new ones only if necessary)
		if (!m_wDescriptionActionRowsContainer)
			return;
		
		for (int i = 0; i < bindCount; i++)
		{
			Widget row;
			
			if (!m_aDescriptionActionRows.IsIndexValid(i))
			{
				row = GetGame().GetWorkspace().CreateWidgets(DESCRIPTION_ACTION_ROW_LAYOUT_PATH, m_wDescriptionActionRowsContainer);
				if (row)
					m_aDescriptionActionRows.Insert(row);
			}
			else
				row = m_aDescriptionActionRows[i];
			
			if (!row)
				continue;
			
			SCR_AdvancedActionRowComponent comp = SCR_AdvancedActionRowComponent.FindComponentInHierarchy(row);
			if (comp)
				comp.Init(actionName, actionPreset, i, m_SettingsKeybindModule, device);
		} 
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRowFocus(SCR_ScriptedWidgetComponent component)
	{
		SCR_KeybindRowComponent row = SCR_KeybindRowComponent.Cast(component);
		if (!row)
			return;
		
		m_SelectedRowComponent = row;
		
		Widget root = row.GetRootWidget();
		if (root && !m_aRowWidgets.IsEmpty())
			m_iLastSelectedRowIndex = m_aRowWidgets.Find(root);
		
		if (m_wActionsScrollLayout)
		{
			float sliderX;
			m_wActionsScrollLayout.GetSliderPos(sliderX, m_fLastSliderY);
		}
		
		UpdateDescription();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRowFocusLost(SCR_ScriptedWidgetComponent component)
	{
		m_SelectedRowComponent = null;
		
		UpdateDescription();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnRowInlineButton(string name)
	{
		switch (name)
		{
			case SCR_KeybindRowComponent.BUTTON_ADVANCED_KEYBIND: 	AdvancedKeybindButtonClick(); break;
			case SCR_KeybindRowComponent.BUTTON_RESET: 				ResetSingleKeybindToDefault(); break;
			case SCR_KeybindRowComponent.BUTTON_UNBIND: 			UnbindSingleAction(); break;					
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnActionTriggered(string name, float multiplier)
	{
		switch (name)
		{
			case ACTION_ADVANCED_KEYBIND:	AdvancedKeybindButtonClick(); break;
			case ACTION_RESET_SINGLE: 		ResetSingleKeybindToDefault(); break;
			case ACTION_UNBIND: 			UnbindSingleAction(); break;					
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnKeyCaptured()
	{
		ListActionsFromCurrentCategory();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnComboBoxChange()
	{
		// Bring focus and slider back to top
		m_iLastSelectedRowIndex = 0;
		ListActionsFromCurrentCategory();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceChange(EInputDeviceType oldDevice, EInputDeviceType newDevice)
	{
		UpdateDescription();
	}
}