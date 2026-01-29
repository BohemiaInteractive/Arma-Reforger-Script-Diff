/*!
Scripted api for handling save sharing UI
*/

class SCR_SaveWorkshopManagerUI
{
	static const ResourceName DIALOGS_CONFIG = "{326975357A528C2B}Configs/Dialogs/GMSaveDialogs.conf";
	static const ResourceName PUBLISH_BUTTON_TOOLTIPS = "{9DA751645E85FC64}Configs/Editor/Tooltips/EditorSaveTooltips.conf";
	
	protected static ref SCR_SaveWorkshopManagerUI m_Instance;
	
	protected static ref SCR_EditorSaveDialog m_CachedEditorSaveDialog;
	
	protected SCR_EditorModeEntity m_PhotoModeEntity;
	
	//---------------------------------------------------------------------------------------------
	// Static
	//---------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------
	static SCR_SaveWorkshopManagerUI GetInstance()
	{
		if (!m_Instance)
			m_Instance = new SCR_SaveWorkshopManagerUI();
		
		return m_Instance;
	}
	
	//---------------------------------------------------------------------------------------------
	static SCR_ConfigurableDialogUi CreateDialog(string presetName)
	{
		return SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, presetName);
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_GMSaveDialog CreateSaveDetailsnDialog(MissionWorkshopItem scenario, ScriptInvokerBool onFavoritesResponse = null)
	{
		SCR_GMSaveDialog dialogUI = new SCR_GMSaveDialog(scenario, onFavoritesResponse);
		SCR_ConfigurableDialogUi.CreateFromPreset(DIALOGS_CONFIG, "save_detail", dialogUI);

		return dialogUI;
	}
	
	//---------------------------------------------------------------------------------------------
	// Public
	//---------------------------------------------------------------------------------------------
	
	//---------------------------------------------------------------------------------------------
	void SetCachedEditSaveDialog(SCR_EditorSaveDialog dialog)
	{
		m_CachedEditorSaveDialog = dialog;
	}
	
	//---------------------------------------------------------------------------------------------
	SCR_EditorSaveDialog GetCachedEditSaveDialog()
	{
		return m_CachedEditorSaveDialog;
	}
	
	//---------------------------------------------------------------------------------------------
	//! Setup callbacks reacting on photo save capture mode events
	void SetupSaveImageCaptureCallback()
	{
		if (!m_PhotoModeEntity)
		{
			SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
			SCR_EditorManagerEntity editorManager = core.GetEditorManager();
			
			m_PhotoModeEntity = editorManager.FindModeEntity(EEditorMode.PHOTO_SAVE);
		}
		
		m_PhotoModeEntity.GetOnDeactivateServer().Insert(OnImageCapturePhotoModeDeactivate);
	}
	
	//---------------------------------------------------------------------------------------------
	void ClearSaveImageCaptureCallback()
	{
		if (!m_PhotoModeEntity)
		{
			Print("Can't clearup photo mode entity cause it was not setup", LogLevel.ERROR);
			return;
		}
		
		m_PhotoModeEntity.GetOnDeactivateServer().Remove(OnImageCapturePhotoModeDeactivate);
	}
	
	//---------------------------------------------------------------------------------------------
	//! Callback reacting to closing photo mode intended for image capture
	protected void OnImageCapturePhotoModeDeactivate()
	{
		ClearSaveImageCaptureCallback();
		
		// Reopen dialog and fill with cached data 
		new SCR_PublishSaveDialog();
	}
}