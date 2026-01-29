//------------------------------------------------------------------------------------------------
//! Cursor data
class SCR_MapCursorInfo
{
	static bool isGamepad;			// is using gamepad
	bool isFixedMode = true;		// determines whether cursor is fixed to the screen center

	static int x, y; 				// by default, all screen position are DPI Unscaled, when scaled, these will return a range from 0 to window resolution in pixels
	static int startPos[2];			// state start pos pan
	static int startPosMultiSel[2];	// state start pos multi select
	int lastX, lastY;				// last pos of x & y
	EMapCursorEdgePos edgeFlags;	// cursor screen edge flags

	static WorkspaceWidget m_WorkspaceWidget;

	//------------------------------------------------------------------------------------------------
	//! DPI scale of values
	static int Scale(int pos)
	{
		if (!m_WorkspaceWidget)
			m_WorkspaceWidget = GetGame().GetWorkspace();

		return m_WorkspaceWidget.DPIScale(pos);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MapCursorInfo()
	{
		x = 0;
		y = 0;
		startPos = {0, 0};
		startPosMultiSel = {0, 0};
	}
}

//------------------------------------------------------------------------------------------------
//! Map cursor behavior and mode setup
[BaseContainerProps()]
class SCR_MapCursorModule : SCR_MapModuleBase
{
	[Attribute(defvalue: "1.5", uiwidget: UIWidgets.EditBox, desc: "Panning: multiplier of keyboard panning speed", params: "0.1 10")]
	protected float m_fPanKBMMultiplier;

	[Attribute(defvalue: "1.5", uiwidget: UIWidgets.EditBox, desc: "Panning: multiplier of thubmstick panning speed", params: "0.1 10")]
	protected float m_fPanStickMultiplier;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enables panning by moving cursor to the screen edges")]
	protected bool m_bEnableCursorEdgePan;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Center KBM cursor on map open")]
	protected bool m_bIsCursorCenteredOnOpen;

	[Attribute(defvalue: "0.1", uiwidget: UIWidgets.EditBox, desc: "Time it takes to perform zooming of a single step", params: "0.01 10")]
	protected float m_fZoomAnimTime;

	[Attribute(defvalue: "2", uiwidget: UIWidgets.EditBox, desc: "Each zoom step is increased/decreased by the currentZoom/thisVariable, lower number means faster steps", params: "0.1 10")]
	protected float m_fZoomStrength;

	[Attribute(defvalue: "100", uiwidget: UIWidgets.EditBox, desc: "This is multiplier of average frame time resulting in a rotation degree value", params: "100 1000")]
	protected int m_iRotateSpeedMouse;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enables custom map crosshair visuals")]
	protected bool m_bEnableMapCrosshairVisuals;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enables crosshair grid coordinate display")]
	protected bool m_bEnableCrosshairCoords;

	[Attribute(defvalue: "0.01", uiwidget: UIWidgets.EditBox, desc: "How precise grid display should be by multiplying current grid by this value and discarding everything that will be after the comma", params: "0.0001 1")]
	protected float m_fGridPrecision;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Enables crosshair height above sea level display")]
	protected bool m_bEnableCrosshairHeightASL;

	[Attribute(defvalue: "5", uiwidget: UIWidgets.EditBox, desc: "How accurate should be the display of the height", params: "1 1000 1")]
	protected int m_iHeightPrecision;

	[Attribute("0 0 0 1", UIWidgets.ColorPicker, desc: "Cursor grid/guide lines color")]
	protected ref Color m_GuidelineColor;

	[Attribute("", UIWidgets.Object, "Cursor state configuration")]
	protected ref array<ref SCR_CursorVisualState> m_aCursorStatesConfig;

	protected ref map<EMapCursorState, SCR_CursorVisualState> m_aCursorStatesMap = new map<EMapCursorState, SCR_CursorVisualState>;	// used to quickly fetch a desired cursor state from config

	//static
	protected static bool m_bRecentlyTraced;		// recently performed widget trace
	protected static ref array<Widget> s_aTracedWidgets = {};

	// const
	protected const int MAX_GRID_COORD = 1000;
	protected const int CURSOR_CAPTURE_OFFSET = 10;		// hardcoded(code) screen edges in pixels for cursor capture
	protected const int GUILDING_LINE_WIDTH = 16;
	protected const float PAN_DEFAULT_COUNTDOWN = 0.1;
	protected const float SINGLE_SELECTION_RANGE = 50.0;	// range in world pos
	protected const float CIRCLE_SELECTION_RANGE = 500.0;	// range in world pos
	protected const float FREE_CURSOR_RESET = 3.0;		// seconds, time before free cursor resets to locked mode on controller
	protected static const LocalizedString TEXT_METERS = "#AR-ValueUnit_Short_Meters";
	protected static const LocalizedString TEXT_GRID = "%1 %2";

	static const EMapCursorState CUSTOM_CURSOR_LOCKED = EMapCursorState.CS_DISABLE;
	static const EMapCursorState STATE_PAN_RESTRICTED	= EMapCursorState.CS_DRAG | EMapCursorState.CS_MODIFIER | EMapCursorState.CS_CONTEXTUAL_MENU | EMapCursorState.CS_DIALOG;
	static const EMapCursorState STATE_ZOOM_RESTRICTED = EMapCursorState.CS_DRAG | EMapCursorState.CS_MODIFIER | EMapCursorState.CS_CONTEXTUAL_MENU;
	static const EMapCursorState STATE_HOVER_RESTRICTED = EMapCursorState.CS_PAN | EMapCursorState.CS_ZOOM | EMapCursorState.CS_MULTI_SELECTION
												| EMapCursorState.CS_DRAG | EMapCursorState.CS_DRAW | EMapCursorState.CS_CONTEXTUAL_MENU;
	static const EMapCursorState STATE_SELECT_RESTRICTED = EMapCursorState.CS_MULTI_SELECTION | EMapCursorState.CS_CONTEXTUAL_MENU | EMapCursorState.CS_DRAG | EMapCursorState.CS_DRAW;
	static const EMapCursorState STATE_MULTISELECT_RESTRICTED = EMapCursorState.CS_DRAG | EMapCursorState.CS_DRAW | EMapCursorState.CS_CONTEXTUAL_MENU | EMapCursorState.CS_MODIFIER;
	static const EMapCursorState STATE_DRAG_RESTRICTED	= EMapCursorState.CS_CONTEXTUAL_MENU | EMapCursorState.CS_MULTI_SELECTION | EMapCursorState.CS_ROTATE;
	static const EMapCursorState STATE_ROTATE_RESTRICTED = EMapCursorState.CS_PAN | EMapCursorState.CS_ZOOM | EMapCursorState.CS_CONTEXTUAL_MENU;
	static const EMapCursorState STATE_DRAW_RESTRICTED = EMapCursorState.CS_PAN | EMapCursorState.CS_ZOOM | EMapCursorState.CS_CONTEXTUAL_MENU | EMapCursorState.CS_DIALOG;
	static const EMapCursorState STATE_SUBMENU_RESTRICTED = EMapCursorState.CS_CONTEXTUAL_MENU | EMapCursorState.CS_DIALOG;
	static const EMapCursorState STATE_CTXMENU_RESTRICTED = EMapCursorState.CS_DRAG | EMapCursorState.CS_DRAW | EMapCursorState.CS_ROTATE;
	static const EMapCursorState STATE_RESET_RESTRICTED = EMapCursorState.CS_DRAG | EMapCursorState.CS_DRAW | EMapCursorState.CS_ROTATE;
	static const EMapCursorState STATE_POPUP_RESTRICTED = EMapCursorState.CS_PAN | EMapCursorState.CS_DRAW | EMapCursorState.CS_ROTATE | EMapCursorState.CS_COMMAND_NEGATIVE | EMapCursorState.CS_COMMAND_POSITIVE;

