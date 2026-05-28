[BaseContainerProps(visible: false, insertable: false)]
class SCR_ScenarioFrameworkInstigatorContextConditionBase
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in Init method.")]
	bool m_bDebug;

	[Attribute(desc: "Condition returns opposite bool if met.")]
	bool m_bNegation;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] damageContext
	bool Init(SCR_InstigatorContextData instigatorContext)
	{
		if (m_bDebug)
			Print("SCR_ScenarioFrameworkInstigatorContextConditionBase.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		//Condition is separated into another method, so it can be used with negation
		if (m_bNegation)
			return !ConditionCheck(instigatorContext);
		
		return ConditionCheck(instigatorContext);
	}
	
	//------------------------------------------------------------------------------------------------
	bool ConditionCheck(SCR_InstigatorContextData instigatorContext)
	{
		return false;
	}
}