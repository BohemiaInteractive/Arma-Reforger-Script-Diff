[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionOnAnimationEvent : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Entity to listen animation events on")]
	ref SCR_ScenarioFrameworkGet m_Getter;
	
	[Attribute("", UIWidgets.Auto)]
	string m_sAnimationEventString;
	
	[Attribute(desc: "Actions to be executed if conditions' evaluation is successful.")]
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActions;
	
	SCR_CharacterControllerComponent m_CharController;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;
		
		IEntity entity;
		if (!ValidateInputEntity(object, m_Getter, entity))
			return;
		
		m_CharController = SCR_CharacterControllerComponent.Cast(entity.FindComponent(SCR_CharacterControllerComponent));

		if (m_CharController)
		{
			m_CharController.GetOnAnimationEvent().Remove(OnAnimationEvent);
			m_CharController.GetOnAnimationEvent().Insert(OnAnimationEvent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnAnimationEvent(AnimationEventID animEventType, AnimationEventID animUserString, int intParam, float timeFromStart, float timeToEnd)
	{
		string eventName = GameAnimationUtils.GetEventString(animEventType);

		if (eventName == m_sAnimationEventString)
		{
			foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
			{
				actions.OnActivate(m_Entity);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		array<ref SCR_ScenarioFrameworkActionBase> allActions = {};
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActions)
		{
			allActions.Insert(action);
		}
		
		return allActions;
	}
}