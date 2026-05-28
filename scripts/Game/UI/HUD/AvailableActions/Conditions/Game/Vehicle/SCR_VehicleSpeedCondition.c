//! Returns true if current vehicle speed matches the condition
[BaseContainerProps()]
class SCR_VehicleSpeedCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	protected SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Speed compare value\n[km/h]", "")]
	protected float m_fValue;

	protected SignalsManagerComponent m_Signals;
	protected int m_iSpeedID = -1;
	protected const string SIGNAL_NAME = "speed";

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled vehicle speed matches the condition by operator
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		SignalsManagerComponent signals = data.GetCurrentVehicleSignals();
		if (!signals || m_Signals != signals)
		{
			m_Signals = signals;
			m_iSpeedID = -1;
			return false;
		}

		if (m_iSpeedID == -1)
			m_iSpeedID = signals.AddOrFindSignal(SIGNAL_NAME);

		return GetReturnResult(m_iSpeedID != -1 && SCR_Comparer<int>.Compare(m_eOperator, signals.GetSignalValue(m_iSpeedID), (int)m_fValue));
	}
}
