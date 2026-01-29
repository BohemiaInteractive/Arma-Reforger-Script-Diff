/*!
Base dialog class for save and load dialogs handling
Fill data, display list, select entries, load, delete
*/
class SCR_SaveDialogUIComponent : SCR_ScriptedWidgetComponent
{
	[Attribute()]
	protected bool m_bIsLoad;

	[Attribute()]
	protected bool m_bCurrentMissionOnly;

	[Attribute("false", UIWidgets.CheckBox)]
	protected bool m_bVerboseDate;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sCreateLayout;

	[Attribute(uiwidget: UIWidgets.ResourceNamePicker, params: "layout")]
	protected ResourceName m_sEntryLayout;

	[Attribute("session_save_override")]
	protected string m_sConfirmPrompt;

	[Attribute("session_delete")]
	protected string m_sDeletePrompt;

	[Attribute("session_load_bad_version")]
	protected string m_sLoadBadVersionPrompt;

	[Attribute("session_load_bad_addons")]
	protected string m_sLoadBadAddonsPrompt;
	
	[Attribute("save-published")]
	protected string m_sDownloadIconName;
	
	protected ref map<Widget, SCR_SaveLoadEntryComponent> m_mComponentEntries = new map<Widget, SCR_SaveLoadEntryComponent>();
	protected ref array<Widget> m_aEntriesHidden = {};
	protected ref array<Widget> m_aEntriesToShow = {};
	
	protected ref SCR_EditorSaveDialogWidgets m_Widgets = new SCR_EditorSaveDialogWidgets();

	protected Widget m_wSelectedWidget;
	protected SaveGame m_SelectedSave;

