class SCR_BlastedEntityEntry
{
	protected IEntity m_TargetEntity;
	protected vector m_vHitPosDirNorm[3];
	protected float m_fAngleCos;
	protected float m_fDistance;
	protected int m_iNodeId;
	protected int m_iColliderId;
	protected SurfaceProperties m_SurfaceProps;

	//------------------------------------------------------------------------------------------------
	//! \return
	IEntity GetTargetEntity()
	{
		return m_TargetEntity;
	}
	//------------------------------------------------------------------------------------------------
	//! \param[out] hitPosDirNorm
	void GetTargetHitPosDirNorm(out vector hitPosDirNorm[3])
	{
		hitPosDirNorm = m_vHitPosDirNorm;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetAngleToTarget()
	{
		return m_fAngleCos;
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] newangleCos dot product of the blast direction and the diretion from blast origin to the hit position
	void SetAngleToTarget(float newangleCos)
	{
		m_fAngleCos = newangleCos;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	float GetDistanceToTarget()
	{
		return m_fDistance;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetTargetNodeId()
	{
		return m_iNodeId;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	int GetTargetColliderId()
	{
		return m_iColliderId;
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	SurfaceProperties GetTargetSurfaceProps()
	{
		return m_SurfaceProps;
	}

	//------------------------------------------------------------------------------------------------F
	// constructor
	//! \param[in] ent subject of the entry
	//! \param[in] hitPos
	//! \param[in] hitDir
	//! \param[in] hitNorm normal of the hit surface
	//! \param[in] angleCos dot product of the blast direction and the diretion from blast origin to the hit position
	//! \param[in] distance from the blast origin
	//! \param[in] nodeId
	//! \param[in] colliderId
	//! \param[in] surfaceProp
	void SCR_BlastedEntityEntry(notnull IEntity ent, vector hitPos, vector hitDir, vector hitNorm, float angleCos, float distance, int nodeId, int colliderId, SurfaceProperties surfaceProp)
	{
		m_TargetEntity = ent;
		m_vHitPosDirNorm[0] = hitPos;
		m_vHitPosDirNorm[1] = hitDir;
		m_vHitPosDirNorm[2] = hitNorm;
		m_fAngleCos = angleCos;
		m_fDistance = distance;
		m_iNodeId = nodeId;
		m_iColliderId = colliderId;
		m_SurfaceProps = surfaceProp;
	}
}
