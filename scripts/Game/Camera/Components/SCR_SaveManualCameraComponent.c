//! @ingroup ManualCamera

//! Persistently save/load camera params.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_SaveManualCameraComponent: SCR_BaseManualCameraComponent
{
	[Attribute(params: "layout")]
	protected ResourceName m_LayoutSave;
	
	[Attribute(params: "layout")]
	protected ResourceName m_LayoutLoad;
	
	[Attribute("5")]
	protected float m_fFadeInSpeed;
	
	private Widget m_WidgetSave;
	private Widget m_WidgetLoad;
	
	protected void SaveCamera1() { SaveCamera(1); }
	protected void SaveCamera2() { SaveCamera(2); }
	protected void SaveCamera3() { SaveCamera(3); }
	protected void SaveCamera4() { SaveCamera(4); }
	protected void SaveCamera5() { SaveCamera(5); }
	protected void SaveCamera6() { SaveCamera(6); }
	protected void SaveCamera7() { SaveCamera(7); }
	protected void SaveCamera8() { SaveCamera(8); }
	//protected void SaveCamera9() { SaveCamera(9); }

	//------------------------------------------------------------------------------------------------
	protected void SaveCamera(int index)
	{
		SCR_ManualCamera camera = GetCameraEntity();
		ResourceName worldPath = GetGame().GetWorldFile();// SCR_Global.GetRootWorld(); //--- Does not return ResourceName, TODO: use it again when it does
		if (!worldPath)
		{
			Print("Unable to save camera data, world path not found!", LogLevel.WARNING);
			return;
		}
		
		SCR_ManualCameraSettings settings = new SCR_ManualCameraSettings();
		BaseContainer settingsContainer = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		BaseContainerTools.WriteToInstance(settings, settingsContainer);
		
		if (!settings.m_aSavedData)
			settings.m_aSavedData = {};
		
		//--- Find existing save, and override it if found
		SCR_ManualCameraSave save;
		foreach (SCR_ManualCameraSave saveCandidate: settings.m_aSavedData)
		{
			if (saveCandidate.m_iIndex == index && saveCandidate.m_sWorldPath == worldPath)
				save = saveCandidate;
		}
		
		//---- Save on given world with given index does not exist, create it
		if (!save)
		{
			save = new SCR_ManualCameraSave();
			settings.m_aSavedData.Insert(save);
		}
		
		//--- Store data
		save.m_sWorldPath = worldPath;
		save.m_iIndex = index;
		save.m_aComponentData = {};
		camera.SaveComponents(save.m_aComponentData);
		
		//--- Save to settings
		BaseContainerTools.ReadFromInstance(settings, settingsContainer);
		GetGame().UserSettingsChanged();
		
		PlayAnimation(m_WidgetSave);
		
		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SAVED_CAMERA_POSITION, index);
		//Print(string.Format("Camera params #%1 saved", index), LogLevel.VERBOSE);
	}
	
	protected void LoadCamera1() { LoadCamera(1); }
	protected void LoadCamera2() { LoadCamera(2); }
	protected void LoadCamera3() { LoadCamera(3); }
	protected void LoadCamera4() { LoadCamera(4); }
	protected void LoadCamera5() { LoadCamera(5); }
	protected void LoadCamera6() { LoadCamera(6); }
	protected void LoadCamera7() { LoadCamera(7); }
	protected void LoadCamera8() { LoadCamera(8); }
	//protected void LoadCamera9() { LoadCamera(9); }

	//------------------------------------------------------------------------------------------------
	protected void LoadCamera(int index)
	{
		ResourceName worldPath = GetGame().GetWorldFile();// SCR_Global.GetRootWorld(); //--- Does not return ResourceName, TODO: use it again when it does
		if (!worldPath)
		{
			Print("Unable to save camera data, world path not found!", LogLevel.WARNING);
			return;
		}
		
		SCR_ManualCameraSettings settings = new SCR_ManualCameraSettings();
		BaseContainer settingsContainer = GetGame().GetGameUserSettings().GetModule("SCR_ManualCameraSettings");
		BaseContainerTools.WriteToInstance(settings, settingsContainer);
		
		if (!settings.m_aSavedData)
			return;
		
		settings.m_aSavedData.Debug();
		
		//--- Apply saved values to components
		SCR_ManualCamera camera = GetCameraEntity();
		foreach (SCR_ManualCameraSave saveCandidate: settings.m_aSavedData)
		{
			Print(string.Format("%1 == %2 && %3 == %4", saveCandidate.m_iIndex, index, saveCandidate.m_sWorldPath, worldPath), LogLevel.NORMAL);
			if (saveCandidate.m_iIndex == index && saveCandidate.m_sWorldPath == worldPath)
			{
				camera.LoadComponents(saveCandidate.m_aComponentData);
		
				//--- Stop inertia
				m_Camera.GetCameraParam().velocityOriginal = vector.Zero;
				
				PlayAnimation(m_WidgetLoad);
				
				SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_LOADED_CAMERA_POSITION, index);
				//Print(string.Format("Camera params #%1 loaded", index), LogLevel.VERBOSE);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return false;
		
		inputManager.AddActionListener("ManualCameraSave1", EActionTrigger.DOWN, SaveCamera1);
		inputManager.AddActionListener("ManualCameraSave2", EActionTrigger.DOWN, SaveCamera2);
		inputManager.AddActionListener("ManualCameraSave3", EActionTrigger.DOWN, SaveCamera3);
		inputManager.AddActionListener("ManualCameraSave4", EActionTrigger.DOWN, SaveCamera4);
		inputManager.AddActionListener("ManualCameraSave5", EActionTrigger.DOWN, SaveCamera5);
		inputManager.AddActionListener("ManualCameraSave6", EActionTrigger.DOWN, SaveCamera6);
		inputManager.AddActionListener("ManualCameraSave7", EActionTrigger.DOWN, SaveCamera7);
		inputManager.AddActionListener("ManualCameraSave8", EActionTrigger.DOWN, SaveCamera8);
		//inputManager.AddActionListener("ManualCameraSave9", EActionTrigger.DOWN, SaveCamera9);
		
		inputManager.AddActionListener("ManualCameraLoad1", EActionTrigger.DOWN, LoadCamera1);
		inputManager.AddActionListener("ManualCameraLoad2", EActionTrigger.DOWN, LoadCamera2);
		inputManager.AddActionListener("ManualCameraLoad3", EActionTrigger.DOWN, LoadCamera3);
		inputManager.AddActionListener("ManualCameraLoad4", EActionTrigger.DOWN, LoadCamera4);
		inputManager.AddActionListener("ManualCameraLoad5", EActionTrigger.DOWN, LoadCamera5);
		inputManager.AddActionListener("ManualCameraLoad6", EActionTrigger.DOWN, LoadCamera6);
		inputManager.AddActionListener("ManualCameraLoad7", EActionTrigger.DOWN, LoadCamera7);
		inputManager.AddActionListener("ManualCameraLoad8", EActionTrigger.DOWN, LoadCamera8);
		//inputManager.AddActionListener("ManualCameraLoad9", EActionTrigger.DOWN, LoadCamera9);
		
		if (m_LayoutSave)
		{
			m_WidgetSave = GetCameraEntity().CreateCameraWidget(m_LayoutSave);
			m_WidgetSave.SetOpacity(0);
		}

		if (m_LayoutLoad)
		{
			m_WidgetLoad = GetCameraEntity().CreateCameraWidget(m_LayoutLoad);
			m_WidgetLoad.SetOpacity(0);
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected void PlayAnimation(Widget widget)
	{
		if (widget)
		{
			widget.SetOpacity(1);
			AnimateWidget.Opacity(widget, 0, m_fFadeInSpeed);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		InputManager inputManager = GetGame().GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.RemoveActionListener("ManualCameraSave1", EActionTrigger.DOWN, SaveCamera1);
		inputManager.RemoveActionListener("ManualCameraSave2", EActionTrigger.DOWN, SaveCamera2);
		inputManager.RemoveActionListener("ManualCameraSave3", EActionTrigger.DOWN, SaveCamera3);
		inputManager.RemoveActionListener("ManualCameraSave4", EActionTrigger.DOWN, SaveCamera4);
		inputManager.RemoveActionListener("ManualCameraSave5", EActionTrigger.DOWN, SaveCamera5);
		inputManager.RemoveActionListener("ManualCameraSave6", EActionTrigger.DOWN, SaveCamera6);
		inputManager.RemoveActionListener("ManualCameraSave7", EActionTrigger.DOWN, SaveCamera7);
		inputManager.RemoveActionListener("ManualCameraSave8", EActionTrigger.DOWN, SaveCamera8);
		//inputManager.RemoveActionListener("ManualCameraSave9", EActionTrigger.DOWN, SaveCamera9);
		
		inputManager.RemoveActionListener("ManualCameraLoad1", EActionTrigger.DOWN, LoadCamera1);
		inputManager.RemoveActionListener("ManualCameraLoad2", EActionTrigger.DOWN, LoadCamera2);
		inputManager.RemoveActionListener("ManualCameraLoad3", EActionTrigger.DOWN, LoadCamera3);
		inputManager.RemoveActionListener("ManualCameraLoad4", EActionTrigger.DOWN, LoadCamera4);
		inputManager.RemoveActionListener("ManualCameraLoad5", EActionTrigger.DOWN, LoadCamera5);
		inputManager.RemoveActionListener("ManualCameraLoad6", EActionTrigger.DOWN, LoadCamera6);
		inputManager.RemoveActionListener("ManualCameraLoad7", EActionTrigger.DOWN, LoadCamera7);
		inputManager.RemoveActionListener("ManualCameraLoad8", EActionTrigger.DOWN, LoadCamera8);
		//inputManager.RemoveActionListener("ManualCameraLoad9", EActionTrigger.DOWN, LoadCamera9);
		
		if (m_WidgetSave)
			m_WidgetSave.RemoveFromHierarchy();
		
		if (m_WidgetLoad)
			m_WidgetLoad.RemoveFromHierarchy();
	}
}
