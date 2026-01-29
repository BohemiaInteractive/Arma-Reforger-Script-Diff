[BaseContainerProps()]
class SCR_CommandMenuOpenCondition : SCR_AvailableActionCondition
{
	protected SCR_PlayerControllerCommandingComponent m_CommandingComponent;
	
	//------------------------------------------------------------------------------------------------
	override bool IsAvailable(SCR_AvailableActionsConditionData data)
	{
		if (!data)
			return false;
		
		if (!m_CommandingComponent)
		{
			m_CommandingComponent = SCR_PlayerControllerCommandingComponent.GetLocalPlayerControllerCommandingComponent();
			
			if (!m_CommandingComponent)
				return GetReturnResult(false);
		}
		
		return GetReturnResult(m_CommandingComponent.IsOpened());
	}
}