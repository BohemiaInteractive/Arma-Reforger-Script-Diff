// Signal based analog gauge, used for 360 deg dials, supports up to 2 needles - small size
class SCR_AnalogGauge360Small : SCR_AnalogGauge360
{	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseAnalogGaugeData GetGaugeData()
	{
		SCR_BaseAnalogGaugeData data = new SCR_BaseAnalogGaugeData();

		// Class defined gauge-specific settings
		data.m_fRingLabelOffset = 0.7;
		data.m_fRingLabelFontSize = 20;
		data.m_sRingMarkSection = "SmallGaugeMarkSection";
		data.m_sRingMarkSubsection = "SmallGaugeMarkSubsection";	
		data.m_sDefaultLayout = "{58046AD946FBFED5}UI/layouts/HUD/VehicleInfo/VehicleGaugeSmall_Default.layout";

		// Custom attributes
		data.m_sOverlay = m_sOverlay;
		data.m_sCustomNeedle = m_sCustomNeedle;		
						
		return data;
	}	
};