	// timers
	protected float m_fPanCountdown;		// used to stop panning cursor state and refresh start position for next drag panning
	protected float m_fModifActionDelay;	// delay between modifier action activation
	protected float m_fZoomHoldTime;		// zoom hold time for adjusting speed of zoom
	protected float m_fHoverTime;			// hover activation
	protected float m_fFreeCursorTime;		// free cursor deactivation
	protected float m_fSelectHoldTime;		// determines activation of multiselect, which is not using input filters so it doesnt create initial click pos delay

	// enums
	EMapCursorSelectType m_eMultiSelType;									// multiselect type
	protected EMapCursorState m_CursorState = EMapCursorState.CS_DEFAULT;	// keeps current cursor state

	protected bool m_bIsInit;				// is module initiated
	protected bool m_bIsDisabled;			// temporary module disable
	protected bool m_bIsDraggingAvailable;
	protected bool m_bIsSelectionAvailable;
	protected bool m_bIsModifierActive;
	protected bool m_bIsJournalActive;
	protected int m_iRotationDirVal;
	protected float m_fZoomMultiplierWheel = 1;
	protected InputManager m_InputManager;
	protected ref SCR_MapCursorInfo m_CursorInfo;
	protected ref SCR_CursorCustom m_CustomCursor;
	protected CanvasWidget m_MapWidget;
	protected SCR_MapSelectionModule m_SelectionModule;

	// positioning & sizing
	protected Widget m_wCrossGrid;		// cross grid root
	protected ImageWidget m_wCrossLTop;	// top left fill image, width and height determined by the cursor pos
	protected ImageWidget m_wCrossRTop;	// top right fill image, width by (screen reso - cursor pos),  height determined by cursos pos
	protected ImageWidget m_wCrossLBot;	// bot left fill image, width set by top left fill, height by (screen reso - cursor pos)
	protected ImageWidget m_wGLLeft;	// guiding line sizing
	protected ImageWidget m_wGLTop;		// guiding line sizing
	protected Widget m_wCrossMCenter;	// used for centering

	// Rest of guide lines for coloring
	protected ImageWidget m_wGLRight;
	protected ImageWidget m_wGLBot;
	protected ImageWidget m_wGLFadeTop;
	protected ImageWidget m_wGLFadeLeft;
	protected ImageWidget m_wGLFadeRight;
	protected ImageWidget m_wGLFadeBot;
	protected TextWidget m_wCoordText;
	protected TextWidget m_wHeightASLText;

	//------------------------------------------------------------------------------------------------
	// GETTERS / SETTERS
	//------------------------------------------------------------------------------------------------
	//! Get cursor data class
	SCR_MapCursorInfo GetCursorInfo() { return m_CursorInfo; }

	//------------------------------------------------------------------------------------------------
	//! Get cursor state
	EMapCursorState GetCursorState() { return m_CursorState; }

	//------------------------------------------------------------------------------------------------
	//! Get widgets under cursor
	static array<Widget> GetMapWidgetsUnderCursor()
	{
		if (!SCR_MapEntity.GetMapInstance().IsOpen())
		{
			s_aTracedWidgets.Clear();
			return s_aTracedWidgets;
		}

		if (!m_bRecentlyTraced)
			TraceMapWidgets();

		return s_aTracedWidgets;
	}

	//------------------------------------------------------------------------------------------------
	//! Set cursor state
	//! \param state is the added state
	protected void SetCursorState(EMapCursorState state)
	{
		if (!m_bIsInit)		// some map components may call this when the module already disabled map cursor
			return;

		m_CursorState |= state;
		SetCursorType(m_CursorState);
	}

