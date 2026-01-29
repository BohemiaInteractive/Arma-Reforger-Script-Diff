[BaseContainerProps()]
class SCR_DaytimeCondition : SCR_AvailableActionCondition
{
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_ComparerOperator.GREATER_THAN_OR_EQUAL), UIWidgets.ComboBox, "Cond operator", "", ParamEnumArray.FromEnum(SCR_ComparerOperator) )]
	private SCR_ComparerOperator m_eOperator;

	[Attribute(defvalue: SCR_Enum.GetDefault(EDayTimeEnums.DAYTIME_DUSK), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(EDayTimeEnums))]
	EDayTimeEnums m_TimeOfDay;

	//------------------------------------------------------------------------------------------------
	//! \param data true if daytime matches the condition
	//! \return true if daytime matches the condition, false if not or if data or time manager is null
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;

		ChimeraWorld world = GetGame().GetWorld();	
		TimeAndWeatherManagerEntity timeManager = world.GetTimeAndWeatherManager();
		if (!timeManager)
			return false;

		SCR_UIInfo uiInfo;
		EDayTimeEnums currentTimeOfDay = timeManager.GetCurrentDayTimeUIInfoAndPhase(uiInfo);

		bool result = SCR_Comparer<int>.Compare(m_eOperator, (int)currentTimeOfDay, (int)m_TimeOfDay);
		return GetReturnResult(result);
	}
};
