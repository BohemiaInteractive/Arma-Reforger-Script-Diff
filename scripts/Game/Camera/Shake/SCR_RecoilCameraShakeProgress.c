/*!
	This object allows advancing and simulating individual camera shake(s).
*/
class SCR_RecoilCameraShakeProgress : SCR_GenericCameraShakeProgress
{
	protected float m_fProgress;
	protected float m_fVelocity;
	
	
	protected float m_fImpulseTime;
	protected float m_fLastImpulseTime;
	
	protected vector m_vImpulseAngular;
	
	override bool IsRunning()
	{
		return true;
	}
	
	protected SCR_RecoilCameraShakeParams GetParams(ChimeraCharacter character)
	{
		BaseWeaponManagerComponent weaponManager = null;
		
		if (character.IsInVehicle())
		{
			CompartmentAccessComponent compartmentAccess = character.GetCompartmentAccessComponent();
			// Fallback to default VehicleCamera behavior if getting out
			if (!compartmentAccess || !compartmentAccess.IsInCompartment() || compartmentAccess.IsGettingOut() || compartmentAccess.IsGettingIn())
				return null;
			
			BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
			if (!compartment)
				return null;
			
			TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
			if (!turretController)
				return null;
			
			weaponManager = turretController.GetWeaponManager();
		}
		else
		{
			weaponManager = character.GetCharacterController().GetWeaponManagerComponent();
		}
		
		if (!weaponManager)
			return null;
		
		BaseWeaponComponent weapon = weaponManager.GetCurrentWeapon();
		if (!weapon)
			return null;
		
		IEntity weaponEntity = weapon.GetOwner();
		if (!weaponEntity)
			return null;
		
		InventoryItemComponent item = InventoryItemComponent.Cast(weaponEntity.FindComponent(InventoryItemComponent));
		if (!item)
			return null;
		
		return SCR_RecoilCameraShakeParams.Cast(item.FindAttribute(SCR_RecoilCameraShakeParams));
	}

	override void Update(IEntity owner, float timeSlice)
	{
		ChimeraCharacter ownerCharacter = ChimeraCharacter.Cast(owner);
		if (!ownerCharacter)
		{
			if (IsRunning())
				Clear();
			
			return;
		}
		
		// Recoil-driven shake is based on the translation of weapon
		// in its model space, fetch the data
		vector recoilTranslation;
		if (ownerCharacter.IsInVehicle())
		{
			CompartmentAccessComponent compartmentAccess = ownerCharacter.GetCompartmentAccessComponent();
			if (compartmentAccess && compartmentAccess.IsInCompartment() && !compartmentAccess.IsGettingOut() && !compartmentAccess.IsGettingIn())
			{
				BaseCompartmentSlot compartment = compartmentAccess.GetCompartment();
				if (compartment)
				{
					TurretControllerComponent turretController = TurretControllerComponent.Cast(compartment.GetController());
					if (turretController)
					{
						TurretComponent turretComponent = turretController.GetTurretComponent();
						if(turretComponent)
							recoilTranslation = turretComponent.GetCurrentRecoilTranslation();
					}
				}
			}
		}
		else
		{
			AimingComponent aimingComponent = ownerCharacter.GetWeaponAimingComponent();
			if (aimingComponent)
				recoilTranslation = aimingComponent.GetCurrentRecoilTranslation();
		}
		
		

		// These values seem to be good enough as defaults,
		// but fetching it from prefab data of weapon (item) is of course preferable
		float recoilTarget = 0.0;
		float maxPercentage = 1.0;
		float blendIn = 0.01;
		float blendOut = 0.2;
		float maxVelocity = 10.0;
		float minRate = 0.1;
		float minThreshold = 0.1;
		float impulseStanceMagnitude = 1.0;
		float continuousStanceMagnitude = 1.0;
		SCR_RecoilCameraShakeParams params = GetParams(ownerCharacter);
		if (params)
		{
			recoilTarget = params.m_fRecoilTarget;
			maxPercentage = params.m_fMaximumPercentage;
			blendIn = params.m_fBlendInTime;
			blendOut = params.m_fBlendOutTime;
			maxVelocity = params.m_fMaxVelocity;
			minRate = params.m_fMinImpulseRate;
			minThreshold = params.m_fMinImpulseThreshold;
			float dynStance = ownerCharacter.GetCharacterController().GetDynamicStance();
			impulseStanceMagnitude = params.GetStanceImpulseMagnitude(dynStance);
			continuousStanceMagnitude = params.GetStanceMagnitude(dynStance);
		}
		
		// Based on the target translation, we convert our recoil amount into a [0,1] range
		float recoil01;
		if (recoilTarget > 0.0) // prevent div by 0
			recoil01 = Math.AbsFloat(recoilTranslation[2] / recoilTarget);
		
		// We scale the recoil value so it's not completely linear
		// and we make sure that we do not overshoot desired percentage applied
		float target = Math.Clamp(Math.Pow(recoil01, 2), 0.0, maxPercentage);
		float smoothTime;
		if (target >= m_fProgress)
			smoothTime = blendIn;
		else
			smoothTime = blendOut;

		// This value is the immediate value [0,1] of continuous shake applied
		m_fProgress = Math.SmoothCD(m_fProgress, target, m_fVelocity, smoothTime, maxVelocity, timeSlice);

		// When the change and time since last impulse is great enough, we can
		// trigger a large impulse that does not change that often as the underlying shake		
		m_fImpulseTime += timeSlice;
		if (m_fProgress - minThreshold > target && m_fImpulseTime > m_fLastImpulseTime + minRate)
		{
			m_fLastImpulseTime = m_fImpulseTime;
			m_vImpulseAngular = vector.Zero;
			if (params)
			{
				m_vImpulseAngular = params.GetRandomYawPitchRollImpulse();
			}
		}
		
		// Interpolate impulse towards zero, clearing it out
		// if value is set previously, the impulse just goes in different direction,
		// which is completely acceptable as shake is concerned
		float impulseTime = Math.Clamp((m_fImpulseTime - m_fLastImpulseTime) * 0.25, 0, 1);		
		m_vImpulseAngular = vector.Lerp(m_vImpulseAngular, vector.Zero, impulseTime);
		
		if (params)
		{
			m_vTranslation = m_fProgress * continuousStanceMagnitude * params.GetRandomTranslation();
			m_vRotation =  m_fProgress * impulseStanceMagnitude * m_vImpulseAngular;
			m_fFovScale = params.GetFovScale(m_fProgress);
		}
		else if (m_fProgress > 0.0)
		{
			m_vImpulseAngular = vector.Lerp(m_vImpulseAngular, vector.Zero, m_fProgress);
			m_vTranslation = vector.Lerp(m_vTranslation, vector.Zero, m_fProgress);
			m_vRotation = vector.Lerp(m_vRotation, vector.Zero, m_fProgress);
			m_fFovScale = Math.Lerp(m_fFovScale, 1.0, m_fProgress);
			
		}
	}
};