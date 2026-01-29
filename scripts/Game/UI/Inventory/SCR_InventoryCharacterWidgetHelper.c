
class SCR_InventoryCharacterWidgetHelper: ScriptedWidgetComponent
{
	private InputManager m_pInputmanager = null;
	private bool m_InputMouseRotate = false;
	private bool m_InputMouseWheel = false;
	private ItemPreviewWidget m_pCharacterWidget = null;
	private WorkspaceWidget m_pWorkspaceWidget = null;
	private float m_fZoomInput = 0;
	private vector m_vRotationInput = vector.Zero;
	//------------------------------------------------------------------------------------------------
	private bool CanProcessEvent(int x, int y)
	{
		if (m_pCharacterWidget)
		{
			float sizeX, sizeY;
			float posX, posY;
			m_pCharacterWidget.GetScreenSize(sizeX, sizeY);
			m_pCharacterWidget.GetScreenPos(posX, posY);
			return (x >= posX && x <= posX + sizeX && y >= posY && y <= posY + sizeY);
		}
		return false;
	}
	//------------------------------------------------------------------------------------------------
	private void ProcessInput()
	{
		// Reset input to prevent update for "fuzzy zero" values
		const float inputResetEpsilon = 10;
		if (m_vRotationInput[0] * m_vRotationInput[0] < inputResetEpsilon)
		{
			m_vRotationInput[0] = 0;
		}
		if (m_vRotationInput[1] * m_vRotationInput[1] < inputResetEpsilon)
		{
			m_vRotationInput[1] = 0;
		}
		if (m_fZoomInput * m_fZoomInput < inputResetEpsilon)
		{
			m_fZoomInput = 0;
		}
		
		if (m_InputMouseRotate)
		{
			m_vRotationInput[1] = Math.Clamp(m_vRotationInput[1] + m_pInputmanager.GetActionValue("Inventory_InspectPanX"), -10, 10);
		}
		if (m_InputMouseWheel)
		{
			m_fZoomInput += 0.01 * m_pInputmanager.GetActionValue("Inventory_InspectZoom");
		}
		
		if (!m_pInputmanager.IsUsingMouseAndKeyboard())
		{
			m_vRotationInput[1] = 1000.0 * m_vRotationInput[1] + m_pInputmanager.GetActionValue("Inventory_InspectPanX");
			m_fZoomInput +=  0.01 * m_pInputmanager.GetActionValue("Inventory_InspectZoom");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button == 0 && w.GetName() == "playerRender" )
		{
			m_InputMouseRotate = CanProcessEvent(x, y);
		}
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button == 0)
			m_InputMouseRotate = false;
		return true;
	}
	
	override bool OnMouseWheel(Widget w, int x, int y, int wheel)
	{
		m_InputMouseWheel = CanProcessEvent(x, y);
		return true;
	}
	
	bool Update(float ts, inout PreviewRenderAttributes playerRenderAttributes)
	{

		const float rotSensitivity = 500;
		const float zoomSensitivity = -64.0;
		const bool extrapolateInput = false;
		
		ProcessInput();
		
		float zoom = 0;
		vector updatedRotation = "0 0 0";
		
		if (extrapolateInput)
		{
			const float maxRotConsume = 400;
			const float maxZoomConsume = 1;
			const float easing = 0.1;
			
			updatedRotation[0] = Math.Clamp(m_vRotationInput[0], -maxRotConsume, maxRotConsume);
			updatedRotation[1] = Math.Clamp(m_vRotationInput[1], -maxRotConsume, maxRotConsume);
		
			float rotXDelta = Math.AbsFloat(updatedRotation[1] / maxRotConsume);
			float rotYDelta = Math.AbsFloat(updatedRotation[0] / maxRotConsume);
		
			updatedRotation[1] = updatedRotation[1] * rotXDelta * Math.Pow(rotXDelta, easing);
			updatedRotation[0] = updatedRotation[0] * rotYDelta * Math.Pow(rotYDelta, easing);
			
			zoom = Math.Clamp(m_fZoomInput, -maxZoomConsume, maxZoomConsume);
			
			float zoomAlpha = Math.AbsFloat(zoom / maxZoomConsume);
			zoom = zoom * zoomAlpha * Math.Pow(zoomAlpha, easing);
		}
		else
		{
			const float interpolationAlpha = 0.15;
			
			updatedRotation[0] = m_vRotationInput[0] * interpolationAlpha;
			updatedRotation[1] = m_vRotationInput[1] * interpolationAlpha;
			zoom = m_fZoomInput * interpolationAlpha;
		}
		
		m_vRotationInput[0] = m_vRotationInput[0] - updatedRotation[0];
		m_vRotationInput[1] = m_vRotationInput[1] - updatedRotation[1];
		
		updatedRotation[2] = 0;
		
		m_fZoomInput -= zoom;
		
		updatedRotation[1] = updatedRotation[1] * ts * rotSensitivity;
		updatedRotation[0] = updatedRotation[0] * ts * rotSensitivity;
		
		zoom = zoom * ts * zoomSensitivity;
		
		bool changesApplied = false;
		
		if (updatedRotation[0] * updatedRotation[0] + updatedRotation[1] * updatedRotation[1] > 0)
		{
			const vector limitsMin = "-30 -180 0";
			const vector limitsMax = "0 180 0";
			playerRenderAttributes.RotateItemCamera(updatedRotation, limitsMin, limitsMax);
			changesApplied = true;
		}
		if (zoom != 0)
		{
			playerRenderAttributes.ZoomCamera(zoom);
			changesApplied = true;
		}
		
		return changesApplied;
	}
	
	//------------------------------------------------------------------------------------------------	
	//!Removes just the UI things
	void Destroy()
	{
		if (m_pWorkspaceWidget)
		{
			m_pWorkspaceWidget.RemoveHandler( this );
			m_pWorkspaceWidget.RemoveFromHierarchy();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryCharacterWidgetHelper( ItemPreviewWidget charWidget, WorkspaceWidget ww )
	{
		if( !charWidget || !ww)
			return;
		
		m_pInputmanager = GetGame().GetInputManager();
		m_pWorkspaceWidget = ww;
		m_pCharacterWidget = charWidget;
		m_pWorkspaceWidget.AddHandler(this);
		
	}
};
