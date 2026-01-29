[BaseContainerProps(visible: false, insertable: false)]
class SCR_ScenarioFrameworkOnProjectileShotConditionBase
{
	[Attribute("Debug parameter to trigger breakpoints")]
	bool m_bDebug;

	[Attribute(defvalue: "0", desc: "Condition returns opposite bool if met.")]
	bool m_bNegation;
	
	//------------------------------------------------------------------------------------------------
	bool Init(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkMedicalConditionItem.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (m_bNegation)
			return !ConditionCheck(playerID, weapon, entity);
		
		return ConditionCheck(playerID, weapon, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	bool ConditionCheck(int playerID, BaseWeaponComponent weapon, IEntity entity)
	{
		return true;
	}
}