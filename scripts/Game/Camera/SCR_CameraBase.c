[EntityEditorProps(category: "GameScripted/Camera", description: "Scripted camera", color: "128 000 255 255")]
class SCR_CameraBaseClass: CameraBaseClass
{
};

/*!
Base class for scripted cameras
*/
class SCR_CameraBase: CameraBase
{
	[Attribute(desc: "Show debug shape in World Editor even when the entity is not selected.")]
	protected bool m_bShowDebugViewCone;
	
	/*!
	Check if position is in camera view for given workspace.
	\param worldPos World position
	\param workspace Workspace widget
	\return True when in view
	*/
	bool IsInView(vector worldPos, WorkspaceWidget workspace = null)
	{
		if (!workspace)
			workspace = GetGame().GetWorkspace();
		
		return IsInView(worldPos, workspace.GetWidth() / (float)workspace.GetHeight());
	}
	/*!
	Check if position is in camera view for given aspect ratio.
	\param worldPos World position
	\param aspectRatio Aspect ratio W/H, e.g., 4:3 = 1.333
	\return True when in view
	*/
	bool IsInView(vector worldPos, float aspectRatio)
	{
		vector localPos = CoordToLocal(worldPos);
		float angleV = SCR_Math.DeltaAngle(Math.Atan2(localPos[2], localPos[1]) * Math.RAD2DEG, 90);
		float angleH = SCR_Math.DeltaAngle(Math.Atan2(localPos[2], localPos[0]) * Math.RAD2DEG, 90);
		
		float fovV = GetVerticalFOV() / 2;
		float fovH = Math.Atan2(Math.Tan(fovV * Math.DEG2RAD) * aspectRatio, 1) * Math.RAD2DEG;
		
		return Math.InverseLerp(-fovV, fovV, angleV) < 1 && Math.InverseLerp(-fovH, fovH, angleH) < 1;
	}
	
	/*!
	Get event called the moment this camera entity is activated.
	\return Script invoker
	*/
	private ref ScriptInvoker Event_EOnActivate = new ScriptInvoker;
	/*!
	Get event called the moment this camera entity is deactivated.
	\return Script invoker
	*/
	private ref ScriptInvoker Event_EOnDeactivate = new ScriptInvoker;
	/*!
	Get event called the moment this camera is activated by SCR_CameraManager.
	\return Script invoker
	*/
	private ref ScriptInvoker Event_CameraActivate = new ScriptInvoker;
	/*!
	Get event called the moment this camera is deactivated by SCR_CameraManager.
	\return Script invoker
	*/
	private ref ScriptInvoker Event_CameraDeactivate = new ScriptInvoker;
	
	ScriptInvoker GetOnActivate()
	{
		return Event_EOnActivate;
	}
	ScriptInvoker GetOnDeactivate()
	{
		return Event_EOnDeactivate;
	}
	ScriptInvoker GetOnCameraActivate()
	{
		return Event_CameraActivate;
	}
	ScriptInvoker GetOnCameraDeactivate()
	{
		return Event_CameraDeactivate;
	}
	
	override void EOnActivate(IEntity owner)
	{
		super.EOnActivate(owner);
		
		Event_EOnActivate.Invoke();
	}
	override void EOnDeactivate(IEntity owner)
	{
		super.EOnDeactivate(owner);
		
		if (Event_EOnDeactivate) //--- Invoker may be null when the entity is being deleted
			Event_EOnDeactivate.Invoke();
	}
	override void CameraActivate()
	{
		Event_CameraActivate.Invoke();
	}
	override void CameraDeactivate()
	{
		Event_CameraDeactivate.Invoke();
	}
	
#ifdef WORKBENCH
	override int _WB_GetAfterWorldUpdateSpecs(IEntitySource src)
	{
		return EEntityFrameUpdateSpecs.CALL_WHEN_ENTITY_VISIBLE;
	}

	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || (!api.IsEntitySelected(api.EntityToSource(this)) && !m_bShowDebugViewCone))
			return;
		
		float length = GetFarPlane();
		float height = Math.Tan(GetVerticalFOV() / 2 * Math.DEG2RAD) * length;
		float width = height * (api.GetScreenWidth() / api.GetScreenHeight());
		
		vector transform[4];
		GetTransform(transform);
		
		vector points[12];
		
		//--- Top
		points[0] = transform[3];
		points[1] = transform[3] + transform[2] * length + transform[1] * height + transform[0] * width;
		points[2] = transform[3] + transform[2] * length + transform[1] * height + transform[0] * -width;

		//--- Right
		points[3] = points[0];
		points[4] = transform[3] + transform[2] * length + transform[1] * -height + transform[0] * width;
		points[5] = points[1];

		//--- Bottom
		points[6] = points[0];
		points[7] = transform[3] + transform[2] * length + transform[1] * -height + transform[0] * -width;
		points[8] = points[4];

		//--- Left
		points[9] = points[0];
		points[10] = points[2];
		points[11] = points[7];

		Shape.CreateTris(ARGBF(0.25, 0.5, 0, 1), ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE, points, 4);
	}
#endif
};