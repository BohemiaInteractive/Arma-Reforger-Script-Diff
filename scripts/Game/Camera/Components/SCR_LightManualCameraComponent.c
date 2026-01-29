//! @ingroup ManualCamera

//! Create a light on camera position.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_LightManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(params: "et")]
	protected ResourceName m_LightPrefab;
	
	[Attribute("-24")]
	protected float m_fLongitudinalOffset;
	
	[Attribute(defvalue: "")]
	protected string m_sSoundEvent;
	
	[Attribute(defvalue: "1 1 1 0", desc: "Color of Camera attached light")]
	protected ref Color m_cCameraLightColor;

	[Attribute(defvalue: "1 1 1 0", desc: "Color of pointing light")]
	protected ref Color m_cPointingLightColor;
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "",  params: "1 1 0 0")]
	protected ref Curve m_cCameraLightIntensityCurve;
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, "",  params: "1 1 0 0")]
	protected ref Curve m_cPointingLightIntensityCurve;
	
	[Attribute("150", desc: "How high in the air is the light compaired to the ground at the given position if it does not follow the camera")]
	protected float m_fPointingLightHeight;
	
	[Attribute("0", desc: "(Keyboard) How high does the camera need to be from the ground before the system considers showing the pointing light (still only showing when ShowPointingLightDistance is reached)")]
	protected float m_fCameraHeightBeforeShowPointing_keyboard;

	[Attribute("0", desc: "(Gamepad) How high does the camera need to be from the ground before the system considers showing the pointing light (still only showing when ShowPointingLightDistance is reached)")]
	protected float m_fCameraHeightBeforeShowPointing_gamepad;
	
	[Attribute("100", desc: "(Keyboard) How far does the hit location need to be before the pointing light is shown")]
	protected float m_fShowPointingLightDistance_keyboard;

	[Attribute("100", desc: "(Gamepad) How far does the hit location need to be before the pointing light is shown")]
	protected float m_fShowPointingLightDistance_gamepad;
	
	[Attribute("1", desc: "If false the rotation is the same as camera. If true it will set the m_fLightDeattachedRotationOverrideY as pointing light rotation Y")]
	protected bool m_bPointingLightRotationOverride;
	
	[Attribute("-75", desc: "Will set pointing light rotation Y to this value if m_bPointingLightRotationOverride is true")]
	protected float m_fPointingLightRotationOverrideY;
	
	[Attribute("100", desc: "Distance from ground when Camera light is at max intensity", params: "0 99999")]
	protected float m_fCameraLightIntensityMaxHeight;
	
	[Attribute("200", desc: "Pointing light will be max light intensity fi this distance from light or higher")]
	protected float m_fPointingLightIntensityMaxDistance;
	

	[Attribute("17", desc: "Min intensity of camera light. This uses the ground level (alpha 0) to calculate the alpha to use in m_cCameraLightIntensityCurve", params: "-8 21")]
	protected float m_fCameraLightIntesityMin;
	
	[Attribute("17", desc: "Max intensity of camera light. This uses m_fCameraLightIntensityMaxHeight from ground (alpha 1) to calculate the alpha to use in m_cCameraLightIntensityCurve", params: "-8 21")]
	protected float m_fCameraLightIntesityMax;
	

	[Attribute("17", desc: "Min intensity of pointer light. This uses the m_fPointingLightIntensityMaxDistance (alpha 0) to calculate the alpha to use in m_cPointingLightIntensityCurve", params: "-8 21")]
	protected float m_fPointingLightIntesityMin;
	
	[Attribute("17", desc: "Max intensity of pointer light. This uses the m_fPointingLightIntensityMaxDistance (alpha 1) to calculate alpha to use in m_cPointingLightIntensityCurve", params: "-8 21")]
	protected float m_fPointingLightIntesityMax;	
	
	protected bool m_bIsUsingKeyboardAndMouse = true;
	
	//~ Max and min values of LV light to cap any overflow and prevent errors
	protected float m_fLightMaxLV = 21;
	protected float m_fLightMinLV = -8;
	
	protected LightEntity m_CameraLight;
	protected LightEntity m_PointingLight;
	protected BaseWorld m_World;
	protected WorkspaceWidget m_WorkSpace;
	
	protected bool m_bIsLightOn;
	protected ref ScriptInvoker Event_OnLightChanged = new ScriptInvoker();
	
	protected bool m_bLightAtCameraPosition = true;
	private const float LIGHT_TRACE_DISTANCE = 1500;
	private const float LIGHT_MOUSE_Y_OFFSET = 20;
	
	//------------------------------------------------------------------------------------------------
	//! Toggle camera light.
	void ToggleLight()
	{
		if (!IsEnabled())
			return;

		SetLight(m_CameraLight == null);
	}

	//------------------------------------------------------------------------------------------------
	//! Set camera light.
	//! \param[in] enable True to turn on the light, false to turn it off
	//! \param[in] noSound True to prevent the sound from being played.
	void SetLight(bool enable, bool noSound = false)
	{
		//--- No change, ignore
		if (enable == (m_CameraLight != null))
			return;

		m_bIsLightOn = enable;
		Event_OnLightChanged.Invoke(enable);
		 
		if (enable)
		{
			m_CameraLight = LightEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_LightPrefab)));
			
			GetCameraEntity().AddChild(m_CameraLight, EAddChildFlags.NONE);
			
			vector lightTrasform[4];
			Math3D.MatrixIdentity4(lightTrasform);
			lightTrasform[3] = Vector(0, 0, m_fLongitudinalOffset);
			m_CameraLight.SetLocalTransform(lightTrasform);
			
			vector cameraTransform[4];
			GetCameraEntity().GetWorldTransform(cameraTransform);
			CameraLightIntensityUpdate(cameraTransform);
		}
		else
		{
			delete m_CameraLight;
			
			if (m_PointingLight)
				delete m_PointingLight;
		}

		if (!noSound && !m_sSoundEvent.IsEmpty())
			SCR_UISoundEntity.SoundEvent(m_sSoundEvent);
	}

	//------------------------------------------------------------------------------------------------
	//~ On mouse and keyboard changed
	protected void OnInputDeviceIsGamepad(bool isGamepad)
	{	
		m_bIsUsingKeyboardAndMouse = !isGamepad;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return if light is on in editor or not
	bool IsLightOn()
	{
		return m_bIsLightOn;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Get current light change ScriptInvoker which is called if Light is turned on/off
	//! \return Script invoker
	ScriptInvoker GetOnLightChanged()
	{
		return Event_OnLightChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		if (m_bIsLightOn)
			data.m_aValues = {m_bIsLightOn};
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		if (data.m_aValues && !data.m_aValues.IsEmpty())
			SetLight(data.m_aValues[0] != 0, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		//~ If there is no light do check the distance from the camera to ground
		if (!m_bIsLightOn || !m_CameraLight)
			return;
		
		//~ Update camera light intensity depending on distance to ground
		CameraLightIntensityUpdate(param.transform);
		
		//~ Check if the light needs to be moved and move it
		if (m_bIsUsingKeyboardAndMouse)
			PointingLightTransformUpdateKeyboard(param);
		else 
			PointingLightTransformUpdateGamepad(param);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Updates the camera light intensity depending on how far the camera is from the ground
	protected void CameraLightIntensityUpdate(vector cameraTransform[4])
	{		
		//~ Get light intensity using distance from ground and max distance intensity and using that value as alpha in the m_cCameraLightIntensityCurve
		float lightIntensity = Math.Lerp(m_fCameraLightIntesityMin, m_fCameraLightIntesityMax, LegacyCurve.Curve(ECurveType.CurveProperty2D, Math.Clamp(SCR_TerrainHelper.GetHeightAboveTerrain(cameraTransform[3], m_World, true) / m_fCameraLightIntensityMaxHeight, 0, 1), m_cCameraLightIntensityCurve)[1]);
		m_CameraLight.SetColor(m_cCameraLightColor, Math.Clamp(lightIntensity, m_fLightMinLV, m_fLightMaxLV));
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Keyboard function to give LightTransformUpdate the correct values
	protected void PointingLightTransformUpdateKeyboard(SCR_ManualCameraParam param)
	{
		PointingLightTransformUpdate(param, GetCursorWorldPosition(), m_fCameraHeightBeforeShowPointing_keyboard, m_fShowPointingLightDistance_keyboard);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Gamepad function to give LightTransformUpdate the correct values
	protected void PointingLightTransformUpdateGamepad(SCR_ManualCameraParam param)
	{
		PointingLightTransformUpdate(param, GetCameraCenterRayCastPosition(), m_fCameraHeightBeforeShowPointing_gamepad, m_fShowPointingLightDistance_gamepad);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Checks if pointing light needs to be spawned and set it to pointing position
	protected void PointingLightTransformUpdate(SCR_ManualCameraParam param, vector pointingPosition, float cameraHeightBeforeDeatach, float lightDetachDistanceSq)
	{		
		//~ Get Camera Transform
		vector cameraTransform[4];
		GetCameraEntity().GetWorldTransform(cameraTransform);
		
		bool showPointingLight = false;
		float distanceSq;
		
		//~ Raycast distance greater than LIGHT_TRACE_DISTANCE so keep light on camera
		if (pointingPosition != vector.Zero)
		{
			//~ Get distance between camera and pointing position
			distanceSq = vector.DistanceSq(cameraTransform[3], pointingPosition);			

			//~ Check if the distance is greater then the light detach distance
			if (distanceSq >= lightDetachDistanceSq)
			{
				//~ Check Camera is higher then start detach hight
				float cameraHeightAboveTerrain = SCR_TerrainHelper.GetHeightAboveTerrain(cameraTransform[3], m_World, true);
				if (cameraHeightAboveTerrain >= cameraHeightBeforeDeatach)
					showPointingLight = true;
			}
		}
		
		//~ Check if should dettach camera
		if (showPointingLight)
		{
			//~ Spawn a pointing light if there is none
			if (!m_PointingLight)
				m_PointingLight = LightEntity.Cast(GetGame().SpawnEntityPrefab(Resource.Load(m_LightPrefab)));

			vector lightTransform[4] = cameraTransform;
			lightTransform[3] = Vector(pointingPosition[0], pointingPosition[1] + m_fPointingLightHeight, pointingPosition[2]);		
			
			//~ Override rotation Y
			if (m_bPointingLightRotationOverride)
			{
				//~ Todo: This should use look at rotation (Camera to cursor position) so it looks like you are shining from the camera
				vector angles = Math3D.MatrixToAngles(cameraTransform);
				angles[1] = m_fPointingLightRotationOverrideY;
				
				Math3D.AnglesToMatrix(angles, lightTransform);
			}
			
			//~ Set Light position
			m_PointingLight.SetWorldTransform(lightTransform);
			
			//~ Set light intensity depending on distance
			float lightIntensity =  Math.Lerp(m_fPointingLightIntesityMin, m_fPointingLightIntesityMax, LegacyCurve.Curve(ECurveType.CurveProperty2D, Math.Clamp((distanceSq - lightDetachDistanceSq) / (m_fPointingLightIntensityMaxDistance - lightDetachDistanceSq), 0, 1), m_cPointingLightIntensityCurve)[1]);
			m_PointingLight.SetColor(m_cPointingLightColor, Math.Clamp(lightIntensity, m_fLightMinLV, m_fLightMaxLV));						
		}
		//~ Delete the light if there is any
		else if (m_PointingLight)
		{
			delete m_PointingLight;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get world cursor position. Though returns Camera center raycast if hovering over UI or if the raycast is too far
	protected vector GetCursorWorldPosition()
	{
		vector cursorWorldPos;

		int mouseX, mouseY;
		WidgetManager.GetMousePos(mouseX, mouseY);
				
		vector outDir;
		vector startPos = m_WorkSpace.ProjScreenToWorld(m_WorkSpace.DPIUnscale(mouseX), m_WorkSpace.DPIUnscale(mouseY + LIGHT_MOUSE_Y_OFFSET), outDir, m_World, -1);
		outDir *= LIGHT_TRACE_DISTANCE;
	
		autoptr TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.WORLD;
		trace.LayerMask = TRACE_LAYER_CAMERA;
		
		//~ If under the Ocean also hit the water
		if (startPos[1] > m_World.GetOceanBaseHeight())
			trace.Flags = trace.Flags | TraceFlags.OCEAN;
		
		float rayDistance = m_World.TraceMove(trace, null);
		
		//~ Max trace reached so make sure light hidden
		if (rayDistance >= 1) 
			cursorWorldPos = Vector(0, - 1000, 0);

		cursorWorldPos = startPos + outDir * rayDistance;
		return cursorWorldPos;
	}
	
	//------------------------------------------------------------------------------------------------
	//~ Get camera raycast position
	protected vector GetCameraCenterRayCastPosition()
	{
		//~ Get Camera Transform
		vector transform[4];
		GetCameraEntity().GetWorldCameraTransform(transform);
		
		//~ Get Starting position and rightVector of camera
		vector startPos = transform[3];
		vector rightVector = {transform[2][0], transform[2][1], transform[2][2]};		

		//~ Set trace params
		autoptr TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + rightVector * LIGHT_TRACE_DISTANCE;
		trace.Flags = TraceFlags.WORLD;
		trace.LayerMask = TRACE_LAYER_CAMERA;
		
		//~ Set trace to ocean as well
		if (startPos[1] > m_World.GetOceanBaseHeight())
			trace.Flags = trace.Flags | TraceFlags.OCEAN;
		
		//~ Get hitDist (value between 0 and 1)
		float rayDistance = m_World.TraceMove(trace, null);
		
		//~ Trace hit nothing or was super far so set light to hidden
		if (rayDistance >= 1)
			return Vector(0, - 1000, 0);
		
		//~ Get hit position
		vector hitPos = startPos + rightVector * (rayDistance * LIGHT_TRACE_DISTANCE);
		
		return hitPos;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_World = GetGame().GetWorld();
				
		if (!m_World)
			Print("SCR_LightManualCameraComponent could not find World!", LogLevel.WARNING);
		
		m_WorkSpace = GetGame().GetWorkspace();
		if (!m_WorkSpace)
			Print("SCR_LightManualCameraComponent could not find Workspace!", LogLevel.WARNING);
		
		m_fShowPointingLightDistance_keyboard = Math.Pow(m_fShowPointingLightDistance_keyboard, 2);
		m_fShowPointingLightDistance_gamepad = Math.Pow(m_fShowPointingLightDistance_gamepad, 2);
		m_fPointingLightIntensityMaxDistance = Math.Pow(m_fPointingLightIntensityMaxDistance, 2);
		
		GetGame().OnInputDeviceIsGamepadInvoker().Insert(OnInputDeviceIsGamepad);
		
		if (m_LightPrefab.IsEmpty())
		{
			Print("SCR_LightManualCameraComponent: Cannot initialize, m_LightPrefab not defined!", LogLevel.WARNING);
			return false;
		}
		
		InputManager inputManager = GetInputManager();
		if (!inputManager)
			return false;
		
		inputManager.AddActionListener("ManualCameraLight", EActionTrigger.DOWN, ToggleLight);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraExit()
	{
		if (m_CameraLight)
			delete m_CameraLight;

		if (m_PointingLight)
			delete m_PointingLight;
		
		if (m_World && m_WorkSpace)
			GetGame().OnInputDeviceIsGamepadInvoker().Remove(OnInputDeviceIsGamepad);
		
		InputManager inputManager = GetInputManager();
		if (!inputManager)
			return;
		
		inputManager.RemoveActionListener("ManualCameraLight", EActionTrigger.DOWN, ToggleLight);
	}
}
