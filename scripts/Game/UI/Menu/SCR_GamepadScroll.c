/*!
A component with some functions related to scrolling with gamepad.
It solves smooth scrolling with thumb stick and auto-focusing of widgets when such scrolling is performed.

Must be attached to a vertical scroll widget.
*/

//#define GAMEPAD_SCROLL_DEBUG

class SCR_GamepadScrollComponent : SCR_ScriptedWidgetComponent
{
	[Attribute("1", UIWidgets.CheckBox, "When true and scroll input is detected, the component will try to find a new focused widget, or will reset focused widget to null if no suitable widget found")]
	protected bool m_bTryFindNewFocus;

	[Attribute("1", UIWidgets.CheckBox, "Set scroll's initial state")]
	protected bool m_bScrollEnabled;
	
	[Attribute("1", UIWidgets.CheckBox, "Is the scroll vertical or horizontal")]
	protected bool m_bIsVerticalScroll;
	
	[Attribute("0", UIWidgets.CheckBox, "Should mouse wheel inputs be allowed")]
	protected bool m_bAllowScrollWheel;

	// Constants
	static const float SCROLL_SPEED_PX_PER_SECOND_MAX = 1500;
	static const float WIDGET_DETECTION_MARGIN_PX = 3;
	
	static const float MOUSE_SCROLL_SPEED = 5;
	static const float MOUSE_RAW_SCROLL_SPEED = 1000;

	protected ScrollLayoutWidget m_wScroll;

#ifdef GAMEPAD_SCROLL_DEBUG
	// Handling of debug markers
	protected ref array<Widget> m_aDebugMarksPool;
	protected ref array<Widget> m_aDebugMarksBusy;
#endif

	protected bool m_bMouseScrolled;
	protected bool m_bShouldBeEnabled = true;
	protected bool m_bForceDisabled;

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wScroll = ScrollLayoutWidget.Cast(w);

		if (!SCR_Global.IsEditMode() && m_bScrollEnabled)
			GetGame().GetCallqueue().CallLater(OnEachFrame, 1, true);

		#ifdef GAMEPAD_SCROLL_DEBUG
		m_aDebugMarksPool = {};
		m_aDebugMarksBusy = {};
		#endif

