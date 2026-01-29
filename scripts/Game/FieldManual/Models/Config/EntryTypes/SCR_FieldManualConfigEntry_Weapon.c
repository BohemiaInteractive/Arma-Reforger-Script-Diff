[BaseContainerProps(), SCR_BaseContainerCustomTitleResourceName("m_Entity", true, "Weapon: %1")]
class SCR_FieldManualConfigEntry_Weapon : SCR_FieldManualConfigEntry
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Targeted entity", params: "et")]
	ResourceName m_sWeaponEntityPath;

	[Attribute(defvalue: "{C5EBC2B28BC84852}UI/layouts/Menus/FieldManual/Entries/FieldManual_Entry_Weapon.layout", uiwidget: UIWidgets.EditBoxWithButton, params: "layout")]
	ResourceName m_Layout;

	ref SCR_FieldManualUI_WeaponStatsHelper m_WeaponStatsHelper;

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_FieldManualConfigEntry_Weapon()
	{
		if (m_Layout.Trim().IsEmpty())
			m_Layout = "{C5EBC2B28BC84852}UI/layouts/Menus/FieldManual/Entries/FieldManual_Entry_Weapon.layout";
	}

	//------------------------------------------------------------------------------------------------
	override Widget CreateWidget(notnull Widget parent)
	{
		return CreateWidgetFromLayout(m_Layout, parent);
	}
}
