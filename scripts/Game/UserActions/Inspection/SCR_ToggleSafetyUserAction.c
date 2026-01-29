class SCR_ToggleSafetyUserAction : SCR_InspectionUserAction
{
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(EMuzzleType))]
	protected EMuzzleType m_eMuzzleType;
	
	protected WeaponAttachmentsStorageComponent m_AttachmentStorage;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(pUserEntity.FindComponent(CharacterControllerComponent));
		charComp.SetSafety(!IsSafetyOn(), false);
	}

	override bool CanBeShownScript(IEntity user)
	{
		if (!m_WeaponComponent)
		{
			if (m_AttachmentStorage)
			{
				InventoryStorageSlot slot = m_AttachmentStorage.GetParentSlot();
				if (!slot)
					return false;

				IEntity weapon = m_AttachmentStorage.GetParentSlot().GetOwner();
				if (weapon)
					m_WeaponComponent = WeaponComponent.Cast(weapon.FindComponent(WeaponComponent));
			}

			return false;
		}

		if (m_WeaponComponent.GetCurrentMuzzle() == NULL)
			return false;
		if (m_WeaponComponent.GetCurrentMuzzle().GetMuzzleType() != m_eMuzzleType)
			return false;

		CharacterControllerComponent charComp = CharacterControllerComponent.Cast(user.FindComponent(CharacterControllerComponent));
		return (charComp && charComp.GetInspect());
	}	

	bool IsSafetyOn()
	{
		BaseMuzzleComponent muzzle = m_WeaponComponent.GetCurrentMuzzle();
		return (muzzle.GetCurrentFireMode().GetFiremodeType() == EWeaponFiremodeType.Safety);
	}
	
	override bool GetActionNameScript(out string outName)
	{
		if (IsSafetyOn())
		{
			outName = WidgetManager.Translate("#AR-Keybind_WeaponSafety (%1)", "#AR-UserAction_State_On-UC");
		}
		else
		{
			outName = WidgetManager.Translate("#AR-Keybind_WeaponSafety (%1)", "#AR-UserAction_State_Off-UC");
		}
		return true;
	}
	
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		if (m_eMuzzleType == EMuzzleType.MT_BaseMuzzle)
		{
			m_WeaponComponent = WeaponComponent.Cast(pOwnerEntity.FindComponent(WeaponComponent));		
		}
		else if (m_eMuzzleType == EMuzzleType.MT_UGLMuzzle)
		{
			// not ideal, but we cannot get parent slot in init and have to retrieve it later
			m_AttachmentStorage = WeaponAttachmentsStorageComponent.Cast(pOwnerEntity.FindComponent(WeaponAttachmentsStorageComponent));
		}
	}	
};