class SCR_PhotoSaveModeEditorUIComponent : SCR_PhotoModeEditorUIComponent
{
	protected const string IMAGE_PATH_TEST = "UI/TestImg.png";
	
	protected static const int RESOLUTION_X = 1920; // save dimension
	protected static const int RESOLUTION_Y = 1080; // save dimension
	
	[Attribute("Toolbar_CaptureButton")]
	protected string m_sCaptureButton;
	
	[Attribute("")]
	protected string m_sCaptureAction;
	
	[Attribute("use_screenshot")]
	protected string m_sConfirmScreenshotDialogTag;
	
	[Attribute("EditorMenuConfirm")]
	protected string m_sConfirmScreenshotAction;
	
	[Attribute("InputButtonRoot0")]
	protected string m_sActionButton;
	
	protected SCR_EventHandlerComponent m_CaptureButtonEvents;
	protected SCR_ConfigurableDialogUi m_UseScreenshotDialog;
	
	protected static ref ImageWidget s_wDebugImage;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		// UI setup
		Widget captureButton = w.FindAnyWidget(m_sCaptureButton);
		m_CaptureButtonEvents = SCR_EventHandlerComponent.Cast(captureButton.FindHandler(SCR_EventHandlerComponent));
		m_CaptureButtonEvents.GetOnClick().Insert(OnCaptureImage);
		
		Widget inputButton = captureButton.FindAnyWidget(m_sActionButton);
		SCR_InputButtonComponent inputButtonCmp = SCR_InputButtonComponent.Cast(inputButton.FindHandler(SCR_InputButtonComponent));
		inputButtonCmp.SetAction(m_sConfirmScreenshotAction);
		
		GetGame().GetInputManager().AddActionListener(m_sConfirmScreenshotAction, EActionTrigger.DOWN, OnCaptureImage);
		
		// Inputs 
		GetGame().GetInputManager().AddActionListener(m_sCaptureAction, EActionTrigger.DOWN, OnCaptureImage);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capture image of current camere view 
	protected void OnCaptureImage()
	{
		CaptureImage(RESOLUTION_X, RESOLUTION_Y);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Capture image of current camere view 
	protected void CaptureImage(int resolutionX, int resolutionY)
	{
		SCR_MenuEditorComponent editorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		
		if (editorMenuManager)
			editorMenuManager.SetVisible(false, true);
		
		GetGame().GetCallqueue().Call(CallFrameLater, resolutionX, resolutionY); // Hiding UI for screenshot needs to happen next frame for UI to update visibility
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMakeScreenshot(int resolutionX, int resolutionY)
	{
		int width, height;
		System.GetNativeResolution(width, height);
		System.MakeScreenshotRawData(OnScreenshot, 0, 0, width, height, resolutionX, resolutionY);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnScreenshot(PixelRawData data, int imageWidth, int imageHeight, int stride)
	{
		#ifdef WORKBENCH
		//Workbench.SavePixelRawData(IMAGE_PATH_TEST, data, imageWidth, imageHeight, stride);
		#endif
		
		// Create comfirm dialog
		m_UseScreenshotDialog = SCR_SaveWorkshopManagerUI.CreateDialog(m_sConfirmScreenshotDialogTag);
		m_UseScreenshotDialog.m_OnConfirm.Insert(OnScreenshotUseDialogConfirm);
		m_UseScreenshotDialog.m_OnCancel.Insert(OnScreenshotUseDialogCancel);
		
		SCR_MenuEditorComponent editorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		
		/*
		string screenshot = IMAGE_PATH_TEST;
		
		SCR_SaveWorkshopManager manager = SCR_SaveWorkshopManager.GetInstance();
		SCR_EditedSaveManifest editedManifest = manager.GetEditedSaveManifest();
		WorldSaveManifest manifest = editedManifest.GetManifest();
		WorldSaveItem save; 
		manager.GetCurrentSave(save);
		
		if (editedManifest.GetEditingValue() == "thumbnail")
		{
			save.SaveJPEG(data, RESOLUTION_X, RESOLUTION_Y, 50, 4, true);
			//editedManifest.m_Manifest.m_sPreview = screenshot;
		}
		else if (editedManifest.GetEditingValue() == "gallery")
		{	
			if (!manifest.m_aScreenshots)
				manifest.m_aScreenshots = {};
			
			for (int i = manifest.m_aScreenshots.Count() - 1; i >= 0; i--)
			{
				if (manifest.m_aScreenshots[i] == string.Empty)
					manifest.m_aScreenshots.Remove(i);
			}
			
			manifest.m_aScreenshots.Insert(screenshot);
		}
		
		manager.SetEditedSaveManifest(manifest, editedManifest.GetEditingValue(), "img");
		*/
		
		if (editorMenuManager)
			editorMenuManager.SetVisible(true, true);
	}	
	
	//---------------------------------------------------------------------------------------------
	protected void OnScreenshotUseDialogConfirm()
	{
		SCR_EditorManagerCore core = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));
		SCR_EditorManagerEntity editorManager = core.GetEditorManager();
		
		SCR_EditorManagerEntity.GetInstance().SetCurrentMode(EEditorMode.EDIT);
	}
	
	//---------------------------------------------------------------------------------------------
	protected void OnScreenshotUseDialogCancel()
	{
		//SCR_SaveWorkshopManager manager = SCR_SaveWorkshopManager.GetInstance();
		//manager.ClearEditedSaveManifest();
	}
	
	//---------------------------------------------------------------------------------------------
	// DEBUG FUNCTIONS FOR TESTING CAPTURING A SCREEN
	//---------------------------------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Capture image of current camere view [DEBUG]
	static void CaptureImageDebug(int resolutionX, int resolutionY, notnull ImageWidget outputWidget)
	{
		SCR_MenuEditorComponent editorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		
		if (editorMenuManager)
			editorMenuManager.SetVisible(false, true);
		
		s_wDebugImage = outputWidget;
		s_wDebugImage.SetVisible(false);
		GetGame().GetCallqueue().Call(CallFrameLaterDebug, resolutionX, resolutionY); // Hiding UI for screenshot needs to happen next frame for UI to update visibility
	}
	
	//------------------------------------------------------------------------------------------------
	protected static void OnMakeScreenshotDebug(int resolutionX, int resolutionY)
	{
		int width, height;
		System.GetNativeResolution(width, height);
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		System.MakeScreenshotTexture(OnScreenshotDebug, 0, 0, width, height, workspace.DPIScale(resolutionX), workspace.DPIScale(resolutionY));
	}
	
	//---------------------------------------------------------------------------------------------
	protected static void OnScreenshotDebug(ScreenshotTextureData data)
	{
		if (s_wDebugImage)
		{
			s_wDebugImage.SetVisible(true);
			s_wDebugImage.CopyImageTexture(s_wDebugImage.GetImage(), data);
		}
		
		SCR_MenuEditorComponent editorMenuManager = SCR_MenuEditorComponent.Cast(SCR_MenuEditorComponent.GetInstance(SCR_MenuEditorComponent));
		
		if (editorMenuManager)
			editorMenuManager.SetVisible(true, true);
	}
	
	//---------------------------------------------------------------------------------------------
	void CallFrameLater(int resX, int resY)
	{
		GetGame().GetCallqueue().Call(OnMakeScreenshot, resX, resY);
	}
	
	//---------------------------------------------------------------------------------------------
	static void CallFrameLaterDebug(int resX, int resY)
	{
		GetGame().GetCallqueue().Call(OnMakeScreenshotDebug, resX, resY);
	}
}
