[BaseContainerProps(visible: false, insertable: false)]
class SCR_ScenarioFrameworkResourceConditionBase
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in CanActivate method from which you can step out to the OnActivate method and debug this specific action. This can be also set during runtime via Debug Menu > ScenarioFramework > Action Inspector")]
	bool m_bDebug;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	//! \param[in] damageManager
	bool Init(notnull IEntity ent, notnull SCR_ResourceComponent resourceComponent, EResourceType resourceType)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkResourceConditionBase.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		return true;
	}
}