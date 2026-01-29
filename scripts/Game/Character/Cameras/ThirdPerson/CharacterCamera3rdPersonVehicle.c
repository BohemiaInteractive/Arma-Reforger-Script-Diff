// *************************************************************************************
// ! CharacterCamera3rdPersonVehicle - 3rd person camera when character is in vehicle
// *************************************************************************************
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class CharacterCamera3rdPersonVehicle extends CharacterCameraBase
{
	//-----------------------------------------------------------------------------
	static const float 	CONST_UD_MIN	= -60.0;		//!< down limit
	static const float 	CONST_UD_MAX	= 60.0;			//!< up limit

	static const float 	CONST_LR_MIN	= -360.0;		//!< left limit
	static const float 	CONST_LR_MAX	= 360.0;		//!< right limit

	static const float STEERING_DEGREES	= 5;
	static const float ANGULAR_INERTIA	= 5;
	protected float m_fSteeringAngle;
	protected float m_fInertiaAngle;

	//------------------------------------------------------------------------------------------------
	protected float m_fHeight;
	protected float m_fDist_Desired;
	protected float m_fDist_Min;
	protected float m_fDist_Max;
	protected float m_fFOV_SpeedAdjustMax;
	protected float m_fBobScale;
	protected float m_fShakeScale;
	protected float m_fSpeedMax;

	IEntity m_OwnerVehicle;
	protected BaseCompartmentSlot m_pCompartment;
	protected vector m_vCameraCenter; // In vehicle space

	protected vector m_vLastVel;
	protected vector m_vLastAngVel;
	protected vector m_vAcceleration;
	
	protected bool  m_bForceBasedCamera;
	protected float m_fCameraMovementForceFactor;
	protected float m_fCameraMovementForceThreshold;
	protected float m_fCameraSpringForceFactor;
	protected float m_fCameraHorizontalLag;
	protected float m_fCameraHorizontalLagMax;
	protected bool  m_bDollyZoomEffect;
	protected float m_fCamVelocity;
	protected float m_fCamDist;
	protected float m_fCamDistTarget;
	protected float m_fZoomTargetWidth;
	protected float m_fCamFOV;

	protected float m_f3rd_TraceClipPct;
	protected float m_fBob_FastUp;
	protected float m_fBob_FastRight;
	protected float m_fBob_SlowUp;
	protected float m_fBob_SlowRight;
	protected float m_fBob_ScaleFast;
	protected float m_fBob_ScaleSlow;
	protected float m_fBob_Acceleration;

	//rad
	protected float m_fAngleThirdPerson;

	protected SCR_VehicleCameraAimpoint m_pCameraAimpointData;
	protected SCR_VehicleCameraAlignment m_pCameraAlignData;
	protected PointInfo m_pCameraPivot;
	protected bool m_bUseNoParent;
	
	protected bool m_bViewBob = true;
	float m_fUpDownAngleCurrent;

	//------------------------------------------------------------------------------------------------
	void CharacterCamera3rdPersonVehicle(CameraHandlerComponent pCameraHandler)
	{
		m_fFOV = GetBaseFOV();
		m_bLRAngleNoLimit = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void InitCameraData()
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			m_pCompartment = compartmentAccess.GetCompartment();
			
			IEntity compartmentOwner = m_pCompartment.GetOwner();
			IEntity vehicle = compartmentOwner.GetRootParent();
			if (vehicle)
			{
				m_OwnerVehicle = vehicle;

				SCR_VehicleCameraDataComponent vehicleCamDataComp = SCR_VehicleCameraDataComponent.Cast(vehicle.FindComponent(SCR_VehicleCameraDataComponent));
				// Can owner replace Vehicle's camera data?
				SCR_VehicleCameraDataComponent ownerCamDataComp = SCR_VehicleCameraDataComponent.Cast(compartmentOwner.FindComponent(SCR_VehicleCameraDataComponent));
				SCR_VehicleCameraDataComponentClass vehicleCamData;
				if (ownerCamDataComp)
					vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(ownerCamDataComp.GetComponentData(compartmentOwner));

				if (!vehicleCamData || !vehicleCamData.m_bOverrideVehicleSettings)
					vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(vehicleCamDataComp.GetComponentData(vehicle));
					
				if(vehicleCamData)
				{
					m_fHeight = vehicleCamData.m_fHeight;
					m_fDist_Desired = vehicleCamData.m_fDist_Desired;
					m_fDist_Min = vehicleCamData.m_fDist_Min;
					m_fDist_Max = vehicleCamData.m_fDist_Max;
					//m_fFOV = vehicleCamData.m_fFOV;
					m_fFOV_SpeedAdjustMax = vehicleCamData.m_fFOV_SpeedAdjustMax;
					m_fBobScale = vehicleCamData.m_fBobScale;
					m_fShakeScale = vehicleCamData.m_fShakeScale;
					m_fSpeedMax = vehicleCamData.m_fSpeedMax;
					m_pCameraAimpointData = vehicleCamData.m_pCameraAimpointData;
					m_pCameraAlignData = vehicleCamData.m_pCameraAlignData;
					m_fRollFactor = vehicleCamData.m_3rdPersonRollFactor;
					m_fPitchFactor = vehicleCamData.m_3rdPersonPitchFactor;
					m_fAngleThirdPerson = vehicleCamData.m_fAngleThirdPerson * Math.DEG2RAD;
					m_pCameraPivot = vehicleCamData.m_pPivot;
					m_bUseNoParent = vehicleCamData.m_bNoParent;
					m_bForceBasedCamera = vehicleCamData.m_UseForceBased_3PVCamera;
					m_fCameraMovementForceFactor = vehicleCamData.m_MovementForceFactor;
					m_fCameraMovementForceThreshold = vehicleCamData.m_MovementForceThreshold;
					m_fCameraSpringForceFactor = vehicleCamData.m_SpringForceFactor;
					m_bDollyZoomEffect = vehicleCamData.m_3PVDollyZoomEffect;
					m_fCameraHorizontalLag = vehicleCamData.m_HorizontalSpringLag;
					m_fCameraHorizontalLagMax = vehicleCamData.m_HorizontalSpringLagMaxAngle;
					
					m_fCamDist = m_fDist_Desired;
					m_fCamDistTarget = m_fDist_Desired;
					m_fZoomTargetWidth = m_fCamDist * 2 * Math.Tan(0.5 * m_fFOV * Math.DEG2RAD);
				}
				
				if (m_pCameraPivot)
				{
					vector matrix[4];
					m_pCameraPivot.GetLocalTransform(matrix);
					m_vCameraCenter = matrix[3];
				}
				else
				{
					Physics physics = vehicle.GetPhysics();
					if (physics)
					{
						m_vCameraCenter = physics.GetCenterOfMass();
					}
					else
					{
						vector mins, maxs;
						vehicle.GetBounds(mins, maxs);
						m_vCameraCenter = (maxs - mins) * 0.5 + mins;
					}
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		super.OnActivate(pPrevCamera, pPrevCameraResult);
		
		m_CharacterHeadAimingComponent.SetLimitAnglesOverride(CONST_UD_MIN, CONST_UD_MAX, CONST_LR_MIN, CONST_LR_MAX);
		
		if (pPrevCamera)
		{
			vector f = pPrevCamera.GetBaseAngles();
			m_fUpDownAngle		= f[0]; 
			m_fLeftRightAngle	= f[1]; 
		}
		
		InitCameraData();
		
		CharacterCamera1stPersonVehicle characterCamera1stPersonVehicle  = CharacterCamera1stPersonVehicle.Cast(pPrevCamera);
		if (characterCamera1stPersonVehicle)
		{
			m_fRollSmooth = characterCamera1stPersonVehicle.m_fRollSmooth;
			m_fRollSmoothVel = characterCamera1stPersonVehicle.m_fRollSmoothVel;
			m_fPitchSmooth = characterCamera1stPersonVehicle.m_fPitchSmooth;
			m_fPitchSmoothVel = characterCamera1stPersonVehicle.m_fPitchSmoothVel;
			
			if (m_OwnerVehicle && m_OwnerCharacter)
			{
				// Offset rotation of 3rd person camera if player is sitting sideways in vehicle
				float thirdPersonCameraOffset = (m_OwnerVehicle.GetYawPitchRoll()[0] - m_OwnerCharacter.GetYawPitchRoll()[0]) * Math.DEG2RAD;
		
				if (!float.AlmostEqual(thirdPersonCameraOffset, 0))
				{	
					vector angles = m_Input.GetLookAtAngles();
					angles[0] = angles[0] + thirdPersonCameraOffset;
					m_Input.SetLookAtAngles(angles);
				}
			}
		}
		
		m_CharacterCameraHandler.SetTraceCharacters(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDeactivate(ScriptedCameraItem pNextCamera)
	{
		super.OnDeactivate(pNextCamera);
		m_CharacterCameraHandler.SetTraceCharacters(true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			m_pCompartment = compartmentAccess.GetCompartment();
			IEntity vehicle = m_pCompartment.GetOwner().GetRootParent();
			if (vehicle && vehicle != m_OwnerVehicle)
			{
				InitCameraData();
			}
		}
		
		m_CharacterHeadAimingComponent.SetPitchLimitReductionMultiplier(0.0); // Remove pitch reduction.
		
		//! update angles
		m_fUpDownAngleCurrent = UpdateUDAngle(m_fUpDownAngle, CONST_UD_MIN, CONST_UD_MAX, pDt);
		m_fLeftRightAngle = UpdateLRAngle(m_fLeftRightAngle, CONST_LR_MIN, CONST_LR_MAX, pDt);
		
		pOutResult.m_vBaseAngles = GetBaseAngles();
		
		//! update fov
		m_fFOV = GetBaseFOV();
		
		// phx speed
		vector characterOffset = vector.Zero;
		vector localVelocity = vector.Zero;
		vector localAngVelocity = vector.Zero;
		float steeringAngle;
		vector vehMat[4];
		
		bool bCharacterAttached = true;
		
		if (m_OwnerVehicle)
		{
			m_OwnerVehicle.GetTransform(vehMat);

			vector charaMat[4];
			if (EntityUtils.GetAncestorToChildTransform(m_OwnerCharacter, m_OwnerVehicle, charaMat))
			{
				characterOffset = m_vCameraCenter.Multiply4(charaMat);
			}
			else
			{
				m_OwnerCharacter.GetTransform(charaMat);
				characterOffset = m_vCameraCenter.Multiply4(vehMat).InvMultiply4(charaMat);
				bCharacterAttached = false;
			}
			
			Physics physics = m_OwnerVehicle.GetPhysics();
			if (physics)
			{
				localVelocity = physics.GetVelocity().InvMultiply3(vehMat);
				localAngVelocity = physics.GetAngularVelocity().InvMultiply3(vehMat);
			}
			
			if (compartmentAccess && PilotCompartmentSlot.Cast(compartmentAccess.GetCompartment()))
			{
				VehicleWheeledSimulation simulation = VehicleWheeledSimulation.Cast(m_OwnerVehicle.FindComponent(VehicleWheeledSimulation));
				if (simulation)
					steeringAngle = simulation.GetSteering();
			}
		}
		else
		{
			Math3D.MatrixIdentity4(vehMat);
		}
		
		// To smoothen out jittering a bit
		vector smoothVelocity = vector.Lerp(m_vLastVel, localVelocity, pDt);
		vector smoothAngVelocity = vector.Lerp(m_vLastAngVel, localAngVelocity, pDt);
		
		m_fInertiaAngle = Math.Lerp(m_fInertiaAngle, smoothAngVelocity[0] * ANGULAR_INERTIA, pDt); //smoothAngVelocity[0] is actually pitch
		m_fSteeringAngle = Math.Lerp(m_fSteeringAngle, steeringAngle * STEERING_DEGREES, pDt);
		
		UpdateCameraDistanceAndFOV(localVelocity, m_vLastVel, pDt);
		
		// store phx values
		m_vLastVel = localVelocity;
		m_vLastAngVel = localAngVelocity;
		
		vector yawPitchRoll = Math3D.MatrixToAngles(vehMat);
		
		// update auto align
		if (m_pCameraAlignData && m_OwnerVehicle)
		{
			bool isFocused = false;
			if (m_CharacterCameraHandler && m_CharacterCameraHandler.GetFocusMode() > 0)
				isFocused = true;
			
			vector aimChange = m_Input.GetAimChange();
			vector newAngles;
			if (m_pCameraAlignData.Update(aimChange, Vector(m_fLeftRightAngle, m_fUpDownAngleCurrent, 0.0), smoothVelocity, isFocused, pDt, newAngles))
			{
				vector v = Vector(GetGame().GetInputManager().GetActionValue("CharacterFreelookHorizontal"), GetGame().GetInputManager().GetActionValue("CharacterFreelookVertical"), 0.0);
				if (float.AlmostEqual(v.LengthSq(), 0.0))
					m_Input.AccumulateFreelookAdjustment(Math.DEG2RAD * (newAngles - Vector(m_fLeftRightAngle, m_fUpDownAngleCurrent, 0.0)));
			}
		}
		
		vector charMat[4];
		m_OwnerCharacter.GetTransform(charMat);
		vector anglesChar = Math3D.MatrixToAngles(charMat);
		CalculateLookAngles(yawPitchRoll, anglesChar, pOutResult);
		
		//! Roll
		if (bCharacterAttached)
		{
			float rollAngle;
			if (!m_bUseNoParent)
			{
				//! Remove roll from parent
				vector orientation[3];
				Math3D.AnglesToMatrix(Vector(0, 0, -yawPitchRoll[2]), orientation);
				Math3D.MatrixMultiply3(orientation, pOutResult.m_CameraTM, pOutResult.m_CameraTM);

				//! Apply roll factor
				rollAngle = yawPitchRoll[2] * m_fRollFactor * Math.DEG2RAD * pOutResult.m_CameraTM[2][2];
				
				// Camera offset
				float heightSign = vehMat[1][1]; // Adjust the sign of the height direction as the vehicle rolls.
				characterOffset = characterOffset + Vector(0, heightSign * m_fHeight, 0);
			}
			else
			{
				//! Apply roll factor
				rollAngle = yawPitchRoll[2] * (1.0 - m_fRollFactor) * Math.DEG2RAD * pOutResult.m_CameraTM[2][2];
				
				// Camera offset
				characterOffset = characterOffset + Vector(0, m_fHeight, 0);
				vector pitchRollMat[3];
				Math3D.AnglesToMatrix({0, yawPitchRoll[1], yawPitchRoll[2]}, pitchRollMat);				
				characterOffset = characterOffset.Multiply3(pitchRollMat); // apply vehicle pitch and roll to camera center
			}
			
			float rollMask = Math.Max(0, vehMat[1][1]); // Do not apply roll factor when the vehicle is upside-down.
			SCR_Math3D.RotateAround(pOutResult.m_CameraTM, vector.Zero, pOutResult.m_CameraTM[2], -rollAngle * rollMask, pOutResult.m_CameraTM);
			
			pOutResult.m_CameraTM[3] = characterOffset;
		}
		else
		{
			pOutResult.m_CameraTM[3] = characterOffset + Vector(0, m_fHeight, 0);
		}
		
		// viewbob update
		UpdateViewBob(pDt, localVelocity, localAngVelocity);
		
		// Translate camera aimpoint based on aimpoing curve data
		if (m_pCameraAimpointData) 
		{	
			vector translation = m_pCameraAimpointData.Sample(smoothVelocity[2] * METERS_PER_SEC_TO_KILOMETERS_PER_HOUR);
			translation[0] = m_pCameraAimpointData.SampleAside(localAngVelocity[1] * METERS_PER_SEC_TO_KILOMETERS_PER_HOUR); 
			pOutResult.m_CameraTM[3] = pOutResult.m_CameraTM[3] + translation;
		}
		
		if (m_OwnerVehicle)
		{
			bool applyCharAngle = true;
			if (sm_TagLyingCamera != -1)
			{
				CharacterAnimationComponent characterAnimationComponent = m_OwnerCharacter.GetAnimationComponent();
				if (characterAnimationComponent && characterAnimationComponent.IsPrimaryTag(sm_TagLyingCamera))
					applyCharAngle = false;
			}
			if (applyCharAngle)
			{
				if (m_bUseNoParent)
					SCR_Math3D.RotateAround(pOutResult.m_CameraTM, pOutResult.m_CameraTM[3], pOutResult.m_CameraTM[0], m_fAngleThirdPerson - (1.0 - m_fPitchFactor) * yawPitchRoll[1] * Math.DEG2RAD, pOutResult.m_CameraTM);
				else
					SCR_Math3D.RotateAround(pOutResult.m_CameraTM, pOutResult.m_CameraTM[3], pOutResult.m_CameraTM[0], m_fAngleThirdPerson - m_fPitchFactor * yawPitchRoll[1] * Math.DEG2RAD, pOutResult.m_CameraTM);
			}
			else
				SCR_Math3D.RotateAround(pOutResult.m_CameraTM, pOutResult.m_CameraTM[3], pOutResult.m_CameraTM[0], 0, pOutResult.m_CameraTM);
		}
		
		// other parameters
		pOutResult.m_fUseHeading 			= 0.0;
		pOutResult.m_fFOV					= m_fCamFOV;
		pOutResult.m_fDistance 				= m_fCamDist;
		pOutResult.m_pWSAttachmentReference = null;
		pOutResult.m_pOwner 				= m_OwnerCharacter;
		pOutResult.m_bAllowCollisionSolver	= bCharacterAttached;
		pOutResult.m_bNoParent				= m_bUseNoParent;
		pOutResult.m_bAllowInterpolation	= true;
		
		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);
	}
	
	void UpdateCameraDistanceAndFOV(vector localVelocity, vector lastLocalVelocity, float pDt)
	{
		float speed = localVelocity.Length();
		float speedScale = Math.Clamp(speed / (m_fSpeedMax * KILOMETERS_PER_HOUR_TO_METERS_PER_SEC), 0, 1);

		if (m_bForceBasedCamera)
		{
			float speedDelta = speed - lastLocalVelocity.Length();
			
			if (speedDelta < m_fCameraMovementForceThreshold)
				speedDelta = 0;
	
			// Calculate camera movement local force
			float fMovementForce = 0;
			if (speedDelta > 0) // When vehicle is accelerating
				fMovementForce = m_fCameraMovementForceFactor; // Move the camera away from the vehicle
			else if (speedDelta < 0) // When vehicle is decelerating
				fMovementForce = -m_fCameraMovementForceFactor; // MOve the camera towards the vehicle
			
			if (localVelocity[2] < 0) // When going reverse
				fMovementForce *= -1;
			
			// Local force on the camera from an imaginary spring trying to bring it back to the m_fDist_Desired
			const float fSpringForce = m_fCameraSpringForceFactor * (m_fDist_Desired - m_fCamDistTarget) / (m_fDist_Max - m_fDist_Min);
	
			// Apply the forces on the camera and Update its velocity
			m_fCamVelocity += fMovementForce * pDt;
			m_fCamVelocity += fSpringForce * pDt;
	
			// Update camera target position and clamp it
			m_fCamDistTarget = Math.Clamp(m_fCamDistTarget + m_fCamVelocity * pDt, m_fDist_Min, m_fDist_Max);
			
			// Add friction and limits to the camera velocity
			m_fCamVelocity = Math.Clamp(0.5 * m_fCamVelocity, -1.25, 1.25);
			
			// Smoothen out camera distance
			m_fCamDist = Math.Lerp(m_fCamDist, m_fCamDistTarget, pDt);			
		}
		else
		{			
			m_fCamDist = Math.Clamp((m_fDist_Max - m_fDist_Min) * speedScale + m_fDist_Desired, m_fDist_Min, m_fDist_Max);
		}
		
		if (m_bDollyZoomEffect)
		{
			m_fCamFOV = 2.0 * Math.RAD2DEG * Math.Atan2(0.5 * m_fZoomTargetWidth, m_fCamDist);
			m_fCamFOV = Math.Clamp(m_fCamFOV, 60, 90);
		}
		else
		{
			m_fCamFOV = m_fFOV + speedScale * m_fFOV_SpeedAdjustMax;
		}
	}
	
	void CalculateLookAngles(vector vehicleAngles, vector characterAngles, out ScriptedCameraItemResult pOutResult)
	{
		// Horizontal 3rd person camera lag
		float horizontalLag = Math.Clamp(-m_vLastAngVel[1] * m_fCameraHorizontalLag, -m_fCameraHorizontalLagMax, m_fCameraHorizontalLagMax); //m_vLastAngVel[1] is actually yaw angular velocity
		
		//! yaw pitch roll vector
		vector lookAngles;
		lookAngles[0] = m_fLeftRightAngle + horizontalLag;
		lookAngles[1] = m_fUpDownAngleCurrent + m_fInertiaAngle;
		lookAngles[2] = 0.0;
		
		//! apply to rotation matrix
		Math3D.AnglesToMatrix(lookAngles, pOutResult.m_CameraTM);
	}
	
	//-----------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		//AddVehiclePitchRoll(m_OwnerVehicle, pDt, pOutResult.m_CameraTM);
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateViewBob(float pDt, vector localVelocity, vector localAngularVelocity)
	{
		if (!m_bViewBob)
			return;
		if (pDt <= 0)
			return;
		
		vector velDiff = localVelocity - m_vLastVel;
		vector angVelDiff = localAngularVelocity - m_vLastAngVel;
		float speed = localVelocity.Length();
		float accel = velDiff.Length() * (1 / pDt) + angVelDiff.Length() * (1 / pDt);
		if (accel > m_fBob_Acceleration)
			m_fBob_Acceleration += (accel - m_fBob_Acceleration) * Math.Clamp(pDt * 12, 0, 1);
		else
			m_fBob_Acceleration += (accel - m_fBob_Acceleration) * Math.Clamp(pDt * 7, 0, 1);
		
		// Add bobbing based on speed
		m_fBob_ScaleSlow += (Math.Clamp(speed / (m_fSpeedMax * KILOMETERS_PER_HOUR_TO_METERS_PER_SEC), 0, 1) - m_fBob_ScaleSlow) * (pDt * 4);
		float slowTimeSlice = (m_fBob_ScaleSlow * m_fBob_ScaleSlow * 1.5 + 1) * pDt;
		m_fBob_SlowUp += Math.Clamp(slowTimeSlice, 0, 1);
		if (m_fBob_SlowUp >= 1)
			m_fBob_SlowUp -= 1;
		m_fBob_SlowRight += Math.Clamp(slowTimeSlice * 0.75, 0, 1);
		if (m_fBob_SlowRight >= 1)
			m_fBob_SlowRight -= 1;
		
		// Add bobbing based on acceleration
		m_fBob_ScaleFast = Math.Clamp(m_fBob_Acceleration * 0.03, 0, 1);
		float fastTimeSlice = m_fBob_ScaleFast * 5 * pDt;
		if (m_fBob_ScaleFast < 0.01)
		{
			m_fBob_FastUp *= 1 - pDt;
			m_fBob_FastRight *= 1 - pDt;
		}
		else
		{
			m_fBob_FastUp += Math.Clamp(fastTimeSlice, 0, 1);
			if (m_fBob_FastUp >= 1)
				m_fBob_FastUp -= 1;
			m_fBob_FastRight += Math.Clamp(fastTimeSlice * 1.4, 0, 1);
			if (m_fBob_FastRight >= 1)
				m_fBob_FastRight -= 1;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void AddViewBobToTransform(inout vector outTransform[4])
	{
		float bobFastScale = m_fBob_ScaleFast * m_fShakeScale;
		float bobSlowScale = m_fBob_ScaleSlow * m_fBob_ScaleSlow * m_fBobScale;
		
		float bobFw = Math.Sin(m_fBob_FastUp * 360 * Math.DEG2RAD) * -0.05 * bobFastScale;
		float bobUp = Math.Sin(m_fBob_FastUp * 360 * Math.DEG2RAD) * 0.05 * bobFastScale;
		float bobRt = Math.Sin(m_fBob_FastRight * 360 * Math.DEG2RAD) * 0.05 * bobFastScale;
		float bobYaw = Math.Sin(m_fBob_FastRight * 360 * Math.DEG2RAD) * 0.3 * bobFastScale;
		float bobPitch = Math.Sin(m_fBob_FastUp * 360 * Math.DEG2RAD) * 0.3 * bobFastScale;
		float bobRoll = Math.Sin(m_fBob_FastRight * 360 * Math.DEG2RAD) * 0.4 * bobFastScale;
		bobFw += Math.Sin(m_fBob_SlowUp * 360 * Math.DEG2RAD) * -0.025 * bobSlowScale;
		bobUp += Math.Sin(m_fBob_SlowUp * 360 * Math.DEG2RAD) * 0.05 * bobSlowScale;
		bobRt += Math.Sin(m_fBob_SlowRight * 360 * Math.DEG2RAD) * 0.05 * bobSlowScale;
		bobYaw += Math.Sin(m_fBob_SlowRight * 360 * Math.DEG2RAD) * 0.2 * bobSlowScale;
		bobPitch += Math.Sin(m_fBob_SlowUp * 360 * Math.DEG2RAD) * 0.2 * bobSlowScale;
		bobRoll += Math.Sin(m_fBob_SlowRight * 360 * Math.DEG2RAD) * 0.45 * bobSlowScale;
		
		vector angBobMat[3], endBobmat[3];
		Math3D.AnglesToMatrix(Vector(bobPitch, bobYaw, bobRoll), angBobMat);
		Math3D.MatrixMultiply3(outTransform, angBobMat, endBobmat);
		outTransform[0] = endBobmat[0];
		outTransform[1] = endBobmat[1];
		outTransform[2] = endBobmat[2];
		outTransform[3] = Vector(bobRt, bobUp, bobFw).Multiply3(outTransform) + outTransform[3];
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetBaseFOV()
	{
		CameraManager cameraManager = GetGame().GetCameraManager();
		if (!cameraManager)
			return 0;
		
		return cameraManager.GetVehicleFOV();
	}
}
//---- REFACTOR NOTE END ----
