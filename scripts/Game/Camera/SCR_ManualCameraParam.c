//! @ingroup ManualCamera

//! Parameter for carrying information between individual camera components.

class SCR_ManualCameraParam
{
	protected const float TRACE_DIS = 250;
	
	float timeSlice; //!< Frame time slice.
	BaseWorld world; //!< World in which the camera exists.
	IEntity target; //!< Entity under cursor
	private vector cursorWorldPos;
	
	EManualCameraFlag flag; //!< Camera flag
	
	vector transformOriginal[4]; //!< Camera transform before components evalulation. DO NOT MODIFY!
	vector transform[4]; //!< Camera transform applied after components evaluation.
	vector rotOriginal; //!< Camera rotation in euler angles before components evaluation. DO NOT MODIFY!
	vector rotDelta; //!< Camera rotation in euler angles applied after components evaluation.
	vector velocityOriginal; //!< Velocity from the previous frame. DO NOT MODIFY!
	
	vector multiplier; //!< Speed multiplier horizontal[0] and vertical[1]
	
	float fovOriginal; //!< Field of view before components evaluation. DO NOT MODIFY!
	float fov; //!< Field of view applied after components evaluation.
	
	bool isManualInputEnabled; //!< Is manual input enabled by the camera entity?
	bool isManualInput; //!< Did manual input modify the camera?
	bool isCursorEnabled = true; //!< Is cursor on empty space and not on some active element, e.g., button?
	bool isDirty; //!< Did camera settings change?
	bool isDirtyExternal; //!< Did some external settings change the camera? DO NOT MODIFY!
	
	//------------------------------------------------------------------------------------------------
	//! Set position as delta change from the current position.
	//! \param deltaPos Delta vector
	void SetDeltaPos(vector deltaPos)
	{
		transform[3] = transform[3]
			+ (transform[0] * deltaPos[0]) //--- Lateral
			+ (transform[1] * deltaPos[1]) //--- Vertical
			+ (transform[2] * deltaPos[2]); //--- Longitudinal
	}

