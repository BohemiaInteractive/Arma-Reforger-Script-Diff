//! Forward declaration of UIInfo
//! Serves as a container of data to be displayed on player UI
class MagazineUIInfo : UIInfo
{
	//! Magazine background icon, created for WeaponInfo HUD element
	[Attribute("", UIWidgets.EditBox, "Magazine ammunition caliber.", "")]
	protected string m_sAmmoCaliber;
	
	// Returns m_sAmmoCalibar if it's defined.
	string GetAmmoCaliber()
	{
		return m_sAmmoCaliber;
	}
	
	
	
	//! Magazine type string (AP, tracer, ball), as if it was written on this magazine. Used for Weapon Info UI.
	[Attribute("", UIWidgets.EditBox, "Magazine type (AP, Tracer, Ball, ...). Used for Weapon Info UI.", "")]
	protected string m_sAmmoType;
	
	// Returns m_sAmmoCalibar if it's defined. Otherwise returns magazine name from the inventory component.
	string GetAmmoType()
	{
		return m_sAmmoType;
	}

	// Magazine icon behaviour in weapon UI
	[Attribute("false", UIWidgets.CheckBox, "Show ammo type extra text next to ammo type icon(s) in Weapon Info UI.")]
	protected bool m_bShowAmmoTypeText;	
	
	bool ShowAmmoTypeText()
	{
		return m_bShowAmmoTypeText;
	}	
	
	//! Magazine ammo type flags, used for displaying ammo type icons on weapon UI	
	[Attribute("1", UIWidgets.Flags, "Set magazine ammo type flags.", "", ParamEnumArray.FromEnum(EAmmoType) )]
	EAmmoType m_eAmmoTypeFlags;
	
	EAmmoType GetAmmoTypeFlags()
	{
		return m_eAmmoTypeFlags;
	}
	
	[Attribute("", UIWidgets.Object)]
	ref SCR_MagazineIndicatorConfiguration m_MagIndicator;
};