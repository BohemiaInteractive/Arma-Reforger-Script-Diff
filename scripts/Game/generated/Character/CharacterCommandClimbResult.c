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
result from static test
*/
class CharacterCommandClimbResult
{
	bool		m_bIsClimb;
	bool		m_bIsClimbOver;
	bool		m_bFinishWithFall;

	float		m_fClimbSpaceHeight;	//! How much space is between the grab point and an object above it.

	vector 		m_ClimbGrabPointLS;		//! grab point for climb && climb over (in local space of it's parent)
	vector 		m_ClimbGrabPointNormalLS;	//! normal to grabpoint position used for character orientation (in local space of it's parent)
	vector 		m_ClimbOverStandPointLS;	//! where climb over ends (in local space of it's parent)

	vector 		mClimbDirectionLS;	//! Climbing direction (in local space of grap-point's parent)

	IEntity		m_ClimbParent;	//! Entity that we are climbing on.
	int				m_LinkingObjectIndex; //! Internal variable used for character linking to climbable entity.


}

/*!
\}
*/
