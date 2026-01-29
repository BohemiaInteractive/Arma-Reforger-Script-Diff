class SCR_OptimizedMuzzleEffectComponentClass : MuzzleEffectComponentClass
{
}

class SCR_OptimizedMuzzleEffectComponent : MuzzleEffectComponent
{
	protected bool m_bInScope;
	protected ref ScriptInvokerVoid m_OnWeaponFired;
	protected const string PREFIX_EMITOR_TO_DISABLE = "noscope";
	
	//------------------------------------------------------------------------------------------------
	static void OptimizeMuzzleEffect(bool inScope, TurretControllerComponent m_TurretController)
	{
		BaseWeaponManagerComponent weaponManager = m_TurretController.GetWeaponManager();
		
		if (!weaponManager)	
			return;

		array<WeaponSlotComponent> weaponSlots = {};
		weaponManager.GetWeaponsSlots(weaponSlots);

		BaseMuzzleComponent baseMuzzle;
		array<GenericComponent> comArray = new array<GenericComponent>();
		SCR_OptimizedMuzzleEffectComponent muzzleEffect;
		
		foreach (WeaponSlotComponent weaponSlot : weaponSlots)
		{
			baseMuzzle = weaponSlot.GetCurrentMuzzle();
			if (!baseMuzzle)
				continue;

			comArray.Clear();
			baseMuzzle.FindComponents(MuzzleEffectComponent, comArray);
			
			foreach (GenericComponent component : comArray)
			{
				muzzleEffect = SCR_OptimizedMuzzleEffectComponent.Cast(component);
				
				if (!muzzleEffect)
					continue;
				
				muzzleEffect.m_bInScope = inScope;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetOnWeaponFired()
	{
		if (!m_OnWeaponFired)
			m_OnWeaponFired = new ScriptInvokerVoid();
		
		return m_OnWeaponFired;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		if (m_OnWeaponFired)
			m_OnWeaponFired.Invoke();
		
		ParticleEffectEntity particleEntity = ParticleEffectEntity.Cast(effectEntity);
				
		if (!particleEntity)
			return;
		
		Particles particles = particleEntity.GetParticles();
		
		if (!particles)
			return;
		
		array<string> names = {};
		particles.GetEmitterNames(names);
		
		foreach (int idx, string name : names)
		{
			if (name.Contains(PREFIX_EMITOR_TO_DISABLE))
			{
				if (m_bInScope)
					particles.MultParam(idx, EmitterParam.BIRTH_RATE, 0);
				else
					particles.MultParam(idx, EmitterParam.BIRTH_RATE, 1);
			}
		}
	}
}
