/** @ingroup Editor_UI Editor_UI_Components
The Content Browser Window in the Content Browser Dialog
*/
class SCR_ContentBrowserEditorUIComponent: SCR_BasePaginationUIComponent//MenuRootSubComponent
{
	[Attribute()]
	protected ref array<ref SCR_ContentBrowserEditorCard> m_aCardPrefabs;
	[Attribute("4", desc: "Speed value passed to WidgetAnimator, 1 = 1sec, higher = faster")]
	private float m_iCardFadeInSpeed;
	
	[Attribute(params: "layout")]
	private ResourceName m_sCardPrefab;
	
	[Attribute(params: "layout")]
	private ResourceName m_sEmptyCardPrefab;
	
	[Attribute(params: "layout")]
	private ResourceName m_sUndefinedCardPrefab;
	
	[Attribute("Title")]
	protected string m_sHeaderWidgetName;
	
	[Attribute("Filters")]
	private string m_sLabelComponentWidget;
	
	[Attribute("NoFilterResults")]
	protected string m_sNoFilterResultsWidgetName;
	
	[Attribute(defvalue: "ResetFilterButton")]
	protected string m_sResetFiltersButtonName;
	
	[Attribute("EditBoxSearch")]
	private string m_sSearchEditBoxName;
	
	[Attribute("Scroll")]
	private string m_sScrollLayoutName;
	
	[Attribute("LoadingCircle")]
	private string m_sContentLoadingWidgetName;
	
	[Attribute(defvalue: "ToggleBetweenCardAndFilter")]
	protected string m_sToggleCardFilterButton;
	
	[Attribute(defvalue: "ToggleSearch")]
	protected string m_sToggleSearchButton;
	
	[Attribute(defvalue: "ActiveFilters")]
	protected string m_sActiveFiltersName;
	
	[Attribute(defvalue: "SelectButton")]
	protected string m_sSelectCardButtonName;
	
	[Attribute(defvalue: "ToggleButton")]
	protected string m_sToggleFilterdButtonName;

	[Attribute("SOUND_E_OPENASSETBROWSER", UIWidgets.EditBox, "")]
	protected string m_sSfxOnOpenDialog;
	
	private SCR_EditableEntityCore m_EntityCore;
	private SCR_BudgetEditorComponent m_BudgetManager;
	private SCR_ContentBrowserEditorComponent m_ContentBrowserManager;
	private SCR_PlacingEditorComponent m_PlacingManager;
	private EditorBrowserDialogUI m_ContentBrowserDialog;
	
	private SCR_ContentBrowserFiltersEditorUIComponent m_LabelsUIComponent;
	
	protected Widget m_wFocusedCard;
	protected int m_iCardFocusIndex;
	protected bool m_bBudgetPreviewUpdateEnabled;
	protected bool m_bUsingGamePad;
	protected bool m_bFocusingAssetCard;
	protected bool m_bSearchIsToggled;
	protected bool m_bContentLoadingWidgetActive;
	protected bool m_bAnimateEntries;
	protected SCR_InputButtonComponent m_wToggleCardFilterButton;
	
	//Widget refs
	protected Widget m_wToggleSearchButton;
	protected Widget m_LastSelectedOnSearch;
	protected Widget m_NoFilterResultsWidget;
	protected Widget m_SelectCardButton;
	protected Widget m_ToggleFilterdButton;
	
	protected Widget m_ContentLoadingVisual;
	protected SCR_EditBoxComponent m_SearchEditBox;
	protected ScrollLayoutWidget m_ScrollLayout;
	protected ref array<string> m_aLocalizationKeys = {};
	
	private ref ScriptInvoker m_LabelApplyCallback = new ScriptInvoker();
	private ref ScriptInvoker m_ShowLoadingCircleCallback = new ScriptInvoker();
	
	protected bool m_bCardSelected; //~ If card selected then no close Sfx is played
	
