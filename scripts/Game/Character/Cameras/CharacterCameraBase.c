class CharacterCameraBase : ScriptedCameraItem
{
	//------------------------------------------------------------------------------------------------
	//! constructor
	void CharacterCameraBase(CameraHandlerComponent pCameraHandler)
	{
		m_fLRAngleVel = 0;
		m_fUDAngleVel = 0;
		m_fFOV = GetBaseFOV();
				
		m_CharacterCameraHandler = SCR_CharacterCameraHandlerComponent.Cast(m_CameraHandler);
		m_OwnerCharacter = ChimeraCharacter.Cast(pCameraHandler.GetOwner());
		m_ControllerComponent = SCR_CharacterControllerComponent.Cast(m_OwnerCharacter.FindComponent(SCR_CharacterControllerComponent));
		m_Input = m_ControllerComponent.GetInputContext();
		if (sm_TagFPCamera == -1)
		{
			sm_TagFPCamera = GameAnimationUtils.RegisterAnimationTag("TagFPCamera");
		}
		if (sm_TagADSTransitionIn == -1)
		{
			sm_TagADSTransitionIn = GameAnimationUtils.RegisterAnimationTag("TagTransitionADSIn");
		}
		if (sm_TagADSTransitionOut == -1)
		{
			sm_TagADSTransitionOut = GameAnimationUtils.RegisterAnimationTag("TagTransitionADSOut");
		}
		if (sm_TagLyingCamera == -1)
		{
			sm_TagLyingCamera = GameAnimationUtils.RegisterAnimationTag("TagLyingCamera");
		}
		if (sm_TagItemUpdateCols == -1)
		{
			sm_TagItemUpdateCols = GameAnimationUtils.RegisterAnimationTag("TagItemUpdateColliders");
		}
		if (sm_iCameraBoneIndex == -1)
		{
			Animation anim = m_OwnerCharacter.GetAnimation();
			sm_iHeadBoneIndex = anim.GetBoneIndex("Head");
			sm_iCameraBoneIndex = anim.GetBoneIndex("Camera");
			s_iNeckBoneIndex = anim.GetBoneIndex("Neck1");
		}
		m_CharacterAnimationComponent = CharacterAnimationComponent.Cast(m_OwnerCharacter.FindComponent(CharacterAnimationComponent));
		CharacterCommandHandlerComponent cmdHandler = CharacterCommandHandlerComponent.Cast(m_CharacterAnimationComponent.FindComponent(CharacterCommandHandlerComponent));
		m_CharacterHeadAimingComponent = CharacterHeadAimingComponent.Cast(m_OwnerCharacter.FindComponent(CharacterHeadAimingComponent));
		m_CommandWeapons = cmdHandler.GetCommandModifier_Weapon();
		m_CompartmentAccessComponent = m_OwnerCharacter.GetCompartmentAccessComponent();
		m_CmdHandler = cmdHandler;
	}

	//------------------------------------------------------------------------------------------------
	float UpdateUDAngle(out float pAngle, float pMin, float pMax, float pDt)
	{
		pAngle = Math.Clamp(m_CharacterHeadAimingComponent.GetAimingRotation()[1], pMin, pMax);
		
		//On Foot
		if (m_CompartmentAccessComponent && !m_CompartmentAccessComponent.IsInCompartment())
			pAngle += m_ControllerComponent.GetInputContext().GetAimingAngles()[1];
		
		return pAngle;
	}

	//------------------------------------------------------------------------------------------------
	float UpdateLRAngle(float pAngle, float pMin, float pMax, float pDt)
	{
		bool bCompartmentForcesFreeLook = false;
		if (m_CompartmentAccessComponent && m_CompartmentAccessComponent.IsInCompartment())
			bCompartmentForcesFreeLook = m_CompartmentAccessComponent.GetCompartment().GetForceFreeLook();
		
		//! lr angle
		if (m_ControllerComponent.IsFreeLookEnabled() || m_ControllerComponent.IsTrackIREnabled() || m_bForceFreeLook || bCompartmentForcesFreeLook)	
		{
			pAngle = m_CharacterHeadAimingComponent.GetAimingRotation()[0];
			
			if (!m_bLRAngleNoLimit)
			{
				pAngle = Math.Clamp(pAngle, pMin, pMax);
			}
			else
			{
				pAngle = SCR_Math.FixAngle(pAngle, 180);
			}
			
			m_fLRAngleVel = 0;	// reset filter
		}
		else
		{
			pAngle = Math.SmoothCD(pAngle, 0, m_fLRAngleVel, 0.14, 1000, pDt);
		}

		return pAngle;
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult);

	//------------------------------------------------------------------------------------------------
	override void OnActivate(ScriptedCameraItem pPrevCamera, ScriptedCameraItemResult pPrevCameraResult)
	{
		// Save shoulder state if camera was 3rd person or pass on last state
		if (pPrevCamera)
		{
			if ( pPrevCamera.IsInherited(CharacterCamera3rdPersonBase) )
			{
				m_fShoulderLastActive = CharacterCamera3rdPersonBase.Cast(pPrevCamera).GetActiveShoulder();
			}
			else if ( pPrevCamera.IsInherited(CharacterCameraBase) )
			{
				m_fShoulderLastActive = CharacterCameraBase.Cast(pPrevCamera).GetShoulderLastActive();
			}
		}
		if (IsInherited(CharacterCamera3rdPersonBase) || IsInherited(CharacterCamera3rdPersonVehicle))
		{
			m_CameraHandler.SetLensFlareSet(CameraLensFlareSetType.ThirdPerson, "");
		}
		else
		{
			m_CameraHandler.SetLensFlareSet(CameraLensFlareSetType.FirstPerson, "");
		}
		
		CharacterCameraBase prevCameraBase = CharacterCameraBase.Cast(pPrevCamera);
		if (prevCameraBase)
		{
			m_fPitchSmooth = prevCameraBase.m_fPitchSmooth;
			m_fPitchSmoothVel = prevCameraBase.m_fPitchSmoothVel;
			m_fRollSmooth = prevCameraBase.m_fRollSmooth;
			m_fRollSmoothVel = prevCameraBase.m_fRollSmoothVel;
		}
		
		m_CharacterHeadAimingComponent.SetPitchLimitReductionMultiplier(1.0);
		m_CharacterHeadAimingComponent.ResetLimitAnglesOverride();
	}

	//------------------------------------------------------------------------------------------------
	void ForceFreelook(bool state)
	{
		m_bForceFreeLook = state;
	}
	
	//-----------------------------------------------------------------------------
	//! Get last 3rd person shoulder state
	//! \return 3rd person shoulder state: -1 = left, 1 = right
	int GetShoulderLastActive()
	{
		return m_fShoulderLastActive;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	float GetShoulderDistance()
	{
		return 0.0;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	float GetInterpolatedUDTransformAngle(float pDt)
	{
		vector physTransform[4];
		m_OwnerCharacter.GetLocalTransform(physTransform);
		m_fTransformUDAngle = Math.SmoothCD(m_fTransformUDAngle, physTransform[2][1], m_fTransformUDAngleVel, 0.14, 1000, pDt);
		return m_fTransformUDAngle * Math.RAD2DEG;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param yawPitchRoll
	//! \param pitchFactor
	//! \param rollFactor
	//! \param transformMS
	void AddPitchRoll(vector yawPitchRoll, float pitchFactor, float rollFactor, inout vector transformMS[4])
	{
		// Get camera forward, without height
		vector cameraForward = transformMS[2];
		cameraForward[1] = 0.0;
		cameraForward.Normalize();
		
		// Get camera right, without height
		vector cameraAside = transformMS[0];
		cameraAside[1] = 0;
		cameraAside.Normalize();
		
		// See how much of individual components to apply
		float rollAmount = vector.Dot(cameraForward, vector.Forward);
		float pitchAmount = vector.Dot(cameraAside, vector.Forward);
		
		float pitch = yawPitchRoll[1];
		float roll = yawPitchRoll[2];
		
		float newPitch = pitchFactor * (pitch * rollAmount - roll * pitchAmount);
		float newRoll = rollFactor * (roll * rollAmount - pitch * pitchAmount);
		
		// Create rotation matrix
		vector newPitchRoll = Vector(0, newPitch, newRoll);
		vector rotMat[3];
		Math3D.AnglesToMatrix(newPitchRoll, rotMat);
		
		// Apply matrix to final transformation
		Math3D.MatrixMultiply3(transformMS, rotMat, transformMS);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddVehiclePitchRoll(IEntity vehicle, float pDt, inout vector transformMS[4])
	{
		if (!vehicle)
			return;
		
		vector vehMat[4];
		vehicle.GetTransform(vehMat);

		// Compensate for roll after camera is updated
		vector yawPitchRoll = Math3D.MatrixToAngles(vehMat);
		m_fPitchSmooth = Math.SmoothCD(m_fPitchSmooth, -yawPitchRoll[1], m_fPitchSmoothVel, 0.14, 1000, pDt);
		m_fRollSmooth = Math.SmoothCD(m_fRollSmooth, -yawPitchRoll[2], m_fRollSmoothVel, 0.14, 1000, pDt);
		yawPitchRoll[0] = 0;
		yawPitchRoll[1] = m_fPitchSmooth;
		yawPitchRoll[2] = m_fRollSmooth;

		float rollMask = Math.Max(0, vehMat[1][1]); // Do not apply roll factor when the vehicle is upside-down.
		AddPitchRoll(yawPitchRoll, m_fPitchFactor, rollMask * m_fRollFactor, transformMS);
	}
	
	//! runtime values 
	protected 	float 	m_fUpDownAngle;		//!< up down angle in rad
	protected 	float 	m_fUpDownAngleAdd;	//!< up down angle in rad
	protected 	float 	m_fLeftRightAngle = 0.0; //!< left right angle in rad
	
	//-----------------------------------------------------------------------------
	override void SetBaseAngles(out vector angles)
	{
		m_fUpDownAngle = SCR_Math.FixAngle(angles[0], 180);
		m_fLeftRightAngle = SCR_Math.FixAngle(angles[1], 180);
	}
	
	//-----------------------------------------------------------------------------
	override vector GetBaseAngles()
	{
		return m_CharacterHeadAimingComponent.GetLookAngles();
	}

	//------------------------------------------------------------------------------------------------
	//! Since camera bone is animated only in certain situations we want to provide camera bone whenever it is requested by animation
	//! otherwise we provide head bone to reduce amount of work animators need to do in order to make camera bone following head bone
	//! \return
	protected TNodeId GetCameraBoneIndex()
	{
		if (m_CharacterAnimationComponent.IsPrimaryTag(sm_TagFPCamera))
		{
			return sm_iCameraBoneIndex;
		}
		return sm_iHeadBoneIndex;
	}
	
	protected float					m_fFOV;

	protected float 				m_fLRAngleVel;
	protected float 				m_fUDAngleVel;
	protected float 				m_fTransformUDAngleVel;
	protected float 				m_fTransformUDAngle;
	protected bool					m_bForceFreeLook;
	protected bool					m_bLRAngleNoLimit;
	
	protected bool m_bIgnoreCharacterPitch;
	
	protected float					m_fFOVFilter;
	protected float					m_fFOVFilterVel;
	
	protected int 					m_fShoulderLastActive = 1;	//!< saved active shoulder for 3rd person cam (1 - right, -1 left)
	
	protected float 				m_fRollFactor;
	protected float 				m_fRollSmooth;
	protected float 				m_fRollSmoothVel;
	protected float					m_fPitchFactor;
	protected float 				m_fPitchSmooth;
	protected float 				m_fPitchSmoothVel;

	protected ChimeraCharacter						m_OwnerCharacter;	
	protected SCR_CharacterControllerComponent		m_ControllerComponent;
	protected CharacterInputContext					m_Input;
	protected SCR_CharacterCameraHandlerComponent	m_CharacterCameraHandler;
	protected CharacterAnimationComponent	m_CharacterAnimationComponent;
	protected CharacterHeadAimingComponent m_CharacterHeadAimingComponent;
	protected CharacterCommandWeapon				m_CommandWeapons;
	protected CompartmentAccessComponent			m_CompartmentAccessComponent;
	protected CharacterCommandHandlerComponent m_CmdHandler;
	protected float 								m_fShakeProgress;
	protected float									m_fShakeVelocity;
	protected static TNodeId sm_iCameraBoneIndex = -1;
	protected static TNodeId sm_iHeadBoneIndex = -1;
	protected static TNodeId s_iNeckBoneIndex = -1;
	protected static AnimationTagID sm_TagFPCamera = -1;
	protected static AnimationTagID sm_TagLyingCamera = -1; // beware: very deceitful!
	protected static AnimationTagID sm_TagItemUpdateCols = -1; // beware: very deceitful!
	protected static AnimationTagID sm_TagADSTransitionOut = -1;
	protected static AnimationTagID sm_TagADSTransitionIn = -1;
}
