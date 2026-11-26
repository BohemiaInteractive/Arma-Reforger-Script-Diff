//------------------------------------------------------------------------------------------------
class SCR_AdvancedKeybindDialogUI: SCR_KeybindDialogBase
{
	protected ref array<ref SCR_InputFilterConfiguration> m_aInputFilterConfigurations;
	protected ref array<ref CanvasWidgetCommand> m_aDrawCommands = {};

	protected SCR_InputButtonComponent m_Select;
	
	protected string m_sActionPreset;
	protected SCR_EActionPrefixType m_eActionPrefixType;
	protected int m_iSelectedKeybind;
	
	protected SCR_ModularButtonComponent m_FocusedBindRowButton;
	protected ref array<SCR_ModularButtonComponent> m_aBindRowButtons = {};
	
	protected Widget m_wPrefixesCombo;
	
	protected const ResourceName KEYBIND_ROW_LAYOUT_PATH = "{25888355688EE402}UI/layouts/Menus/SettingsMenu/BindingMenu/AdvancedKeybindActionRow.layout";
	protected const string DEVICE_ID_DISPLAY_FORMAT = "%1: %2";
	
	//----------------------------------------------------------------------------------------
	override protected void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		super.Init(root, preset, proxyMenu);

		SCR_AdvancedKeybindingPresetDialog advancedPreset = SCR_AdvancedKeybindingPresetDialog.Cast(preset);
		if (advancedPreset)
			m_aInputFilterConfigurations = advancedPreset.GetInputFilterConfigurations();
	}

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
		
		SetupDeviceIdSelector();
		SetupCaptureButton();
		SetupPrefixesCombo();
		SetupBindsCombo();
		SetupCombinationCombo();
		
		ShowBindsForAction();
		ReselectRow();
	}

	//------------------------------------------------------------------------------------------------
	protected void SetupInputFilterFields(string inputFilterType = string.Empty)
	{
		SCR_InputFilterConfiguration validFilter;
		foreach (SCR_InputFilterConfiguration filterConfig : m_aInputFilterConfigurations)
		{
			if (filterConfig.m_sInputFilterClass != inputFilterType)
				continue;

			validFilter = filterConfig;
			break;
		}

		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, m_SettingsKeybindModule.GetInputBindings());
		BaseContainer filter;
		if (inputSource)
			filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);

		ScrollLayoutWidget scrollWidget = ScrollLayoutWidget.Cast(m_wRoot.FindAnyWidget("ModifierScrollLayout"));
		if (scrollWidget)
			scrollWidget.SetSliderPos(0, 0);

		if (m_wPrefixesCombo)
		{
			array<ref SCR_KeyBindingFilter> filtersConfigs = m_SettingsKeybindModule.GetFilters(m_eActionPrefixType);
			int id;
			foreach (int i, SCR_KeyBindingFilter bindingFilterConfig : filtersConfigs)
			{
				if (bindingFilterConfig.m_sFilterString != inputFilterType)
					continue;

				id = i;
				break;
			}

			SCR_ComboBoxComponent comp = SCR_ComboBoxComponent.Cast(m_wPrefixesCombo.FindHandler(SCR_ComboBoxComponent));
			if (comp)
				comp.SetCurrentItem(id, false, false, false);
		}

		ButtonWidget invertButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_Invert"));
		if (invertButtonWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanInvert;
			invertButtonWidget.SetEnabled(canUse);
			invertButtonWidget.SetVisible(canUse);
			SCR_SpinBoxComponent comp = SCR_SpinBoxComponent.Cast(invertButtonWidget.FindHandler(SCR_SpinBoxComponent));
			if (comp)
			{
				comp.m_OnChanged.Remove(OnInvertElementChanged);
				if (canUse)
				{
					int inverted;
					if (!filter.Get(SCR_InputHelper.INPUT_CONTAINER_INVERT_NAME, inverted))
					{
						float multiplier;
						filter.Get(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, multiplier);
						if (multiplier < 0)
							inverted = true;
					}

					comp.SetCurrentItem(inverted, false, false, false);
					comp.m_OnChanged.Insert(OnInvertElementChanged);
				}
			}
		}

		ButtonWidget multiplierButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_Multiplier"));
		if (multiplierButtonWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanAdjustMultiplier;
			multiplierButtonWidget.SetEnabled(canUse);
			multiplierButtonWidget.SetVisible(canUse);
			SCR_SliderComponent comp = SCR_SliderComponent.Cast(multiplierButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnMultiplierSliderChangedFinal);
				if (canUse)
				{
					float value;
					filter.Get(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, value);
					comp.SetValue(Math.AbsFloat(value), false);
					comp.GetOnChangedFinal().Insert(OnMultiplierSliderChangedFinal);
				}
			}
		}

		ButtonWidget offsetButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_Offset"));
		if (offsetButtonWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanAdjustOffset;
			offsetButtonWidget.SetEnabled(canUse);
			offsetButtonWidget.SetVisible(canUse);
			SCR_SliderComponent comp = SCR_SliderComponent.Cast(offsetButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnOffsetSliderChangedFinal);
				if (canUse)
				{
					float value;
					filter.Get(SCR_InputHelper.INPUT_CONTAINER_OFFSET_NAME, value);
					comp.SetValue(value, false);
					comp.GetOnChangedFinal().Insert(OnOffsetSliderChangedFinal);
				}
			}
		}

		Widget deadzoneLayoutWidget = m_wRoot.FindAnyWidget("InputFilter_DeadZones");
		if (deadzoneLayoutWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanAdjustDeadzones;
			deadzoneLayoutWidget.SetEnabled(canUse);
			deadzoneLayoutWidget.SetVisible(canUse);

			ButtonWidget centerDeadzoneButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_CenterDeadzone"));
			SCR_SliderComponent comp = SCR_SliderComponent.Cast(centerDeadzoneButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnCenterDeadZoneSliderChangedFinal);
				if (canUse)
				{
					float value;
					filter.Get(SCR_InputHelper.INPUT_CONTAINER_DEADZONE_NAME, value);
					comp.SetValue(value, false);
					comp.GetOnChangedFinal().Insert(OnCenterDeadZoneSliderChangedFinal);
				}
			}

			ButtonWidget edgeDeadzoneButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_EdgeDeadzone"));
			comp = SCR_SliderComponent.Cast(edgeDeadzoneButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnEdgeDeadzoneSliderChangedFinal);
				if (canUse)
				{
					float value;
					TVectorArray curve = SCR_InputHelper.GetInputCurve(filter);
					if (curve && !curve.IsEmpty())
						value = 1 - curve[curve.Count() - 1][0];

					comp.SetValue(value, false);
					comp.GetOnChangedFinal().Insert(OnEdgeDeadzoneSliderChangedFinal);
				}
			}

			ButtonWidget curveShapeButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_CurveShape"));
			comp = SCR_SliderComponent.Cast(curveShapeButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnCurveShapeSliderChangedFinal);
				if (canUse)
				{
					float value;
					TVectorArray curve = SCR_InputHelper.GetInputCurve(filter);
					if (curve && curve.Count() == SCR_InputHelper.INPUT_CURVE_KNOTS_COUNT)
						value = SCR_InputHelper.GetInputCurveFactor(5, curve[5][1] * 10);

					comp.SetValue(Math.AbsFloat(value), false);
					comp.GetOnChangedFinal().Insert(OnCurveShapeSliderChangedFinal);
				}
			}

			if (canUse)
				DrawInputCurve(inputSource);
		}

		ButtonWidget thresholdButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_Threshold"));
		if (thresholdButtonWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanAdjustThreshold;
			thresholdButtonWidget.SetEnabled(canUse);
			thresholdButtonWidget.SetVisible(canUse);
			SCR_SliderComponent comp = SCR_SliderComponent.Cast(thresholdButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnThresholdSliderChangedFinal);
				if (canUse)
				{
					float value;
					filter.Get(SCR_InputHelper.INPUT_CONTAINER_THRESHOLD_NAME, value);
					comp.SetValue(value, false);
					comp.GetOnChangedFinal().Insert(OnThresholdSliderChangedFinal);
				}
			}
		}

		ButtonWidget holdDurationButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_HoldDuration"));
		if (holdDurationButtonWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanAdjustHoldDuration;
			holdDurationButtonWidget.SetEnabled(canUse);
			holdDurationButtonWidget.SetVisible(canUse);
			SCR_SliderComponent comp = SCR_SliderComponent.Cast(holdDurationButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnHoldDurationSliderChangedFinal);
				if (canUse)
				{
					float value;
					filter.Get(SCR_InputHelper.INPUT_CONTAINER_HOLD_DURATION_NAME, value);
					comp.SetValue(value, false);
					comp.GetOnChangedFinal().Insert(OnHoldDurationSliderChangedFinal);
				}
			}
		}

		ButtonWidget clickDurationButtonWidget = ButtonWidget.Cast(m_wRoot.FindAnyWidget("InputFilter_ClickDuration"));
		if (clickDurationButtonWidget)
		{
			bool canUse = filter && validFilter && validFilter.m_bCanAdjustClickDuration;
			clickDurationButtonWidget.SetEnabled(canUse);
			clickDurationButtonWidget.SetVisible(canUse);
			SCR_SliderComponent comp = SCR_SliderComponent.Cast(clickDurationButtonWidget.FindHandler(SCR_SliderComponent));
			if (comp)
			{
				comp.GetOnChangedFinal().Remove(OnClickDurationSliderChangedFinal);
				if (canUse)
				{
					float value;
					filter.Get(SCR_InputHelper.INPUT_CONTAINER_CLICK_DURATION_NAME, value);
					comp.SetValue(value, false);
					comp.GetOnChangedFinal().Insert(OnClickDurationSliderChangedFinal);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInvertElementChanged(SCR_ChangeableComponentBase comp, int state)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		bool currentVal
		if (!filter.Get(SCR_InputHelper.INPUT_CONTAINER_INVERT_NAME, currentVal))
		{
			float multiplier;
			filter.Get(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, multiplier);
			if (multiplier < 0)
				currentVal = true;
		}

		if (currentVal == state)
			return;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_INVERT_NAME, state);
		float multiplier;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, multiplier);
		multiplier = Math.AbsFloat(multiplier);
		if (state)
			multiplier *= -1;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, multiplier);

		float offset;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_OFFSET_NAME, offset);
		offset = Math.AbsFloat(offset);
		if (state)
			offset *= -1;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_OFFSET_NAME, offset);
		inputBinds.Save();
		SetupInputFilterFields(filter.GetClassName());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMultiplierSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		float currentVal;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, currentVal);

		bool inverted;
		if (!filter.Get(SCR_InputHelper.INPUT_CONTAINER_INVERT_NAME, inverted) && currentVal < 0)
			inverted = true;

		if (inverted)
			value *= -1;

		if (currentVal == value)
			return;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, value);
		inputBinds.Save();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOffsetSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		float currentVal;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_OFFSET_NAME, currentVal);
		if (currentVal == value)
			return;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_OFFSET_NAME, value);
		inputBinds.Save();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnCenterDeadZoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		// dont allow for setting dead zone to 100%, as then the input wont work at all as it
		// will be always 0 - stop hurting yourself type of deal
		value = Math.Min(0.999, value);
		float currentVal;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_DEADZONE_NAME, currentVal);
		if (currentVal == value)
			return;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_DEADZONE_NAME, value);
		inputBinds.Save();
		DrawInputCurve(inputSource);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEdgeDeadzoneSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		if (!SCR_InputHelper.SetCurveDeadZone(filter, value))
			return;

		inputBinds.Save();
		DrawInputCurve(inputSource);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCurveShapeSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		BaseContainer curveContainer;
		TVectorArray curve = SCR_InputHelper.GetInputCurve(filter, curveContainer);
		float oldEdgeDeadZoneValue;
		if (curve && !curve.IsEmpty())
		{
			oldEdgeDeadZoneValue = 1 - curve[curve.Count() - 1][0];
		}
		else
		{
			if (!filter.SetObject(SCR_InputHelper.INPUT_CONTAINER_TRANSITION_CURVE_NAME, BaseContainerTools.CreateContainer(SCR_InputHelper.INPUT_CURVE_CONTAINER_TYPE).GetResource().ToBaseContainer()))
				return;

			curveContainer = filter.GetObject(SCR_InputHelper.INPUT_CONTAINER_TRANSITION_CURVE_NAME);
			if (!curveContainer)
				return;

			curveContainer.Set(SCR_InputHelper.INPUT_CONTAINER_FILTER_SPLINE_TYPE_NAME, typename.EnumToString(ECurveType, ECurveType.Linear));
		}

		curve = SCR_InputHelper.GenerateInputCurve(value);

		array<float> knots = {};
		array<float> values = {};
		foreach (vector point : curve)
		{
			knots.Insert(point[0]);
			values.Insert(point[1]);
		}

		curveContainer.Set(SCR_InputHelper.INPUT_CONTAINER_FILTER_CURVE_KNOTS_NAME, knots);
		curveContainer.Set(SCR_InputHelper.INPUT_CONTAINER_FILTER_CURVE_VALUES_NAME, values);

		if (oldEdgeDeadZoneValue > 0 && !SCR_InputHelper.SetCurveDeadZone(filter, oldEdgeDeadZoneValue))
			return;

		inputBinds.Save();
		DrawInputCurve(inputSource);
	}

	//------------------------------------------------------------------------------------------------
	protected void DelayedDrawCurveCall()
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		DrawInputCurve(inputSource);
	}

	//------------------------------------------------------------------------------------------------
	protected void DrawInputCurve(notnull BaseContainer inputSource)
	{
		CanvasWidget canvas = CanvasWidget.Cast(m_wRoot.FindAnyWidget("CurveCanvas"));
		if (!canvas)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		float maxWidth;
		float maxHeight;
		canvas.GetScreenSize(maxWidth, maxHeight);
		if (maxWidth == 0 && maxHeight == 0)
		{
			GetGame().GetCallqueue().CallLater(DelayedDrawCurveCall); // postpone drawing until the widget is fully initialized and has its proper height/width 
			return;
		}

		string inputBinding;
		inputSource.Get(SCR_InputHelper.INPUT_CONTAINER_INPUT_NAME, inputBinding);

		float mult, offset;
		int inverse;
		GetDefaultAxisFilterValues(inputBinding, mult, offset, inverse);
		bool isHalfAxis = mult != 0.5;

		SCR_InputHelper.DrawInputCurve(filter, canvas, m_aDrawCommands, isHalfAxis);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnThresholdSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		float currentVal;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_THRESHOLD_NAME, currentVal);
		if (currentVal == value)
			return;

		filter.Set(SCR_InputHelper.INPUT_CONTAINER_THRESHOLD_NAME, value);
		inputBinds.Save();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnHoldDurationSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		float currentVal;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_HOLD_DURATION_NAME, currentVal);
		if (currentVal == value)
			return;

		if (value != 0)
			filter.Set(SCR_InputHelper.INPUT_CONTAINER_HOLD_DURATION_NAME, value);
		else
			filter.ClearVariable(SCR_InputHelper.INPUT_CONTAINER_HOLD_DURATION_NAME);

		inputBinds.Save();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnClickDurationSliderChangedFinal(SCR_SliderComponent comp, float value)
	{
		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		float currentVal;
		filter.Get(SCR_InputHelper.INPUT_CONTAINER_CLICK_DURATION_NAME, currentVal);
		if (currentVal == value)
			return;

		if (value != 0)
			filter.Set(SCR_InputHelper.INPUT_CONTAINER_CLICK_DURATION_NAME, value);
		else
			filter.ClearVariable(SCR_InputHelper.INPUT_CONTAINER_CLICK_DURATION_NAME);

		inputBinds.Save();
	}

	//------------------------------------------------------------------------------------------------
	// Reselect correct keybind row
	protected void ReselectRow()
	{
		if (!m_aBindRowButtons.IsIndexValid(m_iSelectedKeybind))
		{
			SetupInputFilterFields();
			return;
		}
		
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
		const InputBinding inputBindings = m_SettingsKeybindModule.GetInputBindings();
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
			
			rowComponent.Init(m_sActionName, m_sActionPreset, i, m_SettingsKeybindModule, m_eDevice, inputBindings);
			
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
	protected void OnKeyCaptured()
	{
		ShowBindsForAction();
		
		// Select the last row
		m_iSelectedKeybind = m_aBindRowButtons.Count() - 1;
		ReselectRow();

		array<string> bindings = {};
		m_SettingsKeybindModule.GetInputBindings().GetBindings(m_sActionName, bindings, m_eDevice, m_sActionPreset, false);
		if (!bindings.IsEmpty() && bindings.IsIndexValid(m_iSelectedKeybind) && bindings[m_iSelectedKeybind].Contains("axis"))
			SetDefaultFilterForAxis();
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

		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, m_SettingsKeybindModule.GetInputBindings());
		BaseContainer filter;
		if (inputSource)
			filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);

		string filterClassName;
		if (filter)
			filterClassName = filter.GetClassName();

		SetupInputFilterFields(filterClassName);
	}

	//-----------------------------------------------------------------------------------------------
	protected void ResetBindings()
	{
		m_SettingsKeybindModule.ResetAction(m_sActionName, m_sActionPreset, m_eDevice);

		ShowBindsForAction();
		
		m_iSelectedKeybind = 0;
		ReselectRow();
		
		SCR_AnalyticsApplication analytics = SCR_AnalyticsApplication.GetInstance();
		if (analytics)
			analytics.ChangeKeybind(m_sActionName, m_sActionPreset);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void DeleteSelectedBind()
	{
		m_SettingsKeybindModule.DeleteActionBindByIndex(m_sActionName, m_iSelectedKeybind, m_sActionPreset, m_eDevice);
		
		ShowBindsForAction();
		
		// Select the row immediately above
		m_iSelectedKeybind = Math.Clamp(m_iSelectedKeybind - 1, 0, m_aBindRowButtons.Count() - 1);
		ReselectRow();
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void StartCapturingKeybind()
	{
		m_SettingsKeybindModule.StartCaptureForAction(m_sActionName, m_sActionPreset, m_eDevice, true);

		SCR_SimpleKeybindDialogUI dialog = SCR_KeybindDialogs.CreateSimpleKeybindDialog(
			m_Entry,
			m_sDisplayName,
			m_sActionName,
			m_SettingsKeybindModule.GetInputBindings(), 
			m_eDevice,
			false
		);
		
		if (!dialog)
			return;

		dialog.GetOnKeyCaptured().Insert(OnKeyCaptured);
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void UnbindAction()
	{
		m_SettingsKeybindModule.UnbindAction(m_sActionName, m_sActionPreset, m_eDevice);
		
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

	//------------------------------------------------------------------------------------------------
	protected void SetupDeviceIdSelector()
	{
		bool mustBeVisible = m_eDevice == EInputDeviceType.JOYSTICK;
		Widget deviceIdSelector = m_wRoot.FindAnyWidget("DeviceId");
		if (!deviceIdSelector)
		{
			if (mustBeVisible)
				Print("SCR_AdvancedKeybindDialogUI: Device id selector is was not found!", LogLevel.WARNING);

			return;
		}

		deviceIdSelector.SetEnabled(mustBeVisible);
		deviceIdSelector.SetVisible(mustBeVisible);
		if (!mustBeVisible)
			return;

		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(deviceIdSelector.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return;

		BaseContainer inputDeviceSettings = GetGame().GetEngineUserSettings().GetModule("InputDeviceUserSettings");
		BaseContainer joystickSetting = inputDeviceSettings.GetObject("JoystickSettings");
		if (!joystickSetting)
			return;

		BaseContainerList rules = joystickSetting.SetObjectArray("DeviceOrderRules");
		if (!rules)
			return;

		const bool isConsole = !SCR_SystemHelper.IsPC();

		const JoystickDeviceHandler joystickHandler = GetGame().GetInputManager().GetJoystickDeviceHandler();
		array<string> connectedDevices = {};
		if (isConsole)
		{
			bool elementSelected;
			for (int i; i < 4; i++)
			{
				if (joystickHandler.IsJoystickConnected(i))
				{
					if (!elementSelected)
					{
						elementSelected = true;
						comboBoxComponent.SetCurrentItem(i, false, false, false);
					}

					continue;
				}

				comboBoxComponent.ChangeItemName(string.Format("%1 (%2)", i, "#AR-GamepadRemovalDialog_Header"), i);
			}

			return;
		}

		for (int i = 0; i <= 3 ; i++)
		{
			string deviceName = joystickHandler.GetJoystickProductName(i);
			if (!deviceName.IsEmpty())
				connectedDevices.Insert(deviceName);
		}

		BaseContainer rule;
		string name;
		array<string> options = {string.Empty, string.Empty, string.Empty, string.Empty};
		bool hasCustomOption;
		for (int i, count = rules.Count(); i < count; i++)
		{
			rule = rules.Get(i);
			int deviceSlotNumber;
			rule.Get(SCR_InputHelper.INPUT_DEVICE_CONTAINER_DEVICE_SLOT_NAME, deviceSlotNumber);
			if (!options[deviceSlotNumber].IsEmpty())
				continue;

			rule.Get(SCR_InputHelper.INPUT_DEVICE_CONTAINER_DEVICE_NAME, name);
			if (!connectedDevices.Contains(name))
				continue;

			connectedDevices.RemoveItemOrdered(name);
			options[deviceSlotNumber] = string.Format(DEVICE_ID_DISPLAY_FORMAT, deviceSlotNumber, name);
			hasCustomOption = true;
		}

		foreach (int i, string optionName : options)
		{
			if (!optionName.IsEmpty())
				continue;

			if (connectedDevices.IsEmpty())
				break;

			options[i] = string.Format(DEVICE_ID_DISPLAY_FORMAT, i, connectedDevices[0]);
			connectedDevices.RemoveOrdered(0);
			hasCustomOption = true;
		}

		if (hasCustomOption)
		{
			comboBoxComponent.ClearAll();
			bool elementSelected;
			foreach (int id, string option : options)
			{
				if (option.IsEmpty())
					option = id.ToString();

				comboBoxComponent.AddItem(option);
				if (elementSelected)
					continue;

				elementSelected = joystickHandler.IsJoystickConnected(id);
				if (elementSelected)
					comboBoxComponent.SetCurrentItem(id, false, false, false);
			}
		}
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
			if (bind.m_eDeviceType != m_eDevice)
				continue;

			comboBoxComponent.AddItem(bind.GetBindingDisplayName());
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

		int addedEntries;
		array<ref SCR_KeyBindingCombo> combos = m_SettingsKeybindModule.GetCustomComboKeys();
		foreach (SCR_KeyBindingCombo combo : combos)
		{
			if (combo.m_eDeviceType != m_eDevice)
				continue;

			addedEntries = comboBoxComponent.AddItem(combo.m_sComboDisplayName);
		}

		comboBoxComponent.m_OnChanged.Remove(OnCombinationComboChanged);
		if (addedEntries > 0)
		{
			combinationBindCombo.SetEnabled(true);
			combinationBindCombo.SetVisible(true);
			comboBoxComponent.m_OnChanged.Insert(OnCombinationComboChanged);
		}
		else
		{
			combinationBindCombo.SetEnabled(false);
			combinationBindCombo.SetVisible(false);
		}
	}
	
	//-----------------------------------------------------------------------------------------------
	protected void OnPrefixesComboChanged(SCR_ComboBoxComponent component, int index)
	{
		m_SettingsKeybindModule.SetFilterForActionByIndex(m_sActionName, m_sActionPreset, index, m_iSelectedKeybind, m_eActionPrefixType, m_eDevice);
		
		ShowBindsForAction();
		
		ReselectRow();
		if (component)
			GetGame().GetWorkspace().SetFocusedWidget(component.GetRootWidget());
	}

	int GetCurrentlySelectedDeviceId()
	{
		if (m_eDevice != EInputDeviceType.JOYSTICK)
			return -1;

		Widget deviceIdSelector = m_wRoot.FindAnyWidget("DeviceId");
		if (!deviceIdSelector)
			return -1;

		SCR_ComboBoxComponent comboBoxComponent = SCR_ComboBoxComponent.Cast(deviceIdSelector.FindHandler(SCR_ComboBoxComponent));
		if (!comboBoxComponent)
			return -1;

		return comboBoxComponent.GetCurrentIndex();
	}

	//-----------------------------------------------------------------------------------------------
	protected void OnBindsComboChanged(SCR_ComboBoxComponent component, int index)
	{
		array<ref SCR_KeyBindingBind> binds = m_SettingsKeybindModule.GetCustomBinds();
		int filteredIndex = -1;
		int selectedBindUnfiltered;
		bool isAxis;
		foreach (int i, SCR_KeyBindingBind binding : binds)
		{
			if (binding.m_eDeviceType != m_eDevice)
				continue;

			filteredIndex++;
			if (filteredIndex != index)
				continue;

			selectedBindUnfiltered = i;
			isAxis = binding.m_bIsAxis;
			break;
		}
		
		m_SettingsKeybindModule.AddKeybindToActionByIndex(m_sActionName, m_sActionPreset, selectedBindUnfiltered, deviceId: GetCurrentlySelectedDeviceId());
		
		ShowBindsForAction();
		
		// Select the last row
		m_iSelectedKeybind = m_aBindRowButtons.Count() - 1;
		ReselectRow();
		if (isAxis)
			SetDefaultFilterForAxis();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetDefaultFilterForAxis()
	{
		array<ref SCR_KeyBindingFilter> filters = m_SettingsKeybindModule.GetFilters(m_eActionPrefixType);		
		
		int index;
		foreach (int id, SCR_KeyBindingFilter filter : filters)
		{
			if (filter.m_sFilterString != SCR_InputHelper.INPUT_FILTER_VALUE_TYPE_NAME)
				continue;

			index = id;
			break;
		}
		
		m_SettingsKeybindModule.SetFilterForActionByIndex(m_sActionName, m_sActionPreset, index, m_iSelectedKeybind, m_eActionPrefixType, m_eDevice);

		InputBinding inputBinds = m_SettingsKeybindModule.GetInputBindings();
		BaseContainer inputSource = SCR_InputHelper.GetInputBinding(m_sActionName, m_sActionPreset, m_iSelectedKeybind, inputBinds);
		if (!inputSource)
			return;

		string input;
		inputSource.Get(SCR_InputHelper.INPUT_CONTAINER_INPUT_NAME, input);
		float multiplier;
		float offset;
		int inverted;
		GetDefaultAxisFilterValues(input, multiplier, offset, inverted);

		SetInputFilterValueConfiguration(inputSource, inputBinds, multiplier, offset, inverted);
		ReselectRow();
	}

	//------------------------------------------------------------------------------------------------
	protected void GetDefaultAxisFilterValues(string inputBinding, out float multiplier, out float offset, out int inverted)
	{
		multiplier = 1;
		inverted = -1;

		if (inputBinding.Contains("+"))
		{
			inverted = 1;
		}
		else if (!inputBinding.Contains("-"))
		{
			multiplier = 0.5;
			offset = 1;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetInputFilterValueConfiguration(notnull BaseContainer inputSource, notnull InputBinding inputBindings, float multiplier = -1, float offset = -2, int invert = -1)
	{
		BaseContainer filter = inputSource.GetObject(SCR_InputHelper.INPUT_CONTAINER_FILTER_NAME);
		if (!filter)
			return;

		bool valueSet;
		if (invert < 0)
			filter.Get(SCR_InputHelper.INPUT_CONTAINER_INVERT_NAME, invert);
		else
			valueSet = filter.Set(SCR_InputHelper.INPUT_CONTAINER_INVERT_NAME, invert);

		if (invert > 0)
			multiplier *= -1;

		if (multiplier > -2)
			valueSet = filter.Set(SCR_InputHelper.INPUT_CONTAINER_MULTIPLIER_NAME, multiplier) || valueSet;

		if (offset > -2)
			valueSet = filter.Set(SCR_InputHelper.INPUT_CONTAINER_OFFSET_NAME, offset) || valueSet;

		if (valueSet)
			inputBindings.Save();
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
	void SCR_AdvancedKeybindDialogUI(SCR_KeyBindingEntry entry, string displayName, string actionName, string actionPreset, SCR_EActionPrefixType prefixType, EInputDeviceType deviceType = EInputDeviceType.INVALID)
	{
		if (deviceType == EInputDeviceType.INVALID)
			deviceType = EInputDeviceType.KEYBOARD;

		Setup(entry, displayName, actionName, deviceType);
		m_sActionPreset = actionPreset;
		m_eActionPrefixType = prefixType;
	}
}
