class SCR_MapRulerUI : SCR_MapUIBaseComponent
{
	[Attribute(defvalue: "1994", uiwidget: UIWidgets.EditBox, desc: "pix, exact length of the ruler within the provided image, default base length being 1km in world space")]
	float m_fRulerLength;

	[Attribute("RulerFrame", UIWidgets.EditBox, desc: "Root frame widget name")]
	string m_sRootWidgetName;
	
	[Attribute("RulerImage", UIWidgets.EditBox, desc: "Ruler image widget name")]
	string m_sImageWidgetName; 
	
	[Attribute("ruler", UIWidgets.EditBox, desc: "Toolmenu imageset quad name")]
	string m_sToolMenuIconName;
	
	[Attribute("", UIWidgets.Object, desc: "Array of sizes, multiplicator values of default base ruler length of 1km")]
	protected ref array<int> m_aSizesArray; 

	protected bool m_bIsVisible;
	protected bool m_bWantedVisible;
	protected bool m_bIsDragged;
	protected bool m_bIsZooming;
	protected int m_iCurrentSizeIndex;			// current id of m_SizesArray
	protected int m_iSizesCount;				// count of m_SizesArray
	protected float m_fPosX, m_fPosY;			// widget position in scaled screen coords
	protected float m_fWorldX, m_fWorldY;		// widget world pos
	protected float m_fAngle;					// cached angle for map reopen
	protected float m_fBaseImageSize[2];		// for calculation of ruler scaling
	protected float m_fSizeCoef;				// ruler size coefficient based on image size and ruler length 
	protected vector m_vMapPan;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	
	// Widgets
	protected Widget m_wFrame;
	protected ImageWidget m_wImage;
	protected WorkspaceWidget m_wWorkspace;
		
