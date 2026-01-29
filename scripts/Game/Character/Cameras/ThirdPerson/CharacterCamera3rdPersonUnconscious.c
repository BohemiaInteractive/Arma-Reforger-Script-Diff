// *************************************************************************************
// ! CharacterCamera3rdPersonUnconscious - 3rd person camera in unconscious state
// *************************************************************************************
class CharacterCamera3rdPersonUnconscious extends CharacterCamera3rdPersonBase
{
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonUnconscious(CameraHandlerComponent pCameraHandler)
	{
		m_iBoneIndex		= m_OwnerCharacter.GetAnimation().GetBoneIndex("Neck1");
		m_CameraOffsetLS	= "0.0 0.0 0.0";
		m_CameraOffsetMS	= "0.0 0.0 0.0";
		m_fDistance 		= 1.6;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);

		// follow character origin when ragdolling
		if(m_OwnerCharacter.GetAnimationComponent().IsRagdollActive())
		{
			pOutResult.m_CameraTM[3] = Vector(0, 0, 0);
		}
	}
};