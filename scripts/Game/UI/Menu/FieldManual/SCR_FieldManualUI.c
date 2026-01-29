class SCR_FieldManualUI : MenuRootBase
{
	protected TextWidget m_wMenuTitle;
	protected VerticalLayoutWidget m_wMenuCategoryList;
	protected SCR_EditBoxSearchComponent m_MenuSearchbar;
	protected SCR_InputButtonComponent m_MenuBtnBack;

	protected Widget m_wReadingWidget;
	protected SCR_BreadCrumbsComponent m_BreadCrumbsComponent;
	protected TextWidget m_wPageEntryTitle;
	protected Widget m_wPageFrame;
	protected ScrollLayoutWidget m_wGridScrollLayoutWidget;
	protected GridLayoutWidget m_wMenuGridLayout;

	protected string m_sLastSearch;

	protected ref SCR_FieldManualConfigRoot m_ConfigRoot;
	protected ref array<ref SCR_FieldManualConfigEntry> m_aAllEntries;
	protected ref map<Widget, ref SCR_FieldManualConfigEntry> m_mWidgetEntryMap;
	protected ref map<Widget, ref SCR_FieldManualConfigCategory> m_mWidgetSubCategoryMap;
	protected ref SCR_FieldManualSubCategoryScriptedWidgetEventHandler m_SubCategoryButtonEventHandler;
	protected ref SCR_FieldManualEntryScriptedWidgetEventHandler m_EntryButtonEventHandler;
	protected Widget m_wLastClickedSubCategory;

	protected Widget m_wLoadingServerQueue;

	protected SCR_FieldManualConfigEntry m_CurrentEntry;

	protected ButtonWidget m_wFirstSubCategoryButton;
	protected Widget m_wLastSelectedWidget;

	protected bool m_bIsInEntryViewMode;
	protected bool m_bIsInSearchMode;
	protected bool m_bOpenedFromOutside;

	protected bool m_bArmaWarning;
	
	protected float m_fSliderYPos;

	protected static const int TILES_GRID_WIDTH = 5;
	protected static const string SEARCH_RESULT_KEY = "#AR-FieldManual_SearchResult";
	protected static const string SEARCH_NO_RESULT_KEY = "#AR-FieldManual_SearchResult_None";
	protected static const string SCROLL_NAME = "mainScroll";
	protected static const int ENTRY_INPUT_REFRESH_DELAY = 500;					// min time in ms before KBM/pad entry refreshes
	protected static const int ENTRY_SCROLL_INPUT_REFRESH_DELAY = 1000 / 60;	// time in ms between KBM/pad entry scroll refreshes

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuOpen()
	{
		super.OnMenuOpen();

		Widget rootWidget = GetRootWidget();
		SCR_ConfigUIComponent component = SCR_ConfigUIComponent.Cast(rootWidget.FindHandler(SCR_ConfigUIComponent));
		if (!component)
		{
			Print("No config component found | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
			Close();
			return;
		}

		SCR_FieldManualConfigRoot configRoot = SCR_FieldManualConfigLoader.LoadConfigRoot(component.m_ConfigPath);
		if (!configRoot)
		{
			Print("Could not load provided config | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
			Close();
			return;
		}

		m_ConfigRoot = configRoot;
		SetAllEntriesAndParents();

		// which background do you want, gents?
		bool isMainMenu = GetGame().m_bIsMainMenuOpen;
		Widget blur = GetRootWidget().FindAnyWidget("Blur");
		Widget backgroundImage = GetRootWidget().FindAnyWidget("MenuBackground");
		if (blur && isMainMenu)
			blur.RemoveFromHierarchy();

		if (backgroundImage && !isMainMenu)
			backgroundImage.RemoveFromHierarchy();

		Widget menuFrame = rootWidget.FindAnyWidget("MenuFrame");
		TextWidget menuTitle = TextWidget.Cast(menuFrame.FindAnyWidget("Title"));
		VerticalLayoutWidget menuCategoryList = VerticalLayoutWidget.Cast(rootWidget.FindAnyWidget("CategoryList"));
		GridLayoutWidget menuGridLayout = GridLayoutWidget.Cast(rootWidget.FindAnyWidget("GridLayout"));
		if (!menuFrame || !menuTitle || !menuCategoryList || !menuGridLayout)
		{
			Print("Important elements (menu frame & title, category list, grid layout) are missing | " + __FILE__ + ": " + __LINE__, LogLevel.WARNING);
			Close();
			return;
		}

		Widget readingWidget = rootWidget.FindAnyWidget("ReadingWidget");

		TextWidget entryTitle = TextWidget.Cast(readingWidget.FindAnyWidget("EntryTitle"));
		Widget entryFrame = readingWidget.FindAnyWidget("EntryFrame");
		if (!readingWidget || !entryTitle || !entryFrame)
		{
			Print(string.Format("missing one: reading widget (%1), entryTitle (%2), entryFrame (%3) | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, readingWidget != null, entryTitle != null, entryFrame != null), LogLevel.WARNING);
			Close();
			return;
		}

		SCR_InputButtonComponent backButtonMenuFrame = SCR_InputButtonComponent.GetInputButtonComponent(UIConstants.BUTTON_BACK, menuFrame);
		if (!backButtonMenuFrame || !backButtonMenuFrame.m_OnClicked)
		{
			Print(string.Format("missing menu frame's back button menu (%1) or m_OnClicked (%2) | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, backButtonMenuFrame != null, backButtonMenuFrame.m_OnClicked != null), LogLevel.WARNING);
			Close();
			return;
		}

		m_MenuBtnBack = backButtonMenuFrame;

		m_wMenuTitle = menuTitle;
		m_wMenuCategoryList = menuCategoryList;
		m_wMenuGridLayout = menuGridLayout;

		m_wReadingWidget = readingWidget;
		m_wPageEntryTitle = entryTitle;
		m_wPageFrame = entryFrame;

		// breadcrumbs is not mandatory
		m_BreadCrumbsComponent = SCR_BreadCrumbsComponent.Cast(SCR_BreadCrumbsComponent.GetComponent(SCR_BreadCrumbsComponent, "BreadCrumbs", rootWidget));

		// scrolling reset is not mandatory
		m_wGridScrollLayoutWidget = ScrollLayoutWidget.Cast(rootWidget.FindAnyWidget("GridScrollLayout"));

		// searchbar functionality is not mandatory
		m_MenuSearchbar = SCR_EditBoxSearchComponent.Cast(SCR_EditBoxSearchComponent.GetEditBoxComponent("Searchbar", rootWidget));
		if (m_MenuSearchbar && m_MenuSearchbar.m_OnConfirm)
				m_MenuSearchbar.m_OnConfirm.Insert(ProcessSearch);

		// Server Queue
		m_wLoadingServerQueue = rootWidget.FindAnyWidget("ServerQueue");
		HideQueueMessage();

		m_MenuBtnBack.m_OnActivated.Insert(CloseMenuOrReadingPanel);

#ifdef WORKBENCH
		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.AddActionListener(UIConstants.MENU_ACTION_OPEN_WB, EActionTrigger.DOWN, CloseMenuOrReadingPanel);
			inputManager.AddActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, CloseMenuOrReadingPanel);
		}
#endif // WORKBENCH

		m_SubCategoryButtonEventHandler = new SCR_FieldManualSubCategoryScriptedWidgetEventHandler(this);
		m_EntryButtonEventHandler = new SCR_FieldManualEntryScriptedWidgetEventHandler(this);
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnMenuShow()
	{
		super.OnMenuShow();

		CreateCategoryMenuWidgets();
		CloseReadingPanel();
		OpenFirstSubCategory();

		SCR_AnalyticsApplication.GetInstance().OpenFieldManual();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuHide()
	{	
		super.OnMenuHide();
			
		SCR_AnalyticsApplication.GetInstance().CloseFieldManual();
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	protected override void OnMenuClose()
	{
		super.OnMenuClose();

		InputManager inputManager = GetGame().GetInputManager();
		if (inputManager)
		{
			inputManager.RemoveActionListener(UIConstants.MENU_ACTION_OPEN_WB, EActionTrigger.DOWN, CloseMenuOrReadingPanel);
			inputManager.RemoveActionListener(UIConstants.MENU_ACTION_BACK_WB, EActionTrigger.DOWN, CloseMenuOrReadingPanel);
		}
	}
#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	//! \param[in] w
	void OnSubCategoryClicked(Widget w)
	{
		bool resetTiles = m_bIsInSearchMode || m_bIsInEntryViewMode || w != m_wLastClickedSubCategory;
		m_bIsInSearchMode = false;

		if (m_wLastClickedSubCategory)
			FlipButtonState(m_wLastClickedSubCategory, false);

		FlipButtonState(w, true);

		m_wLastClickedSubCategory = w;
		m_wLastSelectedWidget = null;
		CloseReadingPanel();
		ResetLastSearch();
		if (resetTiles)
			SetTilesByWidget(w);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] widget sub category button
	//! \param[in] bool toggle state
	protected bool FlipButtonState(Widget w, bool state)
	{
		ButtonWidget btnWidget = ButtonWidget.Cast(w);
		if (!btnWidget)
			return false;

		SCR_ModularButtonComponent btnModularComponent = SCR_ModularButtonComponent.Cast(btnWidget.FindHandler(SCR_ModularButtonComponent));
		if (!btnModularComponent)
			return false;

		btnModularComponent.SetToggled(state);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Open the clicked tile card
	//! \param[in] w
	void OnTileClicked(Widget w)
	{
		m_bIsInSearchMode = false;
		SetCurrentEntryByWidget(w);

		m_wLastSelectedWidget = w;

		FocusLastSelectedEntry();
	}

	//------------------------------------------------------------------------------------------------
	//! creates a tile and sets its image, background & text
	protected Widget CreateTileWidget(notnull SCR_FieldManualConfigEntry entry, notnull Widget parent)
	{
		Widget createdWidget = GetGame().GetWorkspace().CreateWidgets(m_ConfigRoot.m_MenuEntryTileLayout, parent);
		if (!createdWidget)
		{
			Print("could not create tile widget for entry | " + FilePath.StripPath(__FILE__) + ":" + __LINE__, LogLevel.WARNING);
			return null;
		}

		SCR_AssetCardFrontUIComponent cardComp = SCR_AssetCardFrontUIComponent.Cast(createdWidget.FindHandler(SCR_AssetCardFrontUIComponent));
		if (cardComp)
		{
			cardComp.SetImage(entry.m_Image);
			cardComp.SetBackgroundImage(m_ConfigRoot.m_aTileBackgrounds.GetRandomElement());
			cardComp.SetText(entry.m_sTitle);
		}

		return createdWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! sets entry tiles from a sub-category's widget
	// used in SCR_FieldManualSubCategoryScriptedWidgetEventHandler.OnClick
	protected void SetTilesByWidget(Widget widget)
	{
		SCR_FieldManualConfigCategory subCategory = m_mWidgetSubCategoryMap.Get(widget);

		if (!subCategory)
			return;

		if (m_BreadCrumbsComponent && subCategory.m_Parent)
			m_BreadCrumbsComponent.Set(subCategory.m_Parent.m_sTitle, subCategory.m_sTitle);

		SetTiles(subCategory.m_aEntries);
	}

	//------------------------------------------------------------------------------------------------
	//! sets main Field Manual grid tiles from entries
	protected void SetTiles(array<ref SCR_FieldManualConfigEntry> entries)
	{
		if (!m_wMenuGridLayout)
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wMenuGridLayout);

		if (!entries || entries.IsEmpty())
			return;

		m_mWidgetEntryMap.Clear();

		int column, line;
		Widget assetCard;
		ButtonWidget button;
		foreach (SCR_FieldManualConfigEntry entry : entries)
		{
			assetCard = CreateTileWidget(entry, m_wMenuGridLayout);
			if (!assetCard)
				continue;

			GridSlot.SetColumn(assetCard, column);
			GridSlot.SetRow(assetCard, line);

			column++;
			if (column >= TILES_GRID_WIDTH)
			{
				column = 0;
				line++;
			}

			button = ButtonWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(assetCard, "AssetCard0"));
			if (button)
			{
				button.AddHandler(m_EntryButtonEventHandler);
				m_mWidgetEntryMap.Insert(button, entry);
			}
		}

		if (m_wGridScrollLayoutWidget)
			m_wGridScrollLayoutWidget.SetSliderPos(0, 0);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets the current entry to read - can be null (leave reading mode) or any SCR_FieldManualConfigEntry
	protected void SetCurrentEntry(SCR_FieldManualConfigEntry entry)
	{
		if (!m_wPageFrame || !m_wPageEntryTitle)
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wPageFrame);

		m_bIsInEntryViewMode = entry != null;

		m_wGridScrollLayoutWidget.SetVisible(!m_bIsInEntryViewMode);
		m_wReadingWidget.SetVisible(m_bIsInEntryViewMode);

		
		if (!m_bIsInEntryViewMode)
		{
			if (m_BreadCrumbsComponent && m_CurrentEntry && m_CurrentEntry.m_Parent && m_CurrentEntry.m_Parent.m_Parent)
				m_BreadCrumbsComponent.Set(m_CurrentEntry.m_Parent.m_Parent.m_sTitle, m_CurrentEntry.m_Parent.m_sTitle);
			
			m_wGridScrollLayoutWidget.SetSliderPos(0, m_fSliderYPos);
			m_CurrentEntry = null;
			return;
		}
		
		float x;
		m_wGridScrollLayoutWidget.GetSliderPos(x, m_fSliderYPos);

		if (m_BreadCrumbsComponent && entry.m_Parent && entry.m_Parent.m_Parent)
			m_BreadCrumbsComponent.Set(entry.m_Parent.m_Parent.m_sTitle, entry.m_Parent.m_sTitle, entry.m_sTitle);

		m_CurrentEntry = entry;

		m_wPageEntryTitle.SetText(entry.m_sTitle);

		Widget readingWidget = entry.CreateWidget(m_wPageFrame);
		SCR_FieldManualConfigEntry_Weapon weaponEntry = SCR_FieldManualConfigEntry_Weapon.Cast(entry);
		if (weaponEntry)
			FillEntry_Weapon(readingWidget, weaponEntry);
	}

	//------------------------------------------------------------------------------------------------
	//! Add current entry's refresh to callqueue (used by KBM/Gamepad switching event)
	protected void QueueRefreshCurrentEntry()
	{
		if (!m_CurrentEntry || !m_CurrentEntry.CanRefresh())
			return;

		// safety to not saturate the call queue if someone has fun using pad and kbm at the same time
		GetGame().GetCallqueue().Remove(RefreshCurrentEntry); // in case it was added
		GetGame().GetCallqueue().CallLater(RefreshCurrentEntry, ENTRY_INPUT_REFRESH_DELAY, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Re-creates the current entry (called by callqueue added in QueueRefreshCurrentEntry)
	protected void RefreshCurrentEntry()
	{
		if (!m_CurrentEntry.CanRefresh())
			return;

		float x, y;
		ScrollLayoutWidget scroll = ScrollLayoutWidget.Cast(m_wPageFrame.FindWidget(SCROLL_NAME)); // a bit ugly?
		if (scroll)
			scroll.GetSliderPos(x, y);

		SetCurrentEntry(m_CurrentEntry);

		if (!scroll)
			return;

		scroll = ScrollLayoutWidget.Cast(m_wPageFrame.FindWidget(SCROLL_NAME));
		if (scroll)
			SetScrollAfterRefresh(scroll, x, y);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetScrollAfterRefresh(notnull ScrollLayoutWidget scroll, float x, float y)
	{
		float w, h;
		scroll.GetScreenSize(w, h);
		if (h <= 0) // ugly, but no other choice as setting scroll right after Widget creation does nothing
		{
			GetGame().GetCallqueue().CallLater(SetScrollAfterRefresh, ENTRY_SCROLL_INPUT_REFRESH_DELAY, false, scroll, x, y);
		}
		else
		{
			GetGame().GetCallqueue().Remove(SetScrollAfterRefresh); // safety, as spam occured
			scroll.SetSliderPos(x, y);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Opens an entry "from outside", e.g a hint
	//! \param[in] entryId if EFieldManualEntryId.NONE is used or if the entry is not found, the Field Manual is normally opened
	void OpenEntry(EFieldManualEntryId entryId)
	{
		if (entryId == EFieldManualEntryId.NONE)
		{
			SetCurrentEntry(null);
			return;
		}

		foreach (SCR_FieldManualConfigEntry entry : m_aAllEntries)
		{
			if (entry.m_eId == entryId)
			{
				m_bOpenedFromOutside = true;
				SetCurrentEntry(entry);
				return;
			}
		}

		SetCurrentEntry(null);
	}

	//------------------------------------------------------------------------------------------------
	//! sets the read entry linked from that widget
	protected void SetCurrentEntryByWidget(Widget widget)
	{
		SCR_FieldManualConfigEntry entry = m_mWidgetEntryMap.Get(widget);
		if (!entry)
			return;

		SetCurrentEntry(entry);
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon(notnull Widget widget, notnull SCR_FieldManualConfigEntry_Weapon entry)
	{
		Widget weaponRender = widget.FindAnyWidget("weaponRender");
		if (true /* remove later */ || !FillEntry_Weapon_Render(RenderTargetWidget.Cast(weaponRender), entry.m_sWeaponEntityPath))
			if (weaponRender)
				weaponRender.RemoveFromHierarchy();

		FillEntry_Weapon_Statistics(widget.FindAnyWidget("statsLayout"), entry);
	}

	//------------------------------------------------------------------------------------------------
	protected bool FillEntry_Weapon_Render(RenderTargetWidget renderTarget, ResourceName weaponResourceName)
	{
		if (!renderTarget || !weaponResourceName)
			return false;

		Resource resource = Resource.Load(weaponResourceName);
		IEntity entity = GetGame().SpawnEntityPrefabLocal(resource, null, null);
		if (!entity)
			return false;

		ChimeraWorld world = ChimeraWorld.CastFrom(GetGame().GetWorld());
		if (!world)
			return false;

		ItemPreviewManagerEntity manager = world.GetItemPreviewManager();
		if (!manager)
			return false;

		ItemPreviewWidget renderPreview = ItemPreviewWidget.Cast(renderTarget);
		if (!renderPreview)
			return false;

		manager.SetPreviewItem(renderPreview, entity);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics(Widget statsLayout, SCR_FieldManualConfigEntry_Weapon entry)
	{
		if (!statsLayout)
			return;

		SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper = entry.m_WeaponStatsHelper;
		if (!weaponStatsHelper)
		{
			SCR_WidgetHelper.RemoveAllChildren(statsLayout);
			return;
		}

		FillEntry_Weapon_Statistics_Mass(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_SightAdjustments(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_DefaultSightAdjustment(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_RateOfFire(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_MuzzleVelocity(statsLayout, weaponStatsHelper);
		FillEntry_Weapon_Statistics_FireModes(statsLayout, weaponStatsHelper);
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_Mass(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsMass = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsWeight");
		if (!statsMass)
			return;

		float mass = weaponStatsHelper.GetMass();
		if (mass < 0)
			statsMass.RemoveWidgetFromHierarchy();
		else
			statsMass.SetTranslatedValue("#AR-ValueUnit_Short_Kilograms", mass.ToString(-1, 2));
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_SightAdjustments(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsSightAdjustments = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsSightAdjustments");
		if (!statsSightAdjustments)
			return;

		array<int> sightDistanceSettings = weaponStatsHelper.GetSightDistanceSettings();
		if (!sightDistanceSettings || sightDistanceSettings.IsEmpty())
		{
			statsSightAdjustments.RemoveWidgetFromHierarchy();
			return;
		}

		array<string> settingsStr = {};
		foreach (int sightDistanceSetting : sightDistanceSettings)
		{
			settingsStr.Insert(WidgetManager.Translate("#AR-ValueUnit_Short_Meters", sightDistanceSetting));
		}

		statsSightAdjustments.SetValue(ArrayJoin(settingsStr));
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_DefaultSightAdjustment(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsDefaultSightAdjustment = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsDefaultSightAdjustment");
		if (!statsDefaultSightAdjustment)
			return;

		int defaultSightDistanceSetting = weaponStatsHelper.GetDefaultSightDistanceSetting();
		if (defaultSightDistanceSetting < 0)
			statsDefaultSightAdjustment.RemoveWidgetFromHierarchy();
		else
			statsDefaultSightAdjustment.SetTranslatedValue("#AR-ValueUnit_Short_Meters", defaultSightDistanceSetting.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_RateOfFire(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsRateOfFire = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsRateOfFire");
		if (!statsRateOfFire)
			return;

		if (weaponStatsHelper.GetRateOfFire() < 1)
			statsRateOfFire.RemoveWidgetFromHierarchy();
		else
			statsRateOfFire.SetTranslatedValue("#AR-ValueUnit_Short_RoundsPerMinute", weaponStatsHelper.GetRateOfFire().ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_MuzzleVelocity(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsMuzzleVelocity = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsMuzzleVelocity");
		if (!statsMuzzleVelocity)
			return;

		if (weaponStatsHelper.m_iMuzzleVelocity < 1)
				statsMuzzleVelocity.RemoveWidgetFromHierarchy();
		else
			statsMuzzleVelocity.SetTranslatedValue("#AR-ValueUnit_Short_MetersPerSeconds", weaponStatsHelper.m_iMuzzleVelocity.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void FillEntry_Weapon_Statistics_FireModes(notnull Widget statsLayout, notnull SCR_FieldManualUI_WeaponStatsHelper weaponStatsHelper)
	{
		SCR_FieldManual_StatisticsLineComponent statsFireModes = SCR_FieldManual_StatisticsLineComponent.GetComponent(statsLayout, "statsFireModes");
		if (!statsFireModes)
			return;

		array<string> fireModes = weaponStatsHelper.GetFireModes();
		if (!fireModes || fireModes.IsEmpty())
				statsFireModes.RemoveWidgetFromHierarchy();
		else
			statsFireModes.SetValue(ArrayJoin(fireModes));
	}

	//------------------------------------------------------------------------------------------------
	//! one-time call that goes through all categories and entries and adds enabled entries to a one-level array
	// TODO: break down in smaller methods?
	protected void SetAllEntriesAndParents()
	{
		m_aAllEntries.Clear();
		array<SCR_FieldManualConfigCategory> categoriesToRemove = {};
		array<SCR_FieldManualConfigCategory> subCategoriesToRemove = {};
		array<SCR_FieldManualConfigEntry> entriesToRemove = {};
		foreach (SCR_FieldManualConfigCategory category : m_ConfigRoot.m_aCategories)
		{
			if (!category.m_bEnabled)
			{
				categoriesToRemove.Insert(category);
				continue;
			}

			subCategoriesToRemove.Clear();
			foreach (SCR_FieldManualConfigCategory subCategory : category.m_aCategories) // only two levels are supported by the UI for now
			{
				if (!subCategory.m_bEnabled)
				{
					subCategoriesToRemove.Insert(subCategory);
					continue;
				}

				entriesToRemove.Clear();
				foreach (SCR_FieldManualConfigEntry entry : subCategory.m_aEntries)
				{
					if (!entry.m_bEnabled || entry.m_aContent.IsEmpty())
					{
						entriesToRemove.Insert(entry);
						continue;
					}

					entry.m_Parent = subCategory;
					m_aAllEntries.Insert(entry);
				}

				foreach (SCR_FieldManualConfigEntry entry : entriesToRemove)
				{
					subCategory.m_aEntries.RemoveItemOrdered(entry);
				}

				if (subCategory.m_aEntries.IsEmpty() && subCategory.m_aCategories.IsEmpty())
				{
					subCategoriesToRemove.Insert(subCategory);
					continue;
				}

				subCategory.m_Parent = category;
			}

			foreach (SCR_FieldManualConfigCategory subCategory : subCategoriesToRemove)
			{
				category.m_aCategories.RemoveItemOrdered(subCategory);
			}

			if (category.m_aEntries.IsEmpty() && category.m_aCategories.IsEmpty())
				categoriesToRemove.Insert(category);
		}

		foreach (SCR_FieldManualConfigCategory category : categoriesToRemove)
		{
			m_ConfigRoot.m_aCategories.RemoveItemOrdered(category);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Create main categories buttons list
	protected void CreateCategoryMenuWidgets()
	{
		if (!m_ConfigRoot || !m_ConfigRoot.m_aCategories || !m_wMenuCategoryList)
			return;

		m_wFirstSubCategoryButton = null;
		m_mWidgetSubCategoryMap.Clear();
		SCR_WidgetHelper.RemoveAllChildren(m_wMenuCategoryList);

		if (m_wMenuTitle)
			m_wMenuTitle.SetText(m_ConfigRoot.m_sTitle);

		Widget subCategoryWidget;
		ButtonWidget button;
		foreach (SCR_FieldManualConfigCategory category : m_ConfigRoot.m_aCategories)
		{
			category.CreateWidget(m_wMenuCategoryList);

			foreach (SCR_FieldManualConfigCategory subCategory : category.m_aCategories)
			{
				subCategoryWidget = subCategory.CreateWidget(m_wMenuCategoryList);
				button = ButtonWidget.Cast(subCategoryWidget);
				if (button)
				{
					button.AddHandler(m_SubCategoryButtonEventHandler);
					m_mWidgetSubCategoryMap.Insert(button, subCategory);

					if (!m_wFirstSubCategoryButton)
						m_wFirstSubCategoryButton = button;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Open the first sub-category, showing its entries
	protected void OpenFirstSubCategory()
	{
		OnSubCategoryClicked(m_wFirstSubCategoryButton);
		GetGame().GetCallqueue().CallLater(GetGame().GetWorkspace().SetFocusedWidget, 0, false, m_wFirstSubCategoryButton, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Open the last selected Sub Category and selected it
	protected void OpenLastSelectedSubCategory()
	{
		OnSubCategoryClicked(m_wLastClickedSubCategory);
		GetGame().GetCallqueue().CallLater(FocusLastSelectedEntry, 0, false, m_wLastSelectedWidget, false);
	}

	//------------------------------------------------------------------------------------------------
	//! Focus the last selected valid widget
	protected void FocusLastSelectedEntry(Widget w = null)
	{
		if (w)
			m_wLastSelectedWidget = w;

		if (m_wLastSelectedWidget)
			GetGame().GetWorkspace().SetFocusedWidget(m_wLastSelectedWidget);
	}

	//------------------------------------------------------------------------------------------------
	//! resets last search value
	// used in SCR_FieldManualSubCategoryScriptedWidgetEventHandler.OnClick
	protected void ResetLastSearch()
	{
		m_sLastSearch = string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//! the search process itself - finds entries and set tiles
	protected void ProcessSearch(SCR_EditBoxSearchComponent searchbox, string search)
	{
		search = search.Trim();

		if (search.IsEmpty())
		{			
			ResetLastSearch();
			if (m_bIsInSearchMode)
				OpenLastSelectedSubCategory();

			return;
		}
		
		if (m_wLastClickedSubCategory)
			FlipButtonState(m_wLastClickedSubCategory, false);

		CloseReadingPanel();

		if (!m_bArmaWarning && search.Contains("ArmA"))
		{
			m_bArmaWarning = true;

			if (m_BreadCrumbsComponent)
				m_BreadCrumbsComponent.Set("#AR-FieldManual_Page_EasterEgg_ArmA_BreadCrumbs");

			SCR_FieldManualConfigEntry_Standard ArmaEntry = new SCR_FieldManualConfigEntry_Standard();
			ArmaEntry.m_sTitle = "#AR-FieldManual_Page_EasterEgg_ArmA_Title";
			SCR_FieldManualPiece_Text ArmaPiece = new SCR_FieldManualPiece_Text();
			ArmaPiece.m_sText = "#AR-FieldManual_Page_EasterEgg_ArmA_Text";
			ArmaEntry.m_aContent.Insert(ArmaPiece);
			SetCurrentEntry(ArmaEntry);

			EditBoxWidget searchEditBox = EditBoxWidget.Cast(GetRootWidget().FindAnyWidget("Searchbar").FindAnyWidget("EditBox"));
			string searchText = searchEditBox.GetText();
			searchText.Replace("ArmA", "Arma");
			searchEditBox.SetText(searchText);

			ResetLastSearch();

			return;
		}

		m_bIsInSearchMode = true;

		string originalSearch = search;
		search.ToLower();
		if (search == m_sLastSearch)
			return;

		m_sLastSearch = search;

		array<ref SCR_FieldManualConfigEntry> entries = {};

		if (!m_sLastSearch.IsEmpty())
		{
			array<int> indexList = {};

			array<string> allEntriesKeys = {};
			foreach (SCR_FieldManualConfigEntry entry : m_aAllEntries)
			{
				allEntriesKeys.Insert(entry.m_sTitle);
			}

			WidgetManager.SearchLocalized(m_sLastSearch, allEntriesKeys, indexList);

			foreach (int index : indexList)
			{
				entries.Insert(m_aAllEntries.Get(index));
			}
		}

		if (m_BreadCrumbsComponent)
		{
			if (entries.IsEmpty())
				m_BreadCrumbsComponent.SetRichFormat(SEARCH_NO_RESULT_KEY, originalSearch);
			else
				m_BreadCrumbsComponent.SetRichFormat(SEARCH_RESULT_KEY, originalSearch, entries.Count().ToString());
		}

		SetTiles(entries);
	}

	//------------------------------------------------------------------------------------------------
	//! closes the reading panel (sets the current entry to null)
	protected void CloseReadingPanel()
	{
		SetCurrentEntry(null);
	}

	//------------------------------------------------------------------------------------------------
	//! determines what happens when ESC is pressed
	//! - if in reading mode, closes the reading panel
	//! - if in main view, closes the Field Manual
	protected void CloseMenuOrReadingPanel()
	{
		if (m_bIsInEntryViewMode && !m_bOpenedFromOutside)
			CloseReadingPanel();
		else
			Close();
	}

	//------------------------------------------------------------------------------------------------
	//! Show the Server Queue
	void ShowQueueMessage()
	{
		if (m_wLoadingServerQueue)
			m_wLoadingServerQueue.SetVisible(true);
	}

	//------------------------------------------------------------------------------------------------
	//! Hide the Server Queue
	void HideQueueMessage()
	{
		if (m_wLoadingServerQueue)
			m_wLoadingServerQueue.SetVisible(false);
	}

	//------------------------------------------------------------------------------------------------
	//----- STATIC METHODS
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	//! opens the first entry matching the EFieldManualEntryId enum value
	static SCR_FieldManualUI Open(EFieldManualEntryId entryId)
	{
		SCR_FieldManualUI ui = SCR_FieldManualUI.Cast(GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.FieldManualDialog));
		ui.OpenEntry(entryId);
		return ui;
	}

	//------------------------------------------------------------------------------------------------
	//----- my little helper!
	//------------------------------------------------------------------------------------------------
	//! joins strings with a stringtable separator - from 0 to 5 entries
	protected static string ArrayJoin(notnull array<string> entries)
	{
		int count = entries.Count();
		switch (count)
		{
			case 0: return "-";
			case 1: return entries[0];
			case 2: return WidgetManager.Translate("#AR-General_List_Short_2", entries[0], entries[1]);
			case 3: return WidgetManager.Translate("#AR-General_List_Short_3", entries[0], entries[1], entries[2]);
			case 4: return WidgetManager.Translate("#AR-General_List_Short_4", entries[0], entries[1], entries[2], entries[3]);
			case 5: return WidgetManager.Translate("#AR-General_List_Short_5", entries[0], entries[1], entries[2], entries[3], entries[4]);
		}
		Print(string.Format("Too many list entries to join as a string - missing #AR-General_List_Short_%1 translation?", count), LogLevel.WARNING);
		return WidgetManager.Translate("#AR-General_List_Short_5", entries[0], entries[1], entries[2], entries[3], entries[4]);
	}

	//------------------------------------------------------------------------------------------------
	//----- CONSTRUCTOR / DESTRUCTOR
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualUI()
	{
		m_aAllEntries = {};
		m_mWidgetEntryMap = new map<Widget, ref SCR_FieldManualConfigEntry>();
		m_mWidgetSubCategoryMap = new map<Widget, ref SCR_FieldManualConfigCategory>();

		ScriptInvoker inputDeviceIsGamepadInvoker = GetGame().OnInputDeviceIsGamepadInvoker();
		if (inputDeviceIsGamepadInvoker)
			inputDeviceIsGamepadInvoker.Insert(QueueRefreshCurrentEntry);
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_FieldManualUI()
	{
		ScriptInvoker inputDeviceIsGamepadInvoker = GetGame().OnInputDeviceIsGamepadInvoker();
		if (inputDeviceIsGamepadInvoker)
			inputDeviceIsGamepadInvoker.Remove(QueueRefreshCurrentEntry);
	}
}
