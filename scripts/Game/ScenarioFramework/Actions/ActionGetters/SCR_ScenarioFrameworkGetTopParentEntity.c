[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkGetTopParentEntity : SCR_ScenarioFrameworkGet
{
	[Attribute(desc: "Entity to get the top parent from")]
	ref SCR_ScenarioFrameworkGet m_Getter;

	//------------------------------------------------------------------------------------------------
	override SCR_ScenarioFrameworkParamBase Get()
	{
		if (!m_Getter)
		{
			PrintFormat("ScenarioFramework Getter: Getter not provided for %1.", this, level: LogLevel.ERROR);
			return null;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			PrintFormat("ScenarioFramework Getter: Issue with Getter detected for Getter %1.", this, level: LogLevel.ERROR);
			return null;
		}

		IEntity entity = entityWrapper.GetValue();
		if (!entity)
		{
			PrintFormat("ScenarioFramework Getter: Issue with Getter detected for Getter %1. Entity not found.", this, level: LogLevel.ERROR);
			return null;
		}

		IEntity rootOwner = entity.GetRootParent();
		if (!rootOwner)
		{
			PrintFormat("ScenarioFramework Getter: Issue with Getter detected for Getter %1. Top Parent Entity not found.", this, level: LogLevel.ERROR);
			return null;
		}

		return new SCR_ScenarioFrameworkParam<IEntity>(rootOwner);
	}
}
