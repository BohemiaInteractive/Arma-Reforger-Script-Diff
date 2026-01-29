// *************************************************************************************
// ! CharacterCamera1stPersonFloat - 1st person camera when floating
// *************************************************************************************
class CharacterCamera1stPersonFloat extends CharacterCamera1stPersonReady
{
	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonFloat(CameraHandlerComponent pCameraHandler)
	{
		m_bIgnoreCharacterPitch = true;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{		
		pOutResult.m_fFOV = m_fFOV;
		
		super.OnUpdate(pDt, pOutResult);
	}
};