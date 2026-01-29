class SCR_MapEditorUIComponent : SCR_BaseEditorUIComponent
{
	protected SCR_MapEntity m_MapEntity;
	protected InputManager m_InputManager;
	
	protected SCR_ManualCamera m_EditorCamera;
	
	protected SCR_EditorManagerCore m_EditorCore;
	protected SCR_EditorManagerEntity m_EditorManager;
	protected SCR_MapEditorComponent m_EditorMapManager;
	
	protected ref Color m_CameraIconColor = Color.White;
	protected Widget m_MapWidget;
	
	protected ResourceName m_EditorMapConfigPrefab;
	protected ref MapConfiguration m_MapConfigEditor;
	protected bool m_bIsFirstTimeOpened = true;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] show
	//! \param[in] mapConfigPrefab
	void ToggleMap(bool show, ResourceName mapConfigPrefab)
	{
		if (mapConfigPrefab.IsEmpty())
			return;
		
		if (show)
		{
			m_MapConfigEditor = m_MapEntity.SetupMapConfig(EMapEntityMode.EDITOR, mapConfigPrefab, m_MapWidget);
			m_MapEntity.OpenMap(m_MapConfigEditor);
		}
		else if (m_MapEntity.IsOpen())
		{
			m_MapEntity.CloseMap();
		}
		
		if (m_EditorCamera)
			m_EditorCamera.SetInputEnabled(!show);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapInit(MapConfiguration config)
	{
		if (IsConfigEditor(config))
		{
			m_MapWidget.SetVisible(true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapOpen(MapConfiguration config)
	{
		if (IsConfigEditor(config))
		{
			if (m_bIsFirstTimeOpened)
			{
				m_bIsFirstTimeOpened = false;
				m_MapEntity.ZoomOut();
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		if (IsConfigEditor(config))
			m_MapWidget.SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsEditorMapOpen()
	{
		return m_MapEntity && m_MapEntity.IsOpen() && IsConfigEditor(m_MapEntity.GetMapConfig());
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsConfigEditor(MapConfiguration config)
	{
		return config && config.MapEntityMode == EMapEntityMode.EDITOR;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorModeChange(SCR_EditorModeEntity newModeEntity, SCR_EditorModeEntity oldModeEntity)
	{
		SCR_CameraEditorComponent cameraComponent;
		if (oldModeEntity)
		{
			cameraComponent = SCR_CameraEditorComponent.Cast(oldModeEntity.FindComponent(SCR_CameraEditorComponent));
			if (cameraComponent)
				cameraComponent.GetOnCameraCreate().Remove(OnEditorCameraCreate);
		}

		if (newModeEntity)
		{
			cameraComponent = SCR_CameraEditorComponent.Cast(newModeEntity.FindComponent(SCR_CameraEditorComponent));
			if (cameraComponent)
			{
				cameraComponent.GetOnCameraCreate().Insert(OnEditorCameraCreate);
				OnEditorCameraCreate(cameraComponent.GetCamera());
			}

			SetCameraIconColor(newModeEntity.GetModeType());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetCameraIconColor(EEditorMode mode)
	{
		if (!m_EditorCore)
			return;
		
		SCR_EditorModeUIInfo modeUiInfo = m_EditorCore.GetDefaultModeInfo(mode);
		if (modeUiInfo)
			m_CameraIconColor = modeUiInfo.GetModeColor();
		else if (m_EditorCore.GetDefaultModeInfo(EEditorMode.EDIT))
			m_CameraIconColor = m_EditorCore.GetDefaultModeInfo(EEditorMode.EDIT).GetModeColor();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEditorCameraCreate(SCR_ManualCamera editorCamera)
	{
		if (!m_MapEntity || !editorCamera)
			return;
		
		m_EditorCamera = editorCamera;		
		
		if (m_EditorCamera)
			m_EditorCamera.SetInputEnabled(!IsEditorMapOpen());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMenuUpdate()
	{
		if (IsEditorMapOpen())
			m_InputManager.ActivateContext("MapContext");
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		super.HandlerAttachedScripted(w);
		
		if (SCR_Global.IsEditMode())
			return;
		
		m_InputManager = GetGame().GetInputManager();
		if (!m_InputManager)
			return;
		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		if (!m_MapEntity)
			return;
		
		m_MapEntity.GetOnMapInit().Insert(OnMapInit);
		m_MapEntity.GetOnMapOpen().Insert(OnMapOpen);
		m_MapEntity.GetOnMapClose().Insert(OnMapClose);
		
		m_EditorManager = SCR_EditorManagerEntity.GetInstance();
		if (!m_EditorManager)
			return;
		
		m_EditorManager.GetOnModeChange().Insert(OnEditorModeChange);
		
		m_EditorMapManager = SCR_MapEditorComponent.Cast(SCR_MapEditorComponent.GetInstance(SCR_MapEditorComponent));
		if (!m_EditorMapManager)
			return;
		
		m_MapWidget = w;
		
		m_EditorMapManager.SetMapHandler(this);
		m_EditorMapConfigPrefab = m_EditorMapManager.GetMapConfigPrefab();
		
		m_EditorCore = SCR_EditorManagerCore.Cast(SCR_EditorManagerCore.GetInstance(SCR_EditorManagerCore));

		GetMenu().GetOnMenuUpdate().Insert(OnMenuUpdate);
		
		OnEditorModeChange(m_EditorManager.GetCurrentModeEntity(), null);
	}
	
	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.HandlerDeattached(w);
		
		if (IsEditorMapOpen())
			ToggleMap(false, m_EditorMapConfigPrefab);
		
		if (GetMenu())
			GetMenu().GetOnMenuUpdate().Remove(OnMenuUpdate);
	}
}