	//------------------------------------------------------------------------------------------------
	//! Unset cursor state
	//! \param state is the removed state
	protected void UnsetCursorState(EMapCursorState state)
	{
		m_CursorState &= ~state;
		SetCursorType(m_CursorState);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets cursor type - visual style
	protected void SetCursorType(EMapCursorState type)
	{
		if (!m_bEnableMapCrosshairVisuals)
			return;

		SCR_CursorVisualState cursorState = GetCursorStateCfg();

		if (type == EMapCursorState.CS_DISABLE || m_bIsDisabled)
			m_CustomCursor.SetCursorVisual(null);
		else
			m_CustomCursor.SetCursorVisual(cursorState);

		SetCrosshairGridVisible(!cursorState.m_DisableCursorLines);
	}

	//------------------------------------------------------------------------------------------------
	//! Set visibility of a custom crosshair grid lines
	protected void SetCrosshairGridVisible(bool state)
	{
		if (!m_wCrossGrid || !m_bEnableMapCrosshairVisuals)
			return;

		m_wCrossGrid.SetVisible(state);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets active device - KB+M or controller
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{
		m_CursorInfo.isGamepad = isGamepad;
		if (m_CursorInfo.isGamepad)
			ForceCenterCursor();
		else if (m_fFreeCursorTime != 0)	// if scheme switched during fade
			m_CustomCursor.SetOpacity(1);
	}

	//------------------------------------------------------------------------------------------------
	//! Sets zoom strength multiplier for mouse wheel zoom
	void SetWheelZoomMultiplier(float value)
	{
		m_fZoomMultiplierWheel = value;
	}

	//------------------------------------------------------------------------------------------------
	//! Force cursor to screen center
	//! Only use when necessarry as some methods rely on current cursor position
	void ForceCenterCursor()
	{
		float screenX, screenY;
		m_MapWidget.GetScreenSize(screenX, screenY);
		m_InputManager.SetCursorPosition(screenX * 0.5, screenY * 0.5);

		WorkspaceWidget workspace = GetGame().GetWorkspace();
		m_CursorInfo.x = workspace.DPIUnscale(screenX * 0.5);
		m_CursorInfo.y = workspace.DPIUnscale(screenY * 0.5);

		if (!m_CursorInfo.isFixedMode)
			m_fFreeCursorTime = FREE_CURSOR_RESET;
	}

	//------------------------------------------------------------------------------------------------
	// CURSOR STATE HANDLERS
	//------------------------------------------------------------------------------------------------
	//! Handle move state
	protected void HandleMove()
	{
		//begin move
		if (m_CursorInfo.lastX != m_CursorInfo.x || m_CursorInfo.lastY != m_CursorInfo.y)
		{
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			if (m_CursorInfo.isGamepad && workspace)
				m_InputManager.SetCursorPosition(workspace.DPIScale(m_CursorInfo.x), workspace.DPIScale(m_CursorInfo.y));


			if (~m_CursorState & EMapCursorState.CS_MOVE)
				SetCursorState(EMapCursorState.CS_MOVE);
		}
		//end move
		else if (m_CursorState & EMapCursorState.CS_MOVE)
			UnsetCursorState(EMapCursorState.CS_MOVE);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle pan
	protected void HandlePan(float timeSlice)
	{
		// pan disabled
		if (m_CursorState & STATE_PAN_RESTRICTED)
		{
			if (m_CursorState & EMapCursorState.CS_PAN)
				m_fPanCountdown = 0;
			else
				return;
		}

		// Start panning if cursor is on the screen edges
		if (m_CursorInfo.edgeFlags > 0 && (m_CursorState & EMapCursorState.CS_PAN) == 0)	// dont allow edge panning while panning using another method
		{
			if (m_fPanCountdown == 0)
				m_fPanCountdown = PAN_DEFAULT_COUNTDOWN;

			float frameTime = System.GetFrameTimeS();
			int px = (int)(frameTime * 1000) * m_fPanKBMMultiplier;

			if (m_CursorInfo.edgeFlags & EMapCursorEdgePos.LEFT)
				m_MapEntity.Pan(EMapPanMode.HORIZONTAL, px);
			else if (m_CursorInfo.edgeFlags & EMapCursorEdgePos.RIGHT)
				m_MapEntity.Pan(EMapPanMode.HORIZONTAL, -px);

			if (m_CursorInfo.edgeFlags & EMapCursorEdgePos.TOP)
				m_MapEntity.Pan(EMapPanMode.VERTICAL, px);
			else if (m_CursorInfo.edgeFlags & EMapCursorEdgePos.BOTTOM)
				m_MapEntity.Pan(EMapPanMode.VERTICAL, -px);
		}

		// stop pan state
		if (m_fPanCountdown <= 0)
		{
			if (m_CursorState & EMapCursorState.CS_PAN)
			{
				UnsetCursorState(EMapCursorState.CS_PAN);
				m_CursorInfo.startPos = {0, 0};
			}

			return;
		}
		// begin pan state
		if (~m_CursorState & EMapCursorState.CS_PAN)
			SetCursorState(EMapCursorState.CS_PAN);

		m_fPanCountdown -= timeSlice;
	}

	//------------------------------------------------------------------------------------------------
	//! Handle zoom
	protected void HandleZoom()
	{
		// stop zoom state
		if (!m_MapEntity.IsZooming())
		{
			if (m_CursorState & EMapCursorState.CS_ZOOM)
			{
				UnsetCursorState(EMapCursorState.CS_ZOOM);
			}

			return;
		}

		//! zoom disabled
		if (m_CursorState & STATE_ZOOM_RESTRICTED)
			return;

		// begin zoom state
		if (~m_CursorState & EMapCursorState.CS_ZOOM)
		{
			SetCursorState(EMapCursorState.CS_ZOOM);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle hover state displaying details over target
	protected void HandleHover(float timeSlice)
	{
		// begin hover
		if (m_CursorState == EMapCursorState.CS_DEFAULT)
		{
			m_fHoverTime += timeSlice;

			if (m_fHoverTime >= 0.25)
			{
				float worldX, worldY;
				m_MapEntity.ScreenToWorldNoFlip(m_CursorInfo.Scale(m_CursorInfo.x), m_CursorInfo.Scale(m_CursorInfo.y), worldX, worldY);
				vector curPos = Vector(worldX, 0, worldY);

				MapItem closeItem = m_MapEntity.GetClose(curPos, SINGLE_SELECTION_RANGE / m_MapEntity.GetCurrentZoom());
				if (closeItem)
				{
					SetCursorState(EMapCursorState.CS_HOVER);
					m_MapEntity.HoverItem(closeItem);
				}

				return;
			}
		}
		// end hover
		else if (m_CursorState & EMapCursorState.CS_HOVER)
		{
			float worldX, worldY;
			m_MapEntity.ScreenToWorldNoFlip(m_CursorInfo.Scale(m_CursorInfo.x), m_CursorInfo.Scale(m_CursorInfo.y), worldX, worldY);
			vector curPos = Vector(worldX, 0, worldY);
			MapItem closeItem = m_MapEntity.GetClose(curPos, SINGLE_SELECTION_RANGE / m_MapEntity.GetCurrentZoom());

			if ((m_CursorState & STATE_HOVER_RESTRICTED)
				|| !closeItem
				|| (closeItem && closeItem != m_MapEntity.GetHoveredItem())
				)
			{
				m_MapEntity.ClearHover();
				m_fHoverTime = 0;
				UnsetCursorState(EMapCursorState.CS_HOVER);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle select state
	protected void HandleSelect()
	{
		//! select disabled
		if (m_CursorState & STATE_SELECT_RESTRICTED)
			return;

		vector vScreenPos = Vector(m_CursorInfo.Scale(m_CursorInfo.x), 0, m_CursorInfo.Scale(m_CursorInfo.y));
		m_MapEntity.InvokeOnSelect(vScreenPos);

		// start select
		if (~m_CursorState & EMapCursorState.CS_SELECT)
		{
			SetCursorState(EMapCursorState.CS_SELECT);

			float worldX, worldY;
			m_MapEntity.ScreenToWorldNoFlip(m_CursorInfo.Scale(m_CursorInfo.x), m_CursorInfo.Scale(m_CursorInfo.y), worldX, worldY);
			vector curPos = Vector(worldX, 0, worldY);

			MapItem selected = m_MapEntity.GetClose(curPos, SINGLE_SELECTION_RANGE / m_MapEntity.GetCurrentZoom());
			autoptr array<MapItem> selectedItems = new array<MapItem>;
			m_MapEntity.GetSelected(selectedItems);

			// only select if different item or multiple selected
			if (selected && (selectedItems.Count() != 1 || selectedItems[0] != selected))
			{
				m_MapEntity.ClearSelection();
				m_MapEntity.SelectItem(selected);
			}
			else
				m_MapEntity.ClearSelection();

		}

		// Selection state end instantly after a click
		UnsetCursorState(EMapCursorState.CS_SELECT);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle multi select - rectangular selection
	protected void HandleMultiSelect(bool activate)
	{
		// multiselect state end or disabled
		if ((m_CursorState & STATE_MULTISELECT_RESTRICTED) != 0 || !activate)
		{
			// never started
			if (~m_CursorState & EMapCursorState.CS_MULTI_SELECTION)
				return;

			// select items
			m_MapEntity.ClearSelection();
			autoptr array<MapItem> selectedItems = new array<MapItem>;
			GetCursorSelection(selectedItems);
			int count = selectedItems.Count();

			for (int i = 0; i < count; i++)
			{
				m_MapEntity.SelectItem(selectedItems[i]);
			}

			// multiselect end
			UnsetCursorState(EMapCursorState.CS_MULTI_SELECTION);
			m_CursorInfo.startPosMultiSel = {0, 0};
		}
		else if (activate)
		{
			// multiselect begin
			if (~m_CursorState & EMapCursorState.CS_MULTI_SELECTION)
				SetCursorState(EMapCursorState.CS_MULTI_SELECTION);

			// highlight for items within selection
			m_MapEntity.ResetHighlighted();
			autoptr array<MapItem> selectedItems = new array<MapItem>;
			GetCursorSelection(selectedItems);
			int count = selectedItems.Count();

			for (int i = 0; i < count; i++)
			{
				selectedItems[i].SetHighlighted(true);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle drag state
	//! \param startDrag determines whether dragging should begin or stop
	protected void HandleDrag(bool startDrag)
	{
		// begin drag
		if (startDrag)
		{
			// disable drag state
			if (m_CursorState & STATE_DRAG_RESTRICTED)
				return;

			if (~m_CursorState & EMapCursorState.CS_DRAG)
			{
				if (SCR_MapToolInteractionUI.StartDrag())
				{
					SetCursorState(EMapCursorState.CS_DRAG);
				}
			}
		}
		// end drag
		else
		{
			if (m_CursorState & EMapCursorState.CS_DRAG)
			{
				UnsetCursorState(EMapCursorState.CS_DRAG);
				SCR_MapToolInteractionUI.EndDrag();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle rotate map tool
	protected void HandleRotateTool(bool startRotate)
	{
		if (m_CursorState & STATE_ROTATE_RESTRICTED)
			return;

		if (startRotate)
		{
			if (~m_CursorState & EMapCursorState.CS_ROTATE)
			{
				if (m_CursorState & EMapCursorState.CS_DRAG)
					HandleDrag(false);

				if (SCR_MapToolInteractionUI.StartRotate())
					SetCursorState(EMapCursorState.CS_ROTATE);
			}
		}
		else
		{
			if (m_CursorState & EMapCursorState.CS_ROTATE)
			{
				UnsetCursorState(EMapCursorState.CS_ROTATE);
				SCR_MapToolInteractionUI.EndRotate();
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handle draw state
	bool HandleDraw(bool active)
	{
		// begin draw state
		if (active && (m_CursorState & STATE_DRAW_RESTRICTED) == 0)
		{
			if (~m_CursorState & EMapCursorState.CS_DRAW)
			{
				SetCursorState(EMapCursorState.CS_DRAW);
				return true;
			}
		}
		// end draw state
		else if (m_CursorState & EMapCursorState.CS_DRAW)
		{
			UnsetCursorState(EMapCursorState.CS_DRAW);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Handle sub menu state
	bool HandleSubMenu(bool active)
	{
		// begin sub menu state
		if (active && (m_CursorState & STATE_SUBMENU_RESTRICTED) == 0)
		{
			if (~m_CursorState & EMapCursorState.CS_SUB_MENU)
			{
				SetCursorState(EMapCursorState.CS_SUB_MENU);
				return true;
			}
		}
		// end sub menu state
		else if (m_CursorState & EMapCursorState.CS_SUB_MENU)
		{
			UnsetCursorState(EMapCursorState.CS_SUB_MENU);
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Handle dialog state
	bool HandleDialog(bool active)
	{
		if (active)
		{
			if (~m_CursorState & EMapCursorState.CS_DIALOG)
			{
				if (m_CursorInfo.isGamepad)
					ForceCenterCursor();

				SetCursorState(EMapCursorState.CS_DIALOG);

				return true;
			}
		}
		else if (m_CursorState & EMapCursorState.CS_DIALOG)
		{
			UnsetCursorState(EMapCursorState.CS_DIALOG);

			if (m_CursorInfo.isGamepad)
				ForceCenterCursor();
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	void ToggleFastTravelDestinationSelection(bool enable)
	{
		if (enable)
			SetCursorState(EMapCursorState.CS_ASSIGN_WP);
		else
			UnsetCursorState(EMapCursorState.CS_ASSIGN_WP);
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle map command positive cursor
	//! \param enable
	void ToggleMapCommandPositiveCursor(bool enable)
	{
		if (enable)
			SetCursorState(EMapCursorState.CS_COMMAND_POSITIVE);
		else
			UnsetCursorState(EMapCursorState.CS_COMMAND_POSITIVE);
	}

	//------------------------------------------------------------------------------------------------
	//! Toggle map command negative cursor
	//! \param enable
	void ToggleMapCommandNegativeCursor(bool enable)
	{
		if (enable)
			SetCursorState(EMapCursorState.CS_COMMAND_NEGATIVE);
		else
			UnsetCursorState(EMapCursorState.CS_COMMAND_NEGATIVE);
	}

	//------------------------------------------------------------------------------------------------
	void ToggleLocationSelection(bool enable)
	{
		if (enable)
			SetCursorState(EMapCursorState.CS_ASSIGN_WP);
		else
			UnsetCursorState(EMapCursorState.CS_ASSIGN_WP);
	}

	//------------------------------------------------------------------------------------------------
	//! Handle contextual menu
	//! \param doClose determines whether the context menu should close
	//! \return state of the menu where false = close / true = open
	bool HandleContextualMenu(bool doClose = false)
	{
		//! context menu disabled
		if ((m_CursorState & STATE_CTXMENU_RESTRICTED) != 0 && !doClose)
			return false;

		SCR_MapRadialUI radialMenu = SCR_MapRadialUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapRadialUI));
		if (!radialMenu)
			return false;

		if (doClose)	// close
		{
			if (m_CursorState & EMapCursorState.CS_CONTEXTUAL_MENU)
			{
				UnsetCursorState(EMapCursorState.CS_CONTEXTUAL_MENU);

				return false;
			}
		}
		else if (~m_CursorState & EMapCursorState.CS_CONTEXTUAL_MENU) // open
		{
			SetCursorState(EMapCursorState.CS_CONTEXTUAL_MENU);

			return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Handle journal/task list menu
	//! \param isOpened determines whether the menu is opened
	void SetJournalVisibility(bool isOpened)
	{
		m_bIsJournalActive = isOpened;
	}

	//------------------------------------------------------------------------------------------------
	// SUPPORT METHODS
	//------------------------------------------------------------------------------------------------
	//! Acquires array of map items for multi selection based on selection mode
	//! \param mapItems are map items within selection
	protected void GetCursorSelection(out array<MapItem> mapItems)
	{
			//! screen to world conversion
			float startWorldX, startWorldY, worldX, worldY;
			m_MapEntity.ScreenToWorldNoFlip(m_CursorInfo.Scale(m_CursorInfo.startPosMultiSel[0]), m_CursorInfo.Scale(m_CursorInfo.startPosMultiSel[1]), startWorldX, startWorldY);
			m_MapEntity.ScreenToWorldNoFlip(m_CursorInfo.Scale(m_CursorInfo.x), m_CursorInfo.Scale(m_CursorInfo.y), worldX, worldY);
			vector startPos = Vector(startWorldX, 0, startWorldY);
			vector curPos = Vector(worldX, 0, worldY);

			//! get entities inside the selection rectangle
			if (m_eMultiSelType == EMapCursorSelectType.RECTANGLE)
				m_MapEntity.GetInsideRect(mapItems, startPos, curPos);
			else if (m_eMultiSelType == EMapCursorSelectType.CIRCLE)
			{
				float circleRadius = m_SelectionModule.GetSelCircleSize() / 2; // size is in pixels while we want radius
				m_MapEntity.GetInsideCircle(mapItems, curPos, circleRadius / m_MapEntity.GetCurrentZoom());
			}
	}

	//------------------------------------------------------------------------------------------------
	//! Actual cursor position screen (DPI Unscaled)
	//! \param [out] unscaled cursor x
	//! \param [out] unscaled cursor y
	protected void GetCursorPosition(out int x, out int y)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		if (!m_MapWidget)
			m_MapWidget = m_MapEntity.GetMapWidget();

		float screenX, screenY, offX, offY;
		m_MapWidget.GetScreenSize(screenX, screenY);
		m_MapWidget.GetScreenPos(offX, offY);

		// Needed for precision sake
		float fX, fY;

		// gamepad cursor
		if (m_CursorInfo.isGamepad)
		{
			fX = workspace.DPIScale(x);
			fY = workspace.DPIScale(y);
		}
		else
		{
			int iX, iY;
			WidgetManager.GetMousePos(iX, iY);
			fX = iX;
			fY = iY;
		}

		// If the widget is not fullscreen, cursor position needs to be offset to match it
		fX -= offX;
		fY -= offY;

		// Screen edge pan
		EMapEntityMode mode = m_MapEntity.GetMapConfig().MapEntityMode;
		if (m_bEnableCursorEdgePan)
			TestEdgePan(fX, fY, screenX, screenY);

		// unscale the result
		x = workspace.DPIUnscale(fX);
		y = workspace.DPIUnscale(fY);
	}

	//------------------------------------------------------------------------------------------------
	//! Window edge pan
	//! \param x is scaled cursor pos x
	//! \param y is scaled cursor pos y
	//! \param screenX is screen size x
	//! \param screenY is screen size y
	protected void TestEdgePan(inout float x, inout float y, int screenX, int screenY)
	{
		float windowWidth, windowHeight;
		m_MapWidget.GetScreenSize(windowWidth, windowHeight);

		if (x == 0)	// hack so the game doesnt autoscroll to top left when cursor is in default pos (f.e. during window unfocus)
			return;

		if (x <= CURSOR_CAPTURE_OFFSET)
			m_CursorInfo.edgeFlags |= EMapCursorEdgePos.LEFT;
		else
			m_CursorInfo.edgeFlags &= ~EMapCursorEdgePos.LEFT;

		if (y <= CURSOR_CAPTURE_OFFSET)
			m_CursorInfo.edgeFlags |= EMapCursorEdgePos.TOP;
		else
			m_CursorInfo.edgeFlags &= ~EMapCursorEdgePos.TOP;

		if (x >= (int)windowWidth - CURSOR_CAPTURE_OFFSET - 1)
			m_CursorInfo.edgeFlags |= EMapCursorEdgePos.RIGHT;
		else
			m_CursorInfo.edgeFlags &= ~EMapCursorEdgePos.RIGHT;

		if (y >= (int)windowHeight - CURSOR_CAPTURE_OFFSET - 1)
			m_CursorInfo.edgeFlags |= EMapCursorEdgePos.BOTTOM;
		else
			m_CursorInfo.edgeFlags &= ~EMapCursorEdgePos.BOTTOM;
	}

	//------------------------------------------------------------------------------------------------
	// INPUTS
	//------------------------------------------------------------------------------------------------
	//! Digital pan input for activating drag mode
	protected void OnInputPanDrag(float value, EActionTrigger reason)
	{
		// pan disabled
		if (m_CursorState & STATE_PAN_RESTRICTED)
			return;

		m_fPanCountdown = PAN_DEFAULT_COUNTDOWN;
		m_MapEntity.Pan(EMapPanMode.DRAG);
	}

	//------------------------------------------------------------------------------------------------
	//! Digital pan KBM
	protected void OnInputPanH(float value, EActionTrigger reason)
	{
		CalculatePan(value, EMapPanMode.HORIZONTAL, m_fPanKBMMultiplier);	// value determines direction for digital inputs
	}

	//------------------------------------------------------------------------------------------------
	//! Digital pan vertical
	protected void OnInputPanV(float value, EActionTrigger reason)
	{
		CalculatePan(value, EMapPanMode.VERTICAL, m_fPanKBMMultiplier);		// value determines direction for digital inputs
	}

	//------------------------------------------------------------------------------------------------
	//! Analog pan horizontal
	protected void OnInputPanHGamepad(float value, EActionTrigger reason)
	{
		if (value == 0)
			return;

		CalculatePan(1, EMapPanMode.HORIZONTAL, value * m_fPanStickMultiplier);
	}

	//------------------------------------------------------------------------------------------------
	//! Analog pan vertical
	protected void OnInputPanVGamepad(float value, EActionTrigger reason)
	{
		if (value == 0)
			return;

		CalculatePan(1, EMapPanMode.VERTICAL, value * m_fPanStickMultiplier);
	}

	//------------------------------------------------------------------------------------------------
	//! Process pan
	//! \param direction determines direction of digital inputs for the horizontal/vertical axis
	//! \param panMode is mode of panning
	//! \param multiPlier is configurable speed adjustment
	protected void CalculatePan(float direction, EMapPanMode panMode, float multiPlier)
	{
		if (m_CursorState & STATE_PAN_RESTRICTED)
			return;

		m_fPanCountdown = PAN_DEFAULT_COUNTDOWN;
		int px = (System.GetFrameTimeS() * 1000);	// speed of pan based on time elapsed to avoid slowdown during lower fps

		if ((int)direction == 1)
			px = px * multiPlier;
		else if ((int)direction == 2)
			px = -px * multiPlier;

		m_MapEntity.Pan(panMode, px);
	}

	//------------------------------------------------------------------------------------------------
	//! Cursor motion horizontal
	protected void OnInputGamepadCursorH(float value, EActionTrigger reason)
	{
		if (!m_CursorInfo.isGamepad)
			return;

		if (value == 0)
			return;

		m_fFreeCursorTime = 0;
		m_CustomCursor.SetOpacity(1);
		m_CursorInfo.isFixedMode = false;

		float maxX, maxY;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (m_wRootWidget && workspace)
		{
			m_wRootWidget.GetScreenSize(maxX, maxY);
			maxX = workspace.DPIUnscale(maxX);
			maxY = workspace.DPIUnscale(maxY);
		}

		m_CursorInfo.x = Math.Clamp(m_CursorInfo.x + value * System.GetFrameTimeS() * 1000 * m_fPanStickMultiplier, 1, maxX);
	}

	//------------------------------------------------------------------------------------------------
	//! Cursor motion vertical
	protected void OnInputGamepadCursorV(float value, EActionTrigger reason)
	{
		if (!m_CursorInfo.isGamepad)
			return;

		if (value == 0)
			return;

		m_fFreeCursorTime = 0;
		m_CustomCursor.SetOpacity(1);
		m_CursorInfo.isFixedMode = false;

		float maxX, maxY;
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (m_wRootWidget && workspace)
		{
			m_wRootWidget.GetScreenSize(maxX, maxY);
			maxX = workspace.DPIUnscale(maxX);
			maxY = workspace.DPIUnscale(maxY);
		}

		m_CursorInfo.y = Math.Clamp(m_CursorInfo.y - value * System.GetFrameTimeS() * 1000 * m_fPanStickMultiplier, 1, maxY);
	}

	//------------------------------------------------------------------------------------------------
	//! Digital zoom in. Methods are split for InputButton display purposes.
	protected void OnInputZoomIn(float value, EActionTrigger reason)
	{
		if (Math.AbsFloat(value) < 0.001)
			return;

		//! zoom disabled
		if (m_CursorState & STATE_ZOOM_RESTRICTED)
			return;

		m_fZoomHoldTime += System.GetFrameTimeS();
		if ((m_CursorState & EMapCursorState.CS_ZOOM) && m_fZoomHoldTime < 0.05)	// if pressed, call once every 0.05 second
			return;

		float zoomPPU = m_MapEntity.GetCurrentZoom();
		if (value != 0)
		{
			float zoomValue = value.Sign() * Math.Min(1, value * value) * zoomPPU / m_fZoomStrength;
			m_MapEntity.ZoomSmooth(zoomPPU + zoomValue, m_fZoomAnimTime, false);
		}

		m_fZoomHoldTime = 0;
	}

	//------------------------------------------------------------------------------------------------
	//! Digital zoom out. Methods are split for InputButton display purposes.
	protected void OnInputZoomOut(float value, EActionTrigger reason)
	{
		if (Math.AbsFloat(value) < 0.001)
			return;

		//! zoom disabled
		if (m_CursorState & STATE_ZOOM_RESTRICTED)
			return;

		m_fZoomHoldTime += System.GetFrameTimeS();
		if ((m_CursorState & EMapCursorState.CS_ZOOM) && m_fZoomHoldTime < 0.05)	// if pressed, call once every 0.05 second
			return;

		float zoomPPU = m_MapEntity.GetCurrentZoom();
		if (value != 0)
		{
			float zoomValue = -1 * Math.Min(1, value * value) * zoomPPU / m_fZoomStrength;
			m_MapEntity.ZoomSmooth(zoomPPU + zoomValue, m_fZoomAnimTime, false);
		}

		m_fZoomHoldTime = 0;
	}


	//------------------------------------------------------------------------------------------------
	//! Digital zoom mouse wheel
	protected void OnInputZoomWheelUp(float value, EActionTrigger reason)
	{
		//! zoom disabled
		if (m_bIsJournalActive || m_CursorState & STATE_ZOOM_RESTRICTED)
			return;

		float targetPPU = m_MapEntity.GetTargetZoomPPU();
		value = value * m_fZoomMultiplierWheel;
		m_MapEntity.ZoomSmooth(targetPPU + targetPPU * (value * 0.001), m_fZoomAnimTime, false);	// the const here is adjusting the value to match the input with zoom range
	}

	//------------------------------------------------------------------------------------------------
	//! Digital zoom mouse wheel
	protected void OnInputZoomWheelDown(float value, EActionTrigger reason)
	{
		//! zoom disabled
		if (m_bIsJournalActive || m_CursorState & STATE_ZOOM_RESTRICTED)
			return;

		float targetPPU = m_MapEntity.GetTargetZoomPPU();
		value = value * m_fZoomMultiplierWheel;
		m_MapEntity.ZoomSmooth(targetPPU - targetPPU/ 2 * (value * 0.001), m_fZoomAnimTime, false);	// the const here is adjusting the value to match the input with zoom range
	}

	//------------------------------------------------------------------------------------------------
	//! Digital drag
	protected void OnInputDrag(float value, EActionTrigger reason)
	{
		if (!m_bIsDraggingAvailable)
			return;

		if (m_CursorState & EMapCursorState.CS_MODIFIER)
			return;

		if (reason == EActionTrigger.DOWN)
			HandleDrag(true);
		else
			HandleDrag(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Digital drag toggle
	protected void OnInputDragToggle(float value, EActionTrigger reason)
	{
		if (!m_bIsDraggingAvailable)
			return;

		if (m_CursorState & EMapCursorState.CS_MODIFIER)
			return;

		if (~m_CursorState & EMapCursorState.CS_DRAG)
			HandleDrag(true);
		else
			HandleDrag(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Digital modifier key
	protected void OnInputModifier(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.DOWN)
		{
			SetCursorState(EMapCursorState.CS_MODIFIER);
			if (!SCR_MapToolInteractionUI.s_bIsRotating)
				HandleRotateTool(true);
		}
		else
		{
			UnsetCursorState(EMapCursorState.CS_MODIFIER);
			if (SCR_MapToolInteractionUI.s_bIsRotating)
				HandleRotateTool(false);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Digital click
	protected void OnInputModifClick(float value, EActionTrigger reason)
	{
		SCR_MapToolInteractionUI.ActivateAction();
	}

	//------------------------------------------------------------------------------------------------
	//! Digital multiselect mouse
	protected void OnInputMultiSel(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.PRESSED)
		{
			if (m_fSelectHoldTime == 0)	// first call
				m_CursorInfo.startPosMultiSel = {m_CursorInfo.x, m_CursorInfo.y};

			m_fSelectHoldTime += System.GetFrameTimeS();
			if (m_fSelectHoldTime < 0.2)
				return;

			HandleMultiSelect(true);
			m_eMultiSelType = EMapCursorSelectType.RECTANGLE;

		}
		else
		{
			HandleMultiSelect(false);
			m_fSelectHoldTime = 0;
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Digital multiselect gamepad
	protected void OnInputMultiSelGamepad(float value, EActionTrigger reason)
	{
		if (reason == EActionTrigger.PRESSED)
		{
			m_CursorInfo.startPosMultiSel = {m_CursorInfo.x, m_CursorInfo.y};
			HandleMultiSelect(true);
			m_eMultiSelType = EMapCursorSelectType.CIRCLE;
		}
		else
			HandleMultiSelect(false);

	}

	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI event
	protected void OnPauseMenuOpened()
	{
		m_bIsDisabled = true;
		m_CursorState = EMapCursorState.CS_DEFAULT;
		SetCursorType(EMapCursorState.CS_DISABLE);
	}

	//------------------------------------------------------------------------------------------------
	//! PauseMenuUI event
	protected void OnPauseMenuClosed()
	{
		m_bIsDisabled = false;
		SetCursorType(m_CursorState);
	}

	//------------------------------------------------------------------------------------------------
	//! Get cursor visual state config
	//! \return highest configured cursor state config relative to current cursor state
	protected SCR_CursorVisualState GetCursorStateCfg()
	{
		EMapCursorState state = EMapCursorState.CS_LAST;
		EMapCursorState stateMin = EMapCursorState.CS_DEFAULT;
		SCR_CursorVisualState cfg;
		while (state != stateMin)
		{
			if (m_CursorState & state)
			{
				cfg = m_aCursorStatesMap.Get(state);
				if (cfg)
					return cfg;
			}

			state = state >> 1;
		}

		return m_aCursorStatesMap.Get(EMapCursorState.CS_DEFAULT);
	}

	//------------------------------------------------------------------------------------------------
	//! Update crosshair UI
	protected void UpdateCrosshairUI()
	{
		float sizeX, sizeY, cursorX, cursorY;
		m_wCrossMCenter.GetScreenSize(sizeX, sizeY);

		cursorX = m_CursorInfo.x;
		cursorY = m_CursorInfo.y;

		// calculate and set new size of top & left fill imageWidgets
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		cursorX = cursorX - workspace.DPIUnscale(sizeX)/ 2;
		cursorY = cursorY - workspace.DPIUnscale(sizeY)/ 2;

		m_CustomCursor.Update(m_CursorInfo.x, m_CursorInfo.y);

		m_MapWidget.GetScreenSize(sizeX, sizeY);

		m_wCrossLTop.SetSize(cursorX, cursorY);
		m_wCrossRTop.SetSize(workspace.DPIUnscale(sizeX) - cursorX, cursorY);
		m_wCrossLBot.SetSize(0, workspace.DPIUnscale(sizeY) - cursorY);
		m_wGLTop.SetSize(GUILDING_LINE_WIDTH, cursorY);
		m_wGLLeft.SetSize(cursorX, GUILDING_LINE_WIDTH);

		float wX, wY;
		m_MapEntity.ScreenToWorld(m_CursorInfo.Scale(m_CursorInfo.x), m_CursorInfo.Scale(m_CursorInfo.y), wX, wY);

		if (m_bEnableCrosshairHeightASL)
		{
			float heightASL = Math.Round(GetGame().GetWorld().GetSurfaceY(wX, wY) / m_iHeightPrecision) * m_iHeightPrecision;
			m_wHeightASLText.SetTextFormat(TEXT_METERS, heightASL.ToString(0, 0));
		}

		if (m_bEnableCrosshairCoords)
		{
			wX *= m_fGridPrecision;
			if (wX < 0)
				wX = MAX_GRID_COORD + wX;

			wY *= m_fGridPrecision;
			if (wY < 0)
				wY = MAX_GRID_COORD + wY;

			string posY = (Math.Floor(wY)).ToString(3, 0); // to match grid precision
			string posX = (Math.Floor(wX)).ToString(3, 0); // to match grid precision
			m_wCoordText.SetTextFormat(TEXT_GRID, posX, posY);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Trace widgets under cursor
	protected static void TraceMapWidgets()
	{
		WidgetManager.TraceWidgets(SCR_MapCursorInfo.Scale(SCR_MapCursorInfo.x), SCR_MapCursorInfo.Scale(SCR_MapCursorInfo.y), SCR_MapEntity.GetMapInstance().GetMapMenuRoot(), s_aTracedWidgets);
		m_bRecentlyTraced = true;
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize widgets
	protected void InitWidgets(Widget root)
	{
		m_MapWidget = m_MapEntity.GetMapWidget();

		m_wCrossGrid = root.FindAnyWidget("CursorCrosshair");
		if (m_bEnableMapCrosshairVisuals)
		{
			SetCrosshairGridVisible(true);
			// positioning widgets
			m_wCrossMCenter = m_wCrossGrid.FindAnyWidget("CrossMCenter");
			m_wCrossRTop = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("CrossRTop"));
			m_wCrossLTop = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("CrossLTop"));
			m_wCrossLBot = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("CrossLBot"));
			m_wGLTop = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLTop"));
			m_wGLLeft = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLLeft"));
			// guidelines
			m_wGLFadeLeft = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLFadeLeft"));
			m_wGLRight = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLRight"));
			m_wGLFadeRight = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLFadeRight"));
			m_wGLBot = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLBot"));
			m_wGLFadeBot = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLFadeBot"));
			m_wGLFadeTop = ImageWidget.Cast(m_wCrossGrid.FindAnyWidget("GLFadeTop"));
			m_wCoordText = TextWidget.Cast(m_wCrossGrid.FindAnyWidget("CoordText"));
			m_wHeightASLText = TextWidget.Cast(m_wCrossGrid.FindAnyWidget("HeightText"));

			// TODO configuration
			// Colors
			m_wGLTop.SetColor(m_GuidelineColor);
			m_wGLLeft.SetColor(m_GuidelineColor);
			m_wGLRight.SetColor(m_GuidelineColor);
			m_wGLBot.SetColor(m_GuidelineColor);
			m_wGLFadeTop.SetColor(m_GuidelineColor);
			m_wGLFadeLeft.SetColor(m_GuidelineColor);
			m_wGLFadeRight.SetColor(m_GuidelineColor);
			m_wGLFadeBot.SetColor(m_GuidelineColor);
			m_wCoordText.SetColor(m_GuidelineColor);
			m_wHeightASLText.SetColor(m_GuidelineColor);

			m_wCoordText.SetOpacity(0); // text

			if (m_bEnableCrosshairCoords)
				m_wCoordText.SetOpacity(1);

			if (m_bEnableCrosshairHeightASL)
				m_wHeightASLText.SetOpacity(1);
		}
		else
		{
			SetCrosshairGridVisible(false);
		}

		// cursor
		if (!m_CustomCursor)
			m_CustomCursor = new SCR_CursorCustom();
	}

	//------------------------------------------------------------------------------------------------
	//! Initialize inputs
	protected void InitInputs()
	{
		m_InputManager = g_Game.GetInputManager();

		// controller detection
		OnInputDeviceIsGamepad(!GetGame().GetInputManager().IsUsingMouseAndKeyboard());
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);

		// pause menu
		PauseMenuUI.m_OnPauseMenuOpened.Insert(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Insert(OnPauseMenuClosed);

		m_InputManager.AddActionListener("MapPanDrag", EActionTrigger.PRESSED, OnInputPanDrag);
		m_InputManager.AddActionListener("MapPanH", EActionTrigger.PRESSED, OnInputPanH);
		m_InputManager.AddActionListener("MapPanV", EActionTrigger.PRESSED, OnInputPanV);
		m_InputManager.AddActionListener("MapPanHGamepad", EActionTrigger.VALUE, OnInputPanHGamepad);	// requires trigger by value since thumbstick val ranges from -1 to 1
		m_InputManager.AddActionListener("MapPanVGamepad", EActionTrigger.VALUE, OnInputPanVGamepad);
		m_InputManager.AddActionListener("MapGamepadCursorX", EActionTrigger.VALUE, OnInputGamepadCursorH);
		m_InputManager.AddActionListener("MapGamepadCursorY", EActionTrigger.VALUE, OnInputGamepadCursorV);

		m_InputManager.AddActionListener("MapZoomIn", EActionTrigger.VALUE, OnInputZoomIn);
		m_InputManager.AddActionListener("MapZoomOut", EActionTrigger.VALUE, OnInputZoomOut);

		m_InputManager.AddActionListener("MapWheelUp", EActionTrigger.PRESSED, OnInputZoomWheelUp);
		m_InputManager.AddActionListener("MapWheelDown", EActionTrigger.PRESSED, OnInputZoomWheelDown);
		m_InputManager.AddActionListener("MapSelect", EActionTrigger.UP, HandleSelect);

		// multi selection
		m_SelectionModule = SCR_MapSelectionModule.Cast(m_MapEntity.GetMapModule(SCR_MapSelectionModule));
		if (m_SelectionModule)
		{
			m_InputManager.AddActionListener("MapMultiSelect", EActionTrigger.PRESSED, OnInputMultiSel);
			m_InputManager.AddActionListener("MapMultiSelect", EActionTrigger.UP, OnInputMultiSel);
			m_InputManager.AddActionListener("MapMultiSelectGamepad", EActionTrigger.PRESSED, OnInputMultiSelGamepad);
			m_InputManager.AddActionListener("MapMultiSelectGamepad", EActionTrigger.UP, OnInputMultiSelGamepad);
		}

		// tool interaction UI
		if (SCR_MapToolInteractionUI.Cast(m_MapEntity.GetMapUIComponent(SCR_MapToolInteractionUI)))
		{
			m_bIsDraggingAvailable = true;

			m_InputManager.AddActionListener("MapModifierKey", EActionTrigger.DOWN, OnInputModifier);
			m_InputManager.AddActionListener("MapModifierKey", EActionTrigger.UP, OnInputModifier);
			m_InputManager.AddActionListener("MapModifClick", EActionTrigger.DOWN, OnInputModifClick);
			m_InputManager.AddActionListener("MapDrag", EActionTrigger.DOWN, OnInputDrag);
			m_InputManager.AddActionListener("MapDrag", EActionTrigger.UP, OnInputDrag);
			m_InputManager.AddActionListener("MapDragGamepad", EActionTrigger.DOWN, OnInputDragToggle);
		}
		else
			m_bIsDraggingAvailable = false;

	}

	//------------------------------------------------------------------------------------------------
	//! Cleanup inputs
	protected void CleanupInputs()
	{
		if (!m_InputManager)
			m_InputManager = g_Game.GetInputManager();

		// controller detection
		GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);

		// pause menu
		PauseMenuUI.m_OnPauseMenuOpened.Remove(OnPauseMenuOpened);
		PauseMenuUI.m_OnPauseMenuClosed.Remove(OnPauseMenuClosed);

		m_InputManager.RemoveActionListener("MapPanDrag", EActionTrigger.PRESSED, OnInputPanDrag);
		m_InputManager.RemoveActionListener("MapPanH", EActionTrigger.PRESSED, OnInputPanH);
		m_InputManager.RemoveActionListener("MapPanV", EActionTrigger.PRESSED, OnInputPanV);
		m_InputManager.RemoveActionListener("MapPanHGamepad", EActionTrigger.VALUE, OnInputPanHGamepad);
		m_InputManager.RemoveActionListener("MapPanVGamepad", EActionTrigger.VALUE, OnInputPanVGamepad);
		m_InputManager.RemoveActionListener("MapGamepadCursorX", EActionTrigger.VALUE, OnInputGamepadCursorH);
		m_InputManager.RemoveActionListener("MapGamepadCursorY", EActionTrigger.VALUE, OnInputGamepadCursorV);

		m_InputManager.RemoveActionListener("MapZoomIn", EActionTrigger.VALUE, OnInputZoomIn);
		m_InputManager.RemoveActionListener("MapZoomOut", EActionTrigger.VALUE, OnInputZoomOut);

		m_InputManager.RemoveActionListener("MapWheelUp", EActionTrigger.PRESSED, OnInputZoomWheelUp);
		m_InputManager.RemoveActionListener("MapWheelDown", EActionTrigger.PRESSED, OnInputZoomWheelDown);
		m_InputManager.RemoveActionListener("MapSelect", EActionTrigger.UP, HandleSelect);

		m_InputManager.RemoveActionListener("MapModifierKey", EActionTrigger.DOWN, OnInputModifier);
		m_InputManager.RemoveActionListener("MapModifierKey", EActionTrigger.UP, OnInputModifier);
		m_InputManager.RemoveActionListener("MapModifClick", EActionTrigger.DOWN, OnInputModifClick);
		m_InputManager.RemoveActionListener("MapDrag", EActionTrigger.DOWN, OnInputDrag);
		m_InputManager.RemoveActionListener("MapDrag", EActionTrigger.UP, OnInputDrag);
		m_InputManager.RemoveActionListener("MapDragGamepad", EActionTrigger.DOWN, OnInputDragToggle);

		m_InputManager.RemoveActionListener("MapMultiSelect", EActionTrigger.PRESSED, OnInputMultiSel);
		m_InputManager.RemoveActionListener("MapMultiSelect", EActionTrigger.UP, OnInputMultiSel);
		m_InputManager.RemoveActionListener("MapMultiSelectGamepad", EActionTrigger.PRESSED, OnInputMultiSelGamepad);
		m_InputManager.RemoveActionListener("MapMultiSelectGamepad", EActionTrigger.UP, OnInputMultiSelGamepad);
	}

	//------------------------------------------------------------------------------------------------
	// OVERRIDES
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		if (!m_CursorInfo)
			m_CursorInfo = new SCR_MapCursorInfo();

		m_bIsDisabled = false;
		InitWidgets(config.RootWidgetRef);
		InitInputs();
		m_bIsInit = true;

		SetCursorType(m_CursorState);

		if (m_bIsCursorCenteredOnOpen || m_CursorInfo.isGamepad)
			ForceCenterCursor();
	}

	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		m_CursorState = EMapCursorState.CS_DEFAULT;
		SetCursorType(EMapCursorState.CS_DISABLE);
		CleanupInputs();

		m_bIsInit = false;
	}

	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (m_bIsDisabled)
			return;

		m_bRecentlyTraced = false;

		// update current pos
		GetCursorPosition(m_CursorInfo.x, m_CursorInfo.y);

		// frame handlers
		HandleMove();
		HandleHover(timeSlice);
		HandlePan(timeSlice);
		HandleZoom();

		// crosshair grid lines
		if (m_bEnableMapCrosshairVisuals && (m_CursorState & CUSTOM_CURSOR_LOCKED) == 0)
			UpdateCrosshairUI();
	}

	//------------------------------------------------------------------------------------------------
	void SCR_MapCursorModule()
	{
		foreach (SCR_CursorVisualState cursorState : m_aCursorStatesConfig)
		{
			EMapCursorState stateEnum = cursorState.m_eCursorState;
			if (stateEnum)
				m_aCursorStatesMap.Insert(stateEnum, cursorState);
		}
	}
}

//------------------------------------------------------------------------------------------------
//! Cursor visual state config
[BaseContainerProps(), SCR_CursorStateTitle()]
class SCR_CursorVisualState
{
	[Attribute("1", UIWidgets.ComboBox, "Configure selected cursor state", "", ParamEnumArray.FromEnum(EMapCursorState))]
	int m_eCursorState;

	[Attribute("{E75FB4134580A496}UI/Textures/Cursor/cursors.imageset", UIWidgets.ResourceNamePicker, desc: "Imageset selection", params: "imageset")]
	ResourceName m_sCursorIconsImageset;

	[Attribute("default", UIWidgets.EditBox, desc: "imageset quad")]
	string m_sImageQuad;

	[Attribute("", UIWidgets.EditBox, desc: "imageset quad when controller is active instead of KBM \nIf this is not defined, attibute from above is used for both cases")]
	string m_sImageQuadController;

	[Attribute("0", UIWidgets.Slider, desc: "Padding from top, in pixels", "-32 32 1")]
	float m_fPaddingTop;

	[Attribute("0", UIWidgets.Slider, desc: "Padding from left, in pixels", "-32 32 1")]
	float m_fPaddingLeft;

	[Attribute("0.76 0.38 0.08 1", UIWidgets.ColorPicker, desc: "Cursor color")]
	ref Color m_Color;

	[Attribute(desc: "Cursor lines are disabled in this state if otherwise active")]
	bool m_DisableCursorLines;
}

//------------------------------------------------------------------------------------------------
//! Custom names for cursor visual states
class SCR_CursorStateTitle : BaseContainerCustomTitle
{
	//------------------------------------------------------------------------------------------------
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		int type;
		source.Get("m_eCursorState", type);
		title = typename.EnumToString(EMapCursorState, type);

		return true;
	}
}
