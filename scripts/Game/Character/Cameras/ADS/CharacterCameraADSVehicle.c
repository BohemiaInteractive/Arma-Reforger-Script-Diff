// *************************************************************************************
// ! CharacterCameraADSVehicle - ADS camera when character is in vehicle
// *************************************************************************************
//---- REFACTOR NOTE START: This code will need to be refactored as current implementation is not conforming to the standards ----
class CharacterCameraADSVehicle extends CharacterCameraADS
{
	protected TurretControllerComponent m_TurretController;
	protected IEntity m_OwnerVehicle;
	protected PointInfo m_CameraSlot;

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param pCameraHandler
	void CharacterCameraADSVehicle(CameraHandlerComponent pCameraHandler)
	{
		m_WeaponManager = null;
		m_OwnerVehicle = null;
		m_TurretController = null;

		if (!m_OwnerCharacter)
			return;

		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (!compartmentAccess || !compartmentAccess.IsInCompartment())
			return;

		BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
		if (!compartment)
			return;

		m_TurretController = TurretControllerComponent.Cast(compartment.GetController());
		if (!m_TurretController)
			m_TurretController = compartment.GetAttachedTurret();
		
		if (m_TurretController)
			m_WeaponManager = m_TurretController.GetWeaponManager();

		m_OwnerVehicle = compartment.GetVehicle();
		if (!m_OwnerVehicle)
			return;

		SCR_VehicleCameraDataComponent vehicleCamDataComp = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
		if (!vehicleCamDataComp)
			return;

		SCR_VehicleCameraDataComponentClass vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(vehicleCamDataComp.GetComponentData(m_OwnerVehicle));
		if (!vehicleCamData)
			return;

		m_fRollFactor = vehicleCamData.m_fRollFactor;
		m_fPitchFactor = vehicleCamData.m_fPitchFactor;
	}

	//------------------------------------------------------------------------------------------------
	override void OnBlendIn()
	{
		OnBlendingIn(1.0);
	}

	//------------------------------------------------------------------------------------------------
	override void OnBlendOut()
	{
		OnBlendingOut(1.0);
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void OnBlendingIn(float blendAlpha)
	{
		if (blendAlpha < GetSightsADSActivationPercentage())
			return;

		// Only enable if not enabled
		if (m_TurretController && !m_TurretController.GetCurrentSightsADS())
		{
			m_TurretController.SetCurrentSightsADS(true);
			
			SCR_OptimizedMuzzleEffectComponent.OptimizeMuzzleEffect(true, m_TurretController);
	
		}
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnBlendingOut(float blendAlpha)
	{
		if (blendAlpha < GetSightsADSDeactivationPercentage())
			return;

		// Only disable if enabled
		if (m_TurretController && m_TurretController.GetCurrentSightsADS())
		{
			m_TurretController.SetCurrentSightsADS(false);
			
			SCR_OptimizedMuzzleEffectComponent.OptimizeMuzzleEffect(false, m_TurretController);
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnUpdate(float pDt, out ScriptedCameraItemResult pOutResult)
	{
		if (m_CameraHandler && m_CameraHandler.IsCameraBlending())
		{
			if (m_CameraHandler.GetCurrentCamera() == this)
				OnBlendingIn(m_CameraHandler.GetBlendAlpha(this));
			else
				OnBlendingOut(m_CameraHandler.GetBlendAlpha(m_CameraHandler.GetCurrentCamera()));
		}

		BaseSightsComponent sights;
		
		if (m_TurretController)
		{
			TurretComponent turret = m_TurretController.GetTurretComponent();
			vector aimingTranslationWeaponLS = turret.GetRawAimingTranslation();
	
			// Disable recoil camera translation of turret mounted weapons
			aimingTranslationWeaponLS -= turret.GetCurrentRecoilTranslation();
	
			// Sight camera slot
			sights = turret.GetSights();
			PointInfo cameraSlot;
			if (sights)
				cameraSlot = sights.GetPositionPointInfo();
	
			// Legacy camera slot of TurretComponent
			if (!cameraSlot)
				cameraSlot = turret.GetCameraAttachmentSlot();
	
			if (cameraSlot != m_CameraSlot)
				m_CameraSlot = cameraSlot;
	
			//! sights transformation and FOV get
			if (cameraSlot)
			{
				vector sightLSMat[4];
				m_TurretController.GetCurrentSightsCameraLocalTransform(sightLSMat, pOutResult.m_fFOV);
				sightLSMat[3] = sightLSMat[3] - aimingTranslationWeaponLS;
	
				vector cameraSlotMat[4];
				cameraSlot.GetModelTransform(cameraSlotMat);
				
				vector zeroingLS[4];
				if (m_WeaponManager.GetCurrentWeapon().GetCurrentSightsZeroingTransform(zeroingLS))
				{
					zeroingLS[3] = -zeroingLS[3];
					Math3D.MatrixMultiply4(sightLSMat, zeroingLS, sightLSMat);
				}
	
				pOutResult.m_CameraTM[3]            = (sightLSMat[3] - cameraSlotMat[3]).InvMultiply3(cameraSlotMat);
				pOutResult.m_pWSAttachmentReference = cameraSlot;
				pOutResult.m_pOwner                 = m_OwnerCharacter;
			}
			else
			{
				//Add translation
				vector sightWSMat[4];
				m_TurretController.GetCurrentSightsCameraTransform(sightWSMat, pOutResult.m_fFOV);
				sightWSMat[3] = sightWSMat[3] - aimingTranslationWeaponLS.Multiply3(sightWSMat);
	
				// character matrix
				vector charMat[4];
				m_OwnerCharacter.GetTransform(charMat);
				Math3D.MatrixInvMultiply4(charMat, sightWSMat, pOutResult.m_CameraTM);
	
				pOutResult.m_pWSAttachmentReference = null;
				pOutResult.m_pOwner            		= m_OwnerCharacter; //! m_pOwner is only set if there is no WSAttachment which might cause issues in the future
			}
		}

		pOutResult.m_iDirectBone         = -1;
		pOutResult.m_iDirectBoneMode     = EDirectBoneMode.None;
		pOutResult.m_bUpdateWhenBlendOut = false;
		pOutResult.m_fDistance           = 0;
		pOutResult.m_fUseHeading         = 0;
		pOutResult.m_fNearPlane          = 0.025;
		pOutResult.m_bAllowInterpolation = true;

		// Apply shake
		if (m_CharacterCameraHandler)
			m_CharacterCameraHandler.AddShakeToToTransform(pOutResult.m_CameraTM, pOutResult.m_fFOV);

		if (!sights)
			return;

		SCR_SightsZoomFOVInfo fovInfo = SCR_SightsZoomFOVInfo.Cast(sights.GetFOVInfo());
		if (fovInfo) 
			fovInfo.ForceUpdate(m_TurretController.GetOwner(), sights, pDt);
	}

	//------------------------------------------------------------------------------------------------
	override void OnAfterCameraUpdate(float pDt, bool pIsKeyframe, inout vector transformMS[4])
	{
		if (!m_CameraSlot)
			return;

		IEntity owner = m_OwnerVehicle;
		if (m_TurretController)
			owner = m_TurretController.GetOwner();

		m_fPitchFactor = 0;
		AddVehiclePitchRoll(owner, pDt, transformMS);
	}
}
//---- REFACTOR NOTE END ----
