[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnUserActionEvent : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity that has the UserAction on it (Optional if action is attached on Slot that spawns target entity)")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute(desc: "ID corresponding to the action attached on ActionsManagerComponent on target entity")]
	int m_iActionID;
	
	[Attribute(desc: "Only listen to changes when UserAction is activated by specific Entity (Optional - leave it empty to trigger by anyone)")]
	ref SCR_ScenarioFrameworkGet m_GetterUser;
	
	[Attribute(defvalue: EUserActionEvent.PerformAction.ToString(), UIWidgets.ComboBox, "On which user action event this ScenarioFramework action will be triggered", "", ParamEnumArray.FromEnum(EUserActionEvent))]
	EUserActionEvent m_eUserActionEvent;
	
	[Attribute(desc: "Which actions will be executed once user action is used")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	IEntity m_ActionHolder;
	ref array<IEntity> m_aUserEntities;

	//------------------------------------------------------------------------------------------------
	void OnInvoked(IEntity user, ScriptedUserAction action, EUserActionEvent e)
	{
		if (e != m_eUserActionEvent)
			return;
		
		if (m_aUserEntities && !m_aUserEntities.IsEmpty() && !m_aUserEntities.Contains(user))
			return;
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
		
		BaseActionsManagerComponent actionsManager = action.GetActionsManager();
		if (actionsManager)
			actionsManager.RemoveUserActionEventListener(action, OnInvoked);
	}

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
		if (!userAction)
			return;
		
		ScriptedUserAction scriptedUserAction = ScriptedUserAction.Cast(userAction);
		if (!scriptedUserAction)
			return;
		
		if (m_GetterUser)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_GetterUser.Get());
			if (entityWrapper)
			{	
				m_aUserEntities = {};
				entity = entityWrapper.GetValue();
				if (entity)
					m_aUserEntities.Insert(entity);
			}
			else
			{
				SCR_ScenarioFrameworkParam<array<IEntity>> arrayOfEntitiesWrapper = SCR_ScenarioFrameworkParam<array<IEntity>>.Cast(m_GetterUser.Get());
				if (arrayOfEntitiesWrapper)
				{
					m_aUserEntities = {};
					m_aUserEntities.InsertAll(arrayOfEntitiesWrapper.GetValue());
				}
			}
		}
		
		m_ActionHolder = entity;
		actionsManager.AddUserActionEventListener(scriptedUserAction, OnInvoked);
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}