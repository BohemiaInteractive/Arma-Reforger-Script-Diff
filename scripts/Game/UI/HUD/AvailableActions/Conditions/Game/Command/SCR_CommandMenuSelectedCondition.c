[BaseContainerProps()]
class SCR_CommandMenuSelectedCondition : SCR_AvailableActionCondition
{
	protected SCR_PlayerControllerCommandingComponent m_CommandingComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(notnull SCR_AvailableActionsConditionData data)
	{
		if (!m_CommandingComponent)
		{
			m_CommandingComponent = SCR_PlayerControllerCommandingComponent.GetLocalPlayerControllerCommandingComponent();
			
			if (!m_CommandingComponent)
				return GetReturnResult(false);
		}
		
		return GetReturnResult(m_CommandingComponent.IsCommandSelected());
	}
}