//! Map line
class MapLine
{
	protected static const ref Color BUTTON_RED = Color.FromSRGBA(197, 75, 75, 255);
	
	bool m_bIsLineDrawn;		// if line is drawn when closing the map, redraw it on reopen
	protected bool m_bIsInFocus;
	protected float m_fStartPointX, m_fStartPointY;
	protected float m_fEndPointX, m_fEndPointY;
	protected Widget m_wLine;
	protected ImageWidget m_wLineImage;
	protected ButtonWidget m_wDeleteButton;
	protected SCR_ButtonImageComponent m_DeleteButtonComp;
	protected SCR_MapEntity m_MapEntity;
	protected SCR_MapDrawingUI m_OwnerComponent;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] rootW
	//! \param[in] drawStart
	void CreateLine(notnull Widget rootW, bool drawStart = false)
	{
		Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame)
			return;
		
		if (drawStart)
			m_MapEntity.GetMapCursorWorldPosition(m_fStartPointX, m_fStartPointY);
		else 
			m_bIsLineDrawn = true;
		
		m_wLine = GetGame().GetWorkspace().CreateWidgets("{E8850FCD9219C411}UI/layouts/Map/MapDrawLine.layout", mapFrame);
		m_wLineImage = ImageWidget.Cast(m_wLine.FindAnyWidget("DrawLineImage"));
			
		Widget deleteButtonFrame = GetGame().GetWorkspace().CreateWidgets("{F486FAEEA00A5218}UI/layouts/Map/MapLineDeleteButton.layout", mapFrame);
		if (!deleteButtonFrame)
			return;

		m_wDeleteButton = ButtonWidget.Cast(deleteButtonFrame.FindAnyWidget("DelButton"));
		if (!m_wDeleteButton)
			return;

		m_DeleteButtonComp = SCR_ButtonImageComponent.Cast(m_wDeleteButton.FindHandler(SCR_ButtonImageComponent));
		m_DeleteButtonComp.m_OnClicked.Insert(OnButtonClick);
		m_DeleteButtonComp.m_OnFocus.Insert(OnButtonFocus);
		m_DeleteButtonComp.m_OnFocusLost.Insert(OnButtonFocusLost);
		m_DeleteButtonComp.m_OnMouseEnter.Insert(OnMouseEnter);
		m_DeleteButtonComp.m_OnMouseLeave.Insert(OnMouseLeave);
		m_DeleteButtonComp.GetImageWidget().SetColor(BUTTON_RED);
		
		m_OwnerComponent.m_iLinesDrawn++;
		m_OwnerComponent.UpdateLineCount();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonClick()
	{
		if (m_OwnerComponent.m_bIsLineBeingDrawn)
			return;

		if (m_OwnerComponent.IsUsingGamepad() && !m_bIsInFocus)
			return;

		DestroyLine(false);
		m_OwnerComponent.m_bActivationThrottle = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocus()
	{
		if (m_OwnerComponent.m_bIsLineBeingDrawn)
			return;

		if (m_OwnerComponent.IsUsingGamepad())
		{
			m_bIsInFocus = false;
			array<Widget> tracedW = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
			foreach (Widget w : tracedW)
			{
				if (w == m_wDeleteButton)
				{
					m_bIsInFocus = true;
					break;
				}
			}

			if (!m_bIsInFocus)
				return;
		}

		m_DeleteButtonComp.GetImageWidget().SetColor(Color.FromInt(UIColors.CONTRAST_COLOR.PackToInt()));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnButtonFocusLost(Widget w)
	{
		m_bIsInFocus = false;
		m_DeleteButtonComp.GetImageWidget().SetColor(BUTTON_RED);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMouseEnter(Widget w)
	{
		GetGame().GetWorkspace().SetFocusedWidget(w);
		OnButtonFocus();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMouseLeave(Widget w)
	{
		GetGame().GetWorkspace().SetFocusedWidget(null);
		OnButtonFocusLost(w);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] target
	void SetButtonVisible(bool target)
	{
		m_wDeleteButton.SetEnabled(target);
		m_wDeleteButton.SetVisible(target);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] cacheDrawn
	void DestroyLine(bool cacheDrawn = false)
	{
		if (m_wLine)
			m_wLine.RemoveFromHierarchy();
		
		if (m_wDeleteButton)
			m_wDeleteButton.RemoveFromHierarchy();
		
		if (!cacheDrawn)
			m_bIsLineDrawn = false;
		
		m_OwnerComponent.m_iLinesDrawn--;
		m_OwnerComponent.UpdateLineCount();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] updateEndPos
	void UpdateLine(bool updateEndPos)
	{
		if (!m_wLine)	// can happen due to callater used for update
			return;

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace)
			return;

		if (updateEndPos)
			m_MapEntity.GetMapCursorWorldPosition(m_fEndPointX, m_fEndPointY);	

		int screenX, screenY, endX, endY;

		m_MapEntity.WorldToScreen(m_fStartPointX, m_fStartPointY, screenX, screenY, true);
		m_MapEntity.WorldToScreen(m_fEndPointX, m_fEndPointY, endX, endY, true);

		vector lineVector = vector.Zero;
		lineVector[0] = m_fStartPointX - m_fEndPointX;
		lineVector[1] = m_fStartPointY - m_fEndPointY;

		vector angles = lineVector.VectorToAngles();
		if (angles[0] == 90)
			angles[1] =  180 - angles[1]; 	// reverse angles when passing vertical axis
		else if (angles[0] == 0)
			angles[1] =  180 + angles[1];	// fix for case for when line is vertically straight

		m_wLineImage.SetRotation(angles[1]);

		lineVector = lineVector * m_MapEntity.GetCurrentZoom();

		float length = workspace.DPIUnscale(lineVector.Length());

		m_wLineImage.SetSize(length, 50);

		FrameSlot.SetPos(m_wLine, workspace.DPIUnscale(screenX), workspace.DPIUnscale(screenY));	// needs unscaled coords

		lineVector[0] = workspace.DPIUnscale((screenX + endX) * 0.5);
		lineVector[1] = workspace.DPIUnscale((screenY + endY) * 0.5);
		FrameSlot.SetPos(m_wDeleteButton, lineVector[0], lineVector[1]);	//del button

	}
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] mapEnt
	//! \param[in] ownerComp
	void MapLine(SCR_MapEntity mapEnt, SCR_MapDrawingUI ownerComp)
	{
		m_MapEntity = mapEnt;
		m_OwnerComponent = ownerComp;
	}
}

//! Temporary drawing substitute so the protractor can be utilized properly
class SCR_MapDrawingUI : SCR_MapUIBaseComponent
{	
	[Attribute("editor", UIWidgets.EditBox, desc: "Toolmenu imageset quad name")]
	protected string m_sToolMenuIconName;
	
	[Attribute("9", UIWidgets.EditBox, desc: "Max line count")]
	protected int m_iLineCount;
	
	bool m_bActivationThrottle; 	// onclick will be called same frame draw mode activates/ delete button is clicked, this bool is used to ignore it
	protected bool m_bIsDrawModeActive;
	bool m_bIsLineBeingDrawn;
	int m_iLinesDrawn; 				// count of currently drawn lines
	protected int m_iLineID;		// active line id
	
	protected Widget m_wDrawingContainer;
	
	protected SCR_MapCursorModule 	m_CursorModule;
	protected SCR_MapToolEntry m_ToolMenuEntry;
	protected ref array<ref MapLine> m_aLines = new array <ref MapLine>();

	//------------------------------------------------------------------------------------------------
	bool IsUsingGamepad()
	{
		if (!m_CursorModule)
			return false;

		SCR_MapCursorInfo cursorInfo = m_CursorModule.GetCursorInfo();
		if (!cursorInfo)
			return false;

		return cursorInfo && cursorInfo.isGamepad;
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle draw mode
	protected void ToggleDrawMode()
	{
		if (!m_bIsDrawModeActive)
			SetDrawMode(true);
		else 
			SetDrawMode(false);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Start/stop draw mode
	//! \param[in] state
	//! \param[in] cacheDrawn
	protected void SetDrawMode(bool state, bool cacheDrawn = false)
	{
		if (state)
		{
			if (!m_CursorModule.HandleDraw(true))	// draw restricted, return here
				return;

			GetGame().GetInputManager().AddActionListener("MapSelect", EActionTrigger.UP, OnMapClick);
			m_bActivationThrottle = true;
			
			for (int i; i < m_iLineCount; i++)
			{
				if (m_aLines[i].m_bIsLineDrawn)
					m_aLines[i].SetButtonVisible(true);
			}
		}
		else 
		{
			GetGame().GetInputManager().RemoveActionListener("MapSelect", EActionTrigger.UP, OnMapClick);
			GetGame().GetInputManager().RemoveActionListener("MapContextualMenu", EActionTrigger.UP, OnMapModifierClick);
			
			m_CursorModule.HandleDraw(false); // in case drawing was in progress
			
			for (int i; i < m_iLineCount; i++)	
			{	
				if (m_bIsLineBeingDrawn && i == m_iLineID)	// if drawing in progress, dont cache
				{
					m_aLines[i].DestroyLine(false);
					m_bIsLineBeingDrawn = false;
					m_iLineID = -1;
				}
				else if (cacheDrawn)						// map is closing, cache drawn lines and destroy
				{
					m_aLines[i].DestroyLine(true);
				}
				else if (m_aLines[i].m_bIsLineDrawn)
				{
					m_aLines[i].SetButtonVisible(false);	// only draw mode disabled
				}
			}
		}

		m_bIsDrawModeActive = state;
		m_ToolMenuEntry.SetActive(state);
		m_ToolMenuEntry.m_ButtonComp.SetTextVisible(state);
		UpdateLineCount();
	}
	
	//------------------------------------------------------------------------------------------------		
	//!
	void UpdateLineCount()
	{
		m_ToolMenuEntry.m_ButtonComp.SetText(m_iLinesDrawn.ToString() + "/" + m_iLineCount.ToString());
	}
				
	//------------------------------------------------------------------------------------------------		
	protected void OnMapClick(float value, EActionTrigger reason)
	{				
		if (m_bActivationThrottle)
		{
			m_bActivationThrottle = false;
			return;
		}
		
		EMapEntityMode mode = m_MapEntity.GetMapConfig().MapEntityMode;

		if (mode == EMapEntityMode.SPAWNSCREEN)
		{
			SetDrawMode(false);
			return;
		}	

		if (m_bIsLineBeingDrawn)	// state 2, line drawing in progress
		{
			m_bIsLineBeingDrawn = false;
			m_aLines[m_iLineID].m_bIsLineDrawn = true;
			m_iLineID = -1;

			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_MARKER_DRAW_STOP);
			GetGame().GetInputManager().RemoveActionListener("MapContextualMenu", EActionTrigger.UP, OnMapModifierClick);
			
			return;
		}
		
		if (m_iLinesDrawn >= m_iLineCount)
			return;
		
		if (m_CursorModule.GetCursorState() & SCR_MapCursorModule.STATE_DRAW_RESTRICTED)	// draw restricted, return here
			return;
		
		for (int i; i < m_iLineCount; i++)	// state 1, start drawing line
		{
			if (!m_aLines[i].m_bIsLineDrawn)
			{
				m_aLines[i].CreateLine(m_wDrawingContainer, true);
				m_bIsLineBeingDrawn = true;
				m_iLineID = i;
				SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_MARKER_DRAW_START);
				GetGame().GetInputManager().AddActionListener("MapContextualMenu", EActionTrigger.UP, OnMapModifierClick);
				
				return;
			}
				
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method that is used to cancel line drawing and remove line that wasnt finished
	//! \param[in] value
	//! \param[in] reason
	protected void OnMapModifierClick(float value, EActionTrigger reason)
	{
		GetGame().GetInputManager().RemoveActionListener("MapContextualMenu", EActionTrigger.UP, OnMapModifierClick);
		if (!m_bIsLineBeingDrawn || m_iLineID < 0)
			return;

		m_bIsLineBeingDrawn = false;
		m_aLines[m_iLineID].DestroyLine();
		m_iLineID = -1;
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_MARKER_DRAW_STOP);
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	//! \param[in] x
	//! \param[in] y
	//! \param[in] adjustedPan
	protected void OnMapPan(float x, float y, bool adjustedPan)
	{	
		for (int i; i < m_iLineCount; i++)
		{
			if (m_aLines[i].m_bIsLineDrawn) 
				m_aLines[i].UpdateLine(false);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! SCR_MapEntity event
	//! \param[in] x
	//! \param[in] y
	protected void OnMapPanEnd(float x, float y)
	{
		for (int i; i < m_iLineCount; i++)
		{
			if (m_aLines[i].m_bIsLineDrawn) 
				GetGame().GetCallqueue().CallLater(m_aLines[i].UpdateLine, 0, false, false); // needs to be delayed by a frame as it cant always update the size after zoom correctly within the same frame
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	//! SCR_MapToolEntry event
	//! \param[in] entry
	protected void OnEntryToggled(SCR_MapToolEntry entry)
	{
		if (m_bIsDrawModeActive && entry != m_ToolMenuEntry)
			SetDrawMode(false);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnInputQuickBind(float value, EActionTrigger reason)
	{
		if (!GetGame().GetInputManager().IsUsingMouseAndKeyboard() && IsToolMenuFocused())
			return;
		ToggleDrawMode();
	}
		
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		m_wDrawingContainer = FrameWidget.Cast(config.RootWidgetRef.FindAnyWidget(SCR_MapConstants.DRAWING_CONTAINER_WIDGET_NAME));
		
		m_iLinesDrawn = 0;
		
		ScriptCallQueue callQueue = GetGame().GetCallqueue();
		for (int i; i < m_iLineCount; i++)
		{
			if (m_aLines[i].m_bIsLineDrawn)
			{
				m_aLines[i].CreateLine(m_wDrawingContainer);
				callQueue.CallLater(m_aLines[i].UpdateLine, 0, false, false);
				m_aLines[i].SetButtonVisible(false);
			}
		}
						
		m_MapEntity.GetOnMapPan().Insert(OnMapPan);		// pan for scaling
		m_MapEntity.GetOnMapPanEnd().Insert(OnMapPanEnd);
		SCR_MapToolEntry.GetOnEntryToggledInvoker().Insert(OnEntryToggled);
		
		m_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{		
		m_MapEntity.GetOnMapPan().Remove(OnMapPan);
		m_MapEntity.GetOnMapPanEnd().Remove(OnMapPanEnd);
		SCR_MapToolEntry.GetOnEntryToggledInvoker().Remove(OnEntryToggled);
		
		SetDrawMode(false, true);
		m_bIsLineBeingDrawn = false;
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{		
		if (m_bIsLineBeingDrawn)
			m_aLines[m_iLineID].UpdateLine(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init()
	{
		SCR_MapToolMenuUI toolMenu = SCR_MapToolMenuUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolMenuUI));
		if (toolMenu)
		{
			m_ToolMenuEntry = toolMenu.RegisterToolMenuEntry(SCR_MapToolMenuUI.s_sToolMenuIcons, m_sToolMenuIconName, 13, m_bIsExclusive); // add to menu
			m_ToolMenuEntry.m_OnClick.Insert(ToggleDrawMode);
			m_ToolMenuEntry.SetEnabled(true);
			
			GetGame().GetInputManager().AddActionListener("MapToolPencil", EActionTrigger.DOWN, OnInputQuickBind);
		}
		
		for (int i; i < m_iLineCount; i++)
		{
			MapLine line = new MapLine(m_MapEntity, this);
			m_aLines.Insert(line);
		}
	}
}
