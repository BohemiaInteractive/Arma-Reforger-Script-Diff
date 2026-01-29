//! @ingroup Editor_UI Editor_UI_Components

//! The Filter window of the Content Browser

class SCR_ContentBrowserFiltersEditorUIComponent : SCR_BaseEditorUIComponent 
{
	protected static const string FILTERGROUP_VLAYOUT_NAME = "FilterOptions";
	
	[Attribute(desc: "Name of widget in which labels will be spawned.", defvalue: "FilterList")]
	private string m_sLayoutName;
	
	[Attribute(desc: "Name of scroll widget for scroll manipulation.", defvalue: "Scroll")]
	private string m_sScrollLayoutName;
	
	[Attribute(desc: "Name of widget where label groups will be added under", defvalue: "FilterOptions")]
	private string m_sLabelGroupLayoutName;
	
	[Attribute(desc: "Name of text widget with label group title TextWidget", defvalue: "GroupTitle")]
	private string m_sLabelGroupTitleTextName;
	
	[Attribute(desc: "Name of text which shows the name of the entity type the conditional group is linked to.", defvalue: "ConditionalSubHeader")]
	private string m_ConditionalSubHeaderName;
	
	[Attribute(desc: "Format of Conditional SubHeader", defvalue: "#AR-Editor_ContentBrowser_FilterCategory_ConditionalSubheader")]
	private string m_ConditionalSubHeaderFormat;
	
	[Attribute(desc: "Spacing between Header and Filter options. Hidden if ConditionalSubHeader is displayed", defvalue: "FilterOptionSpace")]
	private string m_sFilterOptionSpaceName;
	
	[Attribute(desc: "Name of imagewidget of individual filter layout", defvalue: "Image")]
	private string m_sLabelImageWidgetName;
	
	[Attribute("{B556F6D3FE4B4307}UI/layouts/Editor/ContentBrowser/ContentBrowser_FilterButton.layout")]
	private ResourceName m_LabelPrefab;
	
	[Attribute("{D87E75E2BF96D734}UI/layouts/Editor/ContentBrowser/ContentBrowser_LabelGroup.layout")]
	private ResourceName m_LabelGroupPrefab;
	
	[Attribute("0.5")]
	private float m_fLabelClickDelay;
	
	private Widget m_wLayout;
	
	private ref map<EEditableEntityLabel, SCR_ListBoxElementComponent> m_ButtonByLabel = new map<EEditableEntityLabel, SCR_ListBoxElementComponent>();
	
	private ref map<EEditableEntityLabel, ref set<Widget>> m_GroupWidgetByConditionalLabel = new map<EEditableEntityLabel, ref set<Widget>>();
	private ref map<EEditableEntityLabelGroup, Widget> m_GroupWidgetByGroupLabel = new map<EEditableEntityLabelGroup, Widget>();
	
	private SCR_ContentBrowserEditorComponent m_ContentBrowserEditorComponent;
	
	private ScriptInvoker m_ShowLoadingCallback;
	private ScriptInvoker m_LabelApplyCallback;
//	protected ref ScriptInvoker Event_OnFiltersReset = new ScriptInvoker();
//	protected ref ScriptInvoker Event_OnActiveFilterClicked = new ScriptInvoker();
	
	protected Widget m_wFirstButton;
	
	protected bool m_bIsDoubleClick;
	protected bool m_bUsingGamePad;
	protected Widget m_wLastFocusedLabel;
	protected float m_fLabelSelectedTimer;
	
	protected ScrollLayoutWidget m_wScrollview;
	protected float m_fFiltersScrollPosY;
	
	//------------------------------------------------------------------------------------------------
	protected void SetFilterToggled(EEditableEntityLabel entityLabel, bool state, bool setData = false, SCR_ListBoxElementComponent handler = null, bool instant = false)
	{
		if (!handler)
			handler = m_ButtonByLabel.Get(entityLabel);

		if (handler)
			handler.SetToggled(state, false, instant);

		CheckConditionalGroup(entityLabel, state, instant);
		m_ContentBrowserEditorComponent.SetLabel(entityLabel, state);
	}
	
