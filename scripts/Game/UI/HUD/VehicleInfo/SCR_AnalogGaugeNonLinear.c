// Default signal based analog gauge, needle movement clamped by Min and Max values, supports 1 needle - standard size
class SCR_AnalogGaugeNonLinear : SCR_BaseAnalogGauge
{	
	// Attributes: values definition
	[Attribute("0 10 30 60 120", UIWidgets.EditBox, "Direct definition of values to be displayed on the gauge; float values separated by space")]
	protected string  m_sValues;

	
	// Attributes: labels
	[Attribute("2", UIWidgets.SpinBox, "Section subdivisions on gauge ring", "1 10 1")]
	protected int m_iSectionSubdivisions;

	[Attribute("1", UIWidgets.SpinBox, "Label on gauge ring every N-sections (e.g. if 1 every section is labeled)", "1 10 1")]
	protected int m_iLabelEveryNthSection;

		
	// Attributes: signal
	[Attribute("speed", UIWidgets.EditBox, "Name of the signal to listen to.")]
	protected string  m_sSignalName;

	
	// Attributes: custom data
	[Attribute("", UIWidgets.EditBox, "Gauge overlay texture.")]
	protected string m_sOverlay;	

	[Attribute("", UIWidgets.EditBox, "Gauge custom needle texture. If not selected, default is used.")]
	protected string m_sCustomNeedle;	
	
	protected SignalsManagerComponent m_SignalsManagerComponent;
	protected int m_iSignalIndex = -1;

	protected float m_fValueMin, m_fValueMax, m_fValueRange;
	protected ref array<float> m_aValues = new array<float>();
	
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
		// Parse and validate value data from the editbox input		
		array<string> aValueStrings = new array<string>();
		m_sValues.Split(" ", aValueStrings, true);
		
		float fValue;
		int iValues = 0;
		
		foreach (string sValue : aValueStrings)
		{
			fValue = sValue.ToFloat();
			
			if (iValues == 0 || fValue > m_fValueMax)
			{
				if (iValues == 0)
					m_fValueMin = fValue;

				iValues++;					
				
				m_aValues.Insert(fValue);
				m_fValueMax = fValue;	
			}
		}
		
		// Need at least 2 values (min & max)
		if (iValues < 2)
		{
			bSuccess = false;
			return;
		}
	
		// Get gauge value range
		m_fValueRange = m_fValueMax - m_fValueMin;	

		if (m_fValueRange <= 0)
		{
			bSuccess = false;
			return;
		}	
	}
	
	//------------------------------------------------------------------------------------------------
	override void DestroyGauge()
	{
		m_aValues.Clear();
		
		super.DestroyGauge();
	}		
		
	//------------------------------------------------------------------------------------------------
	override bool CreateGaugeRing(IEntity owner)
	{
		bool bSuccess = super.CreateGaugeRing(owner);
		
		if (!bSuccess)
			return false;
		
		int sections = m_aValues.Count() - 1;
				
		WorkspaceWidget workspace = GetGame().GetWorkspace();
		
		int steps = sections * m_iSectionSubdivisions;
		float markAngle = m_fWidgetRange / steps;
		float angle, value;
		int index;

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
				index = i / m_iSectionSubdivisions;
				value = m_aValues[index];
		
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
		
		// Get segment value is in
		float fMin, fMax;
		int iSegments = m_aValues.Count() - 1;
		
		if (iSegments <= 0)
			return 0;
		
		float fValuePerc = -1;
		float fSegmentSize = 1 / iSegments;

		// Clamp progress to 100% if out of bounds		
		if (value > m_aValues[iSegments])
			return 1;
		
		for (int i = 0; i < iSegments; i++)
		{
			if (fValuePerc > -1)
				continue;
			
			fMin = m_aValues[i];
			fMax = m_aValues[i + 1];
		
			if (value < fMax)
			{
				fValuePerc = i * fSegmentSize;	// percentage based on segment
				fValuePerc += fSegmentSize * (value - fMin) / (fMax - fMin);  // percentage based on progress within segment
			}
		}

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

