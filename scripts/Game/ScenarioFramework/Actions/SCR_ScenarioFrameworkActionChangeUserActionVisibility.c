[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionChangeUserActionVisibility : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity that has the UserAction on it (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "ID corresponding to the action attached on ActionsManagerComponent on target entity")]
	int m_iActionID;
	
	[Attribute(defvalue: "1", desc: "If checked, user action will be visible, otherwise it will be hidden.")]
	bool m_bVisible;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		BaseActionsManagerComponent actionsManager = BaseActionsManagerComponent.Cast(entity.FindComponent(BaseActionsManagerComponent));
		if (!actionsManager)
			return;
		
		BaseUserAction userAction = actionsManager.FindAction(m_iActionID);
		if (userAction)
			userAction.SetActionEnabled_S(m_bVisible);
		else
			PrintFormat("ScenarioFramework Action: User Action %1 not found for object %2", this, entity.GetName(), level: LogLevel.ERROR);
	}
}