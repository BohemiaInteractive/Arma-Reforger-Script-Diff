class SCR_CameraInfoEditorUIComponent : MenuRootSubComponent
{
	[Attribute(desc: "%1: vertical FOV (multiplier)\n%2: focal length (mm)\n%3 focus distance (m)")]
	protected LocalizedString m_Text;
	
	[Attribute("35")]
	protected int m_CameraLens;
	
	protected SCR_ManualCamera m_Camera;
	protected TextWidget m_TextWidget;
	
	protected SCR_FocusManualCameraComponent m_FocusComponent;
	protected SCR_ZoomManualCameraComponent m_ZoomComponent;
	
	//------------------------------------------------------------------------------------------------
	protected void Refresh()
	{
		float verticalFOV = m_Camera.GetVerticalFOV();
		float zoomCoef = m_Camera.GetDefaultFOV() / verticalFOV;
		int focalLength = zoomCoef * m_CameraLens;
		
		float focusDistance = -1;
		if (m_FocusComponent)
			focusDistance = m_FocusComponent.GetFocusDistance();
		
		string focusDistanceString = "-";
		if (focusDistance != -1)
			focusDistanceString = focusDistance.ToString(1, 1);
		
		m_TextWidget.SetTextFormat(m_Text, zoomCoef.ToString(1, 1), focalLength, focusDistanceString);
	}

	//------------------------------------------------------------------------------------------------
	protected void InitCamera(SCR_ManualCamera camera)
	{
		m_Camera = camera;
		
		m_ZoomComponent = SCR_ZoomManualCameraComponent.Cast(m_Camera.FindCameraComponent(SCR_ZoomManualCameraComponent));
		if (m_ZoomComponent)
			m_ZoomComponent.GetOnZoomChange().Insert(Refresh);
		
		m_FocusComponent = SCR_FocusManualCameraComponent.Cast(m_Camera.FindCameraComponent(SCR_FocusManualCameraComponent));
		if (m_FocusComponent)
			m_FocusComponent.GetOnFocusDistanceChange().Insert(Refresh);
		
		Refresh();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnCameraCreate(SCR_ManualCamera camera)
	{
		InitCamera(camera);
		
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent));
		cameraManager.GetOnCameraCreate().Remove(OnCameraCreate);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttachedScripted(Widget w)
	{
		m_TextWidget = TextWidget.Cast(w);		
		if (!m_TextWidget)
		{
			Debug.Error("SCR_CameraInfoEditorUIComponent must be attached to a text widget!");
			return;
		}
		
		SCR_CameraEditorComponent cameraManager = SCR_CameraEditorComponent.Cast(SCR_CameraEditorComponent.GetInstance(SCR_CameraEditorComponent, true));
		if (!cameraManager)
			return;
		
		SCR_ManualCamera camera = cameraManager.GetCamera();
		if (camera)
		{
			InitCamera(camera);
		}
		else
		{
			cameraManager.GetOnCameraCreate().Insert(OnCameraCreate);
			return;
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_ZoomComponent)
			m_ZoomComponent.GetOnZoomChange().Remove(Refresh);
	
		if (m_FocusComponent)
			m_FocusComponent.GetOnFocusDistanceChange().Remove(Refresh);
	}
}
