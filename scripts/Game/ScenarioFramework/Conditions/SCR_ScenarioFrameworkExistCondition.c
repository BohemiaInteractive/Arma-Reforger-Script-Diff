[BaseContainerProps()]
class SCR_ScenarioFrameworkExistCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Checked entity")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (entityWrapper)
		{
			return entityWrapper.GetValue();
		}

		return false;
	}
}