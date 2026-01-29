class SCR_PublishSaveDialog: SCR_EditorSaveDialog
{
	protected ref WorldSaveItem m_SaveItem;
	protected bool m_bNewPublish; // Is true if publishing save for first time
	
	protected SCR_LoadingOverlayDialog m_LoadingOverlay;
	
	//---------------------------------------------------------------------------------------------
	override void Init(Widget root, SCR_ConfigurableDialogUiPreset preset, MenuBase proxyMenu)
	{
		super.Init(root, preset, proxyMenu);
		
		// Fill config list - if the is currently some file selected
		/*SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		saveWorkshopManager.GetCurrentSave(m_SaveItem);
		
		if (m_SaveItem)
			FillListFromSaveItem(m_SaveItem);
		
		// Publish on screenshot taken 
		SCR_EditedSaveManifest editedManifest = saveWorkshopManager.GetEditedSaveManifest();
		if (editedManifest)
		{
			string editing = editedManifest.GetEditingValue();
			string val = editedManifest.GetValue();
			if (editedManifest.GetEditingValue() == "thumbnail" && !editedManifest.GetValue().IsEmpty())
			{
				SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
				SCR_EditorManagerEntity editorManager = core.GetEditorManager();
				
				SCR_EditorModeEntity editMode = editorManager.FindModeEntity(EEditorMode.EDIT);
			
				editMode.GetOnPostActivate().Insert(OnEditModeActivate);
				saveWorkshopManager.ClearEditedSaveManifest();
			}
		}
		*/
	}
	
	//---------------------------------------------------------------------------------------------	
	protected void OnEditModeActivate()
	{
		UploadSave();
	}
	
	//---------------------------------------------------------------------------------------------
	protected override void OnConfirm()
	{
		// Check validity 
		if (m_ConfigList.GetInvalidEntry())
			return;
		
		/*
		SCR_SaveWorkshopManager saveWorshopManager = SCR_SaveWorkshopManager.GetInstance();
		
		// Check connection 
		if (!SCR_ServicesStatusHelper.IsBackendEnabled() || !SCR_ServicesStatusHelper.IsBackendReady())
		{
			SCR_SaveWorkshopManagerUI.CreateDialog("no_connection");
			return;
		}
		
		// Check login 
		if (!BohemiaAccountApi.IsLinked())
		{
			SCR_LoginProcessDialogUI.CreateLoginDialog();
			return;
		}
		
		// Save data for save publish
		WorldSaveManifest manifest = ManifestFromConfigList();
		m_SaveItem.Save(manifest);
		
		// Open capture image
		SCR_SaveWorkshopManager.GetInstance().SetEditedSaveManifest(manifest, "thumbnail", string.Empty);
		SCR_EditorManagerEntity.GetInstance().SetCurrentMode(EEditorMode.PHOTO_SAVE);
		*/
	}
	
	//---------------------------------------------------------------------------------------------
	protected void UploadSave()
	{
		// Setup and upload save  
		WorldSaveManifest manifest = ManifestFromConfigList();
		
		// Create new save if not working with uploaded save
		if (!m_SaveItem)
		{
			m_SaveItem = WorldSaveItem.CreateLocalWorldSave(manifest);
			m_bNewPublish = true;
		}
		
		// Saves save manifest data
		m_SaveItem.Save(manifest);
		
		/*
		SCR_SaveWorkshopManager saveWorkshopManager = SCR_SaveWorkshopManager.GetInstance();
		saveWorkshopManager.GetUploadCallback().SetOnSuccess(OnUploadSaveResponse);
		saveWorkshopManager.GetUploadCallback().SetOnError(OnUploadSaveError);
		saveWorkshopManager.UploadSave(m_SaveItem, manifest, true);
		*/
		
		m_LoadingOverlay = SCR_LoadingOverlayDialog.Create();
		
		m_OnConfirm.Invoke(this);
	}
	
	//---------------------------------------------------------------------------------------------
	protected override void OnCancel()
	{
		// Save last manifest settings 	
		if (m_SaveItem)
		{
			WorldSaveManifest manifest = ManifestFromConfigList();
			m_SaveItem.Save(manifest);
		}
		
		super.OnCancel();
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnUploadSaveResponse()
	{	
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SAVE_PUBLISH_SUCCESS);
		Close();
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnUploadSaveError()
	{	
		if (m_LoadingOverlay)
			m_LoadingOverlay.Close();
		
		SCR_SaveWorkshopManagerUI.CreateDialog("publish_fail");
	}
	
	//---------------------------------------------------------------------------------------------
	void SCR_PublishSaveDialog()
	{
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_SaveWorkshopManagerUI.DIALOGS_CONFIG, "publish", this);
	}
}