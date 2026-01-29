[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in CanActivate method from which you can step out to the OnActivate method and debug this specific action. This can be also set during runtime via Debug Menu > ScenarioFramework > Action Inspector")]
	bool m_bDebug;
	
	SCR_AIGroup m_AIGroup;
	IEntity m_IEntity;

	//------------------------------------------------------------------------------------------------
	void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkAIAction.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		if (!targetAIGroup)
			return;
		
		m_AIGroup = targetAIGroup;
		m_IEntity = entity;

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		// Here you can step out to the OnActivate method and debug this specific action if m_bDebug attribute is set to true.
		// This can be also adjusted during runtime via Debug Menu > ScenarioFramework > Action Inspector
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkAIAction.OnActivate] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] subActions
	ref array<ref SCR_ScenarioFrameworkActionBase> GetSubActions();
}