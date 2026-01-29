[BaseContainerProps(), BaseContainerCustomTitleField("m_sWeaponGroupName")]
class SCR_WeaponGroup
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Name of the weapons group to show the user")]
	string m_sWeaponGroupName;

	[Attribute(desc: "Which Pylons are part of this group. Determined by weapon slot index set on WeaponSlotComponent", params: "0 inf")]
	ref array<int> m_aWeaponsGroupIds;
		
	[Attribute(SCR_EWeaponGroupFireMode.SALVO.ToString(), uiwidget: UIWidgets.Flags, desc: "", enumType: SCR_EWeaponGroupFireMode)]
	SCR_EWeaponGroupFireMode m_eFireMode;
	
	[Attribute(desc: "Which quantities of launches can the ripple firemode fire", params: "0 inf")]
	ref array<int> m_aRippleFireQuantities;
}