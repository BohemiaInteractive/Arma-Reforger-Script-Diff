[BaseContainerProps(configRoot: true)]
class SCR_AIWeaponTypeSelectionConfig
{
	[Attribute("0", UIWidgets.ComboBox, "Weapon type", enums: ParamEnumArray.FromEnum(EWeaponType))]
	EWeaponType m_eWeaponType;
	
	[Attribute("0", UIWidgets.ComboBox, "Muzzle type", enums: ParamEnumArray.FromEnum(EMuzzleType))]
	EMuzzleType m_eMuzzleType;
	
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Behavior tree for this weapon and muzzle type", params: "bt")]
	ResourceName m_sBehaviorTree;
}