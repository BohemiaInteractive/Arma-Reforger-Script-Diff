class SCR_AIDecoHasWeaponOfType : DecoratorScripted
{
	static const string PORT_WEAPON_TYPE_IN = "WeaponTypeIn";
	static const string PORT_WEAPON_TYPE_OUT = "WeaponTypeOut";
	static const string PORT_MAGAZINE_WELL = "MagazineWell";
	
	[Attribute("1", UIWidgets.ComboBox, "Wanted weapon type", "", ParamEnumArray.FromEnum(EWeaponType) )]
	protected EWeaponType m_WeaponType;
	[Attribute("0", UIWidgets.CheckBox, "Check current weapon" )]
	protected bool m_bCurrent;
	
	private BaseWeaponManagerComponent m_WpnManager;
	private ref array<IEntity> m_Weapons;
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_WEAPON_TYPE_IN,
		PORT_MAGAZINE_WELL
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	
	
	protected static ref TStringArray s_aVarsOut = {
		PORT_WEAPON_TYPE_OUT
	};
	override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	override void OnInit(AIAgent owner)
	{
		m_Weapons = new array<IEntity>;
		IEntity contr = owner.GetControlledEntity();
		if (contr)
		{
			m_WpnManager = BaseWeaponManagerComponent.Cast(contr.FindComponent(BaseWeaponManagerComponent));		
		}		
	}
	
	protected override bool TestFunction(AIAgent owner)
	{
		if (!m_WpnManager)
			return false;
		
		typename magazineWell;
		bool findByMagazineWell,result;
		findByMagazineWell = GetVariableIn(PORT_MAGAZINE_WELL,magazineWell);
								
		EWeaponType weaponType;
		if (!GetVariableIn(PORT_WEAPON_TYPE_IN,weaponType))
			weaponType = m_WeaponType;
		
		if (m_bCurrent)
		{
			BaseWeaponComponent weapon = m_WpnManager.GetCurrentWeapon();			
			if (!weapon)
			{
			    if (!findByMagazineWell && weaponType == EWeaponType.WT_NONE)
			        return true;
			    else
			        return false;
			}
			
			if (findByMagazineWell)
			{
				result = magazineWell == SCR_AIGetMagazineWell.GetWeaponMainMagazineWell(weapon);
				if (result)
					SetVariableOut(PORT_WEAPON_TYPE_OUT, weapon.GetWeaponType());
				return result;
			}
			else if (weapon.GetWeaponType() == weaponType)
				return true;
			
			return false;
		}
		
		m_WpnManager.GetWeaponsList(m_Weapons);
		
		foreach (IEntity weapon : m_Weapons)
		{
			BaseWeaponComponent comp = BaseWeaponComponent.Cast(weapon.FindComponent(BaseWeaponComponent));
			
			if (findByMagazineWell)
			{
				result = magazineWell == SCR_AIGetMagazineWell.GetWeaponMainMagazineWell(comp);
				if (result)
				{
					SetVariableOut(PORT_WEAPON_TYPE_OUT, comp.GetWeaponType());
					return result;
				}					
			}	
			else if (comp.GetWeaponType() == weaponType)
			{
				return true;											
			}			
		}
		return false;			
	}	
		
	protected static override bool VisibleInPalette()
	{
		return true;
	}
	
	override protected string GetNodeMiddleText()
	{
		return "Weapon type: " + typename.EnumToString(EWeaponType,m_WeaponType) + "\nCurrent: " +  m_bCurrent.ToString();	
	}	
	
	protected static override string GetOnHoverDescription()
	{
		return "Decorator that tests if entity has a weapon specified in attribute either as weapon type or as magazine well, in that case it returns the weapon type of the weapon";
	}	
};