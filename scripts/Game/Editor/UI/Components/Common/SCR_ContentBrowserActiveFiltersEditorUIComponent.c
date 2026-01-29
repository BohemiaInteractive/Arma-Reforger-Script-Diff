class SCR_ContentBrowserActiveFiltersEditorUIComponent: ScriptedWidgetComponent
{
	[Attribute("")]
	protected ResourceName m_sActiveFilterPrefab;
	
	[Attribute("")]
	protected ResourceName m_sMoreActiveFiltersPrefab;
	
	[Attribute("")]
	protected ResourceName m_NoActiveFiltersPrefab;
	
	[Attribute("+%1")]
	protected LocalizedString m_sOverflowTextFormat;
	
	[Attribute("ActiveFilterLayout")]
	protected string m_sFiltersHolderName;
	
	[Attribute("8")]
	protected int m_sFilterDisplayLimit;
	
	[Attribute("FilterSegment", desc: "Will find the component in parent")]
	protected string m_sContentBrowserFilterUiComponentName;
	
	//Ref
	protected Widget m_wRoot;
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserComponent;
	protected SCR_ContentBrowserFiltersEditorUIComponent m_ContentBrowserFilterUiComponent;
	protected Widget m_FiltersHolder;
	protected ref array<EEditableEntityLabel> m_aActiveFilters = {};
	protected ref map <EEditableEntityLabelGroup, int> m_mGroupLabelOrder = new map <EEditableEntityLabelGroup, int>();
	//protected ref map <EEditableEntityLabel, int> m_mLabelOrder = new map <EEditableEntityLabelGroup, int>();
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilterLabelChanged(EEditableEntityLabel label, bool active)
	{
		if (active)
		{
			//~ Check if not yet in active filters
			if (!m_aActiveFilters.Contains(label))
				InsertOrderedFilter(label);
			else 
				return;
		}
		else 
		{
			if (m_aActiveFilters.Contains(label))
				m_aActiveFilters.RemoveItem(label);
			else 
				return;
		}
		
		CreateFilters();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InsertOrderedFilter(EEditableEntityLabel label)
	{
		if (m_aActiveFilters.IsEmpty())
		{
			m_aActiveFilters.Insert(label);
			return;
		}
		
		EEditableEntityLabelGroup groupLabel;
		m_ContentBrowserComponent.GetLabelGroupType(label, groupLabel);
		int newLabelGroupOrder = m_mGroupLabelOrder[groupLabel];
		int newLabelOrder = m_ContentBrowserComponent.GetLabelOrderInGroup(label);
		int checkGroupOrder;
		int checkLabelOrder;
		int count = m_aActiveFilters.Count();
		bool inserted = false;
		array<SCR_EditableEntityCoreLabelSetting> groupLabels = {};
		int indexInGroup;
		
		for(int i = 0; i < count; i++)
        {
        	m_ContentBrowserComponent.GetLabelGroupType(m_aActiveFilters[i], groupLabel);
			checkGroupOrder = m_mGroupLabelOrder[groupLabel];
			
			//Part of same group check if label order is higher or equal
			if (newLabelGroupOrder == checkGroupOrder)
			{
				checkLabelOrder = m_ContentBrowserComponent.GetLabelOrderInGroup(m_aActiveFilters[i]);
				
				//If label has the same order check which index is first
				if (checkLabelOrder == newLabelOrder)
				{
					if (groupLabels.IsEmpty())
					{
						m_ContentBrowserComponent.GetLabelsOfGroup(groupLabel, groupLabels);
						indexInGroup = GetIndexInGroup(groupLabels, label);
					}
						
					if (indexInGroup < GetIndexInGroup(groupLabels, m_aActiveFilters[i]))
					{
						m_aActiveFilters.InsertAt(label, i);
						inserted = true;
						break;
					}
				}
				//Insert before label
				else if (newLabelOrder < checkLabelOrder)
				{
					m_aActiveFilters.InsertAt(label, i);
					inserted = true;
					break;
				}
			}
			//Insert before group
			else if (newLabelGroupOrder < checkGroupOrder)
			{
				m_aActiveFilters.InsertAt(label, i);
				inserted = true;
				break;
			}
        }
		
		if (!inserted)
			m_aActiveFilters.Insert(label);
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetIndexInGroup(array<SCR_EditableEntityCoreLabelSetting> groupLabels,  EEditableEntityLabel label)
	{
		int count = groupLabels.Count();
		for(int i = 0; i < count; i++)
		{
			if (groupLabels[i].GetLabelType() == label)
				return i;
		}
		
		return -1;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFiltersReset()
	{
		m_ContentBrowserComponent.GetActiveLabels(m_aActiveFilters);
		CreateFilters();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnActiveFilterPressed(SCR_ButtonBaseComponent button)
	{
		SCR_ContentBrowserActiveFilterEditorUIComponent activeFilterComponent = SCR_ContentBrowserActiveFilterEditorUIComponent.Cast(button.GetRootWidget().FindHandler(SCR_ContentBrowserActiveFilterEditorUIComponent));
		
		if (!activeFilterComponent)
			return;
		
		m_ContentBrowserFilterUiComponent.DisableFilterFromActiveFilters(activeFilterComponent.GetLabelReference());
	}
	
	//------------------------------------------------------------------------------------------------
	//Create active filter widgets
	protected void CreateFilters()
	{
		ClearAllFilterWidgets();
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		int count = m_aActiveFilters.Count();
		
		//~ No active filters
		if (count <= 0)
		{
			workspace.CreateWidgets(m_NoActiveFiltersPrefab, m_FiltersHolder);
			return;
		}
		
		int overflow = 0;
		
		if (count > m_sFilterDisplayLimit)
		{
			overflow = count - m_sFilterDisplayLimit;
			count = m_sFilterDisplayLimit;
		}
		
		Widget filterWidget;
		SCR_ContentBrowserActiveFilterEditorUIComponent activeFilterComponent;
		SCR_UIInfo uiInfo;
		SCR_ButtonBaseComponent button;
		int ignoredFilters = 0;
		SCR_EditorContentBrowserDisplayConfig contentBrowserConfig = m_ContentBrowserComponent.GetContentBrowserDisplayConfig();
		
		//~ Get all valid blacklisted labels
		array<EEditableEntityLabel> validBlackListLabels = {};
		m_ContentBrowserComponent.GetValidBlackListedLabels(validBlackListLabels);
		
		for(int i = 0; i < count; i++)
		{
			//~ Ignore if label cannot be shown in active filters (Is always active and can't be shown or if blacklisted label)
			if ((contentBrowserConfig && !contentBrowserConfig.CanShowLabelInActiveFilters(m_aActiveFilters[i])) || validBlackListLabels.Contains(m_aActiveFilters[i]))
			{
				//~ Make sure the system knows the filter is ignored
				ignoredFilters++;
				continue;
			}
			
			filterWidget = workspace.CreateWidgets(m_sActiveFilterPrefab, m_FiltersHolder);
			activeFilterComponent = SCR_ContentBrowserActiveFilterEditorUIComponent.Cast(filterWidget.FindHandler(SCR_ContentBrowserActiveFilterEditorUIComponent));
			button = SCR_ButtonBaseComponent.Cast(filterWidget.FindHandler(SCR_ButtonBaseComponent));
			
			if (!activeFilterComponent || !button)
			{
				Print("SCR_ContentBrowserActiveFiltersEditorUIComponent created active filter is missing components!", LogLevel.ERROR);
				continue;
			}

			m_ContentBrowserComponent.GetLabelUIInfo(m_aActiveFilters[i], uiInfo);
			activeFilterComponent.InitFilterButton(m_aActiveFilters[i], uiInfo);
			button.m_OnClicked.Insert(OnActiveFilterPressed);
		}
		
		//~ No active filters visible
		if ((count - ignoredFilters) <= 0)
		{
			workspace.CreateWidgets(m_NoActiveFiltersPrefab, m_FiltersHolder);
			return;
		}
		
		//~ Set overflow indicator
		if ((overflow - ignoredFilters) > 0)
		{
			filterWidget = workspace.CreateWidgets(m_sMoreActiveFiltersPrefab, m_FiltersHolder);
			TextWidget text = TextWidget.Cast(filterWidget.FindAnyWidget("Count"));
			if (text)
				text.SetTextFormat(m_sOverflowTextFormat, overflow.ToString());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Clear filter widgets
	protected void ClearAllFilterWidgets()
	{
		Widget child = m_FiltersHolder.GetChildren();
		Widget childtemp;
		SCR_ButtonBaseComponent button;
		
		while (child)
		{
			button = SCR_ButtonBaseComponent.Cast(child.FindHandler(SCR_ButtonBaseComponent));
			
			if (button)
				button.m_OnClicked.Remove(OnActiveFilterPressed);
			
			childtemp = child;
			child = child.GetSibling();
			childtemp.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Init of the UI component. Creating active filters
	//! \param[in] contentBrowserFilterUiComponent SCR_ContentBrowserFiltersEditorUIComponent reference to set filters when active filter is pressed
	void Init(SCR_ContentBrowserFiltersEditorUIComponent contentBrowserFilterUiComponent)
	{
		m_ContentBrowserFilterUiComponent = contentBrowserFilterUiComponent;

		m_ContentBrowserComponent = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		
		if (!m_ContentBrowserComponent || !contentBrowserFilterUiComponent)
		{
			m_wRoot.SetVisible(false);
			return;
		}
		
		m_FiltersHolder = m_wRoot.FindAnyWidget(m_sFiltersHolderName);
		if (!m_FiltersHolder)
			return;
				
		array<ref SCR_EditableEntityCoreLabelGroupSetting> labelGroups = {};
		m_ContentBrowserComponent.GetLabelGroups(labelGroups);
		
		foreach (SCR_EditableEntityCoreLabelGroupSetting groupSetting: labelGroups)
		{
			m_mGroupLabelOrder.Insert(groupSetting.GetLabelGroupType(), groupSetting.GetOrder());
		}
		
		m_ContentBrowserComponent.GetOnLabelChanged().Insert(OnFilterLabelChanged);
		m_ContentBrowserComponent.GetOnBrowserStateCleared().Insert(OnFiltersReset);
		
		m_ContentBrowserComponent.GetActiveLabels(m_aActiveFilters);
		CreateFilters();
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		m_wRoot = w;
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (m_ContentBrowserComponent)
		{
			m_ContentBrowserComponent.GetOnLabelChanged().Remove(OnFilterLabelChanged);
			m_ContentBrowserComponent.GetOnBrowserStateCleared().Remove(OnFiltersReset);
		}
	}
}
