// *************************************************************************************
// ! CharacterCamera3rdPersonCrouch - 3rd person camera in erected stance
// *************************************************************************************
class CharacterCamera3rdPersonCrouch extends CharacterCamera3rdPersonBase
{
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonCrouch(CameraHandlerComponent pCameraHandler)
	{
		m_fDistance 		= 1.2;
		m_CameraOffsetMS	= "0.0 0.7 0.0";
		m_CameraOffsetLS	= "0.0 0.3 0.0";
		m_fShoulderWidth	= 0.4;
		
		CharacterCommandHandlerComponent cmdHandler = CharacterCommandHandlerComponent.Cast(m_CharacterAnimationComponent.FindComponent(CharacterCommandHandlerComponent));
		m_CommandMove = cmdHandler.GetCommandMove();
		return;
	}
	
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{		
		//! movement height 
		float movement = m_ControllerComponent.GetMovementSpeed();
		movement = (Math.Clamp(movement, 1, 3) - 1.0) * 0.5;
		movement *= CONST_CAMERAMOVEMENTHEIGH;
		
		const float stanceCameraHeight = 0.0; // TODO: check for good const usage
		
		m_fCameraHeight	= Math.SmoothCD(m_fCameraHeight, Math.Max(movement, stanceCameraHeight), m_fCameraHeightVel, 0.2, 1000, pDt);
		m_CameraOffsetMS[1] = 0.7 + m_fCameraHeight;
	
		//! FOV
		pOutResult.m_fFOV = m_fFOV;
					
		bool sprinting = m_ControllerComponent.IsSprinting();
		if( sprinting )
			pOutResult.m_fFOV = m_fFOV + 5 * m_fBobScale;

		super.OnUpdate(pDt, pOutResult);
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}

	//-----------------------------------------------------------------------------
	static const float 	CONST_CAMERAMOVEMENTHEIGH = 0.1;
	static const float 	CONST_CAMERA_STANCE_ADJ_BY_MOVEMENT = 0.1;

	protected CharacterCommandMove m_CommandMove;
	
	protected float m_fCameraHeight, m_fCameraHeightVel;
};