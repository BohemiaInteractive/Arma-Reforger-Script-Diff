void ScriptInvokerWidgetBool(Widget widget, bool state);
typedef func ScriptInvokerWidgetBool;

//! Attach this component to a widget in a map layout to configure interactions
class SCR_MapElementMoveComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Allows the widget to be dragged")]
	bool m_bCanDrag;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Allows to drag widget further off screen then default: half the size of the widget")]
	bool m_bCanDragOffScreen;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Allows the widget to be rotated")]
	bool m_bCanRotate;
	
	[Attribute(defvalue: "0", uiwidget: UIWidgets.CheckBox, desc: "Allows the widget to be activated")]
	bool m_bCanActivate;

	[Attribute(desc: "When activation is disabled then this will allow for resetting the rotation of the tool")]
	bool m_bCanResetRotation;
}

//! Component for interacting with map tools
class SCR_MapToolInteractionUI : SCR_MapUIBaseComponent
{
	protected const float HOLD_DRAG_EVENT_TIME = 0.2; // seconds, drag event will start immediately but the end event will use this timer to determine if drag happened or not and send it as param
	protected const float ROTATION_DEADZONE = 0.02;
	static bool s_bIsDragging = false;
	static bool s_bIsRotating = false;
	static bool s_bCanDragOffScreen = false;
	static float s_fDragTime;
	
	static protected ref ScriptInvoker<Widget> s_OnDragWidget = new ScriptInvoker();
	static protected ref ScriptInvokerBase<ScriptInvokerWidgetBool> s_OnDragEnd = new ScriptInvokerBase<ScriptInvokerWidgetBool>();	// Widget which was being dragged + bool if it was a real drag or only click
	static protected ref ScriptInvoker<Widget> s_OnActivateTool = new ScriptInvoker();
	
	static protected Widget s_DraggedWidget;
	static protected Widget s_RotatedWidget;
	
	static protected SCR_MapCursorModule s_CursorModule;
	static protected SCR_MapCursorInfo s_CursorInfo;
	
