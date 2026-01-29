// *************************************************************************************
// ! CharacterCamera3rdPersonProne - 3rd person camera in prone stance
// *************************************************************************************
class CharacterCamera3rdPersonProne extends CharacterCamera3rdPersonBase
{
	static private const float FREELOOK_FADE_SPEED = 4.0;
	static private const float SLOPE_FACTOR = 1.0 / 90.0;
	
	private float m_fFreelookBlend;
	
	//------------------------------------------------------------------------------------------------
	void CharacterCamera3rdPersonProne(CameraHandlerComponent pCameraHandler)
	{
		m_fDistance 		= 1.4;
		m_CameraOffsetMS	= "0.0 0.3 0.0";
		m_CameraOffsetLS	= "0.0 0.2 0.0";
		m_fShoulderWidth	= 0.4;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		pOutResult.m_fFOV = m_fFOV;
		bool sprinting = m_ControllerComponent.IsSprinting();
		if( sprinting )
			pOutResult.m_fFOV = m_fFOV + 2 * m_fBobScale;
		
		HandleFreelook(pDt, pOutResult);

		super.OnUpdate(pDt, pOutResult);
	
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//------------------------------------------------------------------------------------------------
	void HandleFreelook(float pDt, inout ScriptedCameraItemResult pOutResult)
	{
		vector charaRot = m_OwnerCharacter.GetLocalYawPitchRoll();

		float tSlope = Math.Clamp(charaRot[1] * SLOPE_FACTOR, -1.0, 1.0); // [-1, 1]
		float tSlopeUp = Math.Clamp(tSlope,  0.0, 1.0); // [0, 1]
		float tSlopeDn = Math.Clamp(tSlope, -1.0, 0.0) * -1.0; // [0, 1]
		
		if (m_ControllerComponent.IsFreeLookEnabled())
		{
			// Override angle limits based on terrain slope
			float downLimit = Math.Lerp(-60.0, 0.0, tSlopeDn);
			float upLimit = Math.Lerp(89.9, 40.0, tSlope);
			float leftLimit = Math.Lerp(-160.0, -40.0, tSlope);
			float rightLimit = Math.Lerp(160.0, 80.0, tSlope);
			m_CharacterHeadAimingComponent.SetLimitAnglesOverride(downLimit, upLimit, leftLimit, rightLimit);
		}
		else
		{
			m_CharacterHeadAimingComponent.ResetLimitAnglesOverride();
		}

		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();

		// Limit yaw on slopes.
		float yawMax = Math.Lerp(160.0, 40.0, Math.Max(0.0, tSlope));
		lookAngles[0] = Math.Clamp(lookAngles[0], -yawMax, yawMax); // Clamp backtrace direction's Yaw
		float tYaw = Math.InverseLerp(0.0, 160.0, Math.AbsFloat(lookAngles[0]));
		
		float lookPitch = lookAngles[1] + charaRot[1]; // Save current pitch to be used later
		lookAngles[1] = Math.Lerp(charaRot[1], -charaRot[1], tYaw); // Calc. backtrace direction's Pitch
		
		pOutResult.m_vBacktraceDir = lookAngles.AnglesToVector(); // Custom backtrace direction
		
		// Additionally, tweak the PitchLimitReduction curve multiplier
		float pitchReductionMultiplier = 1.0;
		pitchReductionMultiplier = Math.Lerp(1.0, 0.7, tSlopeUp);
		m_CharacterHeadAimingComponent.SetPitchLimitReductionMultiplier(Math.Lerp(1.0, pitchReductionMultiplier, m_fFreelookBlend));
	}
}
