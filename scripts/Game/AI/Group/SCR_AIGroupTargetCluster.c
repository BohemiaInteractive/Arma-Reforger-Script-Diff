class SCR_AIPolar
{
	// Limits angle within -PI..PI range
	static float WrapAngleDiff(float angle)
	{
		while (angle < -Math.PI)
        	angle += Math.PI2;
	    while (angle > Math.PI)
	        angle -= Math.PI2;
	    return angle;
	}
	
	// Limits angle within 0..2PI range
	static float WrapAngle(float angle)
	{
		while (angle < 0)
        	angle += Math.PI2;
	    while (angle > Math.PI2)
	        angle -= Math.PI2;
	    return angle;
	}
	
	// Converts a direction vector to angle
	static float DirToAngle(vector dir)
	{
		float a = Math.Atan2(dir[0], dir[2]);
		return WrapAngle(a);
	}
}

class SCR_AITargetInfoPolar : Managed
{
	SCR_AITargetInfo m_Target;
	
	[SortAttribute()] // Sorting is used for clustering
	float m_fAngle;
	
	float m_fDistance;
}

class SCR_AIGroupTargetCluster : Managed
{	
	ref array<SCR_AITargetInfo> m_aTargets = {};	// Those arrays are parallel
	ref array<float> m_aAngles = {};				// Angle to that target
	ref array<float> m_aDistances = {};				// Distance to that target
	ref array<IEntity> m_aEntities = {};			// Entities of the target
	
	ref SCR_AITargetClusterState m_State;
	
	float m_fAngleMin;
	float m_fAngleMax;
	
	//---------------------------------------------------------------------------
	void AddTarget(SCR_AITargetInfo target, float angle, float dist)
	{
		m_aTargets.Insert(target);
		m_aAngles.Insert(angle);
		m_aDistances.Insert(dist);
		m_aEntities.Insert(target.m_Entity);
	}
	
	//---------------------------------------------------------------------------
	// Adds targets from other cluster to this cluster. Adjusts min/max angles limits.
	// ! Doesn't modify orders
	void AddCluster(SCR_AIGroupTargetCluster other)
	{
		// Add other points and angles
        foreach (float angle : other.m_aAngles)
            m_aAngles.Insert(angle);
        foreach (float distance : other.m_aDistances)
            m_aDistances.Insert(distance);
        foreach (SCR_AITargetInfo target : other.m_aTargets)
			m_aTargets.Insert(target);
		foreach (IEntity entity : other.m_aEntities)
			m_aEntities.Insert(entity);
		
        // Resolve the new min-max  angle values
        float span0 = SCR_AIPolar.WrapAngleDiff(other.m_fAngleMax - m_fAngleMin);
        float span1 = SCR_AIPolar.WrapAngleDiff(other.m_fAngleMin - m_fAngleMax);
        
        if (span0 == span1)
            if (span0 > 0)
                m_fAngleMax = other.m_fAngleMax;
            else
                m_fAngleMin = other.m_fAngleMin;
        else if (Math.AbsFloat(span0) > Math.AbsFloat(span1))
            m_fAngleMax = other.m_fAngleMax;
        else
            m_fAngleMin = other.m_fAngleMin;
	}
	
	//---------------------------------------------------------------------------
	// Moves state from other cluster to this one
	// The other cluster's state is reset
	void MoveStateFrom(SCR_AIGroupTargetCluster other)
	{
		m_State = other.m_State;
		m_State.m_Cluster = this;
		other.m_State = null;
	}
	
	//---------------------------------------------------------------------------
	// Returns distance to closest target
	float GetMinDistance()
	{
		if (m_aDistances.IsEmpty())
			return -1;
		
		float minDist = float.MAX;
		foreach (float dist : m_aDistances)
		{
			if (dist < minDist)
				minDist = dist;
		}
		
		return minDist;
	}
}