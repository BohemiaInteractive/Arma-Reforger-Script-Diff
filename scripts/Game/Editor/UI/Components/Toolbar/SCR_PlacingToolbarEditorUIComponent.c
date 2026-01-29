//! @ingroup Editor_UI Editor_UI_Components

class SCR_PlacingToolbarEditorUIComponent : SCR_BaseToolbarEditorUIComponent
{
	[Attribute()]
	protected ref array<ref SCR_ContentBrowserEditorCard> m_aCardPrefabs;
	
	[Attribute("0")]
	protected bool m_bShowBudgetCost;
	
	[Attribute(desc: "Value of this budget will be shown directly on prefab card.", UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_eBudgetToShow;
	
	protected SCR_PlacingEditorComponent m_PlacingManager;
	protected SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	protected SCR_BudgetEditorComponent m_BudgetManager;
	protected ResourceName m_RepeatPrefab;
	protected ResourceName m_DefaultLayout;
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] itemWidget
	//! \param[in] prefabIndex
	//! \param[in] enter
	void OnCardHover(Widget itemWidget, int prefabIndex, bool enter)
	{
		// Do not reset preview cost when entity is selected / window closes
		if (!m_ContentBrowserManager || m_PlacingManager.GetSelectedPrefab())
			return;
		
		if (!enter)
			prefabIndex = -1;

		m_ContentBrowserManager.RefreshPreviewCost(prefabIndex);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] itemWidget
	void OnCardLMB(Widget itemWidget)
	{
		SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(itemWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		if (!assetCard)
			return;
		
		int prefabID = assetCard.GetPrefabIndex();
		ResourceName prefab = m_ContentBrowserManager.GetResourceNamePrefabID(prefabID);
		
		if (!m_PlacingManager.SetSelectedPrefab(prefab, showBudgetMaxNotification: true))
			return;
			
		SCR_PlacingToolbarEditorUIComponent linkedComponent = SCR_PlacingToolbarEditorUIComponent.Cast(m_LinkedComponent);
		if (linkedComponent)
			linkedComponent.m_RepeatPrefab = prefab;
		
		if (m_bIsInDialog)
		{
			EditorMenuBase menu = EditorMenuBase.Cast(GetMenu());
			if (menu)
				menu.CloseSelf();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected Widget CreateItem(int index)
	{
		int prefabID = m_ContentBrowserManager.GetFilteredPrefabID(index);
		if (prefabID < 0)
			return null;
		
		SCR_EditableEntityUIInfo info = m_ContentBrowserManager.GetInfo(prefabID);
		
		//--- Select layout
		m_ItemLayout = m_DefaultLayout;
		if (info)
		{
			foreach (SCR_ContentBrowserEditorCard itemLayoutCandidate: m_aCardPrefabs)
			{
				if (itemLayoutCandidate.m_EntityType == info.GetEntityType())
				{
					m_ItemLayout = itemLayoutCandidate.m_sPrefab;
					break;
				}
			}
		}
		
		//--- Create layout
		Widget itemWidget;
		SCR_BaseToolbarItemEditorUIComponent item;
		if (!CreateItem(itemWidget, item))
			return null;
		
		SCR_UIInfo blockingBudgetInfo;
		array<ref SCR_EntityBudgetValue> entityBudgetCosts = { };
		m_ContentBrowserManager.CanPlace(prefabID, entityBudgetCosts, blockingBudgetInfo);		
		
		SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(itemWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		assetCard.GetOnHover().Insert(OnCardHover);
		assetCard.InitCard(prefabID, info, m_ContentBrowserManager.GetResourceNamePrefabID(prefabID), blockingBudgetInfo);
		
		SCR_EntityBudgetValue budgetCost;
		if (m_bShowBudgetCost && !entityBudgetCosts.IsEmpty())
			foreach (SCR_EntityBudgetValue budgetValue: entityBudgetCosts)
			{
				if (budgetValue.GetBudgetType() == m_eBudgetToShow)
					assetCard.UpdateBudgetCost(budgetValue);
			}
		
		ButtonActionComponent.GetOnAction(itemWidget, true, 0).Insert(OnCardLMB);
		
		SCR_ButtonBaseComponent buttonComponent = SCR_ButtonBaseComponent.Cast(itemWidget.FindHandler(SCR_ButtonBaseComponent));
		if (buttonComponent)
			buttonComponent.SetMouseOverToFocus(false);
		
		return itemWidget;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnPageChanged(int page)
	{
		if (m_ContentBrowserManager.GetPageEntryCount() > 0)
		{
			m_iFirstShownIndex = page * m_Pagination.GetColumns();
			int contentBrowserPageIndex = m_iFirstShownIndex / m_ContentBrowserManager.GetPageEntryCount();
			m_ContentBrowserManager.SetPageIndex(contentBrowserPageIndex);
		}

		super.OnPageChanged(page);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		for (int i = indexStart; i < indexEnd; i++)
		{
			CreateItem(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBudgetMaxReached(EEditableEntityBudget entityBudget, bool maxReached)
	{
		if (!m_ContentBrowserManager || !m_Pagination)
			return;
		
		m_Pagination.RefreshPage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnBrowserEntriesChanged()
	{
		if (!m_ContentBrowserManager || !m_Pagination)
			return;
		
		if (m_ContentBrowserManager.GetContentBrowserDisplayConfig() && m_ContentBrowserManager.GetContentBrowserDisplayConfig().GetSaveContentBrowserState())
			return;
		
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnResourceTypeEnabledChanged(array<EResourceType> disabledResourceTypes)
	{
		Refresh();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void Refresh()
	{		
		m_Pagination.SetEntryCount(m_ContentBrowserManager.GetFilteredPrefabCount());
		
		int contentBrowserPage = m_ContentBrowserManager.GetPageIndex();
		int contentBrowserPageEntryCount = m_ContentBrowserManager.GetPageEntryCount();
		int toolbarPageIndex = (contentBrowserPage * contentBrowserPageEntryCount) / m_Pagination.GetColumns();
		
		m_Pagination.SetPage(toolbarPageIndex);
		
//		SCR_EditorContentBrowserDisplayConfig displayConfig = m_ContentBrowserManager.GetContentBrowserDisplayConfig();
//		if (displayConfig)
//			m_bShowBudgetCost = displayConfig.GetShowAvailableBudgetCost();
//		else
//			m_bShowBudgetCost = false;

		super.Refresh();
	}

	//------------------------------------------------------------------------------------------------
	override void OnRepeat()
	{
		if (m_RepeatPrefab)
			m_PlacingManager.SetSelectedPrefab(m_RepeatPrefab, showBudgetMaxNotification: true);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnResourceTypeEnabledChanged().Insert(OnResourceTypeEnabledChanged);
		
		m_PlacingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		if (!m_PlacingManager)
			return;
		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (!m_ContentBrowserManager)
			return;
		
		m_ContentBrowserManager.GetOnBrowserStatesSaved().Insert(OnBrowserEntriesChanged);
		m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Insert(OnBrowserEntriesChanged);
		
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		if (m_BudgetManager)
			m_BudgetManager.Event_OnBudgetMaxReached.Insert(OnBudgetMaxReached);
		
		//--- Find default card layout
		foreach (SCR_ContentBrowserEditorCard defaultLayoutCandidate: m_aCardPrefabs)
		{
			if (defaultLayoutCandidate.m_EntityType == EEditableEntityType.GENERIC)
			{
				m_DefaultLayout = defaultLayoutCandidate.m_sPrefab;
				break;
			}
		}
		
		super.HandlerAttachedScripted(w);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode)
			gameMode.GetOnResourceTypeEnabledChanged().Remove(OnResourceTypeEnabledChanged);
		
		if (m_ContentBrowserManager)
		{
			m_ContentBrowserManager.GetOnBrowserStatesSaved().Remove(OnBrowserEntriesChanged);
			m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Remove(OnBrowserEntriesChanged);
		}

		if (m_BudgetManager)
			m_BudgetManager.Event_OnBudgetMaxReached.Remove(OnBudgetMaxReached);
	}
}
