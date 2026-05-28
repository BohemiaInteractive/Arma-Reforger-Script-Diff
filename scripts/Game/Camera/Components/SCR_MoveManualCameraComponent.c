//! @ingroup ManualCamera

//! Camera movement above sea level.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_MoveManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "27", desc: "Speed coefficient.")]
	private float m_fSpeed;
	
	protected bool m_bBlockedByRadialMenu;
	
#ifdef ENABLE_DIAG
	protected const float DEBUG_DISTANCE_EXTENSION = 2000;
#endif

	//------------------------------------------------------------------------------------------------
	override void EOnCameraSave(SCR_ManualCameraComponentSave data)
	{
		vector transform[4];
		GetCameraEntity().GetWorldTransform(transform);
		vector angles = Math3D.MatrixToAngles(transform);
		
		data.m_aValues = {
			transform[3][0], transform[3][1], transform[3][2], //--- Pos
			angles[0], angles[1] //--- Yaw, pitch (don't save roll, this component cannot change)
		};
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraLoad(SCR_ManualCameraComponentSave data)
	{
		if (!data.m_aValues || data.m_aValues.Count() < 5)
			return;
		
		vector transform[4];
		transform[3] = Vector(data.m_aValues[0], data.m_aValues[1], data.m_aValues[2]);
		Math3D.AnglesToMatrix(Vector(data.m_aValues[3], data.m_aValues[4], 0), transform);
		
		GetCameraEntity().SetWorldTransform(transform);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isManualInputEnabled)
			return;
		
		bool radialMenu = !m_InputManager.IsUsingMouseAndKeyboard() && (m_InputManager.GetActionValue("RadialX") || m_InputManager.GetActionValue("RadialY"));
		if (radialMenu)
			m_bBlockedByRadialMenu = true;
		
		float lateral = m_InputManager.GetActionValue("ManualCameraMoveLateral");
		float vertical = m_InputManager.GetActionValue("ManualCameraMoveVertical");
		float longitudinal = m_InputManager.GetActionValue("ManualCameraMoveLongitudinal");
		if (lateral == 0 && vertical == 0 && longitudinal == 0)
		{
			if (!radialMenu)
				m_bBlockedByRadialMenu = false;
			
			return;
		}
		
		//--- Radial menu was just closed - block the input until player releases left stick
		if (m_bBlockedByRadialMenu)
			return;
		
		//--- Make sure that horizontal input is not bigger than 1 (happens on gamepad when moving diagonally)
		vector newDir = Vector(lateral, 0, longitudinal);
		if (newDir.LengthSq() > 1)
			newDir.Normalize();
		
		//--- Get horizontal vector
		vector currentDir = param.transform[2];
		currentDir[1] = 0;
		currentDir.Normalize();
		
		float horizontalSpeedCoef = param.multiplier[0] * m_fSpeed;
		param.transform[3] = param.transform[3] + Vector(
			(currentDir[0] * newDir[2] + currentDir[2] * newDir[0]) * horizontalSpeedCoef,
			vertical * param.multiplier[1] * m_fSpeed,
			(currentDir[2] * newDir[2] - currentDir[0] * newDir[0]) * horizontalSpeedCoef,
		);
		
		// Limit position to avoid large numbers
		vector mins, maxs;
		GetGame().GetWorldEntity().GetTerrain(0, 0).GetTerrainBoundBox(mins, maxs); // bounds are in world space
		vector worldPos = CoordFromCamera(param.transform[3]); // translate to world space coordinates

#ifdef ENABLE_DIAG
		mins -= vector.One * DEBUG_DISTANCE_EXTENSION; // its useful to be able to fly a bit further than world bounds
		maxs += vector.One * DEBUG_DISTANCE_EXTENSION; // especially in case of MpTest world
#endif

		worldPos[0] = Math.Clamp(worldPos[0], mins[0], maxs[0]);
		worldPos[1] = Math.Clamp(worldPos[1], -8000.0, 8000.0);
		worldPos[2] = Math.Clamp(worldPos[2], mins[2], maxs[2]);
		param.transform[3] = CoordToCamera(worldPos); // translate back to camera local space
		
		param.isManualInput = true;
		param.isDirty = true;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
