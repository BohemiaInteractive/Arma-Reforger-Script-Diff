[EntityEditorProps(category: "GameScripted/UI/Inventory", description: "Inventory Item Inspection UI class")]

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory Item Inspection UI Layout
class SCR_InventoryInspectionUI : ScriptedWidgetComponent
{
	protected WorkspaceWidget 			m_workspaceWidget 		= null;
	Widget								m_widget 				= null;	
	protected RenderTargetWidget		m_wItemRender 			= null;
	protected InputManager				m_pInputmanager			= null;
	InventoryItemComponent				m_pItem					= null;
	SCR_InventoryStorageBaseUI		m_inventoryStorage		= null;
	BaseWorld							m_PreviewWorld			= null;
	ref SharedItemRef					m_PreviewSharedItemRef	= null;
	int									m_PreviewCamera			= -1;
	GenericEntity						m_PreviewEntity			= null;
	vector								m_PreviewRotation		= vector.Zero;
	float								m_PreviewDistance		= 0;
	vector								m_PreviewTargetRotation	= vector.Zero;
	float								m_PreviewTargetDistance	= 0;
	bool								m_InputMouseRotate		= false;
	protected SCR_InventoryNavigationButtonBack					m_wButtonTraverseBack;
	
	
	const string					ITEM_LAYOUT = "{9A9CCE3707075EF4}UI/layouts/Menus/Inventory/InventoryInspectionPanel.layout";
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonDown(Widget w, int x, int y, int button)
	{
		if (button == 1)
			m_InputMouseRotate = true;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnMouseButtonUp(Widget w, int x, int y, int button)
	{
		if (button == 1)
			m_InputMouseRotate = false;
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Init()
	{
		m_widget = m_workspaceWidget.CreateWidgets( ITEM_LAYOUT ); //, m_inventoryStorage.GetInventoryMenuHandler().GetRootWidget() );
		m_widget.AddHandler( this );
		m_wItemRender = RenderTargetWidget.Cast( m_widget.FindAnyWidget( "InspectRender" ) );
				
		CreatePreview();
		
		m_wButtonTraverseBack = SCR_InventoryNavigationButtonBack.Cast( SCR_InventoryNavigationButtonBack.GetInputButtonComponent( "ButtonTraverseBack", m_widget ) );
		if ( m_wButtonTraverseBack )
		{
			m_wButtonTraverseBack.SetVisible( true );
			m_wButtonTraverseBack.m_OnActivated.Insert( m_inventoryStorage.GetInventoryMenuHandler().Action_Inspect );					
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deletes the preview world
	void DeletePreview()
	{
		if (m_PreviewSharedItemRef)
		{
			m_PreviewCamera = -1;
			m_PreviewEntity = null;
			m_PreviewWorld = null;
			m_PreviewSharedItemRef = null;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates the preview world
	protected void CreatePreview()
	{
		DeletePreview();
		
		m_PreviewSharedItemRef = BaseWorld.CreateWorld("InspectionPreview", "InspectionPreview");
		m_PreviewWorld = m_PreviewSharedItemRef.GetRef();
		m_PreviewCamera = 0;
		
		m_PreviewWorld.SetCameraType(m_PreviewCamera, CameraType.PERSPECTIVE);
		m_PreviewWorld.SetCameraFarPlane(m_PreviewCamera, 50);
		m_PreviewWorld.SetCameraNearPlane(m_PreviewCamera, 0.001);
		//m_PreviewWorld.SetCameraVerticalFOV(m_PreviewCamera, 60);
		
		//Create a preview world entity, this contains the lighting setup for the worlds.
		//Resource rsc = Resource.Load("{43DA8BEC8468C7F5}worlds/TestMaps/PreviewWorld/PreviewWorld.et");
		//Resource rsc = Resource.Load("{C63632BF8C1F7849}worlds/TestMaps/PreviewWorld/InspectionWorld.et");
		Resource rsc = Resource.Load("{4391FE7994EE6FE2}worlds/Sandbox/InventoryPreviewWorld10.et");
		if (rsc.IsValid())
			GetGame().SpawnEntityPrefab(rsc, m_PreviewWorld);
		
		//Create a generic entity that the mesh resource can be assigned to.
		m_PreviewEntity = GenericEntity.Cast(m_pItem.CreatePreviewEntity(m_PreviewWorld, m_PreviewCamera));
		CenterPreviewObject(vector.Zero);
		ResetPreviewCamera();
		
		m_wItemRender.SetWorld(m_PreviewWorld, m_PreviewCamera);
		//m_wItemRender.Update();
	}
	
	//------------------------------------------------------------------------------------------------	
	//!Removes just the UI slot
	void Destroy()
	{
		DeletePreview();
		m_widget.RemoveHandler( this );
		m_widget.RemoveFromHierarchy();
	}
	
	//------------------------------------------------------------------------------------------------	
	Widget GetWidget()	{ return m_widget; }
	
	//------------------------------------------------------------------------------------------------
	//! Centers the preview object to the input position
	void CenterPreviewObject(vector center)
	{
		vector mins, maxs;
		m_PreviewEntity.GetBounds(mins, maxs);
		m_PreviewEntity.SetOrigin((mins - maxs) * 0.5 - mins + center);
		m_PreviewEntity.Update();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Resets the camera to the default rotation and distance based on the preview object's size
	void ResetPreviewCamera()
	{
		vector mins, maxs;
		m_PreviewEntity.GetBounds(mins, maxs);
		
		m_PreviewRotation = "-45 -20 0";
		m_PreviewDistance = vector.Distance(mins, maxs);
		m_PreviewTargetRotation = m_PreviewRotation;
		m_PreviewTargetDistance = m_PreviewDistance;
		
		vector camMat[4];
		Math3D.AnglesToMatrix(m_PreviewRotation, camMat);
		camMat[3] = m_PreviewDistance * camMat[2] * -1;
		m_PreviewWorld.SetCameraEx(m_PreviewCamera, camMat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Update the preview
	void UpdateView(float timeSlice)
	{
		if (!m_PreviewWorld)
			return;
		m_PreviewTargetDistance += timeSlice * -0.003 * Math.Clamp(m_PreviewTargetDistance, 0, 1) * m_pInputmanager.GetActionValue("Inventory_InspectZoom");
		m_PreviewTargetDistance = Math.Clamp(m_PreviewTargetDistance, 0.1, 2);
		
		if (m_InputMouseRotate)
		{
			m_PreviewTargetRotation[0] = timeSlice * 0.1 * m_pInputmanager.GetActionValue("Inventory_InspectPanX") + m_PreviewTargetRotation[0];
			m_PreviewTargetRotation[1] = timeSlice * -0.1 * m_pInputmanager.GetActionValue("Inventory_InspectPanY") + m_PreviewTargetRotation[1];
		}
		else if (m_pInputmanager.GetLastUsedInputDevice() == EInputDeviceType.GAMEPAD)
		{
			m_PreviewTargetRotation[0] = timeSlice * 100.0 * m_pInputmanager.GetActionValue("Inventory_InspectPanX") + m_PreviewTargetRotation[0];
			m_PreviewTargetRotation[1] = timeSlice * 100.0 * m_pInputmanager.GetActionValue("Inventory_InspectPanY") + m_PreviewTargetRotation[1];
		}
		
		float lerpTime = Math.Clamp(timeSlice * 5, 0, 1);
		
		m_PreviewRotation += (m_PreviewTargetRotation - m_PreviewRotation) * lerpTime;
		m_PreviewDistance += (m_PreviewTargetDistance - m_PreviewDistance) * lerpTime;
		
		vector camMat[4];
		Math3D.AnglesToMatrix(m_PreviewRotation, camMat);
		camMat[3] = m_PreviewDistance * camMat[2] * -1;
		m_PreviewWorld.SetCameraEx(m_PreviewCamera, camMat);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_InventoryInspectionUI( InventoryItemComponent pComponent = null, SCR_InventoryStorageBaseUI pStorageUI = null, BaseWorld playerWorld = null )
	{
		if( !pComponent )
			return;
		
		m_pItem = pComponent;
		m_inventoryStorage = pStorageUI;
				
		m_workspaceWidget = GetGame().GetWorkspace();
		m_pInputmanager = GetGame().GetInputManager();
		
		
		Init();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_InventoryInspectionUI()
	{
		DeletePreview();
	}
};
