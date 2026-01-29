#ifdef WORKBENCH
//#define MOVEMENT_DETECTOR_DIAG
#endif

//! This class is a state machine which estimates if entity is currently moving or is stopped.
//! Main goal is to determine if player, as infantry group leader, intends to move, or intends to stay around same place.
//! But it is used for AI squad leaders same way.
class SCR_AIMovementDetector
{
	protected vector m_vPrevPos;
	protected vector m_vPosStopped;
	protected IEntity m_OwnerEntity;
	protected WorldTimestamp m_TimeLastStopped;
	protected WorldTimestamp m_TimeLastMoved;
	protected WorldTimestamp m_TimeLastUpdate;
	protected bool m_bMoving;
	
	#ifdef MOVEMENT_DETECTOR_DIAG
	protected ref Shape m_ShapeLeaderPos;
	protected ref Shape m_ShapePosStopped;
	#endif
	
	// Constants
	protected static const float STOPPED_RADIUS = 6;
	protected static const float STOPPED_RADIUS_SQ = STOPPED_RADIUS * STOPPED_RADIUS;
	
	protected static const float TIME_STOPPED_S = 0.7; // Switch to stopped state after not moving for this time
	protected static const float TIME_MOVING_S = 0.17; // Switch to moving state after moving for this time
	
	protected static const float UPDATE_INTERVAL_S = 0.2; // How often to update state
	
	//-------------------------------------------------------------------------------------------------
	void SCR_AIMovementDetector(IEntity myEntity)
	{
		m_OwnerEntity = myEntity;
	}
	
	//-------------------------------------------------------------------------------------------------
	protected void Update(WorldTimestamp worldTime)
	{
		vector pos = m_OwnerEntity.GetOrigin();
		bool movedSinceLastUpdate = vector.DistanceSq(pos, m_vPrevPos) > 0.01; // Even when standing, character jitters a bit
		m_vPrevPos = pos;
		
		if (m_bMoving)
		{
			if (movedSinceLastUpdate)
				m_TimeLastMoved = worldTime;
			else
			{
				if (worldTime.DiffSeconds(m_TimeLastMoved) > TIME_STOPPED_S)
				{
					m_TimeLastStopped = worldTime;
					m_bMoving = false;
					m_vPosStopped = pos;
				}
			}
		}
		else
		{
			if (movedSinceLastUpdate)
			{
				if (worldTime.DiffSeconds(m_TimeLastStopped) > TIME_MOVING_S)
				{
					if (vector.DistanceSq(pos, m_vPosStopped) > STOPPED_RADIUS_SQ)
					{
						m_TimeLastMoved = worldTime;
						m_bMoving = true;
					}
				}
			}
			else
			{
				m_TimeLastStopped = worldTime;
				m_vPosStopped = pos;
			}
		}	
		
		#ifdef MOVEMENT_DETECTOR_DIAG
		int color = Color.RED;
		if (m_bMoving)
			color = Color.GREEN;
		m_ShapeLeaderPos = Shape.CreateCylinder(color, ShapeFlags.DEFAULT, pos, 0.5, 1.0);
		m_ShapePosStopped = Shape.CreateCylinder(Color.VIOLET, ShapeFlags.DEFAULT, m_vPosStopped, STOPPED_RADIUS, 0.3);
		#endif
	}
	
	//-------------------------------------------------------------------------------------------------
	//! Returns current state, and updates it in lazy manner, not more often than once per UPDATE_INTERVAL_S
	bool GetMoving()
	{
		WorldTimestamp worldTime = GetGame().GetWorld().GetTimestamp();
		if (worldTime.DiffSeconds(m_TimeLastUpdate) > UPDATE_INTERVAL_S)
		{
			Update(worldTime);
			m_TimeLastUpdate = worldTime;
		}
		return m_bMoving;
	}
}
