[BaseContainerProps(visible: false, insertable: false)]
class SCR_ScenarioFrameworkDamageContextConditionBase
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in CanActivate method from which you can step out to the OnActivate method and debug this specific action. This can be also set during runtime via Debug Menu > ScenarioFramework > Action Inspector")]
	bool m_bDebug;

	[Attribute(defvalue: "0", desc: "Condition returns opposite bool if met.")]
	bool m_bNegation;
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] damageContext
	bool Init(BaseDamageContext damageContext)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("SCR_ScenarioFrameworkDamageInstigatorConditionBase.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		//Condition is separated into another method, so it can be used with negation
		if (m_bNegation)
			return !ConditionCheck(damageContext);
		
		return ConditionCheck(damageContext);
	}
	
	//------------------------------------------------------------------------------------------------
	bool ConditionCheck(BaseDamageContext damageContext)
	{
		return false;
	}
}