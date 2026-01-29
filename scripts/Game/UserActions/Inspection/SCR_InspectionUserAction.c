class SCR_InspectionUserAction : SCR_InventoryAction
{
	protected BaseWeaponComponent m_WeaponComponent;

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_WeaponComponent)
			return false;

		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return charComp.GetInspect();
	}

	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_WeaponComponent = WeaponComponent.Cast(pOwnerEntity.FindComponent(WeaponComponent));
		// If there is no WeaponComponent, this might be an attachment. Check up the parent chain
		if (!m_WeaponComponent)
		{
			IEntity parent = pOwnerEntity.GetParent();
			while (parent) 
			{
				m_WeaponComponent = WeaponComponent.Cast(parent.FindComponent(WeaponComponent));
				if (m_WeaponComponent)
					parent = null;
				else
					parent = parent.GetParent();
			}
		}
	}
};