		SCR_MenuHelper.GetOnMenuFocusGained().Insert(OnMenuFocusGained);
		SCR_MenuHelper.GetOnMenuFocusLost().Insert(OnMenuFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		if (!SCR_Global.IsEditMode())
		{
			ArmaReforgerScripted game = GetGame();
			if (game && game.GetCallqueue())
				game.GetCallqueue().Remove(OnEachFrame);
		}

		SCR_MenuHelper.GetOnMenuFocusGained().Remove(OnMenuFocusGained);
		SCR_MenuHelper.GetOnMenuFocusLost().Remove(OnMenuFocusLost);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusGained(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			SetForceDisabled(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuFocusLost(ChimeraMenuBase menu)
	{
		if (menu == ChimeraMenuBase.GetOwnerMenu(m_wRoot))
			SetForceDisabled(true);
	}

	//------------------------------------------------------------------------------------------------
	void OnEachFrame()
	{
		if (!m_wScroll)
			return;

		float tDelta = ftime / 1000.0; // ftime is milliseconds!
		Update(tDelta, m_wScroll);
	}

	//------------------------------------------------------------------------------------------------
	//! Call this from the menu, or if used as widget component it gets called automatically
	void Update(float tDelta, ScrollLayoutWidget wScroll)
	{
#ifdef GAMEPAD_SCROLL_DEBUG
		DebugMarks_StartUpdate();
		
		vector refPoint;
		if (m_bIsVerticalScroll)
			refPoint = GetVReferencePoint(wScroll);
		else
			refPoint = GetHReferencePoint(wScroll);

		PlaceDebugMark(refPoint, "-");
#endif

		// Bail if scroll content is smaller than scroll
		SCR_Rect2D scrollRect = GetWidgetRect(m_wScroll);
		Widget scrollContent = m_wScroll.GetChildren();
		if (!scrollContent)
			return;
		
		SCR_Rect2D scrollContentRect = GetWidgetRect(scrollContent);
		if (m_bIsVerticalScroll)
		{
			if (scrollContentRect.GetHeight() <= scrollRect.GetHeight())
				return;

			// Handle scrolling, try find a new focus
			float vScrollInput = GetVScrollInput();
			if (m_bAllowScrollWheel)
				vScrollInput += GetMouseScrollInput();
			
			if (vScrollInput != 0)
			{
				HandleGamepadVScrolling(tDelta, wScroll);
				if (m_bTryFindNewFocus)
					TryFindNewFocus(wScroll);
			}

#ifdef GAMEPAD_SCROLL_DEBUG
		PlaceDebugMark("200 40 0", string.Format("V Scroll Input: notNull: %1, value: %2", vScrollInput != 0, vScrollInput));
		PlaceDebugMark("800 40 0", string.Format("Current focus: %1", GetGame().GetWorkspace().GetFocusedWidget()));
		DebugMarks_EndUpdate();
#endif
		}
		else
		{
			if (scrollContentRect.GetWidth() <= scrollRect.GetWidth())
				return;

			// Handle scrolling, try find a new focus
			float hScrollInput = GetHScrollInput();
			if (m_bAllowScrollWheel)
				hScrollInput += GetMouseScrollInput();
			
			if (hScrollInput != 0)
			{
				HandleGamepadHScrolling(tDelta, wScroll);
				if (m_bTryFindNewFocus)
					TryFindNewFocus(wScroll);
			}

#ifdef GAMEPAD_SCROLL_DEBUG
		PlaceDebugMark("200 40 0", string.Format("H Scroll Input: notNull: %1, value: %2", hScrollInput != 0, hScrollInput));
		PlaceDebugMark("800 40 0", string.Format("Current focus: %1", GetGame().GetWorkspace().GetFocusedWidget()));
		DebugMarks_EndUpdate();
#endif
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleGamepadVScrolling(float tDelta, ScrollLayoutWidget wScroll)
	{
		Widget scrollContent = wScroll.GetChildren();

		if (!scrollContent)
			return;

		float xScrollSize, yScrollSize;
		wScroll.GetScreenSize(xScrollSize, yScrollSize);

		float xContentSize, yContentSize;
		scrollContent.GetScreenSize(xContentSize, yContentSize);
		
		if (yContentSize <= yScrollSize || yContentSize == 0)
			return;

		float scrollInputValue = GetVScrollInput();
		if (m_bAllowScrollWheel)
			scrollInputValue += GetMouseScrollInput();
		
		float xPosAbsCurrent, yPosAbsCurrent;
		wScroll.GetSliderPosPixels(xPosAbsCurrent, yPosAbsCurrent);

		float yPosAbsNew = yPosAbsCurrent - scrollInputValue * tDelta * SCROLL_SPEED_PX_PER_SECOND_MAX;
		
		wScroll.SetSliderPosPixels(0, yPosAbsNew);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleGamepadHScrolling(float tDelta, ScrollLayoutWidget wScroll)
	{
		Widget scrollContent = wScroll.GetChildren();

		if (!scrollContent)
			return;

		float xScrollSize, yScrollSize;
		wScroll.GetScreenSize(xScrollSize, yScrollSize);

		float xContentSize, yContentSize;
		scrollContent.GetScreenSize(xContentSize, yContentSize);
		
		if (xContentSize <= xScrollSize || xContentSize == 0)
			return;

		float scrollInputValue = GetHScrollInput();
		if (m_bAllowScrollWheel)
			scrollInputValue -= GetMouseScrollInput();
		
		float xPosAbsCurrent, yPosAbsCurrent;
		wScroll.GetSliderPosPixels(xPosAbsCurrent, yPosAbsCurrent);
		
		float xPosAbsNew = xPosAbsCurrent + scrollInputValue * tDelta * SCROLL_SPEED_PX_PER_SECOND_MAX;
		
		wScroll.SetSliderPosPixels(xPosAbsNew, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected static float GetHScrollInput()
	{
		return GetGame().GetInputManager().GetActionValue("MenuScrollHorizontal");
	}

	//------------------------------------------------------------------------------------------------
	protected static float GetVScrollInput()
	{
		return GetGame().GetInputManager().GetActionValue("MenuScrollVertical");
	}

	//------------------------------------------------------------------------------------------------
	protected float GetMouseScrollInput()
	{
		float value = GetGame().GetInputManager().GetActionValue("MouseWheel");
		if (value != 0)
		{
			m_bMouseScrolled = true;
			value = value / MOUSE_RAW_SCROLL_SPEED * MOUSE_SCROLL_SPEED;
			return value;
		}
		
		m_bMouseScrolled = false;
		
		return 0;
	}

	//------------------------------------------------------------------------------------------------
	// Try to unfocus current focused widget if it's out of scroll bounds
	protected static void TryUnfocusWidget(Widget scroll)
	{
		Widget currentFocus = GetGame().GetWorkspace().GetFocusedWidget();

		if (!currentFocus)
			return;

		SCR_Rect2D scrollRect = GetWidgetRect(scroll);
		SCR_Rect2D focusedWidgetRect = GetWidgetRect(scroll);

		if (!scrollRect.HasInside(focusedWidgetRect))
			GetGame().GetWorkspace().SetFocusedWidget(null);
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to find the new widget to focus
	protected void TryFindNewFocus(ScrollLayoutWidget wScroll)
	{
		if (m_bMouseScrolled)
			return;
		
		SCR_Rect2D scrollRect = GetWidgetRect(wScroll);
		scrollRect.ExpandAllDirections(WIDGET_DETECTION_MARGIN_PX); // Otherwise the strict conditions omit some widgets at the edge sometimes

		// Find all potential focus widgets in base
		array<Widget> allButtons = {};
		FindAllButtons(wScroll, allButtons);

		// Find widgets within view area of Base
		array<Widget> widgetsInFrame = {};
		foreach (Widget w : allButtons)
		{
			SCR_Rect2D wRect = GetWidgetRect(w);
			if (scrollRect.HasInside(wRect))
				widgetsInFrame.Insert(w);
		}

		if (widgetsInFrame.IsEmpty())
			return;
		
		vector refPos;
		if (m_bIsVerticalScroll)
			refPos = GetVReferencePoint(wScroll);
		else
			refPos = GetHReferencePoint(wScroll);

		// Sort widgets by their proximity to prev focus, or just select any widget
		Widget newFocus = null;

		float metricMin;
		metricMin = GetFocusMetric(refPos, widgetsInFrame[0]);
		newFocus = widgetsInFrame[0];

		foreach (Widget w : widgetsInFrame)
		{
			float m = GetFocusMetric(refPos, w);

#ifdef GAMEPAD_SCROLL_DEBUG
			SCR_Rect2D wRect = GetWidgetRect(w);
			vector wRectCenter = wRect.GetCenter(); // !! The actual GetFocusMetric is not calculated between widget center!
			PlaceDebugMark(wRectCenter, string.Format("%1", m));
#endif

			if (m < metricMin)
			{
				newFocus = w;
				metricMin = m;
			}
		}

		GetGame().GetWorkspace().SetFocusedWidget(newFocus);
	}
	//------------------------------------------------------------------------------------------------
	//! Returns reference point of the scroll widget
	protected static vector GetVReferencePoint(ScrollLayoutWidget wScroll)
	{
		SCR_Rect2D scrollRect = GetWidgetRect(wScroll);
		float sliderX, sliderY;
		wScroll.GetSliderPos(sliderX, sliderY);
		vector refPoint;
		
		refPoint[0] = scrollRect.p0[0];
		refPoint[1] = scrollRect.p0[1] + sliderY * scrollRect.GetHeight();

		if (!SCR_WidgetTools.InHierarchy(GetGame().GetWorkspace().GetFocusedWidget(), wScroll))
			return refPoint;

		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		float xPos, yPos, width, height;
		focusedWidget.GetScreenPos(xPos, yPos);
		focusedWidget.GetScreenSize(width, height);
		SCR_Rect2D focusedRect = GetWidgetRect(focusedWidget);
		
		refPoint[0] = width * 0.5 + xPos - 1;

		return refPoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns reference point of the scroll widget
	protected static vector GetHReferencePoint(ScrollLayoutWidget wScroll)
	{
		SCR_Rect2D scrollRect = GetWidgetRect(wScroll);
		float sliderX, sliderY;
		wScroll.GetSliderPos(sliderX, sliderY);
		vector refPoint;
		
		refPoint[0] = scrollRect.p0[0] + sliderX * scrollRect.GetWidth();
		refPoint[1] = scrollRect.p0[1];

		if (!SCR_WidgetTools.InHierarchy(GetGame().GetWorkspace().GetFocusedWidget(), wScroll))
			return refPoint;

		Widget focusedWidget = GetGame().GetWorkspace().GetFocusedWidget();
		float xPos, yPos, width, height;
		focusedWidget.GetScreenPos(xPos, yPos);
		focusedWidget.GetScreenSize(width, height);
		SCR_Rect2D focusedRect = GetWidgetRect(focusedWidget);
		
		refPoint[1] = height * 0.5 + yPos - 1;

		return refPoint;
	}

	//------------------------------------------------------------------------------------------------
	// Returns metric between a point and a widget
	protected static float GetFocusMetric(vector v0, Widget w)
	{
		// Find left midpoint of widget
		SCR_Rect2D wRect = GetWidgetRect(w);
		vector wRectPoint = wRect.p0;
		wRectPoint[1] = 0.5 * (wRect.p0[1] + wRect.p1[1]);

		vector v1 = wRectPoint;

		float dx = v0[0] - v1[0];
		float dy = v0[1] - v1[1];

		return 100 *Math.AbsFloat(dy) + Math.AbsFloat(dx);
	}

	//------------------------------------------------------------------------------------------------
	//! Returns a rect created from widget's GetScreenPos and GetScreenSize
	static SCR_Rect2D GetWidgetRect(Widget w)
	{
		vector pos;
		vector size;

		float posX, posY;
		float sizeX, sizeY;

		w.GetScreenPos(posX, posY);
		w.GetScreenSize(sizeX, sizeY);

		pos[0] = posX;
		pos[1] = posY;
		size[0] = sizeX;
		size[1] = sizeY;

		return SCR_Rect2D.FromPosAndSize(pos, size);
	}

	//------------------------------------------------------------------------------------------------
	//! Finds all buttons recursively
	static void FindAllButtons(Widget w, array<Widget> arrayOut)
	{
		// Add yourself to button list
		ButtonWidget wb = ButtonWidget.Cast(w);
		if (wb)
		{
			if (wb.IsFocusable())
				arrayOut.Insert(wb);
		}

		Widget child = w.GetChildren();
		while (child)
		{
			FindAllButtons(child, arrayOut);
			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	void SetTryFindNewFocus(bool tryFindNewFocus)
	{
		m_bTryFindNewFocus = tryFindNewFocus;
	}

	//------------------------------------------------------------------------------------------------
	void SetEnabled(bool enabled)
	{
		m_bShouldBeEnabled = enabled;
		SetEnabled_Internal(enabled);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetEnabled_Internal(bool enabled)
	{
		GetGame().GetCallqueue().Remove(OnEachFrame);

		m_bScrollEnabled = enabled && !m_bForceDisabled;
		if (m_bScrollEnabled)
			GetGame().GetCallqueue().CallLater(OnEachFrame, 1, true);
	}

	//------------------------------------------------------------------------------------------------
	protected void SetForceDisabled(bool forceDisabled)
	{
		m_bForceDisabled = forceDisabled;

		if (forceDisabled)
			SetEnabled_Internal(false);
		else
			SetEnabled_Internal(m_bShouldBeEnabled);
	}

//------------------------------------------------------------------------------------------------
// Handling of debug markers
//------------------------------------------------------------------------------------------------

#ifdef GAMEPAD_SCROLL_DEBUG

	//------------------------------------------------------------------------------------------------
	protected void DebugMarks_StartUpdate()
	{
		// Return debug marks back to pool
		foreach (Widget w : m_aDebugMarksBusy)
		{
			m_aDebugMarksPool.Insert(w);
		}

		m_aDebugMarksBusy.Clear();
	}

	//------------------------------------------------------------------------------------------------
	// Puts a debug mark on this position. Must be called each frame!!
	// This must be used only between DebugMarks_StartUpdate and DebugMarks_EndUpdate
	protected void PlaceDebugMark(vector pos, string text)
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace();

		Widget w;
		if (!m_aDebugMarksPool.IsEmpty())
		{
			w = m_aDebugMarksPool[0];
			m_aDebugMarksPool.Remove(0);
		}
		else
		{
			Widget workspaceRoot = GetGame().GetWorkspace();
			w = workspace.CreateWidgets("{79FC19F28E381C1E}UI/layouts/Menus/DebugTextMark.layout", workspaceRoot);
		}

		m_aDebugMarksBusy.Insert(w);

		TextWidget wtext = TextWidget.Cast(w.FindWidget("Text"));
		wtext.SetText(text);
		w.SetVisible(true);
		FrameSlot.SetPos(w, workspace.DPIUnscale(pos[0]), workspace.DPIUnscale(pos[1]));
	}

	//------------------------------------------------------------------------------------------------
	protected void DebugMarks_EndUpdate()
	{
		// Hide all in the pool - by now they have either been used or they are not needed now
		foreach (Widget w : m_aDebugMarksPool)
		{
			w.SetVisible(false);
		}
	}

#endif
}
