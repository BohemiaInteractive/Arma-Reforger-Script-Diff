// *************************************************************************************
// ! CharacterCamera1stPersonReady - 1st person camera when character is in ready stance
// *************************************************************************************
class CharacterCamera1stPersonReady extends CharacterCamera1stPerson
{
	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonReady(CameraHandlerComponent pCameraHandler)
	{
		// TODO: Offset from current optic position rather than hardcoded offset
		// It should be the other eye, so it should be on the same level as the current optic eyepiece.
		m_OffsetLS = "0 0.08 -0.07"; 
	}
};