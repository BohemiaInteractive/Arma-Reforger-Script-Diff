// *************************************************************************************
// ! CharacterCamera3rdPersonErc - 3rd person camera in erected stance
// *************************************************************************************
class CharacterCamera3rdPersonErc extends CharacterCamera3rdPersonBase
{
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonErc(CameraHandlerComponent pCameraHandler)
	{
		m_fDistance 		= 1.2;
		m_CameraOffsetMS	= "0.0 1.2 0.0";
		m_CameraOffsetLS	= "0.0 0.3 0.0";
		m_fShoulderWidth	= 0.4;
		m_fLeanDistance		= 0.3;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{		
		pOutResult.m_fFOV = m_fFOV;
		
		bool sprinting = m_ControllerComponent.IsSprinting();
		if( sprinting )
			pOutResult.m_fFOV = m_fFOV + 5 * m_fBobScale;
		
		super.OnUpdate(pDt, pOutResult);
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
};