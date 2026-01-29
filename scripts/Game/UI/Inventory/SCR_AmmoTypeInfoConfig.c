[BaseContainerProps(configRoot: true)]
class SCR_AmmoTypeInfoConfig
{
	[Attribute("{A37CF52DBA874559}UI/Imagesets/WeaponInfo/WeaponInfo_Ammo.imageset")]
	ResourceName m_sImagesetPath;

	[Attribute("{C58A75ADB0F471D3}UI/Imagesets/WeaponInfo/WeaponInfo_Ammo_Glow.imageset")]	
	ResourceName m_sGlowImagesetPath;
	
	[Attribute()]
	ref array<ref SCR_AmmoTypeInfoConfigEntry> m_aAmmoTypes;

	//------------------------------------------------------------------------------------------------	
	void SetIconAndDescriptionTo(EAmmoType ammoType, ImageWidget image, TextWidget text)
	{
		foreach (SCR_AmmoTypeInfoConfigEntry info : m_aAmmoTypes)
		{
			if (info.m_eAmmoType == ammoType)
			{
				image.LoadImageFromSet(0, m_sImagesetPath, info.m_sQuadName);
				text.SetText(info.m_sDescription);
				break;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIconAndGlowTo(EAmmoType ammoType, notnull ImageWidget icon, notnull ImageWidget glow)
	{
		foreach (SCR_AmmoTypeInfoConfigEntry info : m_aAmmoTypes)
		{
			if (info.m_eAmmoType == ammoType)
			{
				icon.LoadImageFromSet(0, m_sImagesetPath, info.m_sQuadName);
				glow.LoadImageFromSet(0, m_sGlowImagesetPath, info.m_sQuadName);
				break;
			}
		}
	}
}

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(EAmmoType, "m_eAmmoType")]
class SCR_AmmoTypeInfoConfigEntry
{
	[Attribute("0", UIWidgets.ComboBox, "Ammo type", "", ParamEnumArray.FromEnum(EAmmoType))]
	EAmmoType m_eAmmoType;

	[Attribute("")]
	string m_sQuadName;
	
	[Attribute("")]
	LocalizedString m_sDescription;
}