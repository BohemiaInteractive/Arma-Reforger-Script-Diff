//! Material remap of debris spawned by damage system
[BaseContainerProps(), SCR_DamageRemapTitle()]
class SCR_DamageRemapData
{
	[Attribute("", UIWidgets.EditBox, "Name of the slot which should have its materials changed. Case sensitive.")]
	string m_sMaterialSlotName;

	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Material that will be remaped to slot defined in Material Slot Name parameter", params: "emat")]
	ResourceName m_sMaterialName;
}