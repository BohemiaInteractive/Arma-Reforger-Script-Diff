class SCR_WeaponComponentClass : WeaponComponentClass
{
}

void ScriptInvokerWeaponStateMethod(WeaponComponent weapon, bool active);
typedef func ScriptInvokerWeaponStateMethod;
typedef ScriptInvokerBase<ScriptInvokerWeaponStateMethod> ScriptInvokerWeaponState;

class SCR_WeaponComponent : WeaponComponent
{
	protected ref ScriptInvokerWeaponState m_OnWeaponStateChanged;

	//------------------------------------------------------------------------------------------------
	ScriptInvokerWeaponState GetOnWeaponStateChanged()
	{
		if (!m_OnWeaponStateChanged)
			m_OnWeaponStateChanged = new ScriptInvokerWeaponState();

		return m_OnWeaponStateChanged;
	}

	//------------------------------------------------------------------------------------------------
	override event void OnWeaponActive()
	{
		super.OnWeaponActive();

		if (m_OnWeaponStateChanged)
			m_OnWeaponStateChanged.Invoke(this, true);
	}

	//------------------------------------------------------------------------------------------------
	override event void OnWeaponInactive()
	{
		super.OnWeaponInactive();

		if (m_OnWeaponStateChanged)
			m_OnWeaponStateChanged.Invoke(this, false);
	}
}
