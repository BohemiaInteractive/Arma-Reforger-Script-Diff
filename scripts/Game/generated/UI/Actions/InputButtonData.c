/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_Actions
\{
*/

class InputButtonData: InputMappingKeyData
{
	[Attribute(desc:"Text displayed on Button")]
	string m_sText;

	[Attribute(desc:"Insert image names from defined ImageSet. Images will be displayed from top to bottom.")]
	ref array<ref SCR_ButtonTexture> m_aTextures;

	[Attribute(defvalue:"0", uiwidget : UIWidgets.SearchComboBox, desc : "Button type", enums : ParamEnumArray.FromEnum(SCR_EButtonSize))]
	SCR_EButtonSize m_eType;

}

/*!
\}
*/
