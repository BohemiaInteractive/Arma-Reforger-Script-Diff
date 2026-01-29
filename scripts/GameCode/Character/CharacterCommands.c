// *************************************************************************************
// ! Character command IDs
// *************************************************************************************
enum ECharacterCommandIDs
{
	MOVE,
	FALL,
	DEATH,
	VEHICLE,
	CLIMB,
	LADDER,
	MELEE,
	SWIM,
	DAMAGE_FB,
	
	MOD_LOOKAT,
	MOD_WEAPONS,
	MOD_ITEM,
	MOD_DAMAGE
};

// *************************************************************************************
// ! CharacterCommandMove - main - fullbody
// *************************************************************************************
class CharacterCommandMoveSettings 
{
	float m_fRunSpringTimeout;				//!< filter span value		[s]
	float m_fRunSpringMaxChange;			//!< filter speed value		[val/s]
	float m_fDirFilterTimeout;				//!< direction filter timeout [s]
	float m_fDirFilterSpeed;				//!< direction filter max rot speed [rad/s]
	float m_fMaxSprintAngle;				//!< max sprint angle [rad]

	float m_fTurnAngle;						//!< turn angle
	float m_fTurnEndUpdateNTime;			//!< 
	float m_fTurnNextNTime;					//!< next turn normalized time 

	float m_fSlidingPoseAngle;
	float m_fSlidingPoseTrackTime;
	float m_fSlidingPoseRepTime;

	float m_fHeadingChangeLimiterIdle;
	float m_fHeadingChangeLimiterWalk;
	float m_fHeadingChangeLimiterRun;

	float m_fLeaningSpeed;
	
	float m_fAdjustedStanceChangeSpeed;
	float m_fAdjustedStanceTraceLength;		//!< distance to trace in front of the character
	float m_fAdjustedStanceUpperThreshold;
	float m_fAdjustedStanceLowerThreshold;
	
	float m_fMaxEnterLadderCosFi;			//!< max cosinus value of an angle for entering a ladder
};

//! state of climb command
enum ClimbStates
{
	STATE_MOVE,
	STATE_TAKEOFF,
	STATE_ONTOP,
	STATE_FALLING,
	STATE_FINISH
};