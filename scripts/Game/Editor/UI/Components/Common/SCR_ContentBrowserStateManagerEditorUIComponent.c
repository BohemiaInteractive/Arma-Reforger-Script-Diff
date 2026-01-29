class SCR_ContentBrowserStateManagerEditorUIComponent : ScriptedWidgetComponent
{
	[Attribute("0", desc: "If true will always show tabview even if loaded with a config")]
	protected bool m_bAlwaysShowTabview;
	
	[Attribute("0", desc: "If true it will never save any state changes and will updated when another system saves the state")]
	protected bool m_bDisplayContentOnly;
	
	[Attribute("0", desc: "If true it will never update the tab with active icons. But instead use the tab name")]
	protected bool m_bStaticTabName;
	
	[Attribute(defvalue: "BrowserStateTab")]
	protected string m_sBrowserStateTabViewName;
	
	[Attribute(defvalue: "SearchIcon")]
	protected string m_sStateTabSearchIconName;
	
	[Attribute(defvalue: "ActiveFilterHolder")]
	protected string m_sStateTabActiveFilterHolderName;
	
	[Attribute(defvalue: "Text")]
	protected string m_sStateTabTextName;
	
	[Attribute(defvalue: "TabImage")]
	protected string m_sStateTabActiveImageName;
	
	[Attribute(defvalue: "4", desc: "How many active labels can be shown on the tab before it shows the +x. If search is active then this is -1")]
	protected int m_iActiveFiltersVisibleOnTab;
	
	[Attribute(defvalue: "{6E0A6195BB1C69D3}UI/layouts/Editor/ContentBrowser/ContentBrowser_StateTab_ActiveFilter.layout")]
	protected string m_sStateTabActiveFilterPrefab;
	
	[Attribute(defvalue: "AditionalActiveLabels")]
	protected string m_sStateTabLabelAmountName;
	
	[Attribute(defvalue: "#AR-Editor_ContentBrowser_Filter_More", uiwidget: UIWidgets.EditBox)]
	protected LocalizedString m_sStateTabCountFormating;
	
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected SCR_TabViewComponent m_BrowserStateTabView;
	
	//------------------------------------------------------------------------------------------------
	//~ When tabview changes Browser State
	protected void OnBrowserStateChanged(SCR_TabViewComponent tabView, Widget tabRoot, int stateIndex)
	{
		m_ContentBrowserManager.SetBrowserState(stateIndex, true, false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBrowserEntriesFiltered()
	{
		//~ Update tabs
		if (!m_ContentBrowserManager)
			return;
		if (m_ContentBrowserManager.GetContentBrowserDisplayConfig() && !m_ContentBrowserManager.GetContentBrowserDisplayConfig().GetSaveContentBrowserState())
			return;
		
		array<EEditableEntityLabel> activeLabels = {};
		m_ContentBrowserManager.GetActiveLabels(activeLabels);
		int browserStateIndex = m_ContentBrowserManager.GetBrowserStateIndex();
		UpdateStateTab(browserStateIndex, m_ContentBrowserManager.GetCurrentSearch(), activeLabels);  
		
		//~ Set the saved index as tab
		m_BrowserStateTabView.ShowTab(browserStateIndex, false, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Enable/Disable all tabs
	protected void EnableAllTabs(bool enable)
	{
		m_BrowserStateTabView.EnableAllTabs(enable);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetTabVisible(int tabIndex, bool visible)
	{
		m_BrowserStateTabView.SetTabVisible(tabIndex, visible, false);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Creates state tabs if config is not used
	protected void CreateBrowserStateTabs()
	{		
		if (!m_ContentBrowserManager || !m_BrowserStateTabView)
			return;
		
		//~ Has display config so hide the tab
		if (!m_bAlwaysShowTabview && m_ContentBrowserManager.GetContentBrowserDisplayConfig())
		{
			m_BrowserStateTabView.GetRootWidget().SetVisible(false);
			return;
		}
			
		array<SCR_EditorContentBrowserSaveStateDataUI> contentBrowserStates = {};
		int count = m_ContentBrowserManager.GetContentBrowserTabStates(contentBrowserStates);
		SCR_TabViewContent tab;
		SCR_UIInfo uiInfo;
		
		for(int i = 0; i < count; i++)
        {
			uiInfo = contentBrowserStates[i].GetUIInfo();
			if (!uiInfo)
				continue;
			
			m_BrowserStateTabView.AddTab(string.Empty, uiInfo.GetName(), tabImage: uiInfo.GetIconPath());
		}

		int stateIndex = m_ContentBrowserManager.GetBrowserStateIndex();
		
		m_BrowserStateTabView.Init();
		
		m_BrowserStateTabView.GetOnChanged().Insert(OnBrowserStateChanged);

		m_BrowserStateTabView.ShowTab(stateIndex, !m_bDisplayContentOnly, false);
		
		array<EEditableEntityLabel> savedLabels = {};
		
		//Set initial tab visuals
		for(int i = 0; i < count; i++)		
		{
			contentBrowserStates[i].GetLabels(savedLabels);
			UpdateStateTab(i, contentBrowserStates[i].GetSearchString(), savedLabels);
		}
		
		array<int> hiddenStateTabs = {};
		m_ContentBrowserManager.GetHiddenStateTabs(hiddenStateTabs);
		foreach (int hiddenTabIndex: hiddenStateTabs)
		{
			SetTabVisible(hiddenTabIndex, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//~ Update the state tabs if search or filters changed
	protected void UpdateStateTab(int index, string searchString, notnull array<EEditableEntityLabel> activeLabels)
	{
		SCR_TabViewContent tab = m_BrowserStateTabView.GetEntryContent(index);
		
		if (!tab || !tab.m_ButtonComponent)
			return;
		
		//~ Simply display the name on the tab
		if (m_bStaticTabName)
		{
			TextWidget labelAmountWidget = TextWidget.Cast(tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabLabelAmountName));
			if (labelAmountWidget)
				labelAmountWidget.SetVisible(false);
			
			Widget searchIcon = tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabSearchIconName);
			if (searchIcon)
				searchIcon.SetVisible(false);
			
			return;
		}
		
		bool searchIconActive = false;
		
		Widget searchIcon = tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabSearchIconName);
		if (searchIcon)
		{
			searchIconActive = !searchString.IsEmpty();
			searchIcon.SetVisible(searchIconActive);
		}
			
		int activeFilterCount = activeLabels.Count();
		
		TextWidget labelAmountWidget = TextWidget.Cast(tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabLabelAmountName));
		if (labelAmountWidget)
		{
			//~ Check if additional active filters are active. (- if searchIconActive, which is either -0 (false) or -1 (true))
			if (activeFilterCount > (m_iActiveFiltersVisibleOnTab - searchIconActive))
			{
				labelAmountWidget.SetVisible(true);
				labelAmountWidget.SetTextFormat(m_sStateTabCountFormating, (activeFilterCount - (m_iActiveFiltersVisibleOnTab - searchIconActive)).ToString());
			}
			else 
			{
				labelAmountWidget.SetVisible(false);
			}
		}
		
		Widget tabTextWidget = tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabTextName);
		if (tabTextWidget)
			tabTextWidget.SetVisible(activeLabels.IsEmpty());
		
		Widget stateActiveFilterHolder = tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabActiveFilterHolderName);
		if (!stateActiveFilterHolder)
			return;
		
		//Clear children
		Widget child = stateActiveFilterHolder.GetChildren();
		Widget childtemp;
		while (child)
		{
			childtemp = child;
			child = child.GetSibling();
			childtemp.RemoveFromHierarchy();
		}
		
		Widget tabActiveFiltersHolder = tab.m_ButtonComponent.GetRootWidget().FindAnyWidget(m_sStateTabActiveFilterHolderName);
		if (!tabActiveFiltersHolder)
			return;
		
		int activeFiltersCreated = 0;
		SCR_UIInfo labelUiInfo;
		
		//~ Order Labels
		m_ContentBrowserManager.OrderLabels(activeLabels);
		
		EEditorMode editorMode = 0;
		SCR_EditorManagerEntity editorManager = SCR_EditorManagerEntity.GetInstance();
		if (editorManager) 
		{
			SCR_EditorModeEntity modeEntity = editorManager.GetCurrentModeEntity();
			if (modeEntity)
				editorMode = modeEntity.GetModeType();
		}	
		
		foreach (EEditableEntityLabel label: activeLabels)
		{
			if (!m_ContentBrowserManager.GetLabelUIInfoIfValid(label, editorMode, labelUiInfo))
				continue;
			
			if (!labelUiInfo)
				continue;
			
			Widget filterIcon = GetGame().GetWorkspace().CreateWidgets(m_sStateTabActiveFilterPrefab, tabActiveFiltersHolder);
			if (!filterIcon)
				continue;
			
			labelUiInfo.SetIconTo(ImageWidget.Cast(filterIcon.FindAnyWidget(m_sStateTabActiveImageName)));
			activeFiltersCreated++;
			
			if (activeFiltersCreated >= (m_iActiveFiltersVisibleOnTab - searchIconActive))
				break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (!m_ContentBrowserManager)
			return;
		
		Widget stateTab = w.FindAnyWidget(m_sBrowserStateTabViewName);
		if (stateTab)
			m_BrowserStateTabView = SCR_TabViewComponent.Cast(stateTab.FindHandler(SCR_TabViewComponent));				
		
		if (!m_BrowserStateTabView)
		{
			Print("'SCR_ContentBrowserStateManagerComponent' could not find SCR_TabViewComponent!", LogLevel.ERROR);
			return;
		}
	
		if (!m_bDisplayContentOnly)
		{
			// Hook into label change UI event for instant update
			m_ContentBrowserManager.GetOnLabelChanged().Insert(OnBrowserEntriesFiltered);
			m_ContentBrowserManager.GetOnEnableSaveStateTabs().Insert(EnableAllTabs);
		}
		
		m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Insert(OnBrowserEntriesFiltered);
		m_ContentBrowserManager.GetOnStateTabVisibilityChanged().Insert(SetTabVisible);
		
		//~ Create state tabs
		CreateBrowserStateTabs();
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode()) 
			return;
		
		if (m_ContentBrowserManager)
		{
			m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Remove(OnBrowserEntriesFiltered);
			m_ContentBrowserManager.GetOnStateTabVisibilityChanged().Remove(SetTabVisible);
			
			if (!m_bDisplayContentOnly)
			{
				m_ContentBrowserManager.GetOnLabelChanged().Remove(OnBrowserEntriesFiltered);
				m_ContentBrowserManager.GetOnEnableSaveStateTabs().Remove(EnableAllTabs);
			}
		}
	}
}