	protected SCR_ConfigurableDialogUi m_ConfirmPrompt;
	protected SCR_ConfigurableDialogUi m_DeletePrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadVersionPrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadAddonsPrompt;
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	
	protected float m_fSliderPosY;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Control buttons
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void OnClose(SCR_InputButtonComponent button, string actionName)
	{
		CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnConfirm(SCR_InputButtonComponent button, string actionName);

	//------------------------------------------------------------------------------------------------
	protected void OnConfirmPrompt()
	{
		if (m_bIsLoad)
		{
			LoadEntry();
			return;
		}

		SaveEntry();
		CloseMenu();
	}

	//------------------------------------------------------------------------------------------------
	//! Callback on clicking delete button or actoin
	protected void OnDelete(SCR_InputButtonComponent button, string actionName)
	{
		if (!m_SelectedSave)
			return;
		
		// Open delete dialog
		m_DeletePrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sDeletePrompt); //--- ToDo: Unique tag
		m_DeletePrompt.m_OnConfirm.Insert(OnDeletePrompt);

		// Setup string
		const string displayName = GetSaveDisplayName(m_SelectedSave);
		m_DeletePrompt.SetTitle(displayName);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeletePrompt()
	{
		SelectEntry(null, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void RemoveSaveEntry(notnull SaveGame save)
	{
		foreach (Widget widget, SCR_SaveLoadEntryComponent entry : m_mComponentEntries)
		{
			if (entry.GetSaveData() == save)
			{
				widget.RemoveFromHierarchy();
				m_mComponentEntries.Remove(widget);
				UpdateButtons();
				return;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void CloseMenu()
	{
		//--- Confirmation prompt opened, ignore
		if (GetGame().GetWorkspace().GetModal() != m_wRoot)
			return;

		MenuBase menu = MenuBase.Cast(m_wRoot.FindHandler(MenuBase));
		menu.Close();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Main operations
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void SaveEntry();

	//------------------------------------------------------------------------------------------------
	protected void LoadEntry();

	//------------------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadResponse(BackendCallback callback)
	{
		CloseMenu();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadError(BackendCallback callback)
	{
		SCR_CommonDialogs.CreateRequestErrorDialog();

		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();

		CloseMenu();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Interaction
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void SelectEntry(Widget w, SCR_SaveLoadEntryComponent entryComponent)
	{		
		SCR_SaveLoadEntryComponent previousEntryComponent;
		if (m_mComponentEntries.Find(m_wSelectedWidget, previousEntryComponent))
		{
			previousEntryComponent.GetOnDeleteSave().Remove(OnDelete);
			previousEntryComponent.GetOnLoadSave().Remove(OnConfirm);
			previousEntryComponent.GetOnOverrideSave().Remove(OnConfirm);
		}

		m_wSelectedWidget = w;

		if (entryComponent)
		{
			m_SelectedSave = entryComponent.GetSaveData();
			entryComponent.GetOnDeleteSave().Insert(OnDelete);
			entryComponent.GetOnLoadSave().Insert(OnConfirm);
			entryComponent.GetOnOverrideSave().Insert(OnConfirm);
		}

		UpdateButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons();

	//------------------------------------------------------------------------------------------------
	protected void OnFrame()
	{
		//--- Ignore if all entries were shown
		if (m_aEntriesHidden.IsEmpty())
		{
			GetGame().GetCallqueue().Remove(OnFrame);
			return;
		}

		float sliderPosX, sliderPosY;
		m_Widgets.m_wSaveScroller.GetSliderPos(sliderPosX, sliderPosY);

		if (sliderPosY != m_fSliderPosY)
		{
			float scrollPosX, scrollPosY, scrollSizeW, scrollSizeH;
			m_Widgets.m_wSaveScroller.GetScreenPos(scrollPosX, scrollPosY);
			m_Widgets.m_wSaveScroller.GetScreenSize(scrollSizeW, scrollSizeH);

			//--- Widget not loaded yet, terminate
			if (scrollSizeH == 0)
				return;

			//--- Find widgets in view
			foreach (int i, Widget entryWidget : m_aEntriesHidden)
			{
				//--- Already shown
				if (entryWidget.GetOpacity() > 0)
					continue;

				float posX, posY, sizeW, sizeH;
				entryWidget.GetScreenPos(posX, posY);
				entryWidget.GetScreenSize(sizeW, sizeH);

				if ((posY + sizeH) > scrollPosY && posY < (scrollPosY + scrollSizeH))
				{
					//--- When the entry is in the scrolled view, mark it for showing
					if (!m_aEntriesToShow.Contains(entryWidget))
						m_aEntriesToShow.Insert(entryWidget);
				}
				else
				{
					//--- When the entry is not in the scrolled view, removing it from queue of entries to show again (to prioritize entries that are actually shown)
					m_aEntriesToShow.RemoveItemOrdered(entryWidget);
				}
			}
		}

		//--- Process the queue of entries to show, with one entry per frame
		if (!m_aEntriesToShow.IsEmpty())
			DisplaySaveEntries();

		m_fSliderPosY = sliderPosY;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DisplaySaveEntries()
	{
		Widget entryWidget = m_aEntriesToShow[0];
		
		SCR_SaveLoadEntryComponent entryComponent;
		m_mComponentEntries.Find(entryWidget, entryComponent);
		if (entryWidget.GetOpacity() == 0)
		{
			int y, m, d, hh, mm, s;
			SaveGame saveEntry = entryComponent.GetSaveData();
			saveEntry.GetSavePointCreatedLocalDateTime(y, m, d, hh, mm, s);

			if (false) // TODO: Workshop sharing support
			{
				entryComponent.SetIsDownloaded(true);
				entryComponent.SetSaveIcon(Color.FromInt(Color.WHITE), m_sDownloadIconName);
			}

			entryComponent.SetDisplayName(GetSaveDisplayName(saveEntry));
			entryComponent.SetDateTime(SCR_DateTimeHelper.GetDateString(d, m, y, m_bVerboseDate), SCR_FormatHelper.GetTimeFormattingHoursMinutes(hh, mm));		
			entryComponent.SetVersion(saveEntry.GetSavePointGameVersion(), saveEntry.IsSavePointGameVersionCompatible());
			entryComponent.SetCanLoad(m_bIsLoad);
			entryComponent.SetCanSave(!m_bIsLoad);
			entryComponent.SetCanDelete(true);
			
			if (!saveEntry.AreSavePointAddonsCompatible())
				entryComponent.SetSaveIcon(UIColors.WARNING, m_sDownloadIconName);
			else
				entryComponent.SetSaveIcon(Color.FromInt(Color.WHITE), "save");
			
			/*
			ResourceName headerResourceName = saveEntry.GetMissionResource();
			if (!headerResourceName.IsEmpty())
			{
				Resource missionHeaderResource = Resource.Load(headerResourceName);
				if (missionHeaderResource.IsValid())
				{
					SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(BaseContainerTools.CreateInstanceFromContainer(missionHeaderResource.GetResource().ToBaseContainer()));
					
					entryComponent.SetScenarioDataVisible(true);
					entryComponent.SetScenarioName(missionHeader.m_sName);

					if (missionHeader.m_sIcon)
						entryComponent.SetScenarioIcon(Color.FromInt(Color.WHITE), missionHeader.m_sIcon);
				}
			}
			*/

			AnimateWidget.Opacity(entryWidget, 1, 3);
		}
		else if (entryWidget.GetOpacity() > 0)
		{
			//--- Started fading in, move on to the next entry
			m_aEntriesToShow.RemoveOrdered(0);
			m_aEntriesHidden.RemoveItemOrdered(entryWidget);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected string GetSaveDisplayName(SCR_SaveLoadEntryComponent saveEntry)
	{
		if (!saveEntry)
			return string.Empty;

		return GetSaveDisplayName(saveEntry.GetSaveData());
	}
	
	//------------------------------------------------------------------------------------------------
	protected string GetSaveDisplayName(SaveGame save)
	{
		if (!save )
			return string.Empty;

		return save.GetSavePointName();
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
		{
			//! DEBUG
			Widget parent = w.FindWidget("SizeBase.DialogBase.VerticalLayout.Content.ContentSizeConstraints.ContentVerticalLayout.ContentLayoutContainer.SavesContent.SaveEntryList.m_SaveScroller.m_SaveList");
			for (int i = 0; i < 10; i++)
			{
				GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent);
			}
			return;
		}
		
		m_wRoot = w;
		m_Widgets.Init(w);
		m_fSliderPosY = -1;

		m_Widgets.m_SaveNameInputComponent0.m_OnChanged.Insert(UpdateButtons);

		m_Widgets.m_ExitButtonComponent.m_OnActivated.Insert(OnClose);
		m_Widgets.m_DeleteButtonComponent.m_OnActivated.Insert(OnDelete);
		m_Widgets.m_OverrideButtonComponent.m_OnActivated.Insert(OnConfirm);
		m_Widgets.m_ConfirmButtonComponent.m_OnActivated.Insert(OnConfirm);

		UpdateButtons();

		SaveGameManager manager = GetGame().GetSaveGameManager();
		manager.RetrieveSaveGameInfo({manager.GetCurrentMissionResource()}, new SaveGameOperationCb(OnSavesLoaded));
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSavesLoaded(bool success)
	{
		//--- Create new entries
		array<SaveGame> saves();
		SaveGameManager manager = GetGame().GetSaveGameManager();
		manager.GetSaves(saves, manager.GetCurrentMissionResource());

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		Widget entryWidget;
		SCR_ModularButtonComponent entryButton;
		SCR_SaveLoadEntryComponent entryCompoment;
		SCR_RewindComponent rewind = SCR_RewindComponent.GetInstance();
		foreach (int idx, SaveGame save : saves)
		{
			if (rewind && rewind.IsRewindPoint(save))
				continue;
	
			entryWidget = workspace.CreateWidgets(m_sEntryLayout, m_Widgets.m_wSaveList);
			entryButton = SCR_ModularButtonComponent.Cast(entryWidget.FindHandler(SCR_ModularButtonComponent));
			if (entryButton)
			{
				entryButton.m_OnFocus.Insert(OnEntryFocus);
				entryButton.m_OnDoubleClicked.Insert(OnEntryDoubleClick);
			}
			
			entryCompoment = SCR_SaveLoadEntryComponent.Cast(entryWidget.FindHandler(SCR_SaveLoadEntryComponent));
			entryCompoment.SetSaveData(save);

			m_mComponentEntries.Insert(entryWidget, entryCompoment);
			m_aEntriesHidden.Insert(entryWidget);

			//--- Hide by default
			entryWidget.SetOpacity(0);

			if (m_bIsLoad && idx == 0)
				SelectEntry(entryWidget, entryCompoment);
		}

		//--- Initiate periodic check which will load and show metadata only for entries that are actually shown. Doing it all at once here would be too expensive.
		GetGame().GetCallqueue().CallLater(OnFrame, 1, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnEntryFocus(SCR_ModularButtonComponent button)
	{
		//--- Save entry
		Widget w = button.GetRootWidget();
		SCR_SaveLoadEntryComponent entryComponent;
		if (!m_mComponentEntries.Find(w, entryComponent))
			return;
		
		SelectEntry(w, entryComponent);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntryDoubleClick(SCR_ModularButtonComponent button)
	{
		//--- Activate
		OnConfirm(null, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		GetGame().GetCallqueue().Remove(OnFrame);
	}
}
