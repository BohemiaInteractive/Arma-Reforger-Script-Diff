// *************************************************************************************
// ! CharacterCamera3rdPersonClimb
// *************************************************************************************
class CharacterCamera3rdPersonClimb extends CharacterCamera3rdPersonErc
{
	void CharacterCamera3rdPersonClimb(CameraHandlerComponent pCameraHandler)
	{
		m_fDistance 		= 1.6;
		m_CameraOffsetMS	= "0.0 0.1 -0.3";
		m_iBoneIndex		= m_OwnerCharacter.GetAnimation().GetBoneIndex("Spine1");
	}

	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		m_fYoffsetPrevFrame = 0;
		super.OnUpdate(pDt, pOutResult);
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
};