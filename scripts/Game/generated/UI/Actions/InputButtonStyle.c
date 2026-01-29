/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_Actions
\{
*/

class InputButtonStyle: ScriptAndConfig
{
	[Attribute(desc:"ImageSet")]
	ResourceName m_sImageSet;

	[Attribute(desc:"GlowImageSet")]
	ResourceName m_sGlowImageSet;

	[Attribute(defvalue:"")]
	string m_sKeyBackground;

	[Attribute(defvalue:"")]
	string m_sKeyOutline;

	[Attribute(defvalue:"")]
	string m_sGlow;

	[Attribute(defvalue:"")]
	string m_sGlowOutline;

	[Attribute(defvalue:"2X")]
	string m_sDoubleTabText;

	[Attribute(defvalue:"rectangle-mask")]
	string m_sAlphaMask;

	[Attribute()]
	ResourceName m_sFont;

	[Attribute(defvalue:"3.5")]
	float m_fTextSizeModifier;

	[Attribute(defvalue:"2.0")]
	float m_fLineHeightModifier;

	[Attribute(defvalue:"0 0 0 0.5")]
	ref Color m_GlowColor;

	[Attribute(defvalue:"0.760 0.392 0.078 1")]
	ref Color m_BackgroundColor;

	[Attribute(defvalue:"0 0 0 1")]
	ref Color m_LabelColor;

	[Attribute(defvalue:"1 1 1 1")]
	ref Color m_DoubleTapColor;

	[Attribute(defvalue:"0.411 0.411 0.411 0.580")]
	ref Color m_OutlineColor;

}

/*!
\}
*/