	//------------------------------------------------------------------------------------------------
	//! Get position under cursor.
	//! \param[out] outPos world position under cursor
	//! \return true on success, false otherwise
	bool GetCursorWorldPos(out vector outPos = vector.Zero)
	{
		//--- Return cached position
		if (cursorWorldPos != vector.Zero)
		{
			outPos = cursorWorldPos;
			return true;
		}
			
		//-- Calculate new position
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		if (!workspace || !world) return false;
		
		int mouseX, mouseY;
		if (GetGame().GetInputManager().IsUsingMouseAndKeyboard())
		{
			WidgetManager.GetMousePos(mouseX, mouseY);
		}
		else
		{
			mouseX = workspace.GetWidth();
			mouseY = workspace.GetHeight();			
			mouseX /= 2;
			mouseY /= 2;
		}
		
		vector outDir;
		vector startPos = workspace.ProjScreenToWorld(workspace.DPIUnscale(mouseX), workspace.DPIUnscale(mouseY), outDir, world, -1);
		outDir *= TRACE_DIS;
	
		autoptr TraceParam trace = new TraceParam();
		trace.Start = startPos;
		trace.End = startPos + outDir;
		trace.Flags = TraceFlags.WORLD | TraceFlags.OCEAN | TraceFlags.ENTS;
		trace.LayerMask = EPhysicsLayerPresets.Projectile;
		
		float traceDis = world.TraceMove(trace, null);
		if (traceDis == 1) return false;

		if (trace.TraceEnt && !trace.TraceEnt.IsInherited(GenericTerrainEntity)) target = trace.TraceEnt;
		cursorWorldPos = startPos + outDir * traceDis;
		outPos = cursorWorldPos;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! \param pos
	void SetCursorWorldPos(vector pos)
	{
		cursorWorldPos = pos;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param camera
	void ShowDebug(SCR_ManualCamera camera)
	{
		DbgUI.Begin("SCR_ManualCameraParam", 0, 0);
		
		DbgUI.Text(string.Format("GetParent() = %1", camera.GetParent()));
		DbgUI.Text("flag = ");
		array<int> flagValues = new array<int>;
		SCR_Enum.BitToIntArray(flag, flagValues);
		foreach (int i: flagValues)
		{
			DbgUI.Text("  " + typename.EnumToString(EManualCameraFlag, i));
		}	
		DbgUI.Text(string.Format("target = %1", target));
		DbgUI.Text("isDirty = " + isDirty.ToString());
		DbgUI.Text("isManualInput = " + isManualInput.ToString());
		DbgUI.Text("isManualInputEnabled = " + isManualInputEnabled.ToString());
		DbgUI.Text("isCursorEnabled = " + isCursorEnabled.ToString());
		DbgUI.Text("multiplier = " + multiplier.ToString());
		DbgUI.Text("fov = " + fov.ToString());
		DbgUI.Text("rotOriginal = " + rotOriginal.ToString());
		DbgUI.Text("rotDelta = " + rotDelta.ToString());
		DbgUI.Text("posDelta = " + (transform[3] - transformOriginal[3]).ToString());
		DbgUI.Text("transformOriginal: ");
		DbgUI.Text("o[0] = " + transformOriginal[0].ToString());
		DbgUI.Text("o[1] = " + transformOriginal[1].ToString());
		DbgUI.Text("o[2] = " + transformOriginal[2].ToString());
		DbgUI.Text("o[3] = " + transformOriginal[3].ToString());
		DbgUI.Text("transform: ");
		DbgUI.Text("[0] = " + transform[0].ToString());
		DbgUI.Text("[1] = " + transform[1].ToString());
		DbgUI.Text("[2] = " + transform[2].ToString());
		DbgUI.Text("[3] = " + transform[3].ToString());
		DbgUI.Text("velocityOriginal = " + velocityOriginal.ToString());

		DbgUI.Spacer(30);
		bool showGraphs;
		DbgUI.Check("Show Graphs", showGraphs);
		if (showGraphs)
		{
			DbgUI.Spacer(20);
			DbgUI.Text("multiplier");
			DbgUI.PlotLive("multiplier0", 500, 50, multiplier[0], 0.01, 100, ARGBF(1, 1, 0, 0));
			DbgUI.PlotLive("multiplier1", 500, 50, multiplier[1], 0.01, 100, ARGBF(1, 0, 1, 0));
			DbgUI.PlotLive("multiplier2", 500, 50, multiplier[2], 0.01, 100, ARGBF(1, 0, 0, 1));
			
			DbgUI.Spacer(20);
			DbgUI.Text("transformDelta");
			DbgUI.PlotLive("transform0", 500, 50, transform[3][0] - transformOriginal[3][0], 0.01, 100, ARGBF(1, 1, 0, 0));
			DbgUI.PlotLive("transform1", 500, 50, transform[3][1] - transformOriginal[3][1], 0.01, 100, ARGBF(1, 0, 1, 0));
			DbgUI.PlotLive("transform2", 500, 50, transform[3][2] - transformOriginal[3][2], 0.01, 100, ARGBF(1, 0, 0, 1));
	
			DbgUI.Spacer(20);
			DbgUI.Text("rotDelta");
			DbgUI.PlotLive("rotDelta0", 500, 50, rotDelta[0], 0.01, 100, ARGBF(1, 1, 0, 0));
			DbgUI.PlotLive("rotDelta1", 500, 50, rotDelta[1], 0.01, 100, ARGBF(1, 0, 1, 0));
			DbgUI.PlotLive("rotDelta2", 500, 50, rotDelta[2], 0.01, 100, ARGBF(1, 0, 0, 1));
		}
		DbgUI.End();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void ProcessTransform()
	{
		//--- Rotation (prevent from looking straigh up or down)
		vector angles = Math3D.MatrixToAngles(transform);
		angles += rotDelta * timeSlice;
		angles[1] = Math.Clamp(angles[1], -89, 89);
		Math3D.AnglesToMatrix(angles, transform);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param transformOriginalIn
	//! \param timeSliceIn
	//! \param worldIn
	//! \param fovIn
	//! \param isManualInputEnabledIn
	//! \param velocityOriginalIn
	//! \param flagIn
	void SCR_ManualCameraParam(vector transformOriginalIn[4], float timeSliceIn, BaseWorld worldIn, float fovIn, bool isManualInputEnabledIn, vector velocityOriginalIn, EManualCameraFlag flagIn)
	{
		timeSlice = timeSliceIn;
		//isDirty = timeSlice != 0;
		multiplier[0] = timeSliceIn;
		multiplier[1] = timeSliceIn;
		flag = flagIn;
		isManualInputEnabled = isManualInputEnabledIn;
		fov = fovIn;
		fovOriginal = fovIn;
		world = worldIn;
		transformOriginal = transformOriginalIn;
		velocityOriginal = velocityOriginalIn;
		Math3D.MatrixCopy(transformOriginal, transform);
		rotOriginal = Math3D.MatrixToAngles(transformOriginal);
	}
}
