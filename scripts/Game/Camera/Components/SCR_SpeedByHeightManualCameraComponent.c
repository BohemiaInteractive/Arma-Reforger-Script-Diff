//#define CURVE_APPLIED

//! @ingroup ManualCamera

//! Camera speed based on height from terrain
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_SpeedByHeightManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute("0.06")]
	float m_fSpeedCoef;
	
	[Attribute(defvalue: "0.15")]
	float m_fMinSpeed;
	
	[Attribute(defvalue: "400")]
	float m_fMaxHeight;
	
	[Attribute("50.0")]
	float m_fUnderwaterSpeedHeight; // When underwater, the camera will move at the speed it would if it was at this height
	
	[Attribute("0 0 1 1", UIWidgets.GraphDialog, params: "1 1 0 0")]
	private ref Curve m_fHeightCoef;

	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		vector pos = CoordFromCamera(param.transform[3]);
		float surfaceY = param.world.GetSurfaceY(pos[0], pos[2]);

#ifdef CURVE_APPLIED
		float height = LegacyCurve.Curve(ECurveType.CurveProperty2D, (pos[1] - surfaceY) / m_fMaxHeight, m_fHeightCoef)[1];
		param.multiplier *= Math.Max(height * m_fMaxHeight * m_fSpeedCoef, m_fMinSpeed);			
#else
		float height = Math.AbsFloat(pos[1] - surfaceY);
		if (pos[1] < 0) // When below water level maintain a constant speed no matter the depth
			height = m_fUnderwaterSpeedHeight;
		param.multiplier *= Math.Max(height * m_fSpeedCoef, m_fMinSpeed);
#endif
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
