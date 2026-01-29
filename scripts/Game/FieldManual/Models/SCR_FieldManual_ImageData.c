[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField(propertyName: "m_sCaption", format: "Gallery Image: %1")]
class SCR_FieldManual_ImageData
{
	[Attribute(uiwidget: UIWidgets.ResourcePickerThumbnail, params: "edds imageset")]
	ResourceName m_Image;

	[Attribute(desc: "Only required if Image is an imageset")]
	string m_sImageName;

	[Attribute(defvalue: "1 1 1 1")]
	ref Color m_Color;

	[Attribute()]
	string m_sCaption;

	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EInputTypeCondition.ALL_INPUTS), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_EInputTypeCondition))]
	SCR_EInputTypeCondition m_eInputDisplayCondition;
}
