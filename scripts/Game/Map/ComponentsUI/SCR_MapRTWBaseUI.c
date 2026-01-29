[BaseContainerProps()]
class ToolSize
{
	[Attribute("", UIWidgets.EditBox, desc: "X size")]
	int m_iSizeX;
	
	[Attribute("", UIWidgets.EditBox, desc: "Y size")]
	int m_iSizeY;
}

//! Base map UI component for map tools which are using RenderTargetWidget to display
class SCR_MapRTWBaseUI : SCR_MapUIBaseComponent
{
	[Attribute("", UIWidgets.Object, desc: "Array of x*y sizes in unscaled pix")]
	protected ref array<ref ToolSize> m_aSizesArray;
	
	protected static const string BASE_WORLD_TYPE = "ChimeraWorld";
	protected static const float CAMERA_VERTICAL_FOV = 43;
	
	// configuration
	protected string WIDGET_NAME;
	protected string RT_WIDGET_NAME;
	protected string WORLD_RESOURCE = "{88ABCDC0EEC969DF}Prefabs/World/PreviewWorld/MapCompassWorld.et";
	protected string WORLD_NAME;
	protected vector m_vPrefabPos;
	protected vector m_vCameraPos;
	protected vector m_vCameraAngle;
	
	protected bool m_bIsVisible;				// visibility flag
	protected bool m_bWantedVisible;			// holds wanted visibility state after close
	protected bool m_bIsDragged = false;		// widget is being dragged
	protected int m_iCurrentSizeIndex;			// current id of m_SizesArray
	protected int m_iSizesCount;				// count of m_SizesArray
	protected float m_fPosX, m_fPosY;			// widget position
	
	protected EGadgetType m_eGadgetType; 
	protected GenericEntity m_RTEntity;
	protected ref SharedItemRef m_RTWorld;
	protected SCR_MapToolEntry m_ToolMenuEntry;	// tool menu entry
	
	// Widgets
	protected Widget m_wFrame;						// parent frame
	protected RenderTargetWidget m_wRenderTarget;	// RenderTargetWidget
	
