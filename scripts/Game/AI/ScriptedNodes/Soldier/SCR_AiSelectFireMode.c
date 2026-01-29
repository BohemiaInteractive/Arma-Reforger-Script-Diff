class SCR_AISelectFireMode: AITaskScripted
{
	[Attribute("0", UIWidgets.ComboBox, "Try set Firemode", "", ParamEnumArray.FromEnum(EWeaponFiremodeType) )]
	protected EWeaponFiremodeType m_FiremodeType;
	
	//-----------------------------------------------------------------------------------------------------------------------------
    override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
    {
		GenericEntity controlledEntity = GenericEntity.Cast(owner.GetControlledEntity());
		if (!controlledEntity)
			return ENodeResult.FAIL;
		
		//--------------------------------------------------------------------------
		// Resolve which weapon manager to use
		
		BaseWeaponManagerComponent wpnManagerComponent;
		CharacterControllerComponent controller;
		
		CompartmentAccessComponent compAccess = CompartmentAccessComponent.Cast(controlledEntity.FindComponent(CompartmentAccessComponent));
		if (!compAccess)
			return ENodeResult.FAIL;
		
		BaseCompartmentSlot compartment = compAccess.GetCompartment();
		if (compartment)
		{
			wpnManagerComponent = BaseWeaponManagerComponent.Cast(compartment.GetOwner().FindComponent(BaseWeaponManagerComponent));
			if (wpnManagerComponent)
			{
				// Return success if in turret. Turret weapons have no fire modes anyway.
				return ENodeResult.SUCCESS;
			}
		}
		else
		{
			controller = CharacterControllerComponent.Cast(controlledEntity.FindComponent(CharacterControllerComponent));
			if (controller)
				wpnManagerComponent = controller.GetWeaponManagerComponent();
		}
		
		if (!wpnManagerComponent || !controller)
			return ENodeResult.FAIL;
		
		BaseWeaponComponent wpnComponent = wpnManagerComponent.GetCurrent();
		if (!wpnComponent)
			return ENodeResult.FAIL;
		
		// Bail if we already have the desired fire mode
		if (wpnComponent.GetCurrentFireModeType() == m_FiremodeType)
			return ENodeResult.SUCCESS;
		
		BaseMuzzleComponent muzzle = wpnComponent.GetCurrentMuzzle();
		
		// Fail if there is no muzzle, it is probably a grenade
		if (!muzzle)
			return ENodeResult.FAIL;		
		
		//--------------------------------------------------------------------------
		// Find available firemodes
		
		array<BaseFireMode> fireModes = {};
		muzzle.GetFireModesList(fireModes);
		
		int fireModesCount = fireModes.Count();
		int semiAutoId = -1, autoId = -1, burstId = -1, manualId = -1, safetyId = -1;
		for (int i = 0; i<fireModesCount; i++)
		{
			switch (fireModes[i].GetFiremodeType())
			{
				case EWeaponFiremodeType.Semiauto : 
				{		 
					semiAutoId = i;
					break;
				}
				case EWeaponFiremodeType.Auto :
				{
					autoId = i;
					break;
				}
				case EWeaponFiremodeType.Burst :
				{
					burstId = i;
					break;
				}
				case EWeaponFiremodeType.Safety :
				{
					safetyId = i;
					break;
				}
				case EWeaponFiremodeType.Manual:
				{
					manualId = i;
					break;
				}
			}
		}
		
		//--------------------------------------------------------------------------
		// Try to best suitable firemode
		// If a non-safety firemode is requested, we will still try to find any non-safety firemode,
		// to ensure weapon is not stuck in safety
		
		int bestFm = -1;
		switch (m_FiremodeType)
		{
			case EWeaponFiremodeType.Auto:
			{
				// auto > burst > semi > manual
				if (autoId != -1)
					bestFm = autoId;
				else if (burstId != -1)
					bestFm = burstId;
				else if (semiAutoId != -1)
					bestFm = semiAutoId;
				else if (manualId != -1)
					bestFm = manualId;
				
				break;
			}
			
			case  EWeaponFiremodeType.Burst:
			{
				// burst > auto > semi > manual
				if (burstId != -1)
					bestFm = burstId;
				else if (autoId != -1)
					bestFm = autoId;
				else if (semiAutoId != -1)
					bestFm = semiAutoId;
				else if (manualId != -1)
					bestFm = manualId;
				
				break;
			}
			
			case EWeaponFiremodeType.Semiauto:
			{
				// semi > manual > burst > auto
				if (semiAutoId != -1)
					bestFm = semiAutoId;
				else if (manualId != -1)
					bestFm = manualId;
				else if (burstId != -1)
					bestFm = burstId;
				else if (autoId != -1)
					bestFm = autoId;
				
				break;
			}
			
			case EWeaponFiremodeType.Safety:
			{
				// safety
				if (safetyId != -1)
					bestFm = safetyId;
				
				break;
			}
		}
		
		//--------------------------------------------------------------------------
		// Set found firemode or report error
		
		if (bestFm == -1)
		{
			// We couldn't find desired firemode or adequate replacement, log a warning
			
			WeaponSlotComponent weaponSlotComp = WeaponSlotComponent.Cast(wpnComponent);			
			IEntity weaponEntity;
			if (weaponSlotComp)
				weaponEntity = weaponSlotComp.GetWeaponEntity();
			else
				weaponEntity = wpnComponent.GetOwner();
			
			string prefabName;
			if (weaponEntity)
				prefabName = weaponEntity.GetPrefabData().GetPrefabName();
			
			string str = string.Format("SCR_AISelectFireMode: proper fire mode was not found: %1. Weapon prefab: %2",
				typename.EnumToString(EWeaponFiremodeType, m_FiremodeType),
				prefabName);
			
			Print(str, LogLevel.WARNING);
			
			return ENodeResult.FAIL;
		}
		
		controller.SetFireMode(bestFm);
		return ENodeResult.SUCCESS;
    }
		
	override protected string GetNodeMiddleText()
	{
		return "Selects a firemode closest to provided one.";
	}
	
    static override bool VisibleInPalette() {return true;}
};