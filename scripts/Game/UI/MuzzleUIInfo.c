//! Forward declaration of UIInfo
//! Serves as a container of data to be displayed on player UI
class MuzzleUIInfo : UIInfo
{
	// Show caliber indicator in weapon UI
	[Attribute("false", UIWidgets.CheckBox, "Show caliber indicator in weapon UI.")]
	protected bool m_bShowCaliber;	

	bool ShowCaliber()
	{
		return m_bShowCaliber;
	}	
	
	//! Firemode texture - full auto
	[Attribute("", "auto", "Caliber identification, e.g. 7.62×39mm")]
	protected string m_sCaliber;		

	string GetCaliber()
	{
		return m_sCaliber;
	}		
		
	// Magazine icon behaviour in weapon UI
	[Attribute("true", UIWidgets.CheckBox, "Show firemode indicator.")]
	protected bool m_bShowFiremode;	

	bool ShowFiremodeIcon()
	{
		return m_bShowFiremode;
	}		

			
	//! Firemode imageset with foreground icons
	[Attribute("{CDA6C73DFD789999}UI/Imagesets/WeaponInfo/WeaponInfo.imageset", UIWidgets.ResourceNamePicker, "Imageset with most of weapon info textures", "imageset")]
	protected ResourceName m_sFiremodeIconImageset;

	ResourceName GetFiremodeIconImageset()
	{
		return m_sFiremodeIconImageset;
	}	
	
	//! Firemode imageset with glow icons
	[Attribute("{C6D15495F684E841}UI/Imagesets/WeaponInfo/WeaponInfo_Glow.imageset", UIWidgets.ResourceNamePicker, "Imageset with most of weapon info textures", "imageset")]
	protected ResourceName m_sFiremodeGlowImageset;	

	ResourceName GetFiremodeGlowImageset()
	{
		return m_sFiremodeGlowImageset;
	}	

	//! Firemode texture - single shot
	[Attribute("firemode-rifle-single", "auto", "Firemode indicator - single shot")]
	protected string m_sFiremodeSingle;

	//! Firemode texture - burst
	[Attribute("firemode-rifle-burst3", "auto", "Firemode indicator - burst")]
	protected string m_sFiremodeBurst;	

	//! Firemode texture - full auto
	[Attribute("firemode-rifle-auto", "auto", "Firemode indicator - full auto")]
	protected string m_sFiremodeAuto;	
			
	//! Firemode texture - safety
	[Attribute("firemode-safety", "auto", "Firemode indicator - safety", "edds")]
	protected string m_sFiremodeSafety;
	
	string GetFiremodeIconName(EWeaponFiremodeType firemode)
	{
		string icon = "";
		
		switch (firemode)
		{
			case EWeaponFiremodeType.Semiauto:
				icon = m_sFiremodeSingle;
				break;
			
			case EWeaponFiremodeType.Burst:
				icon = m_sFiremodeBurst;
				break;

			case EWeaponFiremodeType.Auto:
				icon = m_sFiremodeAuto;
				break;		
										
			case EWeaponFiremodeType.Safety:
				icon = 	m_sFiremodeSafety;
				break;
			
			default:
				icon = m_sFiremodeSingle;
				break;
		}		
		
		return icon;
	}
	
	[Attribute("", UIWidgets.Object)]
	ref SCR_MagazineIndicatorConfiguration m_MagIndicator;
};