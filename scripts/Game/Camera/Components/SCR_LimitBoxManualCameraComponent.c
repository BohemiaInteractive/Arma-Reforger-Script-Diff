//! @ingroup ManualCamera

//! Limit camera movement within specific box area
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_LimitBoxManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "-2000 -2000 -2000")]
	private vector m_vBoundsMin;
	
	[Attribute(defvalue: "2000 2000 2000")]
	private vector m_vBoundsMax;
	
	[Attribute(defvalue: "1", desc: "When true, boundaries are calculated from world boundaries, not from 0,0,0")]
	private bool m_RelativeToWorld;
	
	private vector m_vPos;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isDirty) return;
		
		m_vPos = param.transform[3];
		m_vPos[0] = Math.Clamp(m_vPos[0], m_vBoundsMin[0], m_vBoundsMax[0]);
		m_vPos[1] = Math.Clamp(m_vPos[1], m_vBoundsMin[1], m_vBoundsMax[1]);
		m_vPos[2] = Math.Clamp(m_vPos[2], m_vBoundsMin[2], m_vBoundsMax[2]);
		param.transform[3] = m_vPos;
	}

	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		if (!m_RelativeToWorld) return true;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return false;
		
		BaseWorld world = game.GetWorld();
		if (!world)
			return false;
		
		vector worldBoundBoxMin, worldBoundBoxMax;
		world.GetBoundBox(worldBoundBoxMin, worldBoundBoxMax);
		
		m_vBoundsMin += worldBoundBoxMin;
		m_vBoundsMax += worldBoundBoxMax;
		
		return true;
	}
}
