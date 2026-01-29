//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class SCR_MapSelectionModule: SCR_MapModuleBase
{
	[Attribute("{9784E36B47AA068A}UI/Textures/Editor/Cursors/GamepadCursor_MultiSelection.edds", UIWidgets.ResourcePickerThumbnail, desc: "Multiselection texture", params: "edds")]
	ResourceName m_sMultiselectTexture;
		
	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "Adjust speed of multiselection expansion. Multiplier of frame timeSlice", params: "1 1000")]
	float m_fMultiselectSpeed;
	
	const float MAX_HOLD_TIME = 4; 			// seconds, after this point circle multi selection no longer expands
	const float MULTISEL_INIT_SIZE = 0.25; 	// multiplier of initial selection texture size -> multiplies screen size, so val 0.2 will result in texture being 1/5th of screens y size 
	
	// arrays
	protected ref array<ref CanvasWidgetCommand> m_Commands = new array<ref CanvasWidgetCommand>; 	// Stores all commands that will be drawn on that canvas

	//! controller multi selection
	protected float m_fHoldTimeSlice = 0;	// Multiselection hold time slice
	protected float m_fSelCircleSize = 0; 	// Multiselection circle size
	protected ref ImageDrawCommand m_SelectionCircle;
	protected ref SharedItemRef m_SelCircleSharedItem;	

	// cached objects
	protected CanvasWidget m_DrawCanvas;	
	protected SCR_MapCursorModule m_CursorModule;
	protected SCR_MapCursorInfo m_CursorInfo;
	
	//------------------------------------------------------------------------------------------------
	// Get multiselection circle size
	float GetSelCircleSize() 
	{ 
		return m_fSelCircleSize;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Render multiselection circle 
	protected void RenderSelectionCircle(float timeSlice)
	{				
		if ( (m_CursorModule.GetCursorState() & EMapCursorState.CS_MULTI_SELECTION) && m_CursorModule.m_eMultiSelType == EMapCursorSelectType.CIRCLE)
		{
		
			if (!m_SelCircleSharedItem)
			{
				m_SelCircleSharedItem = m_DrawCanvas.LoadTexture(m_sMultiselectTexture);
				if (!m_SelCircleSharedItem.IsValid())
					return;
			}
				
			if (m_fHoldTimeSlice < MAX_HOLD_TIME) // max size after set time
				m_fHoldTimeSlice += timeSlice;
			
			m_SelectionCircle = new ImageDrawCommand();
			if (m_SelectionCircle)
			{
				//m_SelectionCircle.m_iColor = 0xffE2A750;
				m_SelectionCircle.m_pTexture = m_SelCircleSharedItem;

				AdjustCircleSize();
				m_SelectionCircle.m_Size = Vector(m_fSelCircleSize, m_fSelCircleSize, 0);
				
				m_SelectionCircle.m_Position = Vector(m_CursorInfo.Scale(m_CursorInfo.x) - m_SelectionCircle.m_Size[0]/2, m_CursorInfo.Scale(m_CursorInfo.y) - m_SelectionCircle.m_Size[1]/2, 0);
				m_SelectionCircle.m_iFlags = (WidgetFlags.STRETCH |  WidgetFlags.BLEND);
			
				if (m_Commands.Find(m_SelectionCircle) == -1)
						m_Commands.Insert(m_SelectionCircle);
			}			
		}
		else 
		{
			m_fHoldTimeSlice = 0;
			m_fSelCircleSize = 0;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Adjust multiselection circle size based on hold time 
	protected void AdjustCircleSize()
	{
		float sizeX, sizeY;
		m_DrawCanvas.GetScreenSize(sizeX, sizeY);
		
		if (m_fHoldTimeSlice < 0.2)
		{
			m_fSelCircleSize = Math.Lerp(0, sizeY * MULTISEL_INIT_SIZE, m_fHoldTimeSlice * 5);
		}
		else 
		{
			m_fSelCircleSize = (sizeY * MULTISEL_INIT_SIZE) + (m_fHoldTimeSlice * m_fMultiselectSpeed);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Render multiselection rectangle 
	protected void RenderSelectionRectangle()
	{
		if ( (m_CursorModule.GetCursorState() & EMapCursorState.CS_MULTI_SELECTION) && m_CursorModule.m_eMultiSelType == EMapCursorSelectType.RECTANGLE)
		{
			m_MapEntity.SetSelection( Vector(m_CursorInfo.Scale(m_CursorInfo.startPosMultiSel[0]),0,m_CursorInfo.Scale(m_CursorInfo.startPosMultiSel[1])), Vector(m_CursorInfo.Scale(m_CursorInfo.x),0,m_CursorInfo.Scale(m_CursorInfo.y)));
		}
		else
		{
			m_MapEntity.ResetSelection();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
		m_DrawCanvas = CanvasWidget.Cast(config.RootWidgetRef.FindAnyWidget(SCR_MapConstants.DRAWING_WIDGET_NAME));
		m_CursorInfo = m_CursorModule.GetCursorInfo();
		
		m_DrawCanvas.SetZoom(m_MapEntity.GetCurrentZoom() / m_MapEntity.GetMapWidget().PixelPerUnit());
		m_DrawCanvas.SetOffsetPx(m_MapEntity.GetCurrentPan() * -1);
		
		m_MapEntity.ResetSelection();		
	}
		
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		// cleanup
		m_SelectionCircle = null;
		m_Commands.Clear();

		// only when cursor module is active
		if (m_CursorModule)
		{
			RenderSelectionCircle(timeSlice);
			RenderSelectionRectangle();
		}
		
		if(m_Commands.Count() > 0)
		{
			m_DrawCanvas.SetDrawCommands(m_Commands);
		}
	}
};