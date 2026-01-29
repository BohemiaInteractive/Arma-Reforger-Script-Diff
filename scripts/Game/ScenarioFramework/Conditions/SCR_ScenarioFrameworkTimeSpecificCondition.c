[BaseContainerProps()]
class SCR_ScenarioFrameworkTimeSpecificCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(defvalue: "0", desc: "Minimal day time hour", params: "0 24 1", category: "Time")]
	int m_iHours;

	[Attribute(defvalue: "0", desc: "Maximal day time hour", params: "0 59 1", category: "Time")]
	int m_iMinutes;
	
	[Attribute(defvalue: "0", desc: "Maximal day time hour", params: "0 59 1", category: "Time")]
	int m_iSeconds;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		// Here you can debug specific Condition instance.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Condition Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkTimeSpecificCondition.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		ChimeraWorld world = ChimeraWorld.CastFrom(entity.GetWorld());
		if (!world)
			return false;

		TimeAndWeatherManagerEntity manager = world.GetTimeAndWeatherManager();
		if (!manager)
			return false;

		int targetSeconds = m_iHours * 3600 + m_iMinutes * 60 + m_iSeconds;
		
		TimeContainer worldTime = manager.GetTime();
		int worldTimeSeconds = worldTime.m_iHours * 3600 + worldTime.m_iMinutes * 60 + worldTime.m_iSeconds;
		
		return 	((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (worldTimeSeconds < targetSeconds)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (worldTimeSeconds <= targetSeconds)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (worldTimeSeconds == targetSeconds)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (worldTimeSeconds >= targetSeconds)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (worldTimeSeconds > targetSeconds));
	}
}