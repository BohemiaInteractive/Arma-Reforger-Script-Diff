[BaseContainerProps()]
class SCR_ScenarioFrameworkResourceComponentActionBase
{
	[Attribute(desc: "If set to true, when this action gets activated, it will break the breakpoint in the Script Editor in CanActivate method from which you can step out to the OnActivate method and debug this specific action. This can be also set during runtime via Debug Menu > ScenarioFramework > Action Inspector")]
	bool m_bDebug;
	
	protected SCR_ResourceComponent m_ResourceComponent;
	protected EResourceType m_eResourceType;

	//------------------------------------------------------------------------------------------------
	void Init(SCR_ResourceComponent resourceComp, EResourceType resourceType)
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkResourceComponentActionBase.Init] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
		
		m_ResourceComponent = resourceComp;
		m_eResourceType = resourceType;
		
		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate()
	{
		if (m_bDebug)
			Print("[SCR_ScenarioFrameworkResourceComponentActionBase.OnActivate] debug line (" + __FILE__ + " L" + __LINE__ + ")", LogLevel.WARNING);
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[out] subActions
	array<ref SCR_ScenarioFrameworkActionBase> GetSubActions();
	
	//------------------------------------------------------------------------------------------------
	IEntity GetInputEntity(SCR_ScenarioFrameworkGet getter)
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);

			return null;
		}

		return entityWrapper.GetValue();
	}
}