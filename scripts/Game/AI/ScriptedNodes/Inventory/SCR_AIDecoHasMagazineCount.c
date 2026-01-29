enum EVariableTestType
{
	EqualsValue,
	BiggerThanValue,
	SmallerThanValue,
	WasChanged
};

class SCR_AIDecoHasMagazineCount : DecoratorScripted
{
	static const string PORT_MAGAZINE_COUNT = "MagazineCountOut";
	
	[Attribute("0", UIWidgets.ComboBox, "Test Type", "", ParamEnumArray.FromEnum(EVariableTestType) )]
	protected EVariableTestType m_TestType;
	[Attribute("0", UIWidgets.EditBox, "Test Value" )]
	protected int m_iNumberOfMagazines;
		
	protected SCR_InventoryStorageManagerComponent m_inventoryManager;
	protected BaseWeaponComponent m_weapon;
	protected int prevAmmo;
	
	IEntity m_entity;
		
	protected static ref TStringArray s_aVarsOut = {
		PORT_MAGAZINE_COUNT
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	override void OnInit(AIAgent owner)
	{
		m_entity = owner.GetControlledEntity();					
	}
	
	protected override bool TestFunction(AIAgent owner)
	{
		if (m_entity && !m_weapon)
		{
			m_inventoryManager = SCR_InventoryStorageManagerComponent.Cast(m_entity.FindComponent(SCR_InventoryStorageManagerComponent));
			BaseWeaponManagerComponent weaponManager = BaseWeaponManagerComponent.Cast(m_entity.FindComponent(BaseWeaponManagerComponent));
			if (weaponManager)
				m_weapon = weaponManager.GetCurrent();					
		}
		
		if (!m_inventoryManager || !m_weapon)
			return false;
		int ammo = m_inventoryManager.GetMagazineCountByWeapon(m_weapon);
		bool result = false;
		switch (m_TestType)
		{
			case EVariableTestType.EqualsValue: 
			{
				result = ammo == m_iNumberOfMagazines;
				break;
			}
			case EVariableTestType.BiggerThanValue:
			{
				result = ammo > m_iNumberOfMagazines;
				break;
			}
			case EVariableTestType.SmallerThanValue:
			{
				result = ammo < m_iNumberOfMagazines;
				break;
			}
			case EVariableTestType.WasChanged:
			{
				result = ammo != prevAmmo;
				prevAmmo = ammo;
				break;
			}		
		}
		SetVariableOut(PORT_MAGAZINE_COUNT, ammo);
		return result;			
	}	
		
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	override protected string GetNodeMiddleText()
	{
		return "Checks: " + typename.EnumToString(EVariableTestType,m_TestType) + " with value: " +  m_iNumberOfMagazines.ToString();	
	}	
	
	protected static override string GetOnHoverDescription()
	{
		return "Decorator that tests if entity has a weapon specified in attribute either as weapon type or as magazine well, in that case it returns the weapon type of the weapon";
	}	
};