// *************************************************************************************
// ! CharacterCamera3rdPersonFloat - 3rd person camera when floating
// *************************************************************************************
class CharacterCamera3rdPersonFloat extends CharacterCamera3rdPersonBase
{
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonFloat(CameraHandlerComponent pCameraHandler)
	{
		m_bIgnoreCharacterPitch = true;
		
		m_fDistance 		= 1.2;
		m_CameraOffsetMS	= "0.0 1.2 0.0";
		m_CameraOffsetLS	= "0.0 0.3 0.0";
		m_fShoulderWidth	= 0.4;
		m_fLeanDistance		= 0.3;
		
		//m_fDistance 		= 1.6;
		//m_CameraOffsetMS	= "0.0 0.1 -0.3";
		//m_iBoneIndex		= m_OwnerCharacter.GetAnimation().GetBoneIndex("Head");
		
		// TODO move the camera properly.
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{		
		pOutResult.m_fFOV = m_fFOV;
		
		super.OnUpdate(pDt, pOutResult);
	}
};