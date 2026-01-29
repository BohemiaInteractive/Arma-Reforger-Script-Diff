[BaseContainerProps(configRoot: true)]
class SCR_FieldManualConfigRoot
{
	[Attribute(defvalue: "#AR-FieldManual_Name", uiwidget: UIWidgets.Auto, desc: "Field Manual title")]
	string m_sTitle;

	[Attribute(defvalue: "#AR-FieldManual_Category_Reforger_Title", uiwidget: UIWidgets.Auto, desc: "Field Manual Default tab name")]
	string m_sDefaultTabName;

	[Attribute(defvalue: "#AR-FieldManual_Category_Mods_Title", uiwidget: UIWidgets.Auto, desc: "Field Manual Mods tab name")]
	string m_sModsTabName;

	[Attribute(defvalue: "{5EA88E670200AC85}UI/layouts/Menus/FieldManual/MenuParts/FieldManual_EntryTile.layout", uiwidget: UIWidgets.ResourceNamePicker, desc: "Menu Entry layout", params: "layout")]
	ResourceName m_MenuEntryTileLayout;

	[Attribute(uiwidget: UIWidgets.Auto, desc: "Field Manual categories")]
	ref array<ref SCR_FieldManualConfigCategory> m_aCategories;

	[Attribute(defvalue: "", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Tile backgrounds from which randomly pick", params: "edds")]
	ref array<ResourceName> m_aTileBackgrounds;
}
