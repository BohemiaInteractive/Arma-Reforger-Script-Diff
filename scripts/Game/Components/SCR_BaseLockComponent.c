[EntityEditorProps(category: "GameScripted/Components", description: "Base lock component.")]
class SCR_BaseLockComponentClass : ScriptComponentClass
{
}

class SCR_BaseLockComponent : ScriptComponent
{
	protected SCR_VehicleSpawnProtectionComponent m_pVehicleSpawnProtection;

	[Attribute()]
	protected bool m_bIsLocked;

	//------------------------------------------------------------------------------------------------
	void SetLocked(bool locked)
	{
		m_bIsLocked = locked;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_pVehicleSpawnProtection = SCR_VehicleSpawnProtectionComponent.Cast(owner.FindComponent(SCR_VehicleSpawnProtectionComponent));
		ClearEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] user
	//! \return
	LocalizedString GetCannotPerformReason(IEntity user)
	{
		if (user && m_pVehicleSpawnProtection)
			return m_pVehicleSpawnProtection.GetReasonText(user);

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] user
	//! \param[in] compartmentSlot
	//! \return
	bool IsLocked(IEntity user, BaseCompartmentSlot compartmentSlot)
	{
		if (m_bIsLocked)
			return true;

		if (!user || !compartmentSlot)
			return false;

		if (m_pVehicleSpawnProtection)
			return m_pVehicleSpawnProtection.IsProtected(user, compartmentSlot);

		return false;
	}
}
