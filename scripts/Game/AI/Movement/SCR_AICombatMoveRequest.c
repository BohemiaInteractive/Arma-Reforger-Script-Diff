enum SCR_EAICombatMoveRequestType
{
	MOVE,
	STOP,
	CHANGE_STANCE_IN_COVER,
	CHANGE_STANCE
}

// Subtypes which can represent reason for this request
enum SCR_EAICombatMoveReason
{
	STANDARD,				// Moving somewhere during combat
	FF_AVOIDANCE,			// Friendly Fire avoidance
	MOVE_FROM_TARGET,		// Moving backwards from target
	MOVE_FROM_DANGER,		// Danger avoidance
	SUPPRESSED_IN_COVER,
	CHARACTER_AVOIDANCE		// Avoiding collision with a friendly character
}

// For which unit type this move request is meant
enum SCR_EAICombatMoveUnitType
{
	CHARACTER,			// Character on foot
	GROUND_VEHICLE		// Ground vehicle
}

enum SCR_EAICombatMoveDirection
{
	FORWARD,	// Towards the target
	BACKWARD,	// Away from target
	RIGHT,
	LEFT,
	ANYWHERE,	// Without specific direciton, find any position close to current one
	CUSTOM_POS	// Moving directly to the provided position
}


//------------------------------------------------------------------------------------------------
typedef func SCR_AICombatMoveRequest_OnCompleted;
void SCR_AICombatMoveRequest_OnCompleted(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequestBase request);

typedef func SCR_AICombatMoveRequest_OnFailed;
void SCR_AICombatMoveRequest_OnFailed(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequestBase request, SCR_EAICombatMoveRequestFailReason failReason);

class SCR_AICombatMoveRequestBase
{
	SCR_EAICombatMoveRequestType m_eType;	// Type int is needed for decoding in behavior tree
	SCR_EAICombatMoveRequestState m_eState = SCR_EAICombatMoveRequestState.IDLE;
	SCR_EAICombatMoveRequestFailReason m_eFailReason = SCR_EAICombatMoveRequestFailReason.NONE;
	SCR_EAICombatMoveReason m_eReason;		// "Reason" of request - abstract data to help identify our request in combat move logic
	SCR_EAICombatMoveUnitType m_eUnitType;
	
	// It can be used for various timings, up to sender of the request to decide what it means
	float m_f_UserTimer_s;
	
	// Allowing or restricting aiming is common for all request types
	bool m_bAimAtTarget;					// Aim at target while executing the request?
	bool m_bAimAtTargetEnd;					// Aim at target after request is executed?
	
	// Events
	protected ref ScriptInvokerBase<SCR_AICombatMoveRequest_OnCompleted> m_OnCompleted;
	protected ref ScriptInvokerBase<SCR_AICombatMoveRequest_OnFailed> m_OnFailed;
	
	//! Called on successful completion of request
	ScriptInvokerBase<SCR_AICombatMoveRequest_OnCompleted> GetOnCompleted(bool createInvoker = true)
	{
		if (!m_OnCompleted && createInvoker)
			m_OnCompleted = new ScriptInvokerBase<SCR_AICombatMoveRequest_OnCompleted>();
		return m_OnCompleted;
	}
	
	//! Called on failure of request
	ScriptInvokerBase<SCR_AICombatMoveRequest_OnFailed> GetOnFailed(bool createInvoker = true)
	{
		if (!m_OnFailed && createInvoker)
			m_OnFailed = new ScriptInvokerBase<SCR_AICombatMoveRequest_OnFailed>();
		return m_OnFailed;
	}
}


//------------------------------------------------------------------------------------------------
// Movement with potential cover search
typedef func SCR_AICombatMoveRequest_Move_MovementEvent;
void SCR_AICombatMoveRequest_Move_MovementEvent(SCR_AIUtilityComponent utility, SCR_AICombatMoveRequest_Move request, vector movePos, bool destinationIsCover);

class SCR_AICombatMoveRequest_Move : SCR_AICombatMoveRequestBase
{
	vector m_vTargetPos;					// Position of target (enemy) - must be useable for cover search, should be muzzle position or head position.
	vector m_vMovePos;						// Move position relative to which m_eDirection works
	bool m_bTryFindCover;					// Try to find cover or not?
	bool m_bFailIfNoCover;					// If m_bTryFindCover is true, but cover was not found, fail or move anyway?
	bool m_bCheckCoverVisibility;			// (only m_bTryFindCover) Do we need to check target pos. visibility from that cover or not?
	ECharacterStance m_eStanceMoving;		// Stance used for movement
	ECharacterStance m_eStanceEnd;			// Stance used at the end (might be overridden by found cover)
	EMovementType m_eMovementType;			//
	float m_fCoverSearchDistMin;			// (only m_bTryFindCover)
	float m_fCoverSearchDistMax;			// (only m_bTryFindCover)
	float m_fCoverSearchSectorHalfAngleRad = Math.PI;	// (only m_bTryFindCover) Half-angle (in radians) of cover query sector. Pi is full circle, Pi/2 is -90deg...+90deg sector.
	float m_fMoveDuration_s;				// Movement duration for movement to non-cover position
	bool m_bUseCoverSearchDirectivity;		// If trying to find cover, prefer covers in given direction or not. Affects cover scoring.
	SCR_EAICombatMoveDirection m_eDirection;// Direction - Where we want to move !!! relative to m_vMovePos
	vector m_vAvoidStraightPathDir;			// When not 0,0,0, changes pathfinding to try to flank and avoid the path aligned same direction. Makes sense mostly for m_eDirection = CUSTOM_POS.
	
	// Events
	protected ref ScriptInvokerBase<SCR_AICombatMoveRequest_Move_MovementEvent> m_OnMovementStarted;
	
	void SCR_AICombatMoveRequest_Move()
	{
		m_eType = SCR_EAICombatMoveRequestType.MOVE;
	}
	
	// Called right before move node starts
	ScriptInvokerBase<SCR_AICombatMoveRequest_Move_MovementEvent> GetOnMovementStarted(bool createInvoker = true)
	{
		if (!m_OnMovementStarted && createInvoker)
			m_OnMovementStarted = new ScriptInvokerBase<SCR_AICombatMoveRequest_Move_MovementEvent>();
		return m_OnMovementStarted;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_AICombatMoveRequest_Stop : SCR_AICombatMoveRequestBase
{
	void SCR_AICombatMoveRequest_Stop()
	{
		m_eType = SCR_EAICombatMoveRequestType.STOP;
	}
}

//------------------------------------------------------------------------------------------------
// Changes stance when we are in cover
class SCR_AICombatMoveRequest_ChangeStanceInCover : SCR_AICombatMoveRequestBase
{
	bool m_bExposedInCover;
	
	void SCR_AICombatMoveRequest_ChangeStanceInCover()
	{
		m_eType = SCR_EAICombatMoveRequestType.CHANGE_STANCE_IN_COVER;
	}
}

//------------------------------------------------------------------------------------------------
class SCR_AICombatMoveRequest_ChangeStance : SCR_AICombatMoveRequestBase
{
	ECharacterStance m_eStance;
	
	void SCR_AICombatMoveRequest_ChangeStance()
	{
		m_eType = SCR_EAICombatMoveRequestType.CHANGE_STANCE;
	}
}