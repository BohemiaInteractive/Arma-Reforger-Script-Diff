//! Handles UI displaying mortar values
//! - azimuth, elevation, range
class SCR_MortarInfo : SCR_InfoDisplayExtended
{
	[Attribute(defvalue: "6400", desc: "Mils in full 360 degrees angle \nSoviet: 6000\nNATO: 6400")]
	protected float m_fMils;
	
	protected ref SCR_MortarInfoWidgets m_Widgets = new SCR_MortarInfoWidgets();

	//------------------------------------------------------------------------------------------------
	override void DisplayStartDraw(IEntity owner)
	{
		if (!m_wRoot)
			return;
		
		m_Widgets = new SCR_MortarInfoWidgets();
		m_Widgets.Init(m_wRoot);
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMils()
	{
		return m_fMils;
	}
}
