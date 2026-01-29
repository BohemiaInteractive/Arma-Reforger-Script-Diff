//! @ingroup ManualCamera

//! Movement above terrain surface.
//!
//! Based on limits defined by m_fATLMinHeight and m_fATLMaxHeight:
//! - When below min height, camera follows terrain precisely.
//! - When between min and max heights, camera tries to follow the terrain, but the ffect gets weaker as the height increases.
//! - When above max height, camera moves above sea level.
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_ATLManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "5", desc: "Below this height, camera copies terrain height precisely.")]	
	private int m_fATLMinHeight;
	
	[Attribute(defvalue: "50", desc: "When the camera is above this height, it moves ASL, not ATL.")]	
	private int m_fATLMaxHeight;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isDirty)
			return;
		
		float vertical = m_InputManager.GetActionValue("ManualCameraMoveVertical");
		if (vertical != 0.0)
			return;
		
		//--- Get target height
		vector pos = param.transform[3];
		float height = Math.AbsFloat(pos[1] - Math.Max(param.world.GetSurfaceY(pos[0], pos[2]), 0));

		//--- Terminate when the height is above threshold
		if (height > m_fATLMaxHeight)
			return;
		
		//--- Get original height
		vector posOrig = param.transformOriginal[3];
		float heightOrig = Math.AbsFloat(posOrig[1] - Math.Max(param.world.GetSurfaceY(posOrig[0], posOrig[2]), 0));
		
		//--- Get interpolated height between min and max
		float progress = Math.Clamp(Math.InverseLerp(m_fATLMinHeight, m_fATLMaxHeight, height), 0.0, 1.0);
		float heightInterpolated = Math.Lerp(heightOrig - height, 0, progress);
		
		//--- Apply manual input
		float inputDiff = 0;
		if (param.isManualInput)
			inputDiff = pos[1] - posOrig[1];
		
		//--- Apply transormation
		pos[1] = pos[1] + heightInterpolated + inputDiff;
		param.transform[3] = pos;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