	//------------------------------------------------------------------------------------------------
	//! Set widget names
	protected void SetWidgetNames()
	{
		WIDGET_NAME = "";
		RT_WIDGET_NAME = "";
		WORLD_NAME = "MapUIWorld";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Initialise RT camera and prefab positon
	protected void InitPositionVectors()
	{
		m_vPrefabPos = "0 0 0";
		m_vCameraPos = "0 0.3 0";
		m_vCameraAngle = "0 -90 0";
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get prefab resource for display
	//! \return prefab resource
	protected string GetPrefabResource()
	{
		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Find related gadget in inventory
	//! \return gadget or null if not found in inventory
	IEntity FindRelatedGadget()
	{
		IEntity gadget;
		
		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(SCR_PlayerController.GetLocalControlledEntity());
		if (gadgetManager)
			gadget = gadgetManager.GetGadgetByType(m_eGadgetType);
			
		return gadget;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Visibility toggle
	protected void ToggleVisible()
	{
		if (!m_bIsVisible)
			SetVisible(true);
		else
		{
			SetVisible(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set visibility
	//! \param[in] visible is true/false switch
	protected void SetVisible(bool visible)
	{
		if (!m_wFrame)
			return;
		
		if (visible)
		{			
			// RTW preview world
			if (!SetupRTWorld())
				return;
			
			if (!SpawnPrefab())
				return;
						
			m_bIsVisible = true;
			m_wFrame.SetEnabled(true);
			ScriptCallQueue queue = GetGame().GetCallqueue();
			if (queue)
				queue.CallLater(SetFrameVisible, 0, 0);	// delayed by frame so the positioning can be initialised

			FrameSlot.SetPos(m_wFrame, m_fPosX, m_fPosY);
		}
		else 
		{
			m_bIsVisible = false;
			m_wFrame.SetEnabled(false);
			m_wFrame.SetVisible(false);
				
			delete m_RTEntity; // proc anims do not work without this not being refreshed atm
		}
		
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! TODO Frame is now set through script here instead of directly in callqueue to avoid leak
	void SetFrameVisible()
	{
		m_wFrame.SetVisible(true);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup preview world
	//! \return Returns true if successful
	protected bool SetupRTWorld()
	{
		// create new empty base world
		if (!m_RTWorld || !m_RTWorld.IsValid())
		{
			m_RTWorld = BaseWorld.CreateWorld(BASE_WORLD_TYPE, WORLD_NAME);
			if (!m_RTWorld)
				return false;
			World previewRTWorld = m_RTWorld.GetRef();
			previewRTWorld.ReloadSystems();
		}
		
		BaseWorld previewWorld = m_RTWorld.GetRef();
		if (!previewWorld)
			return false;
			
		m_wRenderTarget.SetWorld( previewWorld, 0 );
			
		// spawn preview GenericWorldEntity
		Resource rsc = Resource.Load(WORLD_RESOURCE);
		if (rsc.IsValid())
			GetGame().SpawnEntityPrefabLocal(rsc, previewWorld);
			
		// default cam settings
		previewWorld.SetCameraType(0, CameraType.PERSPECTIVE);
		previewWorld.SetCameraNearPlane(0, 0.001);
		previewWorld.SetCameraFarPlane(0, 50);
		previewWorld.SetCameraVerticalFOV(0, CAMERA_VERTICAL_FOV);
		
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	//! Spawn prefab in the preview world
	//! \return true if successful
	protected bool SpawnPrefab()
	{
		string rscStr = GetPrefabResource();
		if (rscStr == string.Empty)
			return false;
		
		Resource rscItem = Resource.Load(rscStr);
		if (rscItem.IsValid())
		{
			BaseWorld world = m_RTWorld.GetRef();
			if (!world)
				return false;
			
			if (!m_RTEntity)
			{			
				m_RTEntity = GenericEntity.Cast( GetGame().SpawnEntityPrefabLocal(rscItem, world) );
				if (!m_RTEntity) 
					return false;
				
				InitPositionVectors();
				
				m_RTEntity.SetOrigin(m_vPrefabPos);
				m_RTEntity.SetFixedLOD(0);
				
				BaseWorld previewWorld = m_RTWorld.GetRef();
				if (!previewWorld)
					return false;
				
				previewWorld.SetCamera(0, m_vCameraPos, m_vCameraAngle);
				
				SetSize();
			}
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set size of RTW widget, cyclying through an array of prepared sizes 
	//! \param[in] nextSize determines whether current size is kept or swapped to the next one in size array
	protected void SetSize(bool nextSize = false)
	{
		if (nextSize)
		{
			if (m_iCurrentSizeIndex < m_iSizesCount - 1)
				m_iCurrentSizeIndex++;
			else 
				m_iCurrentSizeIndex = 0; 
		}
		
		FrameSlot.SetSize(m_wFrame, m_aSizesArray[m_iCurrentSizeIndex].m_iSizeX, m_aSizesArray[m_iCurrentSizeIndex].m_iSizeY);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Drag event
	//! \param[in] widget
	protected void OnDragWidget(Widget widget)
	{
		if (widget == m_wFrame)
			m_bIsDragged = true;
		else 
			m_bIsDragged = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapToolInteractionUI event
	protected void OnActivateTool(Widget widget)
	{
		if (widget != m_wFrame)
			return;
		
		SetSize(true);
	}
	
	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------

	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{		
		super.OnMapOpen(config);
		
		SetWidgetNames();
		
		// refresh widgets
		m_wFrame = m_RootWidget.FindAnyWidget(WIDGET_NAME);
		m_wRenderTarget = RenderTargetWidget.Cast(m_RootWidget.FindAnyWidget(RT_WIDGET_NAME));		
		m_iSizesCount = m_aSizesArray.Count();
		
		if ( SCR_MapToolInteractionUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolInteractionUI)) )	// if dragging available, add callback
		{
			SCR_MapToolInteractionUI.GetOnDragWidgetInvoker().Insert(OnDragWidget);
			SCR_MapToolInteractionUI.GetOnActivateToolInvoker().Insert(OnActivateTool);
		}
		
		SetVisible(m_bWantedVisible);	// restore last visible state
		
		if (m_ToolMenuEntry)
		{
			if (FindRelatedGadget())
				m_ToolMenuEntry.SetEnabled(true);
			else 
				m_ToolMenuEntry.SetEnabled(false);
		}
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{						
		m_bWantedVisible = m_bIsVisible;	// visibility state
		SetVisible(false);
		
		delete m_RTEntity;
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapRTWBaseUI()
	{
		m_bHookToRoot = true;
	}
}
