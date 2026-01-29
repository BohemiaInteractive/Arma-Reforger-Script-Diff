// Signal based analog gauge, used for re-fuel mechanic prototype - shows delta between initial state (when displayed) and current state
class SCR_AnalogGaugeUni : SCR_AnalogGauge
{
	[Attribute("0", UIWidgets.CheckBox, "Show difference of value")]
	protected bool m_bShowValueDiff;
	
	private TextWidget m_wInfoText2;
	
	private IEntity	m_Owner;
	private bool m_bGaugeShown;
	private float m_fLastValue = 0.0;

	bool IsGaugeShown()
	{
		return m_bGaugeShown;
	};
	
	void EnableHUD( bool enable )
	{
		if( enable )
		{
			DisplayStartDraw( m_Owner );
			m_bGaugeShown = true;
		}
		else
		{
			DisplayStopDraw( m_Owner );
			m_bGaugeShown = false;
		}
	}
	
	//------------------------------------------------------------------------------------------------	
	override void CreateGauge(IEntity owner, out bool bSuccess)
	{
		super.CreateGauge( owner, bSuccess);
		
		m_wInfoText2 = TextWidget.Cast(m_wRoot.FindAnyWidget("InfoText2"));
		m_wInfoText2.SetVisible(true);
		m_wInfoText2.SetText("0");
	}

	//------------------------------------------------------------------------------------------------		
	override void UpdateGauge(IEntity owner, float timeSlice, float value)
	{
		super.UpdateGauge(owner, timeSlice, value);	
		
		if (m_fLastValue == 0)
			m_fLastValue = value;
		
		value = Math.Round(m_fLastValue - value);		
		m_wInfoText2.SetText(value.ToString());	
	}	
};
