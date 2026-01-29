enum SCR_EAICombatMoveRequestState
{
	IDLE,		// There was never a request
	EXECUTING,	// Executing a request
	SUCCESS,	// Last request succeeded
	FAIL,		// Last request failed, see fail reason
	CANCELED	// It was aborted while running
}

enum SCR_EAICombatMoveRequestFailReason
{
	NONE,
	UNDEFINED,
	COVER_NOT_FOUND,
	MOVEMENT_FAILED,
}

class SCR_AICombatMoveState
{
	// Check VerifyCurrentCover
	protected static const float DISTANCE_IN_COVER_MAX_SQ = 1.0;
	
	protected ref SCR_AICoverLock m_CoverLock;		// Cover assigned to us. But it doesn't mean we are at the cover position (yet).
	
	protected ref SCR_AICombatMoveRequestBase m_Request;
	
	bool m_bInCover;								// True if we are in cover
	bool m_bExposedInCover;							// If in cover, false means we are hidden behind cover completely, true means we are exposed and can shoot
	bool m_bAimAtTarget = true;						// True if we are allowed to aim at target due to combat movement constraints. Enabled at start!
	
	// Timers
	float m_fTimerRequest_s;						// Time since last request
	float m_fTimerInCover_s;						// Counts up when we are in cover
	float m_fTimerStopped_s;						// Counts up when we are not moving, no matter if in cover or not
	
	
	//----------------------------------------------------------------
	void AssignCover(SCR_AICoverLock newCover)
	{
		if (m_CoverLock)
			m_CoverLock.Release();
		
		m_CoverLock = newCover;
	}
	
	//----------------------------------------------------------------
	void ReleaseCover()
	{
		// m_CoverLock might be still referenced by something else,
		// Thus if we only unref it, it might still be not destroyed.
		// That's why we also release it manually here.
		
		if (m_CoverLock)
			m_CoverLock.Release();
		m_CoverLock = null;
		m_bInCover = false;
	}
	
	//----------------------------------------------------------------
	void ApplyNewRequest(notnull SCR_AICombatMoveRequestBase request)
	{
		// mark old request as cancelled if it was running, in case something else needs to know its state
		if (m_Request && m_Request.m_eState == SCR_EAICombatMoveRequestState.EXECUTING)
			m_Request.m_eState = SCR_EAICombatMoveRequestState.CANCELED;
		
		m_Request = request; // Old request is unreferenced here
	}
	
	//----------------------------------------------------------------
	//! Unreferences current request and sets it as 'canceled' if it was running
	//! !!! The actual BT which handles requests won't abort the current request, it's not done yet
	void CancelRequest()
	{
		if (m_Request && m_Request.m_eState == SCR_EAICombatMoveRequestState.EXECUTING)
			m_Request.m_eState = SCR_EAICombatMoveRequestState.CANCELED;
		
		m_Request = null;
	}
	
	//----------------------------------------------------------------
	void EnableAiming(bool enable)
	{
		m_bAimAtTarget = enable;
	}
	
	//----------------------------------------------------------------
	//! Verifies our distance to current cover and releases cover if we are too far from it
	//! This should be called periodically
	void VerifyCurrentCover(vector myPos)
	{
		if (m_CoverLock && m_bInCover)
		{
			if (vector.DistanceSq(myPos, m_CoverLock.GetPosition()) > DISTANCE_IN_COVER_MAX_SQ)
			{
				ReleaseCover();
			}
		}
	}
	
	
	//-------------------------------------------------------------------------------
	// Getters
	
	bool IsMoving()
	{
		// Moving when executing a MOVE request
		return SCR_AICombatMoveRequest_Move.Cast(m_Request) && m_Request.m_eState == SCR_EAICombatMoveRequestState.EXECUTING;
	}
	
	bool IsMoving(SCR_EAICombatMoveReason eReason)
	{
		SCR_AICombatMoveRequest_Move rqMove = SCR_AICombatMoveRequest_Move.Cast(m_Request);
		return rqMove && rqMove.m_eState == SCR_EAICombatMoveRequestState.EXECUTING && rqMove.m_eReason == eReason;
	}
	
	bool IsMovingToCover()
	{
		SCR_AICombatMoveRequest_Move moveRequest = SCR_AICombatMoveRequest_Move.Cast(m_Request);
		return moveRequest && moveRequest.m_eState == SCR_EAICombatMoveRequestState.EXECUTING && m_CoverLock;
	}
	
	bool IsInValidCover()
	{
		return m_bInCover && m_CoverLock && m_CoverLock.IsValid();
	}
	
	bool IsHidingInValidCover()
	{
		return m_bInCover && !m_bExposedInCover && m_CoverLock && m_CoverLock.IsValid();
	}
	
	bool IsExecutingRequest()
	{
		return m_Request && m_Request.m_eState == SCR_EAICombatMoveRequestState.EXECUTING;
	}
	
	SCR_AICombatMoveRequestBase GetRequest()
	{
		return m_Request;
	}
	
	SCR_AICoverLock GetAssignedCover()
	{
		return m_CoverLock;
	}
	
	bool IsAssignedCoverValid()
	{
		return m_CoverLock && m_CoverLock.IsValid();
	}
	
	
	
	//-------------------------------------------------------------------------------
	// Helper methods for applying trivial requests
	
	//----------------------------------------------------------------
	void ApplyRequestChangeStanceOutsideCover(ECharacterStance stance)
	{
		SCR_AICombatMoveRequest_ChangeStance rq = new SCR_AICombatMoveRequest_ChangeStance();
		
		rq.m_eStance = stance;
		rq.m_bAimAtTarget = true;
		rq.m_bAimAtTargetEnd = true;
		
		ApplyNewRequest(rq);
	}
	
	//----------------------------------------------------------------
	void ApplyRequestChangeStanceInCover(bool exposed)
	{
		SCR_AICombatMoveRequest_ChangeStanceInCover rq = new SCR_AICombatMoveRequest_ChangeStanceInCover();
		
		rq.m_bExposedInCover = exposed;
		rq.m_bAimAtTarget = true;
		rq.m_bAimAtTargetEnd = true;
		
		ApplyNewRequest(rq);
	}
}