	/*!
	On Card click
	*/
	void OnCardLMB(Widget assetWidget)
	{
		SCR_PlacingEditorComponent placingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, false, true));
		if (!placingManager) return;
		
		int entryIndex = GetEntryIndex(assetWidget);
		int prefabID = m_ContentBrowserManager.GetFilteredPrefabID(entryIndex);
		
		ResourceName prefab = m_ContentBrowserManager.GetResourceNamePrefabID(prefabID);
		
		if (placingManager.SetSelectedPrefab(prefab, showBudgetMaxNotification: true))
		{
			// Only close window if prefab can be selected
			CloseContentBrowser();
			m_bCardSelected = true;
		}
	}
	
	/*!
	Close content browser menu from handler, helper function for e.g. autotests
	*/
	void CloseContentBrowser()
	{
		m_bBudgetPreviewUpdateEnabled = false;
		if (m_ContentBrowserDialog)
			m_ContentBrowserDialog.Close();
	}
	
	//---- REFACTOR NOTE START: Using hardcoded 20
	
	override void ShowEntries(Widget contentWidget, int indexStart, int indexEnd)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		//Show no result indicator and hide grid		
		bool showNoResults = indexEnd == 0 && !m_bContentLoadingWidgetActive;
		if (m_NoFilterResultsWidget)
			m_NoFilterResultsWidget.SetVisible(showNoResults);
		contentWidget.SetVisible(!showNoResults);
		
		ResourceName cardPrefab, defaultCardPrefab;
		foreach (SCR_ContentBrowserEditorCard cardPrefabCandidate: m_aCardPrefabs)
		{
			if (cardPrefabCandidate.m_EntityType == EEditableEntityType.GENERIC)
			{
				defaultCardPrefab = cardPrefabCandidate.m_sPrefab;
				break;
			}
		}
		
		Widget firstCard;
		SCR_EditableEntityUIInfo entityInfo;
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		for (int i = indexStart; i < indexEnd; i++)
		{
			int prefabID = m_ContentBrowserManager.GetFilteredPrefabID(i);
			entityInfo = m_ContentBrowserManager.GetInfo(prefabID);
			
			cardPrefab = m_sUndefinedCardPrefab;
			if (entityInfo)
			{
				cardPrefab = defaultCardPrefab;
				foreach (SCR_ContentBrowserEditorCard cardPrefabCandidate: m_aCardPrefabs)
				{
					if (cardPrefabCandidate.m_EntityType == entityInfo.GetEntityType())
					{
						cardPrefab = cardPrefabCandidate.m_sPrefab;
						break;
					}
				}
			}
			
			//--- Create card
			Widget assetWidget = workspace.CreateWidgets(cardPrefab, contentWidget);
			if (!assetWidget) continue;
			
			SCR_AssetCardFrontUIComponent assetCard = SCR_AssetCardFrontUIComponent.Cast(assetWidget.FindHandler(SCR_AssetCardFrontUIComponent));
			if (!assetCard)
			{
				Print("Asset widget is missing SCR_AssetCardFrontUIComponent!", LogLevel.ERROR);
				assetWidget.RemoveFromHierarchy();
				continue;
			}
			
			SCR_UIInfo blockingBudgetInfo;
			array<ref SCR_EntityBudgetValue> budgetCosts = { };
			
			if (!m_PlacingManager.HasPlacingFlag(EEditorPlacingFlags.CHARACTER_PLAYER))
				m_ContentBrowserManager.CanPlace(prefabID, budgetCosts, blockingBudgetInfo);
			
			//Set card focus
			if (firstCard == null)
				firstCard = assetCard.GetButtonWidget();
			
			//if (prefabID == m_iCardFocusIndex)
			//	m_wFocusedCard = assetCard.GetButtonWidget();
			
			assetCard.GetOnHover().Insert(OnCardHover);
			assetCard.GetOnFocus().Insert(OnCardFocus);
			assetCard.InitCard(prefabID, entityInfo, m_ContentBrowserManager.GetResourceNamePrefabID(prefabID), blockingBudgetInfo);
			ButtonActionComponent.GetOnAction(assetWidget, true, 0).Insert(OnCardLMB);
			
			if (m_bAnimateEntries)
			{
				assetWidget.SetOpacity(0);
				int cardIndexOnPage = (i - indexStart);
				
				callQueue.CallLater(AnimateCardFadeIn, cardIndexOnPage * 20, false, assetWidget);
			}
		}
		
		//No focused card
		if (m_wFocusedCard == null)
			m_wFocusedCard = firstCard;
		
		/* Currently overridden by paging focus
		if (m_wFocusedCard)
		{
			m_FocusedIndex[0] = UniformGridSlot.GetRow(m_wFocusedCard);
			m_FocusedIndex[1] = UniformGridSlot.GetColumn(m_wFocusedCard);
			GetGame().GetWorkspace().SetFocusedWidget(m_wFocusedCard);
		}
		*/
		
		m_bAnimateEntries = false;
		m_bBudgetPreviewUpdateEnabled = true;
	}
	
	//---- REFACTOR NOTE END ----
	
	override int GetEntryCount()
	{
		if (m_ContentBrowserManager && !m_bContentLoadingWidgetActive)
		{
			return m_ContentBrowserManager.GetFilteredPrefabCount();
		}
		else
		{
			return 0;
		}
	}
	
	protected void AnimateCardFadeIn(Widget cardWidget)
	{
		AnimateWidget.Opacity(cardWidget, 1.0, m_iCardFadeInSpeed);
	}
	
	protected void ToggleBetweenCardsAndFilters()
	{
		if (m_bFocusingAssetCard)
			SwitchToFilters();
		else
			SwitchToAssetCards();
		
		GamepadToggleSelectButtonSwap();
	}
	
	protected void SwitchToAssetCards()
	{
		if (!m_wFocusedCard)
			return;
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) 
			return;

		workspace.SetFocusedWidget(m_wFocusedCard);
	}
	
	protected void GamepadToggleSelectButtonSwap()
	{
		m_SelectCardButton.SetVisible(m_bFocusingAssetCard);
		m_ToggleFilterdButton.SetVisible(!m_bFocusingAssetCard);
	}
	
	protected void SwitchToFilters()
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) 
			return;
			
		if (m_LabelsUIComponent)
			workspace.SetFocusedWidget(m_LabelsUIComponent.GetLastFocusedLabel());
	}
	
	protected void GamePadSwitchToggleSearch()
	{
		if (m_bSearchIsToggled)
			GamePadToggleSearch(false, false);
		else
			GamePadToggleSearch(true, false);
	}
	
	protected void GamePadToggleSearch(bool toggle, bool forceToggle = true)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace) 
			return;
		
		Widget focusedWidget = workspace.GetFocusedWidget();
		if (!forceToggle && (focusedWidget == m_SearchEditBox.GetRootWidget() || focusedWidget == m_SearchEditBox.m_wEditBox))
		{
			EditBoxWidget.Cast(m_SearchEditBox.m_wEditBox).ActivateWriteMode();
		}
		else if (m_bSearchIsToggled != toggle) 
		{
			m_bSearchIsToggled = toggle;
			
			if (m_bSearchIsToggled)
			{			
				workspace.SetFocusedWidget(m_SearchEditBox.GetRootWidget());
				EditBoxWidget.Cast(m_SearchEditBox.m_wEditBox).ActivateWriteMode();
			}
			else 
			{
				workspace.SetFocusedWidget(m_wFocusedCard);
			}
		}
	}
	
	protected void ResetSearch()
	{
		if (m_SearchEditBox)
		{
			m_SearchEditBox.SetValue(string.Empty);	
		}
		m_ContentBrowserManager.SetCurrentSearch(string.Empty);
		m_ContentBrowserManager.SetPageIndex(0);
		m_ContentBrowserManager.SaveBrowserState();
		
		if (m_bUsingGamePad)
			GamePadToggleSearch(false);
	}
	
	protected void FilterEntries(bool resetPage = true, bool resetSearch = true, bool animateEntries = false)
	{
		// Search
		if (resetSearch)
		{
			ResetSearch();
		}
		else
		{
			string searchInputText = m_SearchEditBox.GetValue().Trim();
			if (!searchInputText.IsEmpty() && searchInputText != m_ContentBrowserManager.GetCurrentSearch())
			{
				m_ContentBrowserManager.SetCurrentSearch(searchInputText);
			}
		}
		
		if (resetPage)
		{
			m_ContentBrowserManager.SetPageIndex(0);
		}
		
		
		m_ContentBrowserManager.FilterEntries();
		
		// Set page
		m_bAnimateEntries = animateEntries;
						
		SetPage(m_ContentBrowserManager.GetPageIndex(), true);
	}
	
	protected void OnInputDeviceIsGamepadScripted(bool isGamepad)
	{
		m_bUsingGamePad = isGamepad;
		
		if (m_bFocusingAssetCard)
			OnCardHover(null, -1, true);
		else
			OnCardHover(null, -1, false);
		
		if (isGamepad)
		{
			GamepadToggleSelectButtonSwap();
		}
		else 
		{
			m_SelectCardButton.SetVisible(false);
			m_ToggleFilterdButton.SetVisible(false);
		}
		
		if (!m_SearchEditBox)
			return;
		
		if (!m_bUsingGamePad)
		{
			GamePadToggleSearch(false);
		}
		else 
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (!workspace) 
				return;
			
			workspace.SetFocusedWidget(m_SearchEditBox.GetRootWidget());
		}
	}
	
	protected void OnCardHover(Widget card, int prefabIndex, bool hover)
	{		
		if (!hover)
		{
			m_ContentBrowserManager.RefreshPreviewCost();
			return;
		}
		
		if (!m_bBudgetPreviewUpdateEnabled || m_bUsingGamePad) return;
		
		m_ContentBrowserManager.RefreshPreviewCost(prefabIndex);
		
	}
	
	protected void OnCardFocus(Widget card, int prefabIndex, bool focus)
	{			
		if (!m_bUsingGamePad) return;
		
		m_bFocusingAssetCard = focus;
		GamepadToggleSelectButtonSwap();
		
		if (m_bFocusingAssetCard)
		{
			m_wToggleCardFilterButton.SetLabel("#AR-Editor_ContentBrowser_SwitchToFilters");
		}
		else
		{
			m_wToggleCardFilterButton.SetLabel("#AR-Editor_ContentBrowser_SwitchToCards");
			m_ContentBrowserManager.RefreshPreviewCost();
			return;
		}
		
		if (card)
		{
			m_wFocusedCard = card;
			m_iCardFocusIndex = prefabIndex;
			m_ContentBrowserManager.RefreshPreviewCost(prefabIndex);
		}
	}
	
	protected void OnSearchFocusChanged(SCR_EditBoxComponent editBoxComponent, EditBoxWidget editBoxWidget, bool focus)
	{
		if (focus != m_bSearchIsToggled)
			m_bSearchIsToggled = focus	
	}
	
	protected void OnSearchConfirmed(SCR_EditBoxComponent editBox, string text)
	{
		string search = text.Trim();
		m_ContentBrowserManager.SetCurrentSearch(search);
		
		FilterEntries(true, false, true);
		
		//If using gamepad make sure search box is no longer selected
		if (m_bUsingGamePad && m_wFocusedCard)
			GamePadToggleSearch(false);
	}	
	
	//~ After Browser state is loaded
	protected void OnBrowserEntriesFiltered()
	{
		m_bAnimateEntries = true;
		SetPage(m_ContentBrowserManager.GetPageIndex(), true);
	}
	
	//~ After Browser state has reset filters
	protected void OnBrowserStateCleared()
	{
		m_SearchEditBox.SetValue(m_ContentBrowserManager.GetCurrentSearch());
		
		OnBrowserEntriesFiltered()
	}
	
	protected void OnMenuClosed()
	{
		if (m_ContentBrowserManager)
		{
			m_ContentBrowserManager.OnMenuClosed();
		}
	}
	
	protected void OnResetClicked()
	{
		m_LabelsUIComponent.OnResetClicked();
		m_ContentBrowserManager.ResetBrowserState(false);
	}
	
	protected void OnShowLoadingCircle(bool state = true)
	{
		if (m_bContentLoadingWidgetActive == state)
		{
			return;
		}
		m_ContentLoadingVisual.SetVisible(state);
		m_bContentLoadingWidgetActive = state;
		
		RefreshPage();
		
		m_ContentBrowserManager.SetSaveStateTabsEnabled(!state);
	}
	
	protected void OnApplyLabelChanges()
	{
		OnShowLoadingCircle(false);
		FilterEntries(true, false, true);
	}
	
	protected void InitializeLabels()
	{
		Widget labelsWidget = GetWidget().FindAnyWidget(m_sLabelComponentWidget);
		if (!labelsWidget) return;
		
		m_LabelsUIComponent = SCR_ContentBrowserFiltersEditorUIComponent.Cast(labelsWidget.FindHandler(SCR_ContentBrowserFiltersEditorUIComponent));		
		if (!m_LabelsUIComponent) return;
		
		m_LabelsUIComponent.SetContentBrowserCallbacks(m_ShowLoadingCircleCallback, m_LabelApplyCallback);
		
		Widget activeFilters = GetWidget().FindAnyWidget(m_sActiveFiltersName);
		if (activeFilters)
		{
			SCR_ContentBrowserActiveFiltersEditorUIComponent activeFilterComponent = SCR_ContentBrowserActiveFiltersEditorUIComponent.Cast(activeFilters.FindHandler(SCR_ContentBrowserActiveFiltersEditorUIComponent));
			
			if (activeFilterComponent)
				activeFilterComponent.Init(m_LabelsUIComponent);
		}
	}
	
	//~ Returns the UI Info of highest ordered label (in the highest ordered group)
	protected SCR_UIInfo GetHighestOrderedLabelUIInfo(notnull array<EEditableEntityLabel> labels)
	{
		EEditableEntityLabelGroup groupLabel;
	
		int newLabelOrder;
		int newLabelGroupOrder;
		
		int lowestGroupOrder = int.MAX;
		int lowestLabelOrder = int.MAX;

		EEditableEntityLabel highestOrderedLabel;
		
		foreach (EEditableEntityLabel label: labels)
        {
			m_ContentBrowserManager.GetLabelGroupType(label, groupLabel);
			newLabelGroupOrder = m_ContentBrowserManager.GetLabelGroupOrder(groupLabel);
			
			if (newLabelGroupOrder <= lowestGroupOrder)
			{
				newLabelOrder = m_ContentBrowserManager.GetLabelOrderInGroup(label);
				
				if (newLabelOrder < lowestLabelOrder)
				{
					lowestGroupOrder = newLabelGroupOrder;
					lowestLabelOrder = newLabelOrder;
					highestOrderedLabel = label;
				}
			}
        }
		
		SCR_UIInfo uiInfo;
		m_ContentBrowserManager.GetLabelUIInfo(highestOrderedLabel, uiInfo);
		return uiInfo;
	}
	
	protected override bool IsUnique()
	{
		return true;
	}
	
	protected override void OnButtonPrev()
	{
		int previousPageIndex = m_iCurrentPage - 1;
		if (SetPage(previousPageIndex))
		{
			m_ContentBrowserManager.SetPageIndex(previousPageIndex);
		}
	}
	
	protected override void OnButtonNext()
	{
		int nextPageIndex = m_iCurrentPage + 1;
		if (SetPage(nextPageIndex))
		{
			m_ContentBrowserManager.SetPageIndex(nextPageIndex);
		}
	}
	
	override protected void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		Widget toggleBetweenCardAndFilter = w.FindAnyWidget(m_sToggleCardFilterButton);
		
		if (toggleBetweenCardAndFilter)
		{
			ButtonActionComponent.GetOnAction(toggleBetweenCardAndFilter, false, 0).Insert(ToggleBetweenCardsAndFilters);
			m_wToggleCardFilterButton = SCR_InputButtonComponent.Cast(toggleBetweenCardAndFilter.FindHandler(SCR_InputButtonComponent));
		}
		
		// Disable budget preview update during loading, prevent OnCardHover callbacks during init
		m_bBudgetPreviewUpdateEnabled = false;
		
		if (SCR_Global.IsEditMode()) return;
		
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!m_EntityCore)
			return;
		
		m_ContentBrowserManager = SCR_ContentBrowserEditorComponent.Cast(SCR_ContentBrowserEditorComponent.GetInstance(SCR_ContentBrowserEditorComponent, true));
		if (!m_ContentBrowserManager)
			return;
		
		m_ContentBrowserManager.SetPageEntryCount(GetRows() * GetColumns());
		
		m_PlacingManager = SCR_PlacingEditorComponent.Cast(SCR_PlacingEditorComponent.GetInstance(SCR_PlacingEditorComponent, true, true));
		m_BudgetManager = SCR_BudgetEditorComponent.Cast(SCR_BudgetEditorComponent.GetInstance(SCR_BudgetEditorComponent, false, true));
		
		m_ContentBrowserDialog = EditorBrowserDialogUI.Cast(GetMenu());
		m_ContentBrowserDialog.GetOnMenuHide().Insert(OnMenuClosed);
		
		m_NoFilterResultsWidget = w.FindAnyWidget(m_sNoFilterResultsWidgetName);
		
		m_SelectCardButton = w.FindAnyWidget(m_sSelectCardButtonName);
		m_ToggleFilterdButton = w.FindAnyWidget(m_sToggleFilterdButtonName);
		
		Widget resetFiltersButton = w.FindAnyWidget(m_sResetFiltersButtonName);
		if (resetFiltersButton)
		{
			ButtonActionComponent.GetOnAction(resetFiltersButton, false, 0).Insert(OnResetClicked);		
		}
		
		m_SearchEditBox = SCR_EditBoxComponent.GetEditBoxComponent(m_sSearchEditBoxName, w, true);
		if (m_SearchEditBox)
		{
			m_SearchEditBox.m_OnConfirm.Insert(OnSearchConfirmed);
			m_SearchEditBox.m_OnFocusChangedEditBox.Insert(OnSearchFocusChanged);
			
			if (m_ContentBrowserManager.GetCurrentSearch())
			{
				m_SearchEditBox.SetValue(m_ContentBrowserManager.GetCurrentSearch());
			}
		}
		
		m_ContentLoadingVisual = w.FindAnyWidget(m_sContentLoadingWidgetName);
		if (!m_ContentLoadingVisual)
			Print("'SCR_ContentBrowserEditorUIComponent' cannot find 'm_ContentLoadingVisual'!", LogLevel.WARNING);
		
		m_ShowLoadingCircleCallback.Insert(OnShowLoadingCircle);
		m_LabelApplyCallback.Insert(OnApplyLabelChanges);
		
		m_wToggleSearchButton = w.FindAnyWidget(m_sToggleSearchButton);
		if (m_wToggleSearchButton)
			ButtonActionComponent.GetOnAction(m_wToggleSearchButton, false, 0).Insert(GamePadSwitchToggleSearch);
		
		//Input device changed
		OnInputDeviceIsGamepadScripted(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepadScripted);
		
		InitializeLabels();
		
		if (m_ContentBrowserManager.GetContentBrowserDisplayConfig())
		{
			string header = m_ContentBrowserManager.GetContentBrowserDisplayConfig().GetHeader();
			
			if (header != string.Empty)
			{
				TextWidget headerWidget = TextWidget.Cast(w.FindAnyWidget(m_sHeaderWidgetName));
				
				if (headerWidget)
					headerWidget.SetText(header);
			}
		}
		
		m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Insert(OnBrowserEntriesFiltered);
		m_ContentBrowserManager.GetOnBrowserStateCleared().Insert(OnBrowserStateCleared);
		m_ContentBrowserManager.RefreshPreviewCost();
		
		SCR_UISoundEntity.SoundEvent(m_sSfxOnOpenDialog, true);
		
		m_bAnimateEntries = true;
		
		SetPage(m_ContentBrowserManager.GetPageIndex());

		if (m_BudgetManager)
			m_BudgetManager.DemandBudgetUpdateFromServer();
	}
	
	override protected void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		//Input device changed
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepadScripted);
		
		if (SCR_Global.IsEditMode()) return;
		
		m_ShowLoadingCircleCallback.Remove(OnShowLoadingCircle);
		m_LabelApplyCallback.Remove(OnApplyLabelChanges);
		
		m_ContentBrowserDialog.GetOnMenuHide().Remove(OnMenuClosed);
		
		if (m_ContentBrowserManager)
		{
			m_ContentBrowserManager.SetExtendedEntity(null);

			//~ State loading
			m_ContentBrowserManager.GetOnBrowserEntriesFiltered().Remove(OnBrowserEntriesFiltered);
			m_ContentBrowserManager.GetOnBrowserStateCleared().Remove(OnBrowserStateCleared);
		}
		
		if (m_SearchEditBox)
		{
			m_SearchEditBox.m_OnConfirm.Remove(OnSearchConfirmed);
		}
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EEditableEntityType, "m_EntityType")]
class SCR_ContentBrowserEditorCard
{
	[Attribute(uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EEditableEntityType))]
	EEditableEntityType m_EntityType;
	
	[Attribute(params: "layout")]
	ResourceName m_sPrefab;
};