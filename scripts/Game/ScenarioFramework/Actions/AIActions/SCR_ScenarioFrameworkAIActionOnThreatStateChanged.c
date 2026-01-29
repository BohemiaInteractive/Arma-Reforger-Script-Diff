[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionOnThreatStateChanged : SCR_ScenarioFrameworkAIAction
{
	[Attribute(defvalue: EAIThreatState.THREATENED.ToString(), UIWidgets.ComboBox, "On what Threat State will actions be activated", "", ParamEnumArray.FromEnum(EAIThreatState), category: "Common")]
	EAIThreatState m_eAIThreatState;
	
	[Attribute(desc: "Actions that will be executed upon the threat state being changed and matching the desired state")];
	ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnThreatStateChanged;
	
	ref array<ref SCR_AIThreatSystem> m_aAIThreatSystems = {};
	IEntity m_AISlotEntity;
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_AIGroup targetAIGroup, IEntity entity)
	{
		if (entity)
		{
			SCR_ScenarioFrameworkLayerBase layer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layer && (SCR_ScenarioFrameworkSlotAI.Cast(layer) || SCR_ScenarioFrameworkSlotTaskAI.Cast(layer)))
				m_AISlotEntity = entity;
		}
		
		super.Init(targetAIGroup, entity);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		super.OnActivate();
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		foreach (int i, AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
			
			SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
			if (!combatComponent)
				continue;

			SCR_AIInfoComponent infoComponent = combatComponent.GetAIInfoComponent();
			if (!infoComponent)
				continue;
			
			SCR_AIThreatSystem threatSystem = infoComponent.GetThreatSystem();
			if (!threatSystem)
				continue;
			
			m_aAIThreatSystems.Insert(threatSystem);
			threatSystem.GetOnThreatStateChanged().Insert(OnThreatStateChanged);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void OnThreatStateChanged(EAIThreatState prevState, EAIThreatState newState)
	{
		if (newState != m_eAIThreatState)
			return;
		
		foreach(SCR_AIThreatSystem threatSystem : m_aAIThreatSystems)
		{
			threatSystem.GetOnThreatStateChanged().Remove(OnThreatStateChanged);
		}
		
		foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnThreatStateChanged)
		{
			action.Init(m_AISlotEntity);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActionsOnThreatStateChanged;
	}
}