	//------------------------------------------------------------------------------------------------
	//! On Active filter pressed
	//! \param entityLabel EEditableEntityLabel to set disabled
	void DisableFilterFromActiveFilters(EEditableEntityLabel entityLabel)
	{
		SetFilterToggled(entityLabel, false, true);
		ApplyFiltersDelayed();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] filterHandler
	//! \param[out] label
	//! \param[out] isToggled
	//! \param[out] groupLabel
	//! \return
	bool GetLabelFromHandler(SCR_ListBoxElementComponent filterHandler, out EEditableEntityLabel label, out bool isToggled, out EEditableEntityLabelGroup groupLabel)
	{
		if (!filterHandler)
			return false;
		
		SCR_EditableEntityCoreLabelSetting labelSettings = SCR_EditableEntityCoreLabelSetting.Cast(filterHandler.GetData());
		if (!labelSettings)
			return false;
		
		label = labelSettings.GetLabelType();
		groupLabel = labelSettings.GetLabelGroupType();
		isToggled = m_ContentBrowserEditorComponent.IsLabelActive(label);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void CheckConditionalGroup(EEditableEntityLabel filterLabel, bool value, bool instant = false)
	{
		if (!m_GroupWidgetByConditionalLabel.Contains(filterLabel))
			return;
		
		float x;
		m_wScrollview.GetSliderPosPixels(x, m_fFiltersScrollPosY);
		
		// Get filter groups toggled by this filter
		set<Widget> conditionalGroupWidgets = m_GroupWidgetByConditionalLabel.Get(filterLabel);		
		foreach (Widget conditionalGroupWidget : conditionalGroupWidgets)
		{
			if (!conditionalGroupWidget)
				continue;

			conditionalGroupWidget.SetVisible(value);
			
			if (!value)
			{
				// Deactivate all filters in this group if filter is deactivated and conditional group is hidden
				Widget filterVerticalLayout = conditionalGroupWidget.FindAnyWidget(FILTERGROUP_VLAYOUT_NAME);
				if (!filterVerticalLayout)
					continue;
				
				Widget filterLayout = filterVerticalLayout.GetChildren();
				while (filterLayout)
				{
					SCR_ListBoxElementComponent filterHandler = SCR_ListBoxElementComponent.Cast(filterLayout.FindHandler(SCR_ListBoxElementComponent));
					if (!filterHandler)
						continue;
					
					SCR_EditableEntityCoreLabelSetting labelSettings = SCR_EditableEntityCoreLabelSetting.Cast(filterHandler.GetData());
					if (!labelSettings)
						continue;
					
					SetFilterToggled(labelSettings.GetLabelType(), false, false, filterHandler, instant);
					
					filterLayout = filterLayout.GetSibling();
				}
			}
		}
		m_wScrollview.SetSliderPosPixels(0, m_fFiltersScrollPosY);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterToggled(SCR_ModularButtonComponent handler, bool newToggle)
	{
		EEditableEntityLabel toggledLabel;
		EEditableEntityLabelGroup groupLabel;
		bool isToggled;
		if (GetLabelFromHandler(SCR_ListBoxElementComponent.Cast(handler), toggledLabel, isToggled, groupLabel))
		{
			CheckConditionalGroup(toggledLabel, newToggle);
			m_ContentBrowserEditorComponent.SetLabel(toggledLabel, newToggle);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] handler
	void OnMouseEnterFilterWidget(SCR_ModularButtonComponent handler)
	{
		OnFilterLabelFocus(handler.GetRootWidget(), handler);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] label
	//! \param[in] handler
	void OnFilterLabelFocus(Widget label, SCR_ModularButtonComponent handler)
	{
		if (m_bUsingGamePad && label == m_wFirstButton)
			m_wScrollview.SetSliderPos(0,0);
		
		if (label)
			m_wLastFocusedLabel = label;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	Widget GetLastFocusedLabel()
	{
		if (!m_wLastFocusedLabel)
			return m_wFirstButton;
		else
			return m_wLastFocusedLabel;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnResetClicked()
	{
		array<EEditableEntityLabel> activeLabels = {};
		m_ContentBrowserEditorComponent.GetActiveLabels(activeLabels);
		
		foreach	(EEditableEntityLabel toggledLabel : activeLabels)
		{
			SetFilterToggled(toggledLabel, false, false, null, true);
		}
		
		m_wScrollview.SetSliderPos(0, 0);
		
		//~ Check always active labels to make sure conditional groups are displayed correctly on reset
		if (m_ContentBrowserEditorComponent.GetContentBrowserDisplayConfig())
		{
			array<EEditableEntityLabel> alwaysActiveLabels = {};
			m_ContentBrowserEditorComponent.GetContentBrowserDisplayConfig().GetAlwaysActiveLabels(alwaysActiveLabels);
			
			foreach (EEditableEntityLabel label: alwaysActiveLabels)
			{
				CheckConditionalGroup(label, true, true);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Before Browser state is loaded
	protected void OnBrowserStatePreloaded(SCR_EditorContentBrowserSaveStateData state)
	{
		array<EEditableEntityLabel> savedLabels = {};
		m_ContentBrowserEditorComponent.GetActiveLabels(savedLabels);
		
		//~ Reset labels
		foreach (EEditableEntityLabel label: savedLabels)
		{
			SetFilterToggled(label, false, false, null, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ After Browser state is loaded
	protected void OnBrowserStateLoaded(SCR_EditorContentBrowserSaveStateData state)
	{
		array<EEditableEntityLabel> savedLabels = {};
		state.GetLabels(savedLabels);
		
		//~ Set saved Labels
		foreach (EEditableEntityLabel label: savedLabels)
		{
			SetFilterToggled(label, true, false, null, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ApplyLabelChanges()
	{
		if (m_LabelApplyCallback == null)
			return;
		
		//Save focussed widget.
		//~Note currently The pagination sets the focused widget and forces it to the first element if non where selected in the Grid
		//  It would be difficult to start changing the logic shared by many UI so instead I force the focus back.
		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		
		//Send event to apply filter changes
		m_LabelApplyCallback.Invoke(true);
		
		//Set focus back if the widget still extist
		if (focusedWidget)
			GetGame().GetWorkspace().SetFocusedWidget(focusedWidget);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] groupLabel
	//! \param[in] ignoreButton
	void ResetLabelsOfGroupLabel(EEditableEntityLabelGroup groupLabel, Widget ignoreButton = null)
	{
		Widget filterGroupLayout = m_GroupWidgetByGroupLabel.Get(groupLabel);
		
		if (!filterGroupLayout)
			return;
		
		Widget filterWidget = filterGroupLayout.GetChildren();
		int i = 0;
		while (filterWidget)
		{
			SCR_ListBoxElementComponent buttonComponent = SCR_ListBoxElementComponent.Cast(filterWidget.FindHandler(SCR_ListBoxElementComponent));
			if (buttonComponent && buttonComponent.GetToggled() && (!ignoreButton || ignoreButton != buttonComponent.GetRootWidget()))
				buttonComponent.SetToggled(false, true, true);
			
			filterWidget = filterWidget.GetSibling();
			i++;
			if (i > 100)
				break; // Failsafe
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] showLoadingCallback
	//! \param[in] labelApplyCallback
	void SetContentBrowserCallbacks(ScriptInvoker showLoadingCallback, ScriptInvoker labelApplyCallback)
	{
		m_ShowLoadingCallback = showLoadingCallback;
		m_LabelApplyCallback = labelApplyCallback;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void InitializeLabels()
	{
		if (!m_ContentBrowserEditorComponent)
		{
			Print("SCR_ContentBrowserEditorComponent not set on current editor mode prefab", LogLevel.ERROR);
			return;
		}
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		Widget existingLabelWidget = m_wLayout.GetChildren();
		while (existingLabelWidget)
		{
			existingLabelWidget.RemoveFromHierarchy();
			existingLabelWidget = existingLabelWidget.GetSibling();
		}
		
		array<ref SCR_EditableEntityCoreLabelGroupSetting> labelGroups = {};
		
		m_ContentBrowserEditorComponent.GetLabelGroups(labelGroups);
		
		array<SCR_EditableEntityCoreLabelSetting> groupLabels = {};
		Widget labelGroupWidget, labelGroupLayout, labelWidget;
		TextWidget groupTextWidget;
		EEditableEntityLabel conditionalLabel;
		SCR_EditorContentBrowserDisplayConfig contentBrowserConfig = m_ContentBrowserEditorComponent.GetContentBrowserDisplayConfig();
		
		//~ Get all valid blacklisted labels
		array<EEditableEntityLabel> validBlackListLabels = {};
		m_ContentBrowserEditorComponent.GetValidBlackListedLabels(validBlackListLabels);
		
		EEditorMode editorMode = 0;
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) 
		{
			SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
			if (modeEntity)
				editorMode = modeEntity.GetModeType();
		}
		
		SCR_ButtonTextComponent selectableButton;
		foreach (SCR_EditableEntityCoreLabelGroupSetting labelGroup : labelGroups)
		{
			EEditableEntityLabelGroup groupLabel = labelGroup.GetLabelGroupType();
			
			if (contentBrowserConfig && !contentBrowserConfig.CanShowLabelGroup(groupLabel))
				continue;
			
			m_ContentBrowserEditorComponent.GetLabelsOfGroup(groupLabel, groupLabels);
			
			labelGroupWidget = workspace.CreateWidgets(m_LabelGroupPrefab, m_wLayout);
			labelGroupLayout = labelGroupWidget.FindAnyWidget(m_sLabelGroupLayoutName);
			
			SCR_UIInfo labelGroupInfo = labelGroup.GetInfo();
			if (labelGroupInfo)
			{
				groupTextWidget = TextWidget.Cast(labelGroupWidget.FindAnyWidget(m_sLabelGroupTitleTextName));
				labelGroupInfo.SetNameTo(groupTextWidget);
			}
			
			// Show/hide label based on active conditional label if configured
			conditionalLabel = labelGroup.GetConditionalLabelType();
			if (conditionalLabel != EEditableEntityLabel.NONE)
			{
				labelGroupWidget.SetVisible(m_ContentBrowserEditorComponent.IsLabelActive(conditionalLabel));
				
				SCR_UIInfo conditionalEntityTypeUiInfo;
				m_ContentBrowserEditorComponent.GetLabelUIInfo(conditionalLabel, conditionalEntityTypeUiInfo);
				
				if (conditionalEntityTypeUiInfo)
				{
					TextWidget conditionalSubheader = TextWidget.Cast(labelGroupWidget.FindAnyWidget(m_ConditionalSubHeaderName));
					if (conditionalSubheader)
					{
						conditionalSubheader.SetTextFormat(m_ConditionalSubHeaderFormat, conditionalEntityTypeUiInfo.GetName());
						conditionalSubheader.SetVisible(true);
						
						Widget filterOptionSpace = labelGroupWidget.FindAnyWidget(m_sFilterOptionSpaceName);
						if (filterOptionSpace)
							filterOptionSpace.SetVisible(false);
					}
				}
				
				set<Widget> conditionalGroupWidgets = m_GroupWidgetByConditionalLabel.Get(conditionalLabel);
				if (conditionalGroupWidgets)
				{
					conditionalGroupWidgets.Insert(labelGroupWidget);
				}
				else
				{
					conditionalGroupWidgets = new set<Widget>();
					conditionalGroupWidgets.Insert(labelGroupWidget);
					m_GroupWidgetByConditionalLabel.Set(conditionalLabel, conditionalGroupWidgets);
				}
			}
			
			m_GroupWidgetByGroupLabel.Insert(groupLabel, labelGroupLayout);
			
			bool hasAtLeastOneLabelVisible = false;
			
			foreach (SCR_EditableEntityCoreLabelSetting entityLabelSettings : groupLabels)
			{		
				if (!entityLabelSettings.GetFilterEnabled()) 
					continue;
				
				//~ Not a valid label for current mode
				SCR_EditableEntityCoreLabelSettingExtended extended = SCR_EditableEntityCoreLabelSettingExtended.Cast(entityLabelSettings);
				if (extended && !extended.IsValid(editorMode))
					continue;
				
				EEditableEntityLabel entityLabel = entityLabelSettings.GetLabelType();
				
				//~ Skip labels that cannot be shown (Is always active or blacklisted label)
				if ((contentBrowserConfig && !contentBrowserConfig.CanShowLabel(entityLabel)) || validBlackListLabels.Contains(entityLabel))
					continue;
				
				hasAtLeastOneLabelVisible = true;
				
				SCR_UIInfo labelUIInfo = entityLabelSettings.GetInfo();
				string labelTitle = labelUIInfo.GetName();
				Widget labelLayout = workspace.CreateWidgets(m_LabelPrefab, labelGroupLayout);
				
				SCR_ListBoxElementComponent labelOptionHandler = SCR_ListBoxElementComponent.Cast(labelLayout.FindHandler(SCR_ListBoxElementComponent));
				
				ImageWidget labelImageWidget = ImageWidget.Cast(labelOptionHandler.GetRootWidget().FindAnyWidget(m_sLabelImageWidgetName));
				if (labelImageWidget)
					labelUIInfo.SetIconTo(labelImageWidget);
				
				SCR_LinkTooltipTargetEditorUIComponent linkedTooltipComponent = SCR_LinkTooltipTargetEditorUIComponent.Cast(labelLayout.FindHandler(SCR_LinkTooltipTargetEditorUIComponent));
				if (linkedTooltipComponent)
					linkedTooltipComponent.SetInfo(labelUIInfo);
				
				if (!m_wFirstButton)
					m_wFirstButton = labelLayout;
				
				bool isLabelActive = m_ContentBrowserEditorComponent.IsLabelActive(entityLabel);
				
				labelOptionHandler.SetText(labelUIInfo.GetName());
				labelOptionHandler.SetToggled(isLabelActive, false, true);
				labelOptionHandler.SetData(entityLabelSettings);
				labelOptionHandler.m_OnMouseEnter.Insert(OnMouseEnterFilterWidget);
				labelOptionHandler.m_OnFocus.Insert(OnMouseEnterFilterWidget);
				labelOptionHandler.m_OnClicked.Insert(OnFilterClicked);
				labelOptionHandler.m_OnDoubleClicked.Insert(OnFilterDoubleClicked);
				labelOptionHandler.m_OnToggled.Insert(OnFilterToggled);
				m_ButtonByLabel.Insert(entityLabel, labelOptionHandler);
			}
			
			//Hide the label group if all the labels within are hidden
			if (!hasAtLeastOneLabelVisible)
				labelGroupWidget.SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_bUsingGamePad = isGamepad;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterClicked(SCR_ListBoxElementComponent handler)
	{
		if (m_bIsDoubleClick)
		{
			m_bIsDoubleClick = false;
			return;
		}
		
		EEditableEntityLabel entityLabel;
		EEditableEntityLabelGroup groupLabel;
		bool isToggled;
		if (GetLabelFromHandler(handler, entityLabel, isToggled, groupLabel))
		{
			SetFilterToggled(entityLabel, !isToggled, true, handler);
			ApplyFiltersDelayed();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterDoubleClicked(SCR_ListBoxElementComponent handler)
	{
		if (m_ContentBrowserEditorComponent.GetActiveLabelCount() <= 1)
			return;
		
		m_bIsDoubleClick = true;
		
		EEditableEntityLabel entityLabel;
		EEditableEntityLabelGroup groupLabel;
		bool isToggled;
		if (GetLabelFromHandler(handler, entityLabel, isToggled, groupLabel))
		{
			SetFilterToggled(entityLabel, true, true, handler);
			ResetLabelsOfGroupLabel(groupLabel, handler.GetRootWidget());
			ApplyFiltersDelayed();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void ApplyFiltersDelayed()
	{
		m_fLabelSelectedTimer = m_fLabelClickDelay;
		if (m_ShowLoadingCallback)
			m_ShowLoadingCallback.Invoke(true);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate(float timeSlice)
	{
		if (m_fLabelSelectedTimer > 0)
		{
			m_fLabelSelectedTimer -= timeSlice;
			if (m_fLabelSelectedTimer < 0)
				ApplyLabelChanges();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		GetMenu().GetOnMenuUpdate().Insert(OnMenuUpdate);
		
		m_ContentBrowserEditorComponent = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent));
		m_ContentBrowserEditorComponent.GetOnBrowserStateLoaded().Insert(OnBrowserStateLoaded);
		m_ContentBrowserEditorComponent.GetOnBrowserStatePreload().Insert(OnBrowserStatePreloaded);
		
		m_wLayout = w.FindAnyWidget(m_sLayoutName);
		
		m_wScrollview = ScrollLayoutWidget.Cast(w.FindAnyWidget(m_sScrollLayoutName));
		
		// Remove any existing filter groups, used for configuring UI layouts
		Widget debugFilterGroup = m_wLayout.GetChildren();
		int i = 0;
		while(debugFilterGroup && i++ < 100)
		{
			Widget debugFilterSibling = debugFilterGroup.GetSibling();
			debugFilterGroup.RemoveFromHierarchy();
			debugFilterGroup = debugFilterSibling;
		}
		
		if (!m_wLayout)
		{
			Print("Can't find layout for content browser label list", LogLevel.ERROR);
			return;
		}
		
		if (m_LabelPrefab.IsEmpty())
		{
			Print("Content Browser Label List does not have a label prefab assigned", LogLevel.ERROR);
			return;
		}
		
		InitializeLabels();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_ContentBrowserEditorComponent)
		{
			m_ContentBrowserEditorComponent.GetOnBrowserStatePreload().Remove(OnBrowserStatePreloaded);
			m_ContentBrowserEditorComponent.GetOnBrowserStateLoaded().Remove(OnBrowserStateLoaded);
		}
		
		super.HandlerDeattached(w);
		if (SCR_Global.IsEditMode())
			return;

		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		GetMenu().GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
}
