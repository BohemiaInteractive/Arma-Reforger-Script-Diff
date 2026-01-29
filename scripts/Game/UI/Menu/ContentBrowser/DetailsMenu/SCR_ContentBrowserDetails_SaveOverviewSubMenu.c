class SCR_ContentBrowserDetails_SaveOverviewSubMenu : SCR_ContentBrowserDetails_OverviewSubMenu
{
	protected const string DELETE_PUBLISHED_BUTTON = "m_DeletePublishedButton";
	
	protected WorldSaveItem m_SaveItem;
	
	protected SCR_ModularButtonComponent m_DeletePublishedButton;
	
	protected ref ScriptInvokerWorkshopItem m_OnItemDeleted;
	
	//------------------------------------------------------------------------------------------------
	// Override
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnTabCreate(Widget menuRoot, ResourceName buttonsLayout, int index)
	{
		super.OnTabCreate(menuRoot, buttonsLayout, index);

		// Setup UI - can be as part of generated widget class 
		Widget deletePublishedButton = menuRoot.FindAnyWidget(DELETE_PUBLISHED_BUTTON);
		if (deletePublishedButton)
		{
			m_DeletePublishedButton = SCR_ModularButtonComponent.Cast(deletePublishedButton.FindHandler(SCR_ModularButtonComponent));
			m_DeletePublishedButton.m_OnClicked.Insert(OnDeletePublishedClick);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void SetWorkshopItem(SCR_WorkshopItem item)
	{
		m_SaveItem = WorldSaveItem.Cast(item.GetWorkshopItem());
		super.SetWorkshopItem(item);
		
		bool canDelete = m_SaveItem.IsAuthor();
		
		if (m_DeletePublishedButton)
			m_DeletePublishedButton.SetVisible(canDelete);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool Play(MissionWorkshopItem scenario)
	{
		string id = m_SaveItem.Id();
		/*
		string fileName = GetGame().GetSaveManager().FindFileNameById(id);
		if (!fileName)
		{
			Print("Save to play was not found", LogLevel.WARNING);
			return;
		}
		
		GetGame().GetSaveManager().SetFileNameToLoad(fileName);
		super.Play(scenario);*/
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	override bool Continue(MissionWorkshopItem scenario)
	{
		return Play(scenario);
	}

	//------------------------------------------------------------------------------------------------
	override protected void UpdateScenarioLines()
	{
		if (!m_Item)
			return;
		
		foreach (SCR_ContentBrowser_ScenarioLineComponent scenarioLine : m_aScenarioLines)
		{
			if (!scenarioLine)
				continue;
			
			// Show favorites and mouse buttons only when downloaded
			scenarioLine.NotifyScenarioUpdate(m_Item.GetOffline());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool CreateLines(array<MissionWorkshopItem> scenarios, Widget parent)
	{
		Widget w = GetGame().GetWorkspace().CreateWidgets(m_sLinesLayout, parent);
		
		SCR_ContentBrowser_GMSaveLineComponent comp = SCR_ContentBrowser_GMSaveLineComponent.Cast(SCR_ContentBrowser_ScenarioLineComponent.FindComponent(w));
		if (!comp)
			return false;

		comp.SetSaveItem(m_SaveItem);
		//m_aScenarioLines.Insert(comp);

		comp.GetOnFavorite().Insert(OnLineFavorite);
		comp.GetOnMouseInteractionButtonClicked().Insert(OnInteractionButtonPressed);
		comp.GetOnFocus().Insert(OnLineFocus);
		comp.GetOnFocusLost().Insert(OnLineFocusLost);
		comp.GetOnMouseEnter().Insert(OnLineMouseEnter);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateSidePanel()
	{
		super.UpdateSidePanel();
		
		m_Widgets.m_wAddonDetailsPanel.SetVisible(true);
		m_Widgets.m_wScenarioDetailsPanel.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	// Custom
	//------------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Use for deleting published save from workshop
	protected void OnDeletePublishedClick()
	{
		SCR_ConfigurableDialogUi dialog = SCR_SaveWorkshopManagerUI.CreateDialog("delete_published");
		dialog.m_OnConfirm.Insert(DeletePublished);
	}
	
	//------------------------------------------------------------------------------------------------
	void DeletePublished()
	{
		/*SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		saveWorkshopManager.GetDeletePublishedCallback().SetOnSuccess(OnDeletePublishedResponse);
		saveWorkshopManager.GetDeletePublishedCallback().SetOnError(OnDeletePublishedError);
		saveWorkshopManager.DeletePublishedSave(m_SaveItem);
		
		m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();*/
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeletePublishedResponse()
	{	
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		if (m_OnItemDeleted)
			m_OnItemDeleted.Invoke(m_Item.GetWorkshopItem());
		
		GetGame().GetMenuManager().CloseMenuByPreset(ChimeraMenuPreset.ContentBrowserDetailsMenuSave);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDeletePublishedError()
	{
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();

		SCR_CommonDialogs.CreateRequestErrorDialog();
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerWorkshopItem GetItemDeleted()
	{
		if (!m_OnItemDeleted)
			m_OnItemDeleted = new ScriptInvokerWorkshopItem();
		
		return m_OnItemDeleted;
	}
}