[BaseContainerProps(configRoot: true)]
class SCR_HintConditionList
{
	[Attribute()]
	protected ref array<ref SCR_BaseHintCondition> m_aConditions;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Init(IEntity owner)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		for (int i, count = m_aConditions.Count(); i < count; i++)
		{
			m_aConditions[i].InitCondition(owner, hintManager);
		}
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] owner
	void Exit(IEntity owner)
	{
		SCR_HintManagerComponent hintManager = SCR_HintManagerComponent.GetInstance();
		if (!hintManager)
			return;
		
		for (int i, count = m_aConditions.Count(); i < count; i++)
		{
			m_aConditions[i].ExitCondition(owner, hintManager);
		}
	}
}
