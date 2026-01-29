void ScriptInvokerFilterMethod(SCR_FilterEntry filter);
typedef func ScriptInvokerFilterMethod;
typedef ScriptInvokerBase<ScriptInvokerFilterMethod> ScriptInvokerFilter;

//! Filter panel component which is commonly found in content browser, scenarios menu, and other places.
//!
//! It controls a filter list box and a set of filter buttons which are added to the top bar when filter items are selected.
//! When we click on these buttons, they are removed and the according filter in the list is disabled.
//!
//! So, this panel just keeps the list box and buttons synchronized. Use the public API to get selected filters.
class SCR_FilterPanelComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("{3B47431EF3FA03F0}UI/layouts/Menus/ContentBrowser/Buttons/ContentBrowser_ButtonFilterTopBar.layout", UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sTopBarFilterButtonLayout;

	[Attribute("Icon")]
	protected string m_sTopBarFilterButtonIconName;

	[Attribute("Glow")]
	protected string m_sTopBarFilterButtonIconGlowName;

	[Attribute("FiltersTopBarButton", desc:"should be the same as in the tooltip's settings, since it is used to filter out the correct tooltip from a generic tooltip show event")]
	protected string m_sTopBarFilterButtonTooltipTag;

	[Attribute("HiddenFilters", desc:"should be the same as in the tooltip's settings, since it is used to filter out the correct tooltip from a generic tooltip show event")]
	protected string m_sTopBarHiddenFiltersTooltipTag;

	[Attribute("FiltersItemsFound", desc:"should be the same as in the tooltip's settings, since it is used to filter out the correct tooltip from a generic tooltip show event")]
	protected string m_sTopBarItemsFoundTooltipTag;
	
	[Attribute("", desc:"text of the tooltip for the ItemsFoundMessage")]
	protected string m_sTopBarItemsFoundTooltipMessage;
	
	[Attribute()]
	ref SCR_FilterSet m_FilterSet;

	[Attribute("8", UIWidgets.Auto, "When more than this amount of buttons are added to the top bar, they are hidden.")]
	protected int m_iMaxFilterButtons;

	[Attribute("false", UIWidgets.Auto, "Is the filter list or main panel content shown at start?")]
	protected bool m_bFilterShownAtStart;

	[Attribute("", UIWidgets.Auto, "Unique tag which will be used for saving this filter set in use settings. If left empty, filter set is not saved and not loaded.")]
	protected string m_sFilterSetStorageTag;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout for mutually exclusive filter elements", params: "layout")]
	protected ResourceName m_sElementLayoutMututallyExclusive;

	[Attribute("", UIWidgets.ResourceNamePicker, "Layout for general filter elements", params: "layout")]
	protected ResourceName m_sElementLayoutGeneral;

	[Attribute("0.1")]
	protected float m_fTobBarHighlightOpacity;
	
	[Attribute("2.5")]
	protected float m_fTopBarHighlightAnimationSpeed;
	
	[Attribute("1", desc: "1: forward and back once, 2: twice, etc.")]
	protected int m_iTopBarHighlightAnimationCycles;
	
	[Attribute(EAnimationCurve.EASE_OUT_QUART.ToString(), UIWidgets.ComboBox, "Animation curve on forward part of the cycle", "", ParamEnumArray.FromEnum(EAnimationCurve))]
	protected EAnimationCurve m_eTopBarHighlightAnimationCurve;
	
	[Attribute(EAnimationCurve.LINEAR.ToString(), UIWidgets.ComboBox, "Animation curve on backward part of the cycle", "", ParamEnumArray.FromEnum(EAnimationCurve))]
	protected EAnimationCurve m_eTopBarHighlightAnimationCurveReverse;
	
	protected ref SCR_FilterPanelWidgets m_Widgets = new SCR_FilterPanelWidgets();
	protected SCR_FilterEntry m_FocusedTopBarFilter;

	protected ref array<SCR_FilterEntry> m_aHiddenFilters = {};
	
	// TopBar highlight
	protected int m_iTopBarHighlightCycle;
	protected WidgetAnimationOpacity m_TopBarAnimation;

	// Event handlers
	protected ref ScriptInvokerFilter m_OnFilterChanged;
	protected ref ScriptInvokerBool m_OnFilterPanelToggled; // (bool newState) - true when new state is filter panel opened
	
	// Items Found Message
	protected const string ITEMS_FOUND_MESSAGE = "#AR-Filters_EntriesFound_Condensed";
	protected bool m_bItemsFoundMessageEnabled;
	protected int m_iEntriesFiltered;
	protected int m_iEntriesTotal;
	protected SCR_ScriptedWidgetTooltip m_ItemsFoundTooltip;
	
	protected bool m_bIsTooltipHoverWidgetInHierarchy;
	protected bool m_bIsTooltipInitialized;
	
	protected SCR_ScriptedWidgetTooltip m_Tooltip;
	
	protected static ref array<SCR_FilterPanelComponent> m_aActiveFilterPanels = {};

	// --------------- Public API -----------------------

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_FilterSet GetFilter()
	{
		return m_FilterSet;
	}

	//------------------------------------------------------------------------------------------------
	//! \return the component of the search edit box
	SCR_EditBoxSearchComponent GetEditBoxSearch()
	{
		return m_Widgets.m_FilterSearchComponent;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool AnyFilterSelected()
	{
		return m_FilterSet.AnyFilterSelected();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool AnyFilterButtonsVisible()
	{
		int nButtons = 0;
		Widget b = m_Widgets.m_FilterButtonsLayout.GetChildren();

		while (b)
		{
			nButtons++;
			b = b.GetSibling();
		}
	
		return nButtons > 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] message
	//! \return
	bool SetEditBoxSearchMessage(string message)
	{
		if (!m_Widgets.m_FilterSearchComponent)
			return false;
		
		return m_Widgets.m_FilterSearchComponent.SetMessage(message);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] current
	//! \param[in] total
	//! \param[in] enabled
	void SetItemsFoundMessage(int current, int total, bool enabled = true)
	{
		if (!m_Widgets.m_ItemsFoundText)
			return;
		
		EnableItemsFoundMessage(enabled);
		
		m_iEntriesFiltered = current;
		m_iEntriesTotal = total;
		
		m_Widgets.m_ItemsFoundText.SetText(WidgetManager.Translate(ITEMS_FOUND_MESSAGE, current, total));
		SetItemsFoundMessageVisible(true);
		
		UpdateItemsFoundTooltip();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Shows either the main content of the panel or the filter
	//! \param[in] show
	//! \param[in] animate
	void ShowFilterListBox(bool show, bool animate = false)
	{
		if (GetFilterListBoxShown() == show)
			return;

		m_Widgets.m_FilterButtonComponent.SetToggled(show, false);
		ShowFilterListBox_Internal(show, animate);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetFilterListBoxShown()
	{
		return m_Widgets.m_FilterButtonComponent.GetToggled();
	}

	//------------------------------------------------------------------------------------------------
	//! Selects or deselects the filter visually.
	//! !!! Does not update the internal state of the filter. You still need to call filter.SetSelected();
	//! \param[in] filter
	//! \param[in] select
	//! \param[in] invokeOnChanged
	void SelectFilter(SCR_FilterEntry filter, bool select, bool invokeOnChanged = true, bool instant = false)
	{
		Filter_ListBox_EnableFilter(filter, select, instant);

		if (select)
		{
			// Add button at the top bar. But don't add it for a default filter entry in a mutually exclusive category.
			if (! (filter.m_bSelectedAtStart && filter.GetCategory().m_bMutuallyExclusive))
				Filter_TopBar_AddButton(filter);
		}
		else
			Filter_TopBar_DeleteButton(filter);

		if (m_OnFilterChanged && invokeOnChanged)
			m_OnFilterChanged.Invoke(filter);
	}

	//------------------------------------------------------------------------------------------------
	//! Saves the filter configuration in user settings.
	//! \param[in] tag optional parameter, a tag used for saving the filter.
	//! if tag is not provided, the internal m_sFilterSetStorageTag is used from the attribute.
	void Save(string tag = string.Empty)
	{
		if (tag.IsEmpty())
			tag = m_sFilterSetStorageTag;

		if (tag.IsEmpty())
			return;

		if (m_sFilterSetStorageTag && m_FilterSet)
			SCR_AllFilterSetsStorage.SaveFilterSet(tag, m_FilterSet);
	}

	//------------------------------------------------------------------------------------------------
	//!
	void ResetToDefaultValues()
	{
		m_FilterSet.ResetToDefaultValues();
		SyncInternalToUi();
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] enable
	void EnableFilterButton(bool enable)
	{
		m_Widgets.m_FilterButtonComponent.SetEnabled(enable);
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool GetFilterButtonEnabled()
	{
		return m_Widgets.m_FilterButtonComponent.GetEnabled();
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to load filter values. If fails, it restores the filter to default state.
	//! \param[in] tag optional parameter, a tag used for saving the filter.
	//! if tag is not provided, the internal m_sFilterSetStorageTag is used from the attribute.
	//! \return
	bool TryLoad(string tag = string.Empty)
	{
		if (tag.IsEmpty())
			tag = m_sFilterSetStorageTag;

		if (tag.IsEmpty())
			return false;

		bool loadSuccess = SCR_AllFilterSetsStorage.TryLoadFilterSet(tag, m_FilterSet);
		if (!loadSuccess)
			m_FilterSet.ResetToDefaultValues();

		SyncInternalToUi();

		return loadSuccess;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetStorageTag()
	{
		return m_sFilterSetStorageTag;
	}

	//------------------------------------------------------------------------------------------------
	//! \return true when anything inside the filter panel is focused
	bool GetFocused()
	{
		// We check if focused widget is within hierarchy of our root widget

		Widget focused = GetGame().GetWorkspace().GetFocusedWidget();
		if (!focused)
			return false;

		Widget parent = focused.GetParent();

		while (parent)
		{
			if (parent == m_wRoot)
				return true;

			parent = parent.GetParent();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetFocusOnFirstItem()
	{
		m_Widgets.m_FilterListBoxComponent.SetFocusOnFirstItem();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_FilterPanelWidgets GetWidgets()
	{
		return m_Widgets;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerFilter GetOnFilterChanged()
	{
		if (!m_OnFilterChanged)
			m_OnFilterChanged = new ScriptInvokerFilter();

		return m_OnFilterChanged;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	ScriptInvokerBool GetOnFilterPanelToggled()
	{
		if (!m_OnFilterPanelToggled)
			m_OnFilterPanelToggled = new ScriptInvokerBool();

		return m_OnFilterPanelToggled;
	}

	// ---------------- Protected / Private --------------

	//------------------------------------------------------------------------------------------------
	// Conditions for showing the items found message:
	// 1- a filter is active
	// 2- the currently shown entries are lower than total (due to explicit search or particular tab)
	// IF the backend has received the total number of entries
	protected void SetItemsFoundMessageVisible(bool visible)
	{
		if (!m_Widgets.m_ItemsFoundOverlay)
			return;
		
		m_Widgets.m_ItemsFoundOverlay.SetVisible(m_bItemsFoundMessageEnabled && visible);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void EnableItemsFoundMessage(bool enabled = false)
	{
		m_bItemsFoundMessageEnabled = AnyFilterButtonsVisible() || enabled;
		if (!m_bItemsFoundMessageEnabled)
			SetItemsFoundMessageVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Enables/disables this filter in the list box
	//! \param[in] filter
	//! \param[in] enabled
	protected void Filter_ListBox_EnableFilter(SCR_FilterEntry filter, bool enabled, bool instant = false)
	{
		SCR_ListBoxComponent comp = m_Widgets.m_FilterListBoxComponent;

		int item = comp.FindItemWithData(filter);
		if (item == -1)
			return;

		comp.SetItemSelected(item, enabled, false, instant);
	}

	//------------------------------------------------------------------------------------------------
	//! Resets filters in category, all except for one filter, if it is not null
	//! \param[in] category
	//! \param[in] filterExclude
	protected void Filter_ListBox_ResetFiltersInCategory(SCR_FilterCategory category, SCR_FilterEntry filterExclude)
	{
		SCR_ListBoxComponent comp = m_Widgets.m_FilterListBoxComponent;

		for (int i = 0; i < comp.GetItemCount(); i++)
		{
			SCR_FilterEntry filter = SCR_FilterEntry.Cast(comp.GetItemData(i));
			if ((filter.GetCategory() == category) && (filter != filterExclude)) // If same category as the changed item, and not this item, reset it
			{
				comp.SetItemSelected(i, false, false, true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes a button which matches given category and filter
	//! \param[in] filter
	protected void Filter_TopBar_DeleteButton(SCR_FilterEntry filter)
	{
		Widget w = m_Widgets.m_FilterButtonsLayout.GetChildren();
		Widget wPrev = null;
		Widget nextFocus = null;
		int deletedButtonId = 0;
		while (w)
		{
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.FindComponent(w);
			if (comp)
			{
				SCR_FilterEntry buttonFilter = SCR_FilterEntry.Cast(comp.GetData());
				if (buttonFilter == filter)
				{
					// If we deleted a button which was focused, we need to find a new one
					if (GetGame().GetWorkspace().GetFocusedWidget() == w)
					{
						if (w.GetSibling())
							nextFocus = w.GetSibling();
						else if (wPrev)
							nextFocus = wPrev;
						else
							nextFocus = m_Widgets.m_FilterSearch;
					}
					m_Widgets.m_FilterButtonsLayout.RemoveChild(w);
				}
			}
			wPrev = w;
			w = w.GetSibling();
			deletedButtonId++;
		}

		Filter_TopBar_UpdateButtonsVisibility();

		if (nextFocus)
			GetGame().GetWorkspace().SetFocusedWidget(nextFocus);

//		// If we deleted a button which was focused, we need to find a new one
//		if (findNewFocus)
//		{
//			array<Widget> buttons = {};
//			w = m_Widgets.m_FilterButtonsLayout.GetChildren();
//			while (w)
//			{
//				buttons.Insert(w);
//				w = w.GetSibling();
//			}
//
//			if (deletedButtonId < buttons.Count())
//				GetGame().GetWorkspace().SetFocusedWidget(buttons[deletedButtonId]);		// Focus on next button on the list, which now has same position
//			else if (buttons.Count() > 0)
//				GetGame().GetWorkspace().SetFocusedWidget(buttons[deletedButtonId - 1]);	// Focus on prev button if there is a prev button
//			else
//				GetGame().GetWorkspace().SetFocusedWidget(m_Widgets.m_FilterSearch);			// Focus on search bar if there's nothing left
//		}
	}


	//------------------------------------------------------------------------------------------------
	//! Deletes all buttons which are linked to filters of given category
	//! \param[in] category
	protected void Filter_TopBar_DeleteButtonsInCategory(SCR_FilterCategory category)
	{
		Widget w = m_Widgets.m_FilterButtonsLayout.GetChildren();
		array<Widget> widgetsToDelete = {};
		while (w)
		{
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.FindComponent(w);
			if (comp)
			{
				SCR_FilterEntry buttonFilter = SCR_FilterEntry.Cast(comp.GetData());
				if (buttonFilter.GetCategory() == category)
					widgetsToDelete.Insert(w);
			}
			w = w.GetSibling();
		}

		foreach (Widget wdelete : widgetsToDelete)
		{
			m_Widgets.m_FilterButtonsLayout.RemoveChild(wdelete);
		}

		Filter_TopBar_UpdateButtonsVisibility();
	}

	//------------------------------------------------------------------------------------------------
	//! Adds a top bar filter button, links it with this filter category and filter
	//! \param[in] filter
	protected void Filter_TopBar_AddButton(SCR_FilterEntry filter)
	{
		// Bail if another button with same filter already exists
		Widget layoutContent = m_Widgets.m_FilterButtonsLayout.GetChildren();
		while (layoutContent)
		{
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.FindComponent(layoutContent);
			if (comp && comp.GetData() == filter)
				return;

			layoutContent = layoutContent.GetSibling();
		}

		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sTopBarFilterButtonLayout, m_Widgets.m_FilterButtonsLayout);
		SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.FindComponent(w);

		// Icon
		ResourceName iconImageSet = filter.m_sImageTexture;
		if (iconImageSet.IsEmpty())
			iconImageSet = filter.GetCategory().m_sFilterImageSet;

		ImageWidget image = ImageWidget.Cast(comp.GetRootWidget().FindAnyWidget(m_sTopBarFilterButtonIconName));
		image.LoadImageFromSet(0, iconImageSet, filter.m_sImageName);

		// Glow
		ResourceName glowImageSet = filter.m_sGlowTexture;
		if (glowImageSet.IsEmpty())
			glowImageSet = filter.GetCategory().m_sFilterGlowImageSet;

		ImageWidget glow = ImageWidget.Cast(comp.GetRootWidget().FindAnyWidget(m_sTopBarFilterButtonIconGlowName));
		glow.LoadImageFromSet(0, glowImageSet, filter.m_sImageName);

		// Data
		SCR_FilterEntry buttonData = filter;
		comp.SetData(buttonData);

		comp.m_OnClicked.Insert(OnTopBarFilterButtonClicked);
		comp.m_OnFocus.Insert(OnTopBarFilterButtonFocused);
		comp.m_OnFocusLost.Insert(OnTopBarFilterButtonFocusLost);

		Filter_TopBar_UpdateButtonsVisibility();
	}

	//------------------------------------------------------------------------------------------------
	//! Hides latest buttons if there are above m_iMaxFilterButtons buttons.
	//! Also updates the text of extra button count.
	protected void Filter_TopBar_UpdateButtonsVisibility()
	{
		int nButtons = 0;
		Widget b = m_Widgets.m_FilterButtonsLayout.GetChildren();
		Widget firstButton = b;
		m_aHiddenFilters.Clear();

		// Count how many buttons there are...
		while (b)
		{
			nButtons++;
			b = b.GetSibling();
		}

		// Hide buttons above threshold, show others
		// It's easier to redo it again for all buttons in case some where removed from the middle
		b = firstButton;
		for (int i = 0; i < nButtons; i++)
		{
			bool visible = i < m_iMaxFilterButtons;
			b.SetVisible(visible);

			// TODO: filter w/ "if !visible" before all the casts
			SCR_ModularButtonComponent comp = SCR_ModularButtonComponent.FindComponent(b);
			if (comp)
			{
				SCR_FilterEntry data = SCR_FilterEntry.Cast(comp.GetData());

				if (!visible && data)
					m_aHiddenFilters.Insert(data);
			}

			b = b.GetSibling();
		}

		// Show the +123 text
		if (nButtons > m_iMaxFilterButtons)
		{
			int nExtra = nButtons - m_iMaxFilterButtons;
			string s = string.Format("+%1", nExtra);
			m_Widgets.m_FilterButtonsCountText.SetText(s);
			m_Widgets.m_FilterButtonsCountText.SetVisible(true);
		}
		else
		{
			m_Widgets.m_FilterButtonsCountText.SetVisible(false);
		}
		
		if (m_Tooltip)
			OnTooltipShow(m_Tooltip);
	}

	//------------------------------------------------------------------------------------------------
	//! Fills the listbox with categories and filters
	protected void Filter_InitFilterPanel()
	{
		if (m_FilterSet)
		{
			// Create filter list entries according to configuration

			foreach (SCR_FilterCategory category : m_FilterSet.GetFilterCategories())
			{	
				// Don't create for selected platforms
				array<EPlatform> platforms = {};
				category.GetHideOnPlaforms(platforms);
				
				if (platforms.Contains(System.GetPlatform()))
					continue;
				
				category.m_wCategoryTitleWidget = m_Widgets.m_FilterListBoxComponent.AddSeparator(category.m_sDisplayName);
				
				// Resolve filter element layout
				// Mutually exclusive and generic filter category have different layouts and visuals
				ResourceName elementLayout = m_sElementLayoutGeneral;
				if (category.m_bMutuallyExclusive)
					elementLayout = m_sElementLayoutMututallyExclusive;

				foreach (SCR_FilterEntry filter : category.GetFilters())
				{
					// Image set can be from filter or from category
					ResourceName iconImageSet = filter.m_sImageTexture;
					if (iconImageSet.IsEmpty())
						iconImageSet = category.m_sFilterImageSet;

					m_Widgets.m_FilterListBoxComponent.AddItemAndIcon(filter.m_sDisplayName, iconImageSet,
						filter.m_sImageName, filter.m_FilterComponent, filter, elementLayout);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Updates internal values from UI
	protected void SyncUiToInternal()
	{
		SCR_ListBoxComponent comp = m_Widgets.m_FilterListBoxComponent;
		SCR_FilterEntry filter;
		int nElements = comp.GetItemCount();
		for (int i = 0; i < nElements; i++)
		{
			filter = SCR_FilterEntry.Cast(comp.GetItemData(i));
			if (filter)
				filter.SetSelected(comp.IsItemSelected(i));
		}
		
		EnableItemsFoundMessage();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates UI from internal values
	protected void SyncInternalToUi()
	{
		// Select filters which are selected in the filter set.
		array<ref SCR_FilterCategory> categories = m_FilterSet.GetFilterCategories();
		array<ref SCR_FilterEntry> filters;
		foreach (SCR_FilterCategory category : categories)
		{
			filters = category.GetFilters();
			foreach (SCR_FilterEntry filter : filters)
			{
				SelectFilter(filter, filter.GetSelected(), false, true);
			}
		}
		
		EnableItemsFoundMessage();
	}

	//------------------------------------------------------------------------------------------------
	//! Shows either the main content of the panel or the filter
	//! \param[in] show
	//! \param[in] animate
	protected void ShowFilterListBox_Internal(bool show, bool animate = false)
	{
		m_Widgets.m_MainContent.SetVisible(!show);
		m_Widgets.m_FilterListBoxOverlay.SetVisible(show);

		if (m_OnFilterPanelToggled)
			m_OnFilterPanelToggled.Invoke(show);

		// Top bar background animation
		if (!animate)
			return;

		if (!m_TopBarAnimation)
			m_TopBarAnimation = AnimateWidget.Opacity(m_Widgets.m_wTobBarHighlight, m_fTobBarHighlightOpacity, m_fTopBarHighlightAnimationSpeed);
		
		if (!m_TopBarAnimation)
			return;

		m_TopBarAnimation.SetRepeat(true);
		m_TopBarAnimation.SetCurve(m_eTopBarHighlightAnimationCurve);
		m_TopBarAnimation.GetOnCycleCompleted().Insert(OnTopBarHighlightCycleCompleted);
	}
	
	
	//------------------------------------------------------------------------------------------------
	protected void OnTopBarHighlightCycleCompleted()
	{
		// This event gets called each time an animation reaches it's end, meaning in a looping anim cycle 0 is 0 to 1, cycle 1 is 1 to 0, etc.
		if (!m_TopBarAnimation)
			return;

		m_iTopBarHighlightCycle++;
		
		if (m_iTopBarHighlightCycle % 2 != 0)
			m_TopBarAnimation.SetCurve(m_eTopBarHighlightAnimationCurveReverse);
		else
			m_TopBarAnimation.SetCurve(m_eTopBarHighlightAnimationCurve);
		
		// Last cycle 
		if (m_iTopBarHighlightCycle >= (m_iTopBarHighlightAnimationCycles * 2) - 1)
			m_TopBarAnimation.SetRepeat(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when top bar filter button is clicked
	//! Button is deleted, according filter in the listbox is unchecked
	//! \param[in] comp
	protected void OnTopBarFilterButtonClicked(SCR_ModularButtonComponent comp)
	{
		SCR_FilterEntry filter = SCR_FilterEntry.Cast(comp.GetData());

		// If it's last selected entry and we don't allow to have nothing selected here ...
		if (!filter.GetCategory().m_bAllowNothingSelected && filter.GetCategory().GetSelectedCount() == 1)
		{
			// Try to select first entry which is enabled at start
			SCR_FilterEntry defaultFilter;
			foreach (SCR_FilterEntry f : filter.GetCategory().GetFilters())
			{
				if (f.m_bSelectedAtStart)
				{
					defaultFilter = f;
					break;
				}
			}

			// Bail if default filter is not found, this is not a defined behaviour
			if (!defaultFilter)
				return;

			Filter_ListBox_EnableFilter(defaultFilter, true);
		}

		Filter_ListBox_EnableFilter(filter, false);

		// Delete this button
		Filter_TopBar_DeleteButton(filter);

		SyncUiToInternal();

		if (m_OnFilterChanged)
			m_OnFilterChanged.Invoke(filter);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTopBarFilterButtonFocused(SCR_ModularButtonComponent comp)
	{
		m_FocusedTopBarFilter = SCR_FilterEntry.Cast(comp.GetData());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnTopBarFilterButtonFocusLost(SCR_ModularButtonComponent comp)
	{
		m_FocusedTopBarFilter = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when filter listbox selection changes
	//! \param[in] comp
	//! \param[in] item
	//! \param[in] newSelected
	protected void OnFilterListBoxChanged(SCR_ListBoxComponent comp, int item, bool newSelected)
	{
		// If filters in this category are mutually exclusive
		// disable all other filters in same category
		SCR_FilterEntry filter = SCR_FilterEntry.Cast(comp.GetItemData(item));

		bool preventDeselection = !newSelected && !filter.GetCategory().m_bAllowNothingSelected && filter.GetCategory().GetSelectedCount() == 1;

		if (filter.GetCategory().m_bMutuallyExclusive && !preventDeselection)
		{
			Filter_ListBox_ResetFiltersInCategory(filter.GetCategory(), filter);
			Filter_TopBar_DeleteButtonsInCategory(filter.GetCategory());
		}

		// Add button to the top row, if this item was toggled on
		if (newSelected)
		{
			if (! (filter.m_bSelectedAtStart && filter.GetCategory().m_bMutuallyExclusive))
				Filter_TopBar_AddButton(filter);
		}
		else
		{
			// If it's last item deselected, and it's not allowed to have nothing selected, switch it back.
			if (preventDeselection)
				comp.SetItemSelected(item, true, false, true);
			else
				Filter_TopBar_DeleteButton(filter);
		}

		SyncUiToInternal();

		if (!preventDeselection && m_OnFilterChanged)
			m_OnFilterChanged.Invoke(filter);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnToggleFilterPanelButton(SCR_ModularButtonComponent comp, bool newToggled)
	{
		ShowFilterListBox_Internal(newToggled);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSearchConfirm(SCR_EditBoxComponent comp, string value)
	{
		EnableItemsFoundMessage(!value.IsEmpty());
	}
	
	// ---- Overrides ----

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_Widgets.Init(w);

		Filter_InitFilterPanel();

		ShowFilterListBox_Internal(m_bFilterShownAtStart, m_bFilterShownAtStart);
		m_Widgets.m_FilterButtonComponent.SetToggled(m_bFilterShownAtStart, false);

		Filter_TopBar_UpdateButtonsVisibility();
		SetItemsFoundMessageVisible(false);
		
		m_Widgets.m_FilterButtonComponent.m_OnToggled.Insert(OnToggleFilterPanelButton);
		m_Widgets.m_FilterListBoxComponent.m_OnChanged.Insert(OnFilterListBoxChanged);
		m_Widgets.m_FilterSearchComponent.m_OnConfirm.Insert(OnSearchConfirm);
		
		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Insert(OnTooltipShow);
		
		m_aActiveFilterPanels.Insert(this);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);

		SCR_ScriptedWidgetTooltip.GetOnTooltipShow().Remove(OnTooltipShow);
		
		if (m_TopBarAnimation)
			m_TopBarAnimation.SetRepeat(false);
		
		m_aActiveFilterPanels.RemoveItem(this);
	}

	// ---- Tooltips ----

	//------------------------------------------------------------------------------------------------
	protected void OnTooltipShow(SCR_ScriptedWidgetTooltip tooltip)
	{
		if (tooltip.GetTag() != m_sTopBarFilterButtonTooltipTag && tooltip.GetTag() != m_sTopBarHiddenFiltersTooltipTag && tooltip.GetTag() != m_sTopBarItemsFoundTooltipTag)
			return;
		
		MenuManager menuManager = GetGame().GetMenuManager();
		bool isInTopMenu = menuManager.GetOwnerMenu(m_wRoot) == menuManager.GetTopMenu();

		if (!isInTopMenu)
			return;

		// Having no way to access the tooltips other than a static invoker is less than ideal.
		// In this case for instance We need to make sure to update the tooltips with data from the correct filter panel component
		array<ref Widget> children = {};
		SCR_WidgetHelper.GetAllChildren(m_wRoot, children, true);
		
		foreach (Widget child : children)
		{
			m_bIsTooltipHoverWidgetInHierarchy = child == tooltip.GetHoverWidget();
			
			if (m_bIsTooltipHoverWidgetInHierarchy)
				break;
		}
		
		// Last activated filter panel handles the tooltip
		SCR_FilterPanelComponent lastPanel;
		int count = m_aActiveFilterPanels.Count();
		if (m_aActiveFilterPanels.IsIndexValid(count - 1))
			lastPanel = m_aActiveFilterPanels.Get(count - 1);
		
		if (this != lastPanel)
			return;
		
		// Trigger the correct tooltip initialization
		bool validTooltips;
		foreach (SCR_FilterPanelComponent panel : m_aActiveFilterPanels)
		{
			if (panel.IsTooltipHoverWidgetInHierarchy())
			{
				panel.FillTooltip(tooltip);
				validTooltips = true;
			}
			
			// Cleanup
			panel.ResetTooltipInitializationFlags();
		}
		
		// Prevent empty tooltips from being triggered
		if (!validTooltips)
			tooltip.ForceHidden();	
		
		m_Tooltip = tooltip;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateItemsFoundTooltip()
	{
		if (!m_ItemsFoundTooltip || !m_bIsTooltipHoverWidgetInHierarchy)
			return;
		
		SCR_ScriptedWidgetTooltipContentBase content = m_ItemsFoundTooltip.GetContent();
		if (!content)
			return;
		
		content.SetMessage(WidgetManager.Translate(m_sTopBarItemsFoundTooltipMessage, m_iEntriesFiltered, m_iEntriesTotal));
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] tooltip
	void FillTooltip(SCR_ScriptedWidgetTooltip tooltip)
	{
		if (m_bIsTooltipInitialized || !tooltip)
			return;
		
		m_bIsTooltipInitialized = true;
		
		SCR_ScriptedWidgetTooltipContentBase content = tooltip.GetContent();
		if (!content)
			return;
		
		switch (tooltip.GetTag())
		{
			case m_sTopBarFilterButtonTooltipTag:
			{
				if (!m_FocusedTopBarFilter)
					break;

				SCR_FilterCategory category = m_FocusedTopBarFilter.GetCategory();
				if (!category)
					break;

				content.SetMessage(string.Format("%1: %2", category.m_sDisplayName, m_FocusedTopBarFilter.m_sDisplayName));
				
				break;
			}

			case m_sTopBarHiddenFiltersTooltipTag:
			{
				Widget root = content.GetContentRoot();
				if (!root)
					break;
				
				SCR_ListTooltipComponent comp = SCR_ListTooltipComponent.FindComponent(root);
				if (!comp)
					break;

				array<string> names = {};
				foreach (SCR_FilterEntry filter : m_aHiddenFilters)
				{
					SCR_FilterCategory category = filter.GetCategory();
					if (category)
						names.Insert(string.Format("%1: %2", category.m_sDisplayName, filter.m_sDisplayName));
				}

				comp.Init(names);
				break;
			}
			
			case m_sTopBarItemsFoundTooltipTag:
			{
				m_ItemsFoundTooltip = tooltip;
				UpdateItemsFoundTooltip();
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsTooltipHoverWidgetInHierarchy()
	{
		return m_bIsTooltipHoverWidgetInHierarchy;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ResetTooltipInitializationFlags()
	{
		m_bIsTooltipHoverWidgetInHierarchy = false;
		m_bIsTooltipInitialized = false;
	}
}
