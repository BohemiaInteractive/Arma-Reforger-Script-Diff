class SCR_EditorSaveDialog: SCR_ConfigurableDialogUi
{
	protected SCR_ConfigListComponent m_ConfigList;
	protected string m_sSaveName;
	
	// List entries
	protected SCR_WidgetListEntry m_EntryName;
	protected SCR_WidgetListEntry m_EntrySummary;
	protected SCR_WidgetListEntry m_EntryDescription;
	protected SCR_WidgetListEntryMessage m_EntryVersion;
	protected SCR_WidgetListEntryMessage m_EntryScenario;
	protected SCR_WidgetListEntryPublishImages m_EntryImagesPicker;
	protected SCR_WidgetListEntryResourcePicker m_ThumbnailPickerEntry;
	protected SCR_WidgetListEntrySelectionList m_GalleryPickersList;
	protected SCR_WidgetListEntry m_EntryDependenciesLabel;
	protected SCR_WidgetListEntryCheckList m_EntryDependenciesList;
	
	//protected ref ScriptInvokerBase<ScreenshotCallback> m_OnMakeScreenshot;
	
	protected bool m_bEnableDependencies = false;
	
	//---------------------------------------------------------------------------------------------
	override void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		//WorldSaveItem saveItem;
		//SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		//saveWorkshopManager.GetCurrentSave(saveItem);
		
		super.Init(root, preset, proxyMenu);
		
		Widget content = GetContentWidget(root).GetChildren();
		m_ConfigList = SCR_ConfigListComponent.Cast(content.FindHandler(SCR_ConfigListComponent));
		
		// Find list entries
		m_EntryName = m_ConfigList.FindEntry("name");
		m_EntrySummary = m_ConfigList.FindEntry("summary");
		m_EntryDescription = m_ConfigList.FindEntry("description");
		m_EntryVersion = SCR_WidgetListEntryMessage.Cast(m_ConfigList.FindEntry("version"));
		m_EntryScenario = SCR_WidgetListEntryMessage.Cast(m_ConfigList.FindEntry("originalScenario"));
		
		// Gallery 
		m_EntryImagesPicker = SCR_WidgetListEntryPublishImages.Cast(m_ConfigList.FindEntry("imageGallery"));
		
		m_ThumbnailPickerEntry = SCR_WidgetListEntryResourcePicker.Cast(m_ConfigList.FindEntry("thumbnail"));
		m_ThumbnailPickerEntry.GetResourcePicker().GetOnPickerButtonClick().Insert(OnThumbnailPickerClick);
		m_ThumbnailPickerEntry.GetResourcePicker().GetOnResourcePicked().Insert(OnThumbnailPicked);
		
		m_GalleryPickersList = SCR_WidgetListEntrySelectionList.Cast(m_ConfigList.FindEntry("gallery")); 
		m_GalleryPickersList.GetOnAddingButtonElement().Insert(OnGalleryAddElemented);
		
		// Dependencies 
		m_EntryDependenciesLabel = m_ConfigList.FindEntry("dependenciesLabel");
		m_EntryDependenciesList = SCR_WidgetListEntryCheckList.Cast(m_ConfigList.FindEntry("dependencies"));
		
		// Fill dependency list 
		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs); 
		
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop();
		MissionWorkshopItem mission = null;// SCR_SaveWorkshopManager.GetCurrentScenario();
		
		if (m_EntryDependenciesList.GetVisible())
		{
			WorkshopItem item;
			SCR_LocalizedProperty property;
			SCR_ListBoxElementComponent checkbox;
			
			foreach (string guid :addonGUIDs)
			{
				item = workshop.FindItem(guid);
				if (!item)
				{
					Print("Couldn't find item of id: " + guid, LogLevel.WARNING);
					continue;
				}
				
				property = new SCR_LocalizedProperty(item.Name(), guid);
				checkbox = m_EntryDependenciesList.AddElement(property, m_bEnableDependencies);
				
				// Prevent disabling of required dependency - scenario source 
				if (mission && mission.GetOwnerId() == guid)
					checkbox.SetEnabled(false);
			}
		}
		
		bool showDependencies = m_EntryDependenciesList.CheckboxesCount() != 0 && m_EntryDependenciesLabel.GetVisible();
		m_EntryDependenciesLabel.GetEntryRoot().SetVisible(showDependencies);
		
		// Fill scenario 
		//m_EntryScenario.SetValue(SCR_SaveWorkshopManager.GetCurrentScenarioNameTranslated());
		
		// Arma vision callback
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		
		SCR_EditorModeEntity photoModeEntity = editorManager.FindModeEntity(EEditorMode.PHOTO_SAVE);
		if (!photoModeEntity)
			photoModeEntity = editorManager.CreateEditorMode(EEditorMode.PHOTO_SAVE, false);

		photoModeEntity.GetOnActivate().Insert(OnArmaVisionActivated);
		
		// Fill with editing manifest
		/*
		SCR_EditedSaveManifest editedManifest = saveWorkshopManager.GetEditedSaveManifest();
		if (editedManifest)
		{
			FillListFromWorldSaveManifest(editedManifest.GetManifest());
		}*/
	}
	
	//---------------------------------------------------------------------------------------------
	override void OnCancel()
	{
		super.OnCancel();
		
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		SCR_EditorModeEntity photoModeEntity = editorManager.FindModeEntity(EEditorMode.PHOTO_SAVE);
		
		if (photoModeEntity)
			editorManager.RemoveMode(photoModeEntity, false);
	}
	
	//---------------------------------------------------------------------------------------------
	// Create world save manifest base on data set in list
	protected WorldSaveManifest ManifestFromConfigList()
	{
		if (!m_ConfigList)
		{
			Debug.Error("SCR_EditorSaveDialog.ManifestFromConfigList() - Can't read files because list was not found");
			return null;
		}
		
		//SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		
		WorldSaveManifest manifest = new WorldSaveManifest();
		
		// Description
		manifest.m_sName = m_EntryName.ValueAsString();
		manifest.m_sSummary = m_EntrySummary.ValueAsString();
		manifest.m_sDescription = m_EntryDescription.ValueAsString();
		
		// Filled from scenario 
		manifest.m_sScenarioId = ""; //saveWorkshopManager.GetCurrentScenarioId();
		
		//string image = "";
		//image = saveWorkshopManager.CurrentScenarioImage();
		/*
		SCR_SaveImageGalleryComponent imagePicker = m_EntryImagesPicker.GetSaveImagesPicker();
		array<string> imageGallery = {};
		
		if (imagePicker)
		{
			//manifest.m_sPreview = imagePicker.GetThumbnailSourceImage();
			//imageGallery = imagePicker.GetGalleryResources();
			
			manifest.m_aScreenshots = {};
			manifest.m_aScreenshots.InsertAll(imageGallery);
		}
		*/
		
		manifest.m_sPreview = m_ThumbnailPickerEntry.ValueAsString();
		manifest.m_aScreenshots = m_GalleryPickersList.GetElementValues();
		
		// Dependencies 
		manifest.m_aDependencyIds = m_EntryDependenciesList.EnabledCheckListPropertyNames();
		
		// Files 
		/*
		WorldSaveItem saveItem;
		string fileName = saveWorkshopManager.GetCurrentSave(saveItem);
		
		manifest.m_aFileNames = {};
		
		if (saveItem)
		{
			Revision revision = saveItem.GetActiveRevision();
			if (!revision)
				revision = saveItem.GetLocalRevision();
			
			array<string> fileNames = {};
			revision.GetFiles(fileNames);
			
			// Recreate missing save file if needed
			if (!fileNames.IsEmpty() && fileName != fileNames[0])
			{
				SessionStorage storage = GetGame().GetBackendApi().GetStorage();
				GetGame().GetSaveManager().Save(ESaveType.USER, manifest.m_sName, manifest, saveItem);
			}
			
			manifest.m_aFileNames = {fileNames[0]};
		}
		*/
		
		return manifest;
	}
	
	//---------------------------------------------------------------------------------------------
	protected void FillListFromSaveItem(notnull WorldSaveItem saveItem)
	{
		m_EntryName.SetValue(saveItem.Name());
		m_EntrySummary.SetValue(saveItem.Summary());
		m_EntryDescription.SetValue(saveItem.Description());
		
		Revision latestRev = saveItem.GetLatestRevision();
		if (latestRev)
		{
			m_EntryVersion.SetValue(latestRev.GetVersion());
		}
		else 
		{
			latestRev = saveItem.GetLocalRevision();
			m_EntryVersion.SetValue(latestRev.GetVersion());
			
			if (latestRev.GetVersion().IsEmpty())
				m_EntryVersion.SetVisible(false);
		}
		
		// Add image filling
		
		// Add dependencies setup - note: Need to relly on addons from world sve item dependencies list
		/*
		array<string> addonGUIDs = {};
		GameProject.GetLoadedAddons(addonGUIDs); 
		
		WorkshopApi workshop = GetGame().GetBackendApi().GetWorkshop();
		
		foreach (string guid :addonGUIDs)
		{
			WorkshopItem item = workshop.FindItem(guid);
			if (!item)
			{
				Print("Couldn't find item of id: " + guid, LogLevel.WARNING);
				continue;
			}
			
			SCR_ListBoxElementComponent checkbox =  m_EntryDependenciesList.FindCheckboxByPropertyName(guid);
			if (checkbox)
				m_EntryDependenciesList.ToggleCheckbox(checkbox);	
		}
		*/
		
		// Add dependencies setup
		array<WorkshopItem> dependencies = {};
		Revision currentRev = saveItem.GetActiveRevision();
		if (!currentRev)
			currentRev = saveItem.GetLocalRevision();
		
		currentRev.GetDependencies(dependencies);
		
		SCR_ListBoxElementComponent checkbox;
		
		foreach (WorkshopItem dependency : dependencies)
		{
			// Enable required dependency 
			checkbox = m_EntryDependenciesList.FindCheckboxByPropertyName(dependency.Id());
			if (checkbox)
				m_EntryDependenciesList.ToggleCheckbox(checkbox, true);
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected void FillListFromWorldSaveManifest(WorldSaveManifest manifest)
	{
		m_EntryName.SetValue(manifest.m_sName);
		m_EntrySummary.SetValue(manifest.m_sSummary);
		m_EntryDescription.SetValue(manifest.m_sDescription);
		
		//m_EntryImagesPicker.SetValue(manifest.m_sPreview);
		m_ThumbnailPickerEntry.SetValue(manifest.m_sPreview);
		
		SCR_WidgetListEntryResourcePicker picker;
		
		if (manifest.m_aScreenshots)
			return;
		
		foreach (string screenshot : manifest.m_aScreenshots)
		{
			if (screenshot.IsEmpty())
				continue; 
			
			picker = SCR_WidgetListEntryResourcePicker.Cast(m_GalleryPickersList.GetElement());
			m_GalleryPickersList.AddElement(picker);
			
			picker.SetValue(screenshot);
			picker.GetResourcePicker().GetOnResourcePicked().Insert(OnPickerResourcePicked);
			picker.GetResourcePicker().GetOnPickerButtonClick().Insert(OnThumbnailPickerClick);
		}
	}
 	
	//---------------------------------------------------------------------------------------------
	protected void OnPickerResourcePicked(string resource)
	{
		// Remove if empty 
		if (resource.IsEmpty())
		{
			
		}
	}
	
	//---------------------------------------------------------------------------------------------
	protected override void OnConfirm()
	{
		Close();
		m_OnConfirm.Invoke(this);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnThumbnailPicked(string resource)
	{
		//m_EntryImagesPicker.GetSaveImagesGallery().SetThumbnailImage(resource);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnThumbnailPickerClick(SCR_ImagePickerComponent picker)
	{			
		//WorldSaveManifest manifest = ManifestFromConfigList();
		//SCR_SaveWorkshopManager.GetInstance().SetEditedSaveManifest(manifest, "thumbnail", string.Empty);
		
		if (picker.GetResourcePath().IsEmpty())
			m_GalleryPickersList.RemoveElementByWidget(picker.GetRootWidget())
		
		//SCR_EditorManagerEntity.GetInstance().SetCurrentMode(EEditorMode.PHOTO_SAVE);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnGalleryAddElemented()
	{	
		//SCR_SaveWorkshopManager manager = SCR_SaveWorkshopManager.GetInstance();
		//WorldSaveManifest manifest = ManifestFromConfigList();
		//manager.SetEditedSaveManifest(manifest, "gallery", string.Empty);
		
		SCR_EditorManagerEntity.GetInstance().SetCurrentMode(EEditorMode.PHOTO_SAVE);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnArmaVisionActivated()
	{	
		SCR_SaveWorkshopManagerUI managerUI = SCR_SaveWorkshopManagerUI.GetInstance();
		managerUI.SetCachedEditSaveDialog(this);
		managerUI.SetupSaveImageCaptureCallback();
		
		Close();
	}
}