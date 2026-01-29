/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup UI_Actions
\{
*/

class InputButtonLayoutConfig: ScriptAndConfig
{
	[Attribute()]
	protected ref array<ref SCR_InputButtonLayoutEntry> m_aLayoutTypes;

	[Attribute()]
	ResourceName m_sSymbolsImageSet;

	[Attribute()]
	ResourceName m_sSymbolsImageSetGlow;

	[Attribute()]
	string m_sSumImage;

	[Attribute(defvalue:"2.0")]
	float m_fSumScale;

	[Attribute(defvalue:"0.2")]
	float m_fSumOverlap;

	[Attribute()]
	string m_sComboImage;

	[Attribute(defvalue:"1.0")]
	float m_fComboScale;

	[Attribute(defvalue:"0.2")]
	float m_fComboOverlap;

	[Attribute(defvalue:"0.760 0.392 0.078 1")]
	ref Color m_SymbolsColor;

	[Attribute(defvalue:"0 0 0 0.5")]
	ref Color m_SymbolsGlowColor;

	[Attribute()]
	ref array<ref InputButtonState> m_States;

}

/*!
\}
*/