	//------------------------------------------------------------------------------------------------
	//! Visibility toggle
	protected void ToggleVisible()
	{
		if (!m_bIsVisible)
			SetVisible(true);
		else
			SetVisible(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set visibility
	//! \param[in] visible is true/false switch
	//! \param[in] saveState determines whether this is visibility set during closing of the map, so the pos and rotation should be saved
	protected void SetVisible(bool visible, bool saveState = false)
	{
		m_bIsVisible = visible;
		m_wFrame.SetEnabled(visible);
		m_wFrame.SetVisible(visible);
		
		if (visible)
		{
			if (!m_wImage)
				return;

			ChimeraCharacter player = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
			if (!player || !player.GetCharacterController())
				return;

			CharacterControllerComponent controller = player.GetCharacterController();
			if (!controller)
				return;

			IEntity mapItem = controller.GetAttachedGadgetAtLeftHandSlot();
			if (!mapItem)
			{
				SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.GetGadgetManager(player);
				if (!gadgetManager)
					return;

				mapItem = gadgetManager.GetGadgetByType(EGadgetType.MAP);
				if (!mapItem)
					return;
			}

			SCR_MapGadgetComponent mapGadget = SCR_MapGadgetComponent.Cast(mapItem.FindComponent(SCR_MapGadgetComponent));
			if (!mapGadget)
				return;

			SCR_MapGadgetComponentClass mapGadgetData = SCR_MapGadgetComponentClass.Cast(mapGadget.GetComponentData(mapItem));
			if (!mapGadgetData)
				return;

			m_fRulerLength = mapGadgetData.GetRulerLength();
			if (m_wImage.LoadImageTexture(0, mapGadgetData.GetProtractorTexture()))
				m_wImage.SetImage(0);

			float zoomVal = m_MapEntity.GetCurrentZoom();
			m_fSizeCoef = 1000 / (m_fRulerLength / m_fBaseImageSize[0]); // (ruler real length%) / 1000 pix(meters)
			float sizeVal = m_wWorkspace.DPIUnscale(zoomVal * m_fSizeCoef);
			SetSize(sizeVal, sizeVal);
			
			if (m_fPosX == 0 && m_fPosY == 0)
			{
				float sizeX, sizeY;
				m_MapEntity.GetMapWidget().GetScreenSize(sizeX, sizeY);
				m_fPosX = sizeX * 0.5;
				m_fPosY = sizeY * 0.5;
				
				m_MapEntity.ScreenToWorld(m_fPosX, m_fPosY, m_fWorldX, m_fWorldY);
			}
			
			FrameSlot.SetPos(m_wFrame, m_wWorkspace.DPIUnscale(m_fPosX), m_wWorkspace.DPIUnscale(m_fPosY));
			m_wImage.SetRotation(m_fAngle);
			m_vMapPan = m_MapEntity.GetCurrentPan();
			
			m_MapEntity.GetOnMapZoom().Insert(OnMapZoom);	// zoom for scaling
			m_MapEntity.GetOnMapPan().Insert(OnMapPan);		// pan for scaling
		}
		else
		{
			if (saveState)	// save angle
			{
				m_fAngle = m_wImage.GetRotation();
			}
			else 
			{
				m_fAngle = 0;
				m_fPosX = 0;
				m_fPosY = 0;
			}

			m_MapEntity.GetOnMapZoom().Remove(OnMapZoom);	// zoom for scaling
			m_MapEntity.GetOnMapPan().Remove(OnMapPan);		// zoom for scaling
		}
		
		if (m_ToolMenuEntry)
			m_ToolMenuEntry.SetActive(visible);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set size of image, multiply by current size mode
	//! \param[in] x is unscaled size in px
	//! \param[in] y is unscaled size in px
	//! \param[in] nextSize determines whether current size is kept or swapped to the next one in size array
	protected void SetSize(float x, float y, bool nextSize = false)
	{
		if (nextSize)
		{
			if (m_iCurrentSizeIndex < m_iSizesCount - 1)
				m_iCurrentSizeIndex++;
			else 
				m_iCurrentSizeIndex = 0; 
		}
		
		m_wImage.SetSize(x * m_aSizesArray[m_iCurrentSizeIndex], y * m_aSizesArray[m_iCurrentSizeIndex]);

	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapZoom(float zoomVal)
	{
		float sizeVal = m_wWorkspace.DPIUnscale(zoomVal * m_fSizeCoef);
		SetSize(sizeVal, sizeVal);
		m_bIsZooming = true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	protected void OnMapPan(float x, float y, bool adjustedPan)
	{
		if (m_bIsZooming)
		{
			m_bIsZooming = false;
			
			float screenX, screenY;
			m_MapEntity.WorldToScreen( m_fWorldX, m_fWorldY, screenX, screenY, true );
				
			m_fPosX = screenX;
			m_fPosY = screenY;
		}
		else 
		{
			m_fPosX -= m_vMapPan[0] - m_wWorkspace.DPIScale(x);
			m_fPosY -= m_vMapPan[1] - m_wWorkspace.DPIScale(y);
		}
			
		FrameSlot.SetPos(m_wFrame, m_wWorkspace.DPIUnscale(m_fPosX), m_wWorkspace.DPIUnscale(m_fPosY));	
		m_vMapPan = m_MapEntity.GetCurrentPan();
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapToolInteractionUI event
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
		if (!m_wImage)
			return;

		m_wImage.SetRotation(0);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputQuickBind(float value, EActionTrigger reason)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && IsToolMenuFocused())
			return;	
		ToggleVisible();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
				
		// refresh widgets
		m_wFrame = m_RootWidget.FindAnyWidget(m_sRootWidgetName);
		m_wImage = ImageWidget.Cast(m_RootWidget.FindAnyWidget(m_sImageWidgetName));
		
		int x, y;
		m_wImage.GetImageSize(0, x, y);
		m_fBaseImageSize[0] = x;
		m_fBaseImageSize[1] = y;
		
		m_iSizesCount = m_aSizesArray.Count();
		
		if ( SCR_MapToolInteractionUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolInteractionUI)) )	// if dragging available, add callback
		{
			SCR_MapToolInteractionUI.GetOnDragWidgetInvoker().Insert(OnDragWidget);
			SCR_MapToolInteractionUI.GetOnActivateToolInvoker().Insert(OnActivateTool);
		}
				
		SetVisible(m_bWantedVisible);	// restore last visible state
		OnMapZoom(m_MapEntity.GetCurrentZoom());
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{				
		m_bWantedVisible = m_bIsVisible;	// visibility state
		SetVisible(false, true);
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		m_fPosX = 0;
		m_fPosY = 0;
		m_wWorkspace = GetGame().GetWorkspace();
		
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (toolMenu)
		{
			m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, m_sToolMenuIconName, 10, m_bIsExclusive); // add to menu
			m_ToolMenuEntry.m_OnClick.Insert(ToggleVisible);
			m_ToolMenuEntry.SetEnabled(true);
			
			GetGame().GetInputManager().AddActionListener("MapToolProtractor", EActionTrigger.DOWN, OnInputQuickBind);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{		
		if (m_bIsVisible && m_bIsDragged)
		{
			// save position for map reopen
			float sizeX, sizeY;
			m_wFrame.GetScreenPos(m_fPosX, m_fPosY);
			m_wFrame.GetScreenSize(sizeX, sizeY);
			m_fPosX = m_fPosX + sizeX * 0.5;
			m_fPosY = m_fPosY + sizeY * 0.5;
			
			m_MapEntity.ScreenToWorld(m_fPosX, m_fPosY, m_fWorldX, m_fWorldY);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_MapRulerUI()
	{
		m_bHookToRoot = true;
	}
}
