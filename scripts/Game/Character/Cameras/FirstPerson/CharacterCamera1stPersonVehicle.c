
// *************************************************************************************
// ! CharacterCamera1stPersonVehicle - 1st person camera when character is in vehicle
// *************************************************************************************
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class CharacterCamera1stPersonVehicle extends CharacterCamera1stPerson
{
	IEntity m_OwnerVehicle;
	protected float m_fAngleFirstPerson;
	protected float m_fAngleFirstPersonScale;

	//-----------------------------------------------------------------------------
	void CharacterCamera1stPersonVehicle(CameraHandlerComponent pCameraHandler)
	{
		m_ApplyHeadBob = false;
	}
		
	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);

		if (m_pCompartmentAccess)
		{
			BaseCompartmentSlot compartment = m_pCompartmentAccess.GetCompartment();
			if (compartment)
			{
				m_OwnerVehicle = compartment.GetOwner();
				
				if (m_OwnerVehicle)
				{
					SCR_VehicleCameraDataComponent vehicleCamDataComp = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
					if (vehicleCamDataComp)
					{
						SCR_VehicleCameraDataComponentClass vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(vehicleCamDataComp.GetComponentData(m_OwnerVehicle));
						if (vehicleCamData)
						{
							m_fRollFactor = vehicleCamData.m_fRollFactor;
							m_fPitchFactor = vehicleCamData.m_fPitchFactor;
							m_fAngleFirstPerson = vehicleCamData.m_fAngleFirstPerson * Math.DEG2RAD;
						}
					}

					if (m_ControllerComponent && m_ControllerComponent.IsGadgetInHands())
						m_fAngleFirstPersonScale = 0;
					else
						m_fAngleFirstPersonScale = 1;
				}
			}
		}
		
		CharacterCamera3rdPersonVehicle characterCamera3rdPersonVehicle  = CharacterCamera3rdPersonVehicle.Cast(pPrevCamera);
		if (characterCamera3rdPersonVehicle)
		{
			m_fRollSmooth = characterCamera3rdPersonVehicle.m_fRollSmooth;
			m_fRollSmoothVel = characterCamera3rdPersonVehicle.m_fRollSmoothVel;
			m_fPitchSmooth = characterCamera3rdPersonVehicle.m_fPitchSmooth;
			m_fPitchSmoothVel = characterCamera3rdPersonVehicle.m_fPitchSmoothVel;
			
			if (m_OwnerVehicle && m_OwnerCharacter)
			{
				// Offset rotation of 3rd person camera if player is sitting sideways in vehicle
				float thirdPersonCameraOffset = (m_OwnerVehicle.GetYawPitchRoll()[0] - m_OwnerCharacter.GetYawPitchRoll()[0]) * Math.DEG2RAD;
		
				if (!float.AlmostEqual(thirdPersonCameraOffset, 0))
				{	
					vector angles = m_Input.GetLookAtAngles();
					angles[0] = angles[0] - thirdPersonCameraOffset;
					m_Input.SetLookAtAngles(angles);
				}
			}
		}
		
		m_bCameraTransition = false;
	}

	//-----------------------------------------------------------------------------
	
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		if (!m_bCameraTransition)
		{
			m_UseLookPositionOverrideLS = true;

			// Prevent camera from moving behind the character as it orbits around the head.
			float yawOffset = -0.5 * m_CharacterHeadAimingComponent.GetLookAngles()[0];
			vector additiveYaw = { yawOffset, 0.0, 0.0 };
			
			// Camera moves forward as the player looks sideways (providing better view off the shoulders when pitching down).
			const float OneBy90 = 1.0 / 90.0;
			float t = Math.Clamp(Math.AbsFloat(additiveYaw[0] * OneBy90), 0.0, 1.0);
			float yOffset = 0.03 * t;
			float zOffset = -0.07 - 0.13 * t;
			vector offsetLS = { m_OffsetLS[0], m_OffsetLS[1] + yOffset, zOffset };
						
			vector resultTM[4];
			m_CharacterHeadAimingComponent.GetLookTransformationLS(GetCameraBoneIndex(), EDirectBoneMode.RelativePosition, offsetLS, additiveYaw, resultTM);
			m_LookPositionOverrideLS = resultTM[3];

			m_CharacterHeadAimingComponent.SetPitchLimitReductionMultiplier(0.5); // Less aggressive pitch reduction.
		}
		
		super.OnUpdate(pDt, pOutResult);
		pOutResult.m_fUseHeading = 0.0;	
		AddVehiclePitchRoll(m_OwnerVehicle, pDt, pOutResult.m_CameraTM);
		
		// Specific case for character lying down, where assumption for world-up fails
		// If we want to allow such cases on a 'daily basis', reconsider how seating
		// and vehicle cameras work in first person :)
		if (sm_TagLyingCamera != -1 && sm_TagItemUpdateCols != -1)
		{
			CharacterAnimationComponent characterAnimationComponent = m_OwnerCharacter.GetAnimationComponent();
			if (characterAnimationComponent && characterAnimationComponent.IsPrimaryTag(sm_TagLyingCamera))
			{
				// The character has very odd orientation in this case, the entity transformation retains its world up relation,
				// but the character body is not lying flat, just chilling, so let's deploy some quick magic here:
				pOutResult.m_vBaseAngles 		= m_CharacterHeadAimingComponent.GetAimingRotation();
				pOutResult.m_fUseHeading 		= 0.0;
				pOutResult.m_iDirectBoneMode 	= EDirectBoneMode.RelativePosition;
				pOutResult.m_iDirectBone 		= GetCameraBoneIndex();
				pOutResult.m_bAllowInterpolation = true;
				
				vector additiveRotation = "0 0 0";
				vector offset = m_OffsetLS;
				m_CharacterHeadAimingComponent.GetLookTransformationLS(pOutResult.m_iDirectBone, pOutResult.m_iDirectBoneMode, offset, additiveRotation, pOutResult.m_CameraTM);
				if( m_ApplyHeadBob )
					m_CharacterCameraHandler.AddViewBobToTransform(pOutResult.m_CameraTM, 1, false);
				
				vector rotMat[3];
				if (!characterAnimationComponent.IsPrimaryTag(sm_TagItemUpdateCols))
					Math3D.AnglesToMatrix(Vector(0, 90, 0), rotMat);	// Already laying
				else
				{
					vector camMatrix[4];	// Entering, exiting
					m_OwnerCharacter.GetAnimation().GetBoneMatrix(m_OwnerCharacter.GetAnimation().GetBoneIndex("Head"), camMatrix);
					vector camAngles = Math3D.MatrixToAngles(camMatrix);
					Math3D.AnglesToMatrix(Vector(0, -camAngles[1], 0), rotMat);
				}
				
				//Math3D.AnglesToMatrix(Vector(0, -camAngles[1], 0), rotMat);
				Math3D.MatrixMultiply3(rotMat, pOutResult.m_CameraTM, pOutResult.m_CameraTM);
				return;
			}
		}

		// Rotate camera if not having gadget in hands
		if (float.AlmostEqual(m_fAngleFirstPerson, 0))
			return;

		// First person camera angle offset only for pilot
		Vehicle vehicle = Vehicle.Cast(m_OwnerVehicle);
		if (!vehicle || vehicle.GetPilot() != m_OwnerCharacter)
			return;

		if (m_ControllerComponent && m_ControllerComponent.IsGadgetInHands())
			m_fAngleFirstPersonScale -= pDt * 3;
		else
			m_fAngleFirstPersonScale += pDt * 3;

		m_fAngleFirstPersonScale = Math.Clamp(m_fAngleFirstPersonScale, 0, 1);

		SCR_Math3D.RotateAround(pOutResult.m_CameraTM, pOutResult.m_CameraTM[3], pOutResult.m_CameraTM[0], m_fAngleFirstPerson * m_fAngleFirstPersonScale, pOutResult.m_CameraTM);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
	}
	
	//-----------------------------------------------------------------------------	
	override float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;
		
		return cameraManager.GetVehicleFOV();
	}
};
// *************************************************************************************
// ! CharacterCamera1stPersonVehicleTransition - 1st person camera when character is getting in/out vehicle
// ************************************************************************************
class CharacterCamera1stPersonVehicleTransition extends CharacterCamera1stPersonVehicle
{
	protected bool m_isExiting = false;

	//-----------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		m_bCameraTransition = true;
		m_isExiting = false;
	}
	
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		super.OnUpdate(pDt, pOutResult);
		if (m_pCompartmentAccess)
		{
			if (m_pCompartmentAccess.IsGettingOut())
				m_isExiting = true;
			
			if (!m_pCompartmentAccess.IsInCompartment() && m_isExiting)
			{
				pOutResult.m_bAllowInterpolation = false;
				pOutResult.m_fUseHeading		 = 1.0;
			}
		}
	}
}
//---- REFACTOR NOTE END ----
