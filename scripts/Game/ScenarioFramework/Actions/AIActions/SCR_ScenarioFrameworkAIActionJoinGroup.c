[BaseContainerProps(), SCR_ContainerAIActionTitle()]
class SCR_ScenarioFrameworkAIActionJoinGroup : SCR_ScenarioFrameworkAIAction
{
	[Attribute(desc: "SlotAI with group to join (optional)")];
	ref SCR_ScenarioFrameworkGetLayerBase m_Getter;
	
	[Attribute(defvalue:"0", desc: "Id of Player in group to join (optional)", params:"0 inf 1")];
	int m_iPlayerId;
	
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		SCR_AIGroup group;
		SCR_PlayerControllerGroupComponent pControllerGroupComponent;
		
		//Get target group
		if (m_Getter)
		{
			SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
			if (!entityWrapper)
			{
				PrintFormat("ScenarioFramework Action: Issue with Getter detected for Action %1.", this, level: LogLevel.ERROR);
				return;
			}
		
			IEntity entity = entityWrapper.GetValue();
			if (!entity)
			{
				PrintFormat("ScenarioFramework Action: Entity not found for Action %1", this, level: LogLevel.ERROR);
				return;
			}
			
			SCR_ScenarioFrameworkSlotAI slotAI = SCR_ScenarioFrameworkSlotAI.Cast(entity.FindComponent(SCR_ScenarioFrameworkSlotAI));
			if (slotAI)
				group = slotAI.m_AIGroup;
		}
		else if (m_iPlayerId > 0)
		{
			pControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(m_iPlayerId);
			if (!pControllerGroupComponent)
			{
				PrintFormat("ScenarioFramework Action: SCR_PlayerControllerGroupComponent not found for Action %1.", this, level: LogLevel.ERROR);
				return;
			}
			
			group = pControllerGroupComponent.GetPlayersGroup();
		}
		
		if (!group)
		{
			PrintFormat("ScenarioFramework Action: No SCR_AIGroup found for %1.", this, level: LogLevel.ERROR);
			return;
		}
		
		//get all agents from the group
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		if (group.IsPlayable())
		{
			int leaderId = group.GetLeaderID();
			pControllerGroupComponent = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(leaderId);
			if (!pControllerGroupComponent)
			{
				PrintFormat("ScenarioFramework Action: SCR_PlayerControllerGroupComponent not found for Action %1.", this, level: LogLevel.ERROR);
				return;
			}
			
			foreach (AIAgent agent : agents)
			{
				pControllerGroupComponent.RequestAddAIAgent(SCR_ChimeraCharacter.Cast(agent.GetControlledEntity()), leaderId);
			}
		}
		else
		{
			foreach (AIAgent agent : agents)
			{
				group.AddAIEntityToGroup(agent.GetControlledEntity());
			}
		}
	}
}