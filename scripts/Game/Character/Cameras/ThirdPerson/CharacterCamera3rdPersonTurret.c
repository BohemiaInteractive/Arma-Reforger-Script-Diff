// *************************************************************************************
// ! CharacterCamera3rdPersonTurret - 3rd person camera when character is controlling turret
// *************************************************************************************

class CharacterCamera3rdPersonTurret extends CharacterCamera3rdPersonVehicle
{
	protected TurretControllerComponent m_pTurretController;
	protected TurretComponent m_pControlledTurret;
	protected vector m_vVertAimLimits;
	protected vector m_vHorAimLimits;
	protected vector m_vLastCameraAngles; //< Does not update in freelook
	
	//-----------------------------------------------------------------------------
	void CharacterCamera3rdPersonTurret(CameraHandlerComponent pCameraHandler)
	{
		m_bLRAngleNoLimit = true;
		m_bViewBob = false;
	}
	
	override void InitCameraData()
	{
		CompartmentAccessComponent compartmentAccess = m_OwnerCharacter.GetCompartmentAccessComponent();
		if (compartmentAccess && compartmentAccess.IsInCompartment())
		{
			IEntity turret;
			TurretCompartmentSlot compartment = TurretCompartmentSlot.Cast(compartmentAccess.GetCompartment());
			if (compartment)
			{
				m_OwnerVehicle = compartment.GetVehicle();
				turret = compartment.GetOwner();
			}
			else
			{
				m_OwnerVehicle = compartmentAccess.GetCompartment().GetVehicle();
				turret = compartmentAccess.GetCompartment().GetAttachedTurret().GetOwner();
			}
			
			if (turret)
			{
				bool bUsingVehicleCameraData;

				SCR_VehicleCameraDataComponent vehicleCamDataComp = SCR_VehicleCameraDataComponent.Cast(turret.FindComponent(SCR_VehicleCameraDataComponent));
				if (!vehicleCamDataComp)
				{
					vehicleCamDataComp = SCR_VehicleCameraDataComponent.Cast(m_OwnerVehicle.FindComponent(SCR_VehicleCameraDataComponent));
					bUsingVehicleCameraData = true;
				}

				SCR_VehicleCameraDataComponentClass vehicleCamData;
				if (bUsingVehicleCameraData)
					vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(vehicleCamDataComp.GetComponentData(m_OwnerVehicle));
				else
					vehicleCamData = SCR_VehicleCameraDataComponentClass.Cast(vehicleCamDataComp.GetComponentData(turret));

 				if (vehicleCamData)
				{
					m_fHeight = vehicleCamData.m_fHeight;
					m_fDist_Desired = vehicleCamData.m_fDist_Desired;
					m_fDist_Min = vehicleCamData.m_fDist_Min;
					m_fDist_Max = vehicleCamData.m_fDist_Max;
					m_fSpeedMax = vehicleCamData.m_fSpeedMax;
					m_fFOV = vehicleCamData.m_fFOV;
					m_fRollFactor = vehicleCamData.m_fRollFactor;
					m_fPitchFactor = vehicleCamData.m_fPitchFactor;
					m_fAngleThirdPerson = vehicleCamData.m_fAngleThirdPerson * Math.DEG2RAD;
					
					// Get camera center
					vector matrix[4];
					PointInfo pCameraPivot = vehicleCamData.m_pPivot;
					if (pCameraPivot)
					{
						pCameraPivot.GetLocalTransform(matrix);
						m_vCameraCenter = matrix[3]; // Use pivot position
					}
					else
					{
						vector mins, maxs;
						if(bUsingVehicleCameraData)
							m_OwnerVehicle.GetBounds(mins, maxs);
						else
							turret.GetBounds(mins, maxs);
						
						if (bUsingVehicleCameraData)
						{
							Physics physics = m_OwnerVehicle.GetPhysics();
							if (physics)
								m_vCameraCenter = physics.GetCenterOfMass();	// Use COM
							else
								m_vCameraCenter = (maxs - mins) * 0.5 + mins; // Use vehicles bounds center
						}
						else
							m_vCameraCenter = (maxs - mins) * 0.5 + mins; // Use turret bounds center
					}
					if(!bUsingVehicleCameraData && turret != m_OwnerVehicle)
					{
						turret.GetLocalTransform(matrix);
						m_vCameraCenter = m_vCameraCenter.Multiply4(matrix); // 'm_vCameraCenter' should be in vehicle space
					}
				}
				
				// If we'll have multiple turrets, don't cache turret
				if (compartment)
					m_pTurretController = TurretControllerComponent.Cast(compartment.GetController());
				else
					m_pTurretController = compartmentAccess.GetCompartment().GetAttachedTurret();
				
				if (!m_pTurretController)
					return;

				m_pControlledTurret = m_pTurretController.GetTurretComponent();
				if (!m_pControlledTurret)
					return;
				
				m_pControlledTurret.GetAimingLimits(m_vHorAimLimits, m_vVertAimLimits);
				m_vVertAimLimits[0] = CONST_UD_MIN;
				m_vVertAimLimits[1] = CONST_UD_MAX;
			}
		}
	}
		
	override void CalculateLookAngles(vector vehicleAngles, vector characterAngles, out ScriptedCameraItemResult pOutResult)
	{
		vector lookAngles = m_CharacterHeadAimingComponent.GetLookAngles();
		
		CharacterControllerComponent charController = m_OwnerCharacter.GetCharacterController();
		
		//! apply to rotation matrix
		m_vLastCameraAngles = m_pControlledTurret.GetAimingDirectionWorld().VectorToAngles();
		if (charController.IsFreeLookEnabled() || m_pTurretController.GetCanAimOnlyInADS())
		{
			// Do not let gun elevation change view direction in freelook
			m_vLastCameraAngles[1] = 0;
			
			m_vLastCameraAngles += lookAngles;
		}
		Math3D.AnglesToMatrix(m_vLastCameraAngles - characterAngles, pOutResult.m_CameraTM);
	}
}
