//------------------------------------------------------------------------------------------------
//! Returns true if current vehicle speed matches the condition
[BaseContainerProps()]
class SCR_VehicleAltitudeAGLCondition : SCR_AvailableActionCondition
{
	protected const string ALTITUDE_AGL_SIGNAL = "altitudeAGL";
	
	[Attribute(defvalue: "3", desc: "Minimum altitude AGL\n[m]", params: "0 10000 0.1", precision: 1)]
	protected float m_fAltitudeAGL;

	protected SignalsManagerComponent m_Signals;
	protected int m_iAltitudeAGLID = -1;

	//------------------------------------------------------------------------------------------------
	//! Returns true when current controlled vehicle altitude matches the condition by operator
	//! Returns opposite if m_bNegateCondition is enabled
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		SignalsManagerComponent signals = data.GetCurrentVehicleSignals();
		if (!signals || m_Signals != signals)
		{
			m_Signals = signals;
			m_iAltitudeAGLID = -1;
			return false;
		}

		if (m_iAltitudeAGLID == -1)
			m_iAltitudeAGLID = signals.AddOrFindSignal(ALTITUDE_AGL_SIGNAL);

		float altitudeAGL;
		if (m_iAltitudeAGLID != -1)
			altitudeAGL = signals.GetSignalValue(m_iAltitudeAGLID);

		bool result = altitudeAGL >= m_fAltitudeAGL;
		return GetReturnResult(result);
	}
}
