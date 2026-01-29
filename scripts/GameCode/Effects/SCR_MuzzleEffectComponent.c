class SCR_MuzzleEffectComponentClass : MuzzleEffectComponentClass
{
};

void OnWeaponFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity);
typedef func OnWeaponFired;
typedef ScriptInvokerBase<OnWeaponFired> OnItemWeaponFiredInvoker;

class SCR_MuzzleEffectComponent : MuzzleEffectComponent
{
	protected ref OnItemWeaponFiredInvoker m_OnWeaponFired;

	OnItemWeaponFiredInvoker GetOnWeaponFired()
	{
		if (!m_OnWeaponFired)
			m_OnWeaponFired = new OnItemWeaponFiredInvoker();

		return m_OnWeaponFired;
	}

	override void OnFired(IEntity effectEntity, BaseMuzzleComponent muzzle, IEntity projectileEntity)
	{
		if (m_OnWeaponFired)
			m_OnWeaponFired.Invoke(effectEntity, muzzle, projectileEntity);
	}
}