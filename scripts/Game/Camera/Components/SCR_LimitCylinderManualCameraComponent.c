//! @ingroup ManualCamera

//! Limit camera movement within specific cylindrical area
[BaseContainerProps(), SCR_BaseManualCameraComponentTitle()]
class SCR_LimitCylinderManualCameraComponent : SCR_BaseManualCameraComponent
{
	[Attribute(defvalue: "1")]
	protected float m_fRadius;
	
	[Attribute(defvalue: "2")]
	protected float m_fHeightMinor;
	
	private float m_fRadiusSq;
	private vector m_vPos;
	
	//------------------------------------------------------------------------------------------------
	override void EOnCameraFrame(SCR_ManualCameraParam param)
	{
		if (!param.isDirty) return;
		
		m_vPos = param.transform[3];
		
		//--- Horizontal
		if (vector.DistanceSqXZ(m_vPos, vector.Zero) > m_fRadiusSq)
		{
			float angle = Math.Atan2(m_vPos[0], m_vPos[2]);
			m_vPos[0] = Math.Sin(angle) * m_fRadius;
			m_vPos[2] = Math.Cos(angle) * m_fRadius;
		}
		
		//--- Vertical
		m_vPos[1] = Math.Clamp(m_vPos[1], -m_fHeightMinor, m_fHeightMinor);
		
		param.transform[3] = m_vPos;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool EOnCameraInit()
	{
		m_fRadiusSq = m_fRadius * m_fRadius;
		return true;
	}
}
