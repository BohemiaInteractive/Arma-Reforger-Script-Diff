/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_Actions
\{
*/

class ButtonTexture: ScriptAndConfig
{
	[Attribute(desc:"Name of Texture in imageset. Can be empty!")]
	string m_sTexture;

	[Attribute(defvalue:"0.000000 0.000000 0.000000 1.000000", UIWidgets.ColorPicker, desc : "Overrides the image color. Default: Black")]
	ref Color m_Color;

	[Attribute(defvalue:"false", UIWidgets.CheckBox, desc : "Enable to let the texture have a shadow. This can increase visivility of the texture")]
	bool m_bHasShadow;

}

/*!
\}
*/
