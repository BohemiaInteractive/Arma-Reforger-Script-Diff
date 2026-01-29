// Default signal based analog gauge, needle movement clamped by Min and Max values, supports 1 needle - standard size
class SCR_AnalogGauge : SCR_BaseAnalogGauge
{	
	// Attributes: values definition
	[Attribute("0", UIWidgets.SpinBox, "Min. value (usually 0, but can be any value, even negative)")]
	protected float m_fValueMin;	
	
	[Attribute("120", UIWidgets.SpinBox, "Max. value (in the units provided, e.g. km/h)", "0.1 1000000000 0.1")]
	protected float m_fValueMax;
	
	// Attributes: labels
	[Attribute("10", UIWidgets.SpinBox, "Section mark on gauge ring every N-units")]
	protected float m_fSectionEveryNthValue;
		
	[Attribute("2", UIWidgets.SpinBox, "Section subdivisions on gauge ring", "1 10 1")]
	protected int m_iSectionSubdivisions;
	
	[Attribute("2", UIWidgets.SpinBox, "Label on gauge ring every N-sections (e.g. if 1 every section is labeled)", "1 10 1")]
	protected int m_iLabelEveryNthSection;

	
	// Attributes: signal
	[Attribute("speed", UIWidgets.EditBox, "Name of the signal to listen to.")]
	protected string m_sSignalName;

	
	// Attributes: custom data
	[Attribute("", UIWidgets.EditBox, "Gauge overlay texture.")]
	protected string m_sOverlay;	

	[Attribute("", UIWidgets.EditBox, "Gauge custom needle texture. If not selected, default is used.")]
	protected string m_sCustomNeedle;	
	
	
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected int m_iSignalIndex = -1;
	
	protected float m_fValueRange;
	
	
	//------------------------------------------------------------------------------------------------
	override SCR_BaseAnalogGaugeData GetGaugeData()
	{
		SCR_BaseAnalogGaugeData data = new SCR_BaseAnalogGaugeData();

		// Custom attributes
		data.m_sOverlay = m_sOverlay;
		data.m_sCustomNeedle = m_sCustomNeedle;
		
		return data;
	}

	//------------------------------------------------------------------------------------------------
	override void InitGauge(IEntity owner, out bool bSuccess)
	{
		// Verify markings & label settings
		if (m_fSectionEveryNthValue <= 0) 
			m_fSectionEveryNthValue = 10;

		if (m_iLabelEveryNthSection < 1)
			m_iLabelEveryNthSection = 1;

		if (m_iSectionSubdivisions < 1) 
			m_iSectionSubdivisions = 1;

		// Force min & max values to be always labeled
		m_fValueMax = (Math.Ceil(m_fValueMax / (m_fSectionEveryNthValue * m_iLabelEveryNthSection))) * m_fSectionEveryNthValue * m_iLabelEveryNthSection;		
		m_fValueMin = (Math.Floor(m_fValueMin / (m_fSectionEveryNthValue * m_iLabelEveryNthSection))) * m_fSectionEveryNthValue * m_iLabelEveryNthSection;
		
		// Get gauge value range
		m_fValueRange = m_fValueMax - m_fValueMin;		
		
		if (m_fValueRange <= 0)
			bSuccess = false;		
	}	
		
	//------------------------------------------------------------------------------------------------
	override bool CreateGaugeRing(IEntity owner)
	{
		bool bSuccess = super.CreateGaugeRing(owner);
		
		if (!bSuccess)
			return false;
		
		int sections = (m_fValueRange / m_fSectionEveryNthValue);
				
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		int steps = sections * m_iSectionSubdivisions;
		float markAngle = m_fWidgetRange / steps;
		float angle, value;

		string texture;		

		for (int i = 0; i <= steps; i++)
    	{
			angle = m_fZeroValueRotation + (i * markAngle);
			
			// Add marks on the gauge ring
			if (i % m_iSectionSubdivisions == 0)
				texture = m_pGaugeData.m_sRingMarkSection;
			else
				texture = m_pGaugeData.m_sRingMarkSubsection;
			
			CreateRingMark(workspace, angle, texture);
			
			if (i == 0 && !m_bShowLabelMin)
				continue;

			if (i == steps && !m_bShowLabelMax)
				continue;			
						
			// Add labels to section marks
			if (i % (m_iSectionSubdivisions * m_iLabelEveryNthSection) == 0)
			{
				value = m_fValueMin + (i / m_iSectionSubdivisions * m_fSectionEveryNthValue);
				CreateRingLabel(workspace, angle, value, m_bAbsLabelValues, m_fLabelValueMultiplier, m_iLabelValuePrecision);
			}
		}		
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override float GetValue()
	{
		if (!m_SignalsManagerComponent)
			return 0;

		if (m_iSignalIndex == -1)
			return 0;		
				
		float value = m_SignalsManagerComponent.GetSignalValue(m_iSignalIndex);
		
		return value;
	}	

	//------------------------------------------------------------------------------------------------
	override float GetValuePerc(float value)
	{
		if (m_fValueRange == 0)
			return 0;
		
		float fValuePerc = (value - m_fValueMin) / m_fValueRange;

		return fValuePerc;
	}	
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStartDraw(IEntity owner)	
	{
		if (!m_SignalsManagerComponent)
			return;
		
		m_iSignalIndex = m_SignalsManagerComponent.FindSignal(m_sSignalName);

		super.DisplayStartDraw(owner);
	}		

	//------------------------------------------------------------------------------------------------
	override event void DisplayInit(IEntity owner)
	{
		super.DisplayInit(owner);
		
		GenericEntity genericEntity = GenericEntity.Cast(owner);

		if (!genericEntity)
			return;

		m_SignalsManagerComponent = SignalsManagerComponent.Cast(genericEntity.FindComponent(SignalsManagerComponent));
	}
};