	// Invokers
	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnDragWidgetInvoker()
	{
		return s_OnDragWidget;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvokerBase<ScriptInvokerWidgetBool> GetOnDragEndInvoker()
	{
		return s_OnDragEnd;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	static ScriptInvoker GetOnActivateToolInvoker()
	{
		return s_OnActivateTool;
	}
			
	//------------------------------------------------------------------------------------------------
	//! Tool action
	static void ActivateAction()
	{
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		SCR_MapElementMoveComponent moveComp;
		
		if (!CanBeManipulated(widgets))
			return;
		
		foreach ( Widget widget : widgets )
		{
			moveComp = SCR_MapElementMoveComponent.Cast(widget.FindHandler(SCR_MapElementMoveComponent));	
			if (!moveComp || !moveComp.m_bCanActivate)
				continue;
						
			s_OnActivateTool.Invoke(widget);
			return;
		}
	}
		
	//------------------------------------------------------------------------------------------------
	//! Begin drag
	//! \return Returns true if there is a draggable widget under the cursor
	static bool StartDrag()
	{
		if (s_bIsDragging)
			return false;
		
		s_fDragTime = 0;

		if (s_CursorInfo)
		{
			s_CursorInfo.lastX = s_CursorInfo.x;
			s_CursorInfo.lastY = s_CursorInfo.y;
		}
		
		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		SCR_MapElementMoveComponent moveComp;
		
		if (!CanBeManipulated(widgets))
			return false;
		
		foreach ( Widget widget : widgets )
		{
			moveComp = SCR_MapElementMoveComponent.Cast(widget.FindHandler(SCR_MapElementMoveComponent));	
			if (moveComp)
			{
				if (!moveComp.m_bCanDrag)
					continue;
				
				s_DraggedWidget = widget;
				s_bCanDragOffScreen = moveComp.m_bCanDragOffScreen;
				break;
			}
		}
		
		if (s_DraggedWidget)
		{
			s_bIsDragging = true;
			s_OnDragWidget.Invoke(s_DraggedWidget);
			
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_GRAB);
			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Begin rotation
	//! \return Returns true if there is a rotatable widget under the cursor
	static bool StartRotate()
	{
		if (s_CursorInfo)
		{
			s_CursorInfo.lastX = s_CursorInfo.x;
			s_CursorInfo.lastY = s_CursorInfo.y;
		}

		array<Widget> widgets = SCR_MapCursorModule.GetMapWidgetsUnderCursor();
		SCR_MapElementMoveComponent moveComp;
		
		if (!CanBeManipulated(widgets))
			return false;
		
		foreach ( Widget widget : widgets )
		{
			moveComp = SCR_MapElementMoveComponent.Cast(widget.FindHandler(SCR_MapElementMoveComponent));	
			if (moveComp)
			{
				if (!moveComp.m_bCanRotate)
					continue;
				
				s_RotatedWidget = widget;
				break;
			}
		}
		
		if (s_RotatedWidget)
		{
			s_bIsRotating = true;			
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Check whether the tool isnt currently clicked from top of/under of a button
	//! \param[in] tracedWidgets
	//! \return
	static bool CanBeManipulated(array<Widget> tracedWidgets)
	{
		foreach ( Widget widget : tracedWidgets )
		{
			if (ButtonWidget.Cast(widget))
				return false;
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	//! End drag
	static void EndDrag()
	{
		s_bIsDragging = false;
		s_OnDragEnd.Invoke(s_DraggedWidget, s_fDragTime >= HOLD_DRAG_EVENT_TIME);
		
		s_DraggedWidget = null;
		s_OnDragWidget.Invoke(null, false);
		
		SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.SOUND_MAP_GADGET_RELEASE);
	}
	
	//------------------------------------------------------------------------------------------------
	//! End rotation
	static void EndRotate()
	{
		s_bIsRotating = false;
		s_RotatedWidget = null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Drag widget
	//! \param[in] widget
	static void DragWidget(notnull Widget widget)
	{				
		int screenX = GetGame().GetWorkspace().GetWidth();
		int screenY = GetGame().GetWorkspace().GetHeight();
		
		float widgetX, widgetY, fx, fy, minX, minY, maxX, maxY;
		
		// mouse position difference
		fx = s_CursorInfo.x - s_CursorInfo.lastX; 
		fy = s_CursorInfo.y - s_CursorInfo.lastY;

		// no change
		if (fx == 0 && fy == 0)
			return;

		// new pos
		vector pos = FrameSlot.GetPos(widget);
		fx = fx + pos[0];
		fy = fy + pos[1];

		//! get widget size
		widget.GetScreenSize(widgetX, widgetY);

		if (!s_bCanDragOffScreen)
		{
			//! get max screen size
			WorkspaceWidget workspace = GetGame().GetWorkspace();
			minX = workspace.DPIUnscale(-widgetX/2);
			minY = workspace.DPIUnscale(-widgetY/2);
			maxX = workspace.DPIUnscale(screenX) - workspace.DPIUnscale(widgetX/2);
			maxY = workspace.DPIUnscale(screenY) - workspace.DPIUnscale(widgetY/2);
	
			// avoid moving the element off screen
			fx = Math.Clamp(fx, minX, maxX);
			fy = Math.Clamp(fy, minY, maxY);
		}
		
		// set new postion
		FrameSlot.SetPos(widget, fx, fy);

		s_CursorInfo.lastX = s_CursorInfo.x;
		s_CursorInfo.lastY = s_CursorInfo.y;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Rotate widget based on mouse movement
	//! \param[in] widget
	static void RotateWidget(notnull Widget widget)
	{
		ImageWidget image = ImageWidget.Cast(widget.GetChildren());
		if (!image)
			return;

		// TODO ImageWidget.GetPivot(float x, float y)
		// float fx, fy, minX, minY, maxX, maxY;

		// mouse position difference
		float fx = s_CursorInfo.x - s_CursorInfo.lastX; 
		float fy = s_CursorInfo.y - s_CursorInfo.lastY;

		// no change
		if (fx == 0 && fy == 0)
			return;

		//! get widget size
		float widgetX, widgetY, widgetH, widgetV, centerX, centerY;
		widget.GetScreenPos(widgetX, widgetY);
		widget.GetScreenSize(widgetH, widgetV);
		centerX = widgetX + widgetH/2;
		centerY = widgetY + widgetV/2;

		// Get current angle versus previous angle
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		widgetH = workspace.DPIUnscale(widgetH);
		centerX = workspace.DPIUnscale(centerX);
		centerY = workspace.DPIUnscale(centerY);

		float newAngle = Math.RAD2DEG * Math.Atan2(s_CursorInfo.x - centerX, s_CursorInfo.y - centerY);
		float lastAngle = Math.RAD2DEG * Math.Atan2(s_CursorInfo.lastX - centerX, s_CursorInfo.lastY - centerY);

		float rotation = SCR_Math.FixAngle(newAngle - lastAngle, 180);

		// Slow down rotation near the center
		vector grip = Vector(s_CursorInfo.x - centerX, s_CursorInfo.y - centerY, 0);
		float rotationScale = 1;
		if (widgetH > 0 && ROTATION_DEADZONE > 0)
			rotationScale = Math.Clamp(grip.Length() / (widgetH * ROTATION_DEADZONE), 0, 1);
		
		rotation *= rotationScale * rotationScale;

		// Apply new rotation
		image.SetRotation(image.GetRotation() - rotation);

		s_CursorInfo.lastX = s_CursorInfo.x;
		s_CursorInfo.lastY = s_CursorInfo.y;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapOpen(MapConfiguration config)
	{
		super.OnMapOpen(config);
		
		s_CursorModule = SCR_MapCursorModule.Cast(m_MapEntity.GetMapModule(SCR_MapCursorModule));
		s_CursorInfo = s_CursorModule.GetCursorInfo();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnMapClose(MapConfiguration config)
	{
		s_OnDragWidget.Clear();
		s_OnDragEnd.Clear();
		s_OnActivateTool.Clear();
		s_DraggedWidget = null;
		s_RotatedWidget = null;
		s_bIsDragging = false;
		s_bIsRotating = false;
		s_fDragTime = 0;
		s_CursorModule = null;
		s_CursorInfo = null;
		
		super.OnMapClose(config);
	}
	
	//------------------------------------------------------------------------------------------------
	override void Update(float timeSlice)
	{
		if (s_bIsDragging)
		{
			DragWidget(s_DraggedWidget);
			s_fDragTime += timeSlice;
		}
		else if (s_bIsRotating)
		{
			RotateWidget(s_RotatedWidget);
		}
	}
}
