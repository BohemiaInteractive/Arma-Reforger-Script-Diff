class SCR_BaseWeaponAction : SCR_InventoryAction
{
	const string PRIMARY_WEAPON_TYPE = "primary";
	const string SECONDARY_WEAPON_TYPE = "secondary";
	const string GRENADE_WEAPON_TYPE = "grenade";
	
	protected bool m_bIsSwappingWeapons = false;
	protected string m_sWeaponToSwapName = "";
	protected BaseWeaponComponent m_pWeaponOnGroundComponent;
	protected ref array<WeaponSlotComponent> m_aWeaponSlots = {};
	protected string m_sWeaponOnGroundType = "";
	protected int m_iSameTypeSlotsCount = 0;
	
	//------------------------------------------------------------------------------------------------
	// Returns true if the equip or equip holster action can be perfomed. Also modifies the text depending on the conditions
	bool CanEquipOrReplaceWeapon( IEntity userEntity )
	{
		ChimeraCharacter user = ChimeraCharacter.Cast( userEntity );
		if ( !user )
			return false;
		
		BaseWeaponManagerComponent weaponManager = user.GetCharacterController().GetWeaponManagerComponent();
		if ( !weaponManager )
			return false;
			
		m_iSameTypeSlotsCount = 0;
		m_bIsSwappingWeapons = false;
		m_sWeaponToSwapName = "";
		
		// Eventually the equip actions will be removed for the grenades as whole
		if ( m_sWeaponOnGroundType == GRENADE_WEAPON_TYPE )
			return true;
		
		m_aWeaponSlots.Clear();
		weaponManager.GetWeaponsSlots( m_aWeaponSlots );
		
		string weaponSlotType;
		
		// If there is an empty slot of the same type as the weapon on the ground we are able to equip the weapon without replacing anything
		foreach ( WeaponSlotComponent weaponSlot: m_aWeaponSlots )
		{
			weaponSlotType = weaponSlot.GetWeaponSlotType();
			
			if ( weaponSlotType != m_sWeaponOnGroundType && !CharacterHandWeaponSlotComponent.Cast(weaponSlot) )
				continue;
					
			m_iSameTypeSlotsCount++;
			
			if ( !weaponSlot.GetWeaponEntity() )
				return true;
		}
		
		BaseWeaponComponent weaponToSwap = GetWeaponToSwap( weaponManager );
		
		if ( !weaponToSwap )
			return false;
			
		m_bIsSwappingWeapons = true;
		
		UIInfo weaponToSwapUIInfo = weaponToSwap.GetUIInfo();
		
		// If the component is missing configuration we can fall back on the weapon slot name.
		if ( weaponToSwapUIInfo )
		{
			string weaponToSwapName = weaponToSwapUIInfo.GetName();
			
			if ( !weaponToSwapName.IsEmpty() )
				m_sWeaponToSwapName = weaponToSwapName;	
		}
		else
			m_sWeaponToSwapName = m_sWeaponOnGroundType;

		return true;	
	}
	
	//------------------------------------------------------------------------------------------------
	// Gets all the variables that are constant for the entire duration.
	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		if ( !m_Item )
		{
			Print( "Pick up/equip UserAction is attached to an object which is not an item" , LogLevel.WARNING );
			return;
		}
		
		m_pWeaponOnGroundComponent = BaseWeaponComponent.Cast( m_Item.GetOwner().FindComponent( BaseWeaponComponent ) );
		
		if ( !m_pWeaponOnGroundComponent )
			return;
		m_sWeaponOnGroundType = m_pWeaponOnGroundComponent.GetWeaponSlotType();
	}
	

	//------------------------------------------------------------------------------------------------
	// Returns the weapon that will be swapped.
	BaseWeaponComponent GetWeaponToSwap( notnull BaseWeaponManagerComponent weaponManager )
	{
        BaseWeaponComponent currentWeapon = weaponManager.GetCurrentWeapon();
            
		// The equipped weapon will be swapped, if it's the same type as the one on the ground.
        if (currentWeapon)
        {
        	if ( currentWeapon.GetWeaponSlotType() == m_sWeaponOnGroundType )
				return currentWeapon;
        }
		
		BaseWeaponComponent weaponToSwap;
		string weaponSlotType;
		
		// First compatible slot will be used if the equipped weapon doesn't match.
		foreach ( WeaponSlotComponent weaponSlot: m_aWeaponSlots )
		{
			weaponSlotType = weaponSlot.GetWeaponSlotType();
			
			if ( weaponSlotType == m_sWeaponOnGroundType )
			{
				weaponToSwap = BaseWeaponComponent.Cast( weaponSlot.GetWeaponEntity().FindComponent( BaseWeaponComponent ) );
				if ( weaponToSwap )
					return weaponToSwap;
			}
		}

		return null;
	}
};