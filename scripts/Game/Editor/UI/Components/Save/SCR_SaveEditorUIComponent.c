class SCR_SaveEditorUIComponent : ScriptedWidgetComponent
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
	
	protected ref map<Widget, SCR_SaveLoadEntryComponent> m_mComponentEntries = new map<Widget, SCR_SaveLoadEntryComponent>();
	
	protected ref SCR_SaveWidgets m_Widgets = new SCR_SaveWidgets();
	
	protected Widget m_wRoot;
	protected Widget m_wLastFocusedEntry;
	protected Widget m_wSelectedWidget;
	
	protected SCR_ConfigurableDialogUi m_ConfirmPrompt;
	protected SCR_ConfigurableDialogUi m_DeletePrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadVersionPrompt;
	protected SCR_ConfigurableDialogUi m_LoadBadAddonsPrompt;
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Control buttons
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	protected void OnClose(SCR_InputButtonComponent button, string actionName)
	{
		CloseMenu();
	}

	//----------------------------------------------------------------------------------------
	protected void OnConfirm(SCR_InputButtonComponent button, string actionName)
	{
		// Save dialogs
		if (!m_bIsLoad)
		{
			string customName = m_Widgets.m_SaveNameInputComponent0.GetValue();
			if (false /*GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName)*/)
			{
				//--- Confirm prompt
				m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
				m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
			}
			else
			{
				//--- Creating a new file - save directly
				OnConfirmPrompt();
				return;
			}
			
			m_ConfirmPrompt.SetTitle(m_Widgets.m_SaveNameInputComponent0.GetValue());
			return;
		}
		
		SCR_SaveLoadEntryComponent entryComponent;
		if (m_mComponentEntries.Find(m_wSelectedWidget, entryComponent))
		{
			string displayname = entryComponent.GetDisplayName();
			
			/*
			SCR_MetaStruct meta = GetGame().GetSaveManager().GetMeta(entryComponent.GetFileName());
			if (meta)
			{
				if (!meta.IsVersionCompatible())
				{
					// Warning - incompatible version
					m_LoadBadVersionPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadVersionPrompt);
					m_LoadBadVersionPrompt.m_OnConfirm.Insert(LoadEntry);
					m_LoadBadVersionPrompt.SetTitle(displayname);
					return;
				}
				else if (!meta.AreAddonsCompatible())
				{
					// Warning - incompatible addons
					m_LoadBadAddonsPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sLoadBadAddonsPrompt);
					m_LoadBadAddonsPrompt.m_OnConfirm.Insert(LoadEntry);
					m_LoadBadAddonsPrompt.SetTitle(displayname);
					return;
				}
			}
			*/
			
			//--- Confirm prompt
			m_ConfirmPrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sConfirmPrompt);
			m_ConfirmPrompt.m_OnConfirm.Insert(OnConfirmPrompt);
			m_ConfirmPrompt.SetTitle(displayname);
		}
	}

	//----------------------------------------------------------------------------------------
	protected void OnConfirmPrompt()
	{
		if (m_bIsLoad)
			LoadEntry();
		else
			SaveEntry();

		CloseMenu();
	}
	
	//----------------------------------------------------------------------------------------
	//! Callback on clicking delete button or action
	protected void OnDelete(SCR_InputButtonComponent button, string actionName)
	{
		SCR_SaveLoadEntryComponent entryComponent;
		if (!m_mComponentEntries.Find(m_wSelectedWidget, entryComponent))
			return;
		
		// Open delete dialog
		m_DeletePrompt = SCR_ConfigurableDialogUi.CreateFromPreset(SCR_CommonDialogs.DIALOGS_CONFIG, m_sDeletePrompt); //--- ToDo: Unique tag
		m_DeletePrompt.m_OnConfirm.Insert(OnDeletePrompt);
		
		// Setup string
		string displayName = entryComponent.GetDisplayName();
		m_DeletePrompt.SetTitle(displayName);
	}

	//----------------------------------------------------------------------------------------
	protected void OnDeletePrompt()
	{
		SCR_SaveLoadEntryComponent entryComponent = m_mComponentEntries.Get(m_wSelectedWidget);
		
		//--- Delete the file
		//GetGame().GetSaveManager().Delete(entryComponent.GetFileName());
		
		//--- Update GUI
		m_mComponentEntries.Remove(m_wSelectedWidget);
		
		m_wSelectedWidget.RemoveFromHierarchy();
		SelectEntry(null);
	}
	
	//----------------------------------------------------------------------------------------
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
	protected void SaveEntry()
	{
		string customName = m_Widgets.m_SaveNameInputComponent0.GetValue();
		//GetGame().GetSaveManager().Save(m_eWriteSaveType, customName);
	}
	
	//----------------------------------------------------------------------------------------
	protected void LoadEntry()
	{
		SCR_SaveLoadEntryComponent entryComponent;
		if (m_mComponentEntries.Find(m_wLastFocusedEntry, entryComponent))
		{
			/*
			// Load save 		
			SCR_SaveManagerCore saveManager = GetGame().GetSaveManager();
			saveManager.RestartAndLoad(entryComponent.GetFileName());
			
			// Handle server load
			if (!Replication.IsClient())
				return;
			
			BackendCallback uploadCallback = saveManager.GetUploadCallback();
			if (uploadCallback)
			{
				uploadCallback.SetOnSuccess(OnLoadEntryUploadResponse);
				uploadCallback.SetOnError(OnLoadEntryUploadError);
				m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
			}
			*/
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnOverrideSaveEntry(notnull SCR_SaveLoadEntryComponent entryComponent)
	{
		m_wSelectedWidget = m_mComponentEntries.GetKeyByValue(entryComponent);
		OnConfirm(null, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnLoadSaveEntry(notnull SCR_SaveLoadEntryComponent entryComponent)
	{
		m_wSelectedWidget = m_mComponentEntries.GetKeyByValue(entryComponent);
		OnConfirm(null, string.Empty);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDeleteSaveEntry(notnull SCR_SaveLoadEntryComponent entryComponent)
	{
		m_wSelectedWidget = m_mComponentEntries.GetKeyByValue(entryComponent);
		OnDelete(null, string.Empty);
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadResponse()
	{
		CloseMenu();
	}
	
	//----------------------------------------------------------------------------------------
	protected void OnLoadEntryUploadError()
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
	protected void SelectEntry(Widget w, SCR_SaveLoadEntryComponent entryComponent = null)
	{
		string fileName;
		if (entryComponent)
			fileName = entryComponent.GetFileName();
		
		string customName = "";//GetGame().GetSaveManager().GetCustomName(fileName);
		m_Widgets.m_SaveNameInputComponent0.SetValue(customName);

		m_wSelectedWidget = w;
		
		UpdateButtons();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateButtons()
	{
		SCR_SaveLoadEntryComponent entryComponent = m_mComponentEntries.Get(m_wSelectedWidget);
		
		bool canSave = entryComponent.CanOverrideSave();
		bool canLoad = entryComponent.CanLoadSave();
		bool canDelete = entryComponent.CanDeleteSave();
		
		string customName = m_Widgets.m_SaveNameInputComponent0.GetValue();
		bool canOverride;
		if (!customName.IsEmpty())// && !GetGame().GetSaveManager().FileExists(m_eWriteSaveType, customName))
			canOverride = true;
		
		m_Widgets.m_ConfirmButtonComponent.SetVisible(canSave);
		m_Widgets.m_DeleteButtonComponent.SetEnabled(canDelete);
		m_Widgets.m_OverrideButtonComponent.SetEnabled(false);
		m_Widgets.m_ConfirmButtonComponent.SetEnabled(false);
		
		if (canSave)
		{
			m_Widgets.m_OverrideButtonComponent.SetEnabled(canOverride);
			m_Widgets.m_ConfirmButtonComponent.SetEnabled(canOverride);	
		}		
	}

	//------------------------------------------------------------------------------------------------
	protected void OnInit()
	{
		foreach(Widget entryWidget, SCR_SaveLoadEntryComponent entryComponent : m_mComponentEntries)
		{
			/*
			string displayName = GetGame().GetSaveManager().GetCustomName(entryComponent.GetFileName());
			entryComponent.SetDisplayName(displayName);
			
			SCR_MetaStruct meta = GetGame().GetSaveManager().GetMeta(fileName);
			if (meta && meta.IsValid())
			{
				SCR_UIInfo info = GetGame().GetSaveManager().GetSaveTypeInfo(fileName);
				Color imageColor = Color.FromInt(Color.WHITE);
				if (info)
				{
					info.SetIconTo(entryComponent.GetSaveIcon());
					displayName = info.GetName() + " " + displayName; //--- Hardcoded space separator, ToDo: Solve using %1 in the string itself?
					
					SCR_ColorUIInfo colorInfo = SCR_ColorUIInfo.Cast(info);
					if (colorInfo)
						imageColor = colorInfo.GetColor();
				}
								
				string versionText;
				bool isVersionCompatible = meta.IsVersionCompatible(versionText);
				
				int y, m, d, hh, mm;
				meta.GetDateAndTime(y, m, d, hh, mm);
				
				entryComponent.SetSaveName(displayName);
				entryComponent.SetDateTime(SCR_DateTimeHelper.GetDateString(d, m, y, m_bVerboseDate), SCR_FormatHelper.GetTimeFormattingHoursMinutes(hh, mm));
				entryComponent.SetVersion(versionText, isVersionCompatible);
				
				if (!meta.AreAddonsCompatible())
					entryComponent.SetIsWarning();
				
				if (m_bIsLoad)
					entryComponent.SetCanLoad(true);
				else
					entryComponent.SetCanSave(true);
				
				entryComponent.SetCanDelete(true);
				
				ResourceName headerResourceName = meta.GetHeaderResource();
				if (!headerResourceName.IsEmpty())
				{
					Resource missionHeaderResource = Resource.Load(headerResourceName);
					if (missionHeaderResource.IsValid())
					{
						SCR_MissionHeader missionHeader = SCR_MissionHeader.Cast(BaseContainerTools.CreateInstanceFromContainer(missionHeaderResource.GetResource().ToBaseContainer()));
						
						entryComponent.SetSaveName(missionHeader.m_sName);
						
						if (missionHeader.m_sIcon)
							entryComponent.SetSaveIcon(imageColor, missionHeader.m_sIcon);
					}
				}
			}
			*/
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Overrides
	//////////////////////////////////////////////////////////////////////////////////////////

	//------------------------------------------------------------------------------------------------
	override bool OnClick(Widget w, int x, int y, int button)
	{
		if (w.GetParent() != m_Widgets.m_wSaveList)
			return false;
		
		SCR_SaveLoadEntryComponent entryComponent;
		if (m_mComponentEntries.Find(w, entryComponent))
			SelectEntry(w, entryComponent);
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		SCR_SaveLoadEntryComponent entryComponent;
		if (m_mComponentEntries.Find(w, entryComponent))
		{
			m_wLastFocusedEntry = w;
			SelectEntry(w, entryComponent);
		}
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override bool OnDoubleClick(Widget w, int x, int y, int button)
	{
		if (w.GetParent() != m_Widgets.m_wSaveList)
			return false;
		
		//--- Select
		OnClick(w, x, y, button);
		
		//--- Activate
		OnConfirm(null, string.Empty);
		
		return false;
	}
	
	//----------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		if (!GetGame().InPlayMode())
		{
			//! DEBUG
			Widget parent = w.FindWidget("SaveHorizontalLayout.SaveWindow.WindowOverlay.WindowStructure.Saves.SaveEntryList.m_SaveScroller.m_SaveList");
			for (int i = 0; i < 10; i++)
			{
				GetGame().GetWorkspace().CreateWidgets(m_sEntryLayout, parent);
			}
			return;
		}
		
		//--- Find all widgets
		m_wRoot = w;
		m_Widgets.Init(w);
		
		//--- Name input field
		m_Widgets.m_SaveNameInputComponent0.m_OnChanged.Insert(UpdateButtons);
		
		m_Widgets.m_ExitButtonComponent.m_OnActivated.Insert(OnClose);
		m_Widgets.m_DeleteButtonComponent.m_OnActivated.Insert(OnDelete);
		m_Widgets.m_OverrideButtonComponent.m_OnActivated.Insert(OnConfirm);
		m_Widgets.m_ConfirmButtonComponent.m_OnActivated.Insert(OnConfirm);
		
		m_Widgets.m_DeleteButtonComponent.SetEnabled(false);
		m_Widgets.m_OverrideButtonComponent.SetEnabled(false);
		m_Widgets.m_ConfirmButtonComponent.SetEnabled(false);
		
		UpdateButtons();
		
		//--- Create new entries
		/*
		array<string> fileNames = {};
		int fileCount = GetGame().GetSaveManager().GetLocalSaveFiles(fileNames, m_eReadSaveTypes, m_bCurrentMissionOnly);
		fileNames.Sort();
		
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		Widget entryWidget;
		SCR_SaveLoadEntryComponent entryComponent;
		for (int i = 0; i < fileCount; i++)
		{
			int ii = i % fileCount;
			string fileName = fileNames[ii];
			entryWidget = workspace.CreateWidgets(m_sEntryLayout, m_Widgets.m_wSaveList);
			entryComponent = SCR_SaveLoadEntryComponent.Cast(entryWidget.FindHandler(SCR_SaveLoadEntryComponent));
			m_mComponentEntries.Insert(entryWidget, entryComponent);
			
			//--- Hide by default
			entryWidget.SetVisible(false);
			
			entryComponent.SetFileName(fileName);
			
			entryComponent.GetOnOverrideSave().Insert(OnOverrideSaveEntry);
			entryComponent.GetOnLoadSave().Insert(OnLoadSaveEntry);
			entryComponent.GetOnDeleteSave().Insert(OnDeleteSaveEntry);
			
			if (m_bIsLoad && i == 0)
				SelectEntry(entryWidget, entryComponent);
		}
		*/
		
		OnInit();
	}
}
