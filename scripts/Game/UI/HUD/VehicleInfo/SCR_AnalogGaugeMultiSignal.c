//------------------------------------------------------------------------------------------------
//! gauge showing the sum of more than one signal
class SCR_AnalogGaugeMultiSignal : SCR_AnalogGaugeSmall
{
	// Attributes: signal prefix
	[Attribute("", UIWidgets.EditBox, "Name of the signals prefix to listen to.")]
	protected string  m_sSignalNamePrefix;
			
	// Attributes: number of signals ( signal prefix + signal number = signal name )
	[Attribute( defvalue: "2", uiwidget: UIWidgets.EditBox, desc: "Number of signals which values are supposed to be summed up" )]
	private int m_iNrOfSignals;
	
	private ref array<int> 			m_iArraysOfSignalsIDs;
		
	//------------------------------------------------------------------------------------------------
	override float GetValue()
	{
		if (!m_SignalsManagerComponent)
			return 0;
			
		float value = 0;
			
		for( int iLoop = 0; iLoop < m_iArraysOfSignalsIDs.Count(); iLoop++)
			value += m_SignalsManagerComponent.GetSignalValue( m_iArraysOfSignalsIDs.Get( iLoop ) );

		return value;
	}
	
	//------------------------------------------------------------------------------------------------
	override event void DisplayStartDraw(IEntity owner)	
	{
		if ( !m_SignalsManagerComponent )
			return;
		
		m_iArraysOfSignalsIDs.Clear();
		for( int iLoop = 1; iLoop <= m_iNrOfSignals; iLoop++)
			m_iArraysOfSignalsIDs.Insert( m_SignalsManagerComponent.FindSignal( m_sSignalNamePrefix + iLoop.ToString() ) );

		super.DisplayStartDraw( owner );
	}
		
	//------------------------------------------------------------------------------------------------
	void SCR_AnalogGaugeMultiSignal()
	{
		m_iArraysOfSignalsIDs = new array<int>();
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_AnalogGaugeMultiSignal()
	{
		if( m_iArraysOfSignalsIDs )
			m_iArraysOfSignalsIDs = null;
	}

};
