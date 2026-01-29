/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Character
\{
*/

/*!
swim settings
*/
class CharacterCommandSwimSettings
{
	float		m_fAlignIdleTimeout;			//!< filter span value		[s]
	float		m_fAlignIdleMaxChanged;			//!< filter speed value		[val/s]
	float		m_fAlignSlowTimeout;			//!< filter span value		[s]
	float		m_fAlignSlowMaxChanged;			//!< filter speed value		[val/s]
	float		m_fAlignFastTimeout;			//!< filter span value		[s]
	float		m_fAlignFastMaxChanged;			//!< filter speed value		[val/s]

	float		m_fMovementSpeed;
	float		m_fMovementSpeedFltTime;
	float		m_fMovementSpeedFltMaxChange;

	float		m_fWaterLevelSwim;					//!<
	float		m_fWaterLevelIn;					//!<
	float		m_fWaterLevelOut;					//!<

	float		m_fToCrouchLevel;					//!<
	float		m_fToErectLevel;					//!<

	float		m_fExpectedSubmersion;		//!< submersion of the character while swimming
	float		m_fDrag;					//!< drag while underwater and going down, or on surface and going up

	float		m_fUnconsciousStartingBuoyancy;
	float		m_fUnconsciousFinalBuoyancy;
	float		m_fUnconsciousBuoyancyChangeTime;

}

/*!
\}
*/
