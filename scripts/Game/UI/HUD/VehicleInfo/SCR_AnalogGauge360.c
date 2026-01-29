// Signal based analog gauge, used for 360 deg dials, supports up to 2 needles - standard size
class SCR_AnalogGauge360 : SCR_AnalogGauge
{	
	[Attribute("", UIWidgets.EditBox, "Gauge custom needle texture. If not selected, default is used.")]
	protected string m_sCustomNeedle2;

	protected ImageWidget m_wNeedle2;
	protected ImageWidget m_wNeedle2Shadow;	
		
	override void CreateGauge(IEntity owner, out bool bSuccess)
	{
		super.CreateGauge(owner, bSuccess);
		
		// Gauge 2nd needle
		m_wNeedle2 = ImageWidget.Cast(m_wRoot.FindAnyWidget("Needle2"));
		m_wNeedle2Shadow = ImageWidget.Cast(m_wRoot.FindAnyWidget("Needle2Shadow"));
		bSuccess = CreateGaugeNeedle(owner, m_wNeedle2, m_wNeedle2Shadow, m_sCustomNeedle2);

		if (!bSuccess)
			return;	
	}
	
	override void UpdateGauge(IEntity owner, float timeSlice, float value)
	{
		// Get unclamped percentage value of the gauge range
		float fValuePerc = (value - m_fValueMin) / m_fValueRange;
		
		UpdateGaugeNeedle(owner, timeSlice, m_wNeedle, m_wNeedleShadow, fValuePerc, 1, false);	
		UpdateGaugeNeedle(owner, timeSlice, m_wNeedle2, m_wNeedle2Shadow, fValuePerc, 0.1, false);	
	}
};