[BaseContainerProps()]
class SCR_DaytimeCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_ComparerOperator.GREATER_THAN_OR_EQUAL.ToString(), desc: "Condition operator", uiwidget: UIWidgets.ComboBox, enumType: SCR_ComparerOperator)]
	private SCR_ComparerOperator m_eOperator;

	[Attribute(defvalue: EDayTimeEnums.DAYTIME_DUSK.ToString(), desc: "Time Of Day", uiwidget: UIWidgets.ComboBox, enumType: EDayTimeEnums)]
	protected EDayTimeEnums m_TimeOfDay;

	//------------------------------------------------------------------------------------------------
	//! \param data true if daytime matches the condition
	//! \return true if daytime matches the condition, false if not or if data or time manager is null
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		ChimeraWorld world = GetGame().GetWorld();
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return false;

		SCR_UIInfo uiInfo;
		return GetReturnResult(SCR_Comparer<int>.Compare(m_eOperator, timeManager.GetCurrentDayTimeUIInfoAndPhase(uiInfo), m_TimeOfDay));
	}
}
