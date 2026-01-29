[BaseContainerProps(configRoot: true), SCR_BaseContainerCustomTitleEnum(EWeaponType, "m_eWeaponType")]
class SCR_AIWeaponTypeHandlingConfig
{
	static const float DEFAULT_LOW_MAG_THRESHOLD = 1;
	static const float DEFAULT_STABILIZATION_TIME = 0.4;
	static const float DEFAULT_REJECTION_TIME = 1;
	
	[Attribute("0", UIWidgets.ComboBox, "Type of weapon this config will be used for", "", ParamEnumArray.FromEnum(EWeaponType))]
	EWeaponType m_eWeaponType;
	
	[Attribute(DEFAULT_LOW_MAG_THRESHOLD.ToString(), UIWidgets.EditBox, "Magazine count that AI should consider as low")]
	int m_iLowMagCountThreshold;
	
	[Attribute(DEFAULT_LOW_MAG_THRESHOLD.ToString(), UIWidgets.EditBox, "Minimal magazine count, including current magazine, which AI should consider usable in suppressive fire. Ex.: 2 - leave 1 mag, 3 - leave 2 mags.")]
	int m_iMinSuppressiveMagCountThreshold;
	
	[Attribute(DEFAULT_STABILIZATION_TIME.ToString(), UIWidgets.EditBox, "Time to aim, before taking a shot, used as base value for fire pattern randomization")]
	float m_fBaseStabilizationTime;
	
	[Attribute(DEFAULT_REJECTION_TIME.ToString(), UIWidgets.EditBox, "Timeout for shot attept")]
	float m_fBaseRejectionTime;
}