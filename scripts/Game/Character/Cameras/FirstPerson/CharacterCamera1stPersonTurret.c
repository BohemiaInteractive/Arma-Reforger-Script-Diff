// *************************************************************************************
// ! CharacterCamera1stPersonVehicle - 1st person camera when character is in vehicle
// *************************************************************************************
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class CharacterCamera1stPersonTurret extends CharacterCamera1stPerson
{

	protected TurretControllerComponent m_pTurretController;
	protected TurretComponent m_pControlledTurret;
	protected TurretCompartmentSlot m_pCompartment;
	protected vector m_vVertAimLimits;
	protected vector m_vHorAimLimits;
	protected float m_fLRAngleAdd;
	protected vector m_vPrevEyePosition;
	protected IEntity m_OwnerVehicle;

	protected vector m_vLastCameraAngles; //< Does not update in freelook

	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonTurret(CameraHandlerComponent pCameraHandler)
	{
		m_ApplyHeadBob = false;
		m_vPrevEyePosition = m_OwnerCharacter.EyePositionModel();
		m_OffsetLS = "0.0 0.1 0.0";
		m_bLRAngleNoLimit = true;
	}
		
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);

		if (m_pCompartmentAccess && m_pCompartmentAccess.IsInCompartment())
		{
			BaseCompartmentSlot compartment = m_pCompartmentAccess.GetCompartment();
			if (compartment)
			{
				m_OwnerVehicle = compartment.GetVehicle();
				
				SCR_VehicleCameraDataComponent vehicleCamDataComp = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
				if (vehicleCamDataComp)
				{
					SCR_VehicleCameraDataComponentClass vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(vehicleCamDataComp.GetComponentData(m_OwnerVehicle));
					if (vehicleCamData)
					{
						m_fRollFactor = vehicleCamData.m_fRollFactor;
						m_fPitchFactor = vehicleCamData.m_fPitchFactor;
					}
				}
				
				m_pTurretController = TurretControllerComponent.Cast(compartment.GetController());
				if (!m_pTurretController)
					return;

				// If we'll have multiple turrets, don't cache turret
				m_pControlledTurret = m_pTurretController.GetTurretComponent();
				if (!m_pControlledTurret)
					return;

				m_pControlledTurret.GetAimingLimits(m_vHorAimLimits, m_vVertAimLimits);
				m_pCompartment = TurretCompartmentSlot.Cast(compartment);
			}
		}
		
		m_fLRAngleAdd = 0;

		m_CharacterHeadAimingComponent.SetPitchLimitReductionMultiplier(1.25); // Pitch reduction to avoid neckhole
	}
	
	//-----------------------------------------------------------------------------
	override void OnDeactivate(ScriptedCameraItem pNextCamera)
	{
		m_CharacterHeadAimingComponent.ResetLimitAnglesOverride();
	}
	
	//-----------------------------------------------------------------------------
	static float UpdateAngleWithTarget(out float pAngle, out float pAngleAdd, out float addVelocity, vector limits, float pDt, float target, float change, bool freeLook)
	{
		//! update it in degrees
		pAngle = target;
		pAngle = Math.Clamp(pAngle, limits[0], limits[1]);
		// remove freelook when switched back to controlled state
		pAngleAdd = Math.SmoothCD(pAngleAdd, 0, addVelocity, 0.14, 1000, pDt);
		
		return Math.Clamp(pAngle + pAngleAdd, limits[0], limits[1]);
	}
	//-----------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		// no super for now, will calculate angle based on turret aiming (HeadAngles not supported when turret compartment attachment)
		if (!m_pControlledTurret)
			return;
		
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_fPositionModelSpace 	= 1.0;
		pOutResult.m_fFOV 					= m_fFOV;
		pOutResult.m_fNearPlane 			= 0.04;
		pOutResult.m_bAllowInterpolation 	= true;
		pOutResult.m_bUpdateWhenBlendOut 	= true;
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_iDirectBoneMode 		= EDirectBoneMode.None;
		pOutResult.m_iDirectBone 			= -1;
		
		vector offset = m_OffsetLS;
		if (m_pCompartment && m_pCompartment.IsDirectAimMode() && m_pTurretController.GetCanAimOnlyInADS())
			offset = vector.Zero;

		CharacterControllerComponent charController = m_OwnerCharacter.GetCharacterController();

		//! apply to rotation matrix
		m_vLastCameraAngles = m_pControlledTurret.GetAimingDirection().VectorToAngles();
		if (charController.IsFreeLookEnabled() || m_pTurretController.GetCanAimOnlyInADS() || charController.IsTrackIREnabled())
		{
			float downLimit = m_pCompartment.GetFreelookAimLimitOverrideDown();
			float upLimit = m_pCompartment.GetFreelookAimLimitOverrideUp();
			float leftLimit = m_pCompartment.GetFreelookAimLimitOverrideLeft();
			float rightLimit = m_pCompartment.GetFreelookAimLimitOverrideRight();

			m_CharacterHeadAimingComponent.SetLimitAnglesOverride(downLimit, upLimit, leftLimit, rightLimit);
			
			vector lookAnglesOverriden = m_CharacterHeadAimingComponent.GetLookAngles();			

			// Pivot around Neck1 bone to prevent showing neck hole to player
			vector headBoneMat[4];
			vector neckBoneMat[4];
			m_OwnerCharacter.GetAnimation().GetBoneMatrix(sm_iHeadBoneIndex, headBoneMat);
			m_OwnerCharacter.GetAnimation().GetBoneMatrix(s_iNeckBoneIndex, neckBoneMat);

			vector neckPosition = neckBoneMat[3] - headBoneMat[3];

			// Account for body rotation that comes from animation and cannot be retrieved otherwise
			vector aiming = m_pControlledTurret.GetAimingRotation();
			float neckTraverse = aiming[0] * m_pCompartment.GetFreelookCameraNeckFollowTraverse();

			// Rotate around neck
			vector neckOffsetMat[4];
			Math3D.MatrixIdentity4(neckOffsetMat);
			SCR_Math3D.RotateAround(neckOffsetMat, neckPosition, vector.Up, -Math.DEG2RAD * (lookAnglesOverriden[0] + neckTraverse), neckOffsetMat);
			SCR_Math3D.RotateAround(neckOffsetMat, neckPosition, neckOffsetMat[0], -Math.DEG2RAD * lookAnglesOverriden[1], neckOffsetMat);

			// Scale by compartment configuration
			vector neckOffset = neckOffsetMat[3];

			vector neckOffsetScale = m_pCompartment.GetFreelookCameraNeckOffsetScale();
			neckOffset[0] = neckOffset[0] * neckOffsetScale[0];
			neckOffset[1] = neckOffset[1] * neckOffsetScale[1];
			neckOffset[2] = neckOffset[2] * neckOffsetScale[2];
			offset += neckOffset;

			m_vLastCameraAngles[1] = 0; // Ignore gun elevation
			m_vLastCameraAngles = lookAnglesOverriden;
		}

		Math3D.AnglesToMatrix(m_vLastCameraAngles, pOutResult.m_CameraTM);
		
		//! lerp eye position to prevent nosiating shake when character walks around deployed turret
		m_vPrevEyePosition = vector.Lerp(m_vPrevEyePosition, m_OwnerCharacter.EyePositionModel(), 0.25);
		pOutResult.m_CameraTM[3] = m_vPrevEyePosition + offset;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, transformMS);
	}
	
	//-----------------------------------------------------------------------------	
	override float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;
		
		return cameraManager.GetVehicleFOV();
	}
}
// *************************************************************************************
// ! CharacterCamera1stPersonTurretTransition - 1st person camera when character is getting in/out turret
// ************************************************************************************
class CharacterCamera1stPersonTurretTransition extends CharacterCamera1stPersonVehicleTransition
{
	private bool m_bDoorTransformSet = false;
	private vector m_vDoorTransform[4];
	
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
	}
	
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);
		if (m_pCompartmentAccess)
		{
			BaseCompartmentSlot compartment = m_pCompartmentAccess.GetCompartment();
			if (compartment && !m_bDoorTransformSet)
			{
				compartment.GetManager().GetDoorInfo(compartment.PickDoorIndexForPoint(m_OwnerCharacter.GetOrigin())).GetExitPointInfo().GetWorldTransform(m_vDoorTransform);
				m_bDoorTransformSet = true;
			}
			
			if (!m_pCompartmentAccess.IsInCompartment() && m_isExiting)
			{
				m_Input.SetHeadingAngle(m_vDoorTransform[2][0]);
				m_Input.SetAimingAngles(m_vDoorTransform[2]);
			}
		}
	}
}

//---- REFACTOR NOTE END ----
