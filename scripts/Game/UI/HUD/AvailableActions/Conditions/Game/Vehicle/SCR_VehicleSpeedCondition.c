//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle speed matches the condition
[BaseContainerProps()]
class SCR_VehicleSpeedCondition : SCR_AvailableActionCondition
{
	[Attribute("3", UIWidgets.ComboBox, "Comparison operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	protected SCR_ComparerOperator m_eOperator;

	[Attribute("2", UIWidgets.EditBox, "Speed compare value\n[km/h]", "")]
	protected float m_fValue;

	protected SignalsManagerComponent m_Signals;
	protected int m_iSpeedID = -1;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled vehicle speed matches the condition by operator
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		SignalsManagerComponent signals = data.GetCurrentVehicleSignals();
		if (!signals || m_Signals != signals)
		{
			m_Signals = signals;
			m_iSpeedID = -1;
			return false;
		}

		if (m_iSpeedID == -1)
			m_iSpeedID = signals.AddOrFindSignal("speed");

		int speed = 0;
		if (m_iSpeedID != -1)
			speed = signals.GetSignalValue(m_iSpeedID);

		bool result = false;

		result = SCR_Comparer<int>.Compare(m_eOperator, speed, (int)m_fValue);
		return GetReturnResult(result);
	}
};
