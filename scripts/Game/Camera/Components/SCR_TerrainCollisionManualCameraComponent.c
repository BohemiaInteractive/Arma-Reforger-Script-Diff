//! @ingroup ManualCamera

//! Camera collides with terrain
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_TerrainCollisionManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "0.2")]
	private float m_fMinHeight;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		vector pos = CoordFromCamera(param.transform[3]);
		pos[1] = Math.Max(pos[1], param.world.GetSurfaceY(pos[0], pos[2]) + m_fMinHeight);
		param.transform[3] = CoordToCamera(pos);
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		return true;
	}
}
