[BaseContainerProps()]
class SCR_ScenarioFrameworkParentCondition : SCR_ScenarioFrameworkActivationConditionBase
{
	[Attribute(desc: "Entity to check")]
	ref SCR_ScenarioFrameworkGet m_EntityGetter;
	
	[Attribute(desc: "Parent getter. Array of getters can be provided as well")]
	ref SCR_ScenarioFrameworkGet m_ParentGetter;
	
	[Attribute(desc: "Check Main Parent")]
	bool m_bCheckMainParent;
	
	protected IEntity m_ChildEntity;
	//------------------------------------------------------------------------------------------------
	override bool Init(IEntity entity)
	{
		if (m_EntityGetter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_EntityGetter.Get());
			if (!entityWrapper)
				return false;
			
			m_ChildEntity = entityWrapper.GetValue();
		}
		else
		{
			m_ChildEntity = entity;
		}
		
		if (!m_ChildEntity || !m_ParentGetter)
			return false;
		
		//single parent check
		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_ParentGetter.Get());
		if (entityWrapper)
		{
			IEntity parent = entityWrapper.GetValue();
			if (parent)
				return IsParent(parent);
		}
		
		//If array of accepted parents was provided
		SCR_ScenarioFrameworkParam<array<IEntity>> entityArrayWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_ParentGetter.Get());
		if (entityArrayWrapper)
		{
			array<IEntity> parentEntities = entityArrayWrapper.GetValue();
			foreach (IEntity parent : parentEntities)
			{
				if (IsParent(parent))
					return true;
			}
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool IsParent(notnull IEntity parentEnt)
	{	
		if (m_bCheckMainParent)
			return SCR_EntityHelper.GetMainParent(m_ChildEntity) == parentEnt;
		
		return m_ChildEntity.GetParent() == parentEnt;
	}
}