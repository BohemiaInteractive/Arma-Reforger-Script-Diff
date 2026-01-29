[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionOnAgentCountChanged : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "Actions that will be executed upon the threat state being changed and matching the desired state")];
	protected ref array<ref SCR_ScenarioFrameworkActionBase> m_aActionsOnAICountChanged;
	
	[Attribute("0", UIWidgets.ComboBox, "Operator", "", ParamEnumArray.FromEnum(SCR_EScenarioFrameworkComparisonOperator))]
	protected SCR_EScenarioFrameworkComparisonOperator m_eComparisonOperator;
	
	[Attribute(defvalue: "0", desc: "Activation Percentage", params: "0 100 1")]
	protected int m_iActivationPercentage;
	
	protected IEntity m_AISlotEntity;
	
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
		
		m_AIGroup.GetOnAgentAdded().Insert(OnAgentCountIncreased);
		m_AIGroup.GetOnAgentRemoved().Insert(OnAgentCountDecreased);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Evaluate(SCR_AIGroup group, AIAgent child)
	{
		if (!group && child)
			group = SCR_AIGroup.Cast(child.GetParentGroup());
		
		if (!group)
		{
			PrintFormat("SCR_ScenarioFrameworkAIActionOnAgentCountChanged: No group for evaluation found on provided agent. Nothing will happen. %1", this, level:LogLevel.ERROR);
			return;
		}
		
		int membersToSpawn = group.GetNumberOfMembersToSpawn();
		if (membersToSpawn < 1)
		{
			PrintFormat("SCR_ScenarioFrameworkAIActionOnAgentCountChanged: group prefab doesn't have any units to spawn, thus it size change cannot be calculated for action %1", this, level:LogLevel.ERROR);
			return;
		}
		
		float current = (group.GetAgentsCount() / membersToSpawn) * 100;
		
		if (
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_THAN) 			&& (current < m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.LESS_OR_EQUAL) 		&& (current <= m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.EQUAL) 				&& (current == m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_OR_EQUAL) 	&& (current >= m_iActivationPercentage)) 	||
				((m_eComparisonOperator == SCR_EScenarioFrameworkComparisonOperator.GREATER_THEN) 		&& (current > m_iActivationPercentage)) 
		)
		{
			foreach (SCR_ScenarioFrameworkActionBase action : m_aActionsOnAICountChanged)
			{
				action.Init(m_AISlotEntity);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	void OnAgentCountIncreased(AIAgent child)
	{
		Evaluate(null, child);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnAgentCountDecreased(SCR_AIGroup group, AIAgent child)
	{
		Evaluate(group, child);
	}
	
	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActionsOnAICountChanged;
	}
}