class SCR_HealthAndBurn : ScriptAndConfig
{
	[Attribute(defvalue: "0.5", uiwidget: UIWidgets.Slider, desc: "Percent of health above which specified state is meant to be used\n[% of health]", params: "0 1 0.01")]
	protected float m_fSmokeThreshold;

	[Attribute(SCR_EBurningState.SMOKING_HEAVY.ToString(), uiwidget: UIWidgets.ComboBox, desc: "State that is meant to be used if health percentage is above Smoke Threshold", enums: ParamEnumArray.FromEnum(SCR_EBurningState))]
	protected SCR_EBurningState m_eTargetBurnState;

	//------------------------------------------------------------------------------------------------
	//! \return Health percentage expressed in value that is between 0.0 and 1.0
	float GetPercentage()
	{
		return m_fSmokeThreshold;
	}

	//------------------------------------------------------------------------------------------------
	//! \return State that is meant to be used if health percentage is above GetPercentage() value
	SCR_EBurningState GetState()
	{
		return m_eTargetBurnState;
	}
}
