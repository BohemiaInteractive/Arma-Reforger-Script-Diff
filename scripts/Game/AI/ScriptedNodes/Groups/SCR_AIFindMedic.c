class SCR_AIFindMedic : AITaskScripted
{
	static const string PORT_REQUIREMENTS_IN	= "RequirementsIn";
	static const string PORT_ENTITY_IN			= "EntityIn";
	static const string PORT_GROUP_MEMBER_OUT	= "GroupMemberOut";
	static const string PORT_AGENTS_EXCLUDE_ARRAY = "AgentsExcludeArray";
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{	
		SCR_AIGroupUtilityComponent groupUtilityComponent;
		IEntity entityToHeal;
		array<AIAgent> groupAgents = {};
		array<AIAgent> agentsToExclude = {};
		array<AIAgent> agentsOfHandler = {};
		AIAgent medic;

		AIGroup group = AIGroup.Cast(owner);
		if (group)
		{
			groupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
			group.GetAgents(groupAgents);
		}
		
		if (!groupUtilityComponent || !groupUtilityComponent.m_aInfoComponents)
			return ENodeResult.FAIL;
		
		GetVariableIn(PORT_ENTITY_IN, entityToHeal);
		if (!entityToHeal)
			return ENodeResult.FAIL;
		if (!GetVariableIn(PORT_AGENTS_EXCLUDE_ARRAY, agentsToExclude))
			agentsToExclude = {};
		
		ChimeraCharacter charToHeal = ChimeraCharacter.Cast(entityToHeal);
		if (!charToHeal)
			return ENodeResult.FAIL;
		AIControlComponent contr = AIControlComponent.Cast(charToHeal.FindComponent(AIControlComponent));
		if (!contr)
			return ENodeResult.FAIL;
		AIAgent agentToHeal = contr.GetControlAIAgent();
		if (!agentToHeal)
			return ENodeResult.FAIL;
		
		// if entityToHeal is in vehicle, let's find a medic inside vehicle first
		if (charToHeal.IsInVehicle())
		{
			// get entity to heal vehicle 
			int agentToHealHandler = groupUtilityComponent.m_GroupMovementComponent.GetAgentMoveHandlerId(agentToHeal);
			if (agentToHealHandler < AIGroupMovementComponent.DEFAULT_HANDLER_ID)
				Print("Healing agent not member of our group!", LogLevel.WARNING);
			agentToHealHandler = Math.Max(AIGroupMovementComponent.DEFAULT_HANDLER_ID, agentToHealHandler);
			groupUtilityComponent.m_GroupMovementComponent.GetAgentsInHandler(agentsOfHandler, agentToHealHandler);
			
			// try to find a medic from same vehicle subgroup, that is not a driver
			medic = FindAgentMedic(entityToHeal, agentsOfHandler, agentsToExclude, true);
			if (medic)
			{
				SetVariableOut(PORT_GROUP_MEMBER_OUT,medic);
				return ENodeResult.SUCCESS;
			}
			// try to find a medic from same vehicle subgroup, even driver
			medic = FindAgentMedic(entityToHeal, agentsOfHandler, agentsToExclude, false);
			if (medic)
			{
				SetVariableOut(PORT_GROUP_MEMBER_OUT,medic);
				return ENodeResult.SUCCESS;
			}
			// look for any available medic within group
			medic = FindAgentMedic(entityToHeal, groupAgents, agentsToExclude);
			if (medic)
			{
				SetVariableOut(PORT_GROUP_MEMBER_OUT,medic);
				return ENodeResult.SUCCESS;	
			}
			else 
				return ENodeResult.FAIL;
		}
		// if entityToHeal is not in a vehicle
		else 
		{
			// look for a medic on ground first, in case some agents of group are in a vehicle
			groupUtilityComponent.m_GroupMovementComponent.GetAgentsInHandler(agentsOfHandler, AIGroupMovementComponent.DEFAULT_HANDLER_ID);
			medic = FindAgentMedic(entityToHeal, agentsOfHandler, agentsToExclude);
			if (medic)
			{
				SetVariableOut(PORT_GROUP_MEMBER_OUT,medic);
				return ENodeResult.SUCCESS;	
			}
			// look for any available agent
			else
			{
				medic = FindAgentMedic(entityToHeal, groupAgents, agentsToExclude);	
				if (medic)
				{
					SetVariableOut(PORT_GROUP_MEMBER_OUT,medic);
					return ENodeResult.SUCCESS;
				}
				else 
					return ENodeResult.FAIL;
			}
		}
		
		ClearVariable(PORT_GROUP_MEMBER_OUT);
		return ENodeResult.FAIL;
	}

	//----------------------------------------------------------------------------------------------------------------------------------------------
	protected AIAgent FindAgentMedic(IEntity entToHeal, notnull array<AIAgent> agents, notnull array<AIAgent> agentsToExclude, bool excludeDrivers = false)
	{
		AIAgent closestMedic = null;
		float closestMedicDistance = float.MAX;
		
		foreach (AIAgent agent : agents)
		{
			SCR_ChimeraAIAgent chimera = SCR_ChimeraAIAgent.Cast(agent);
			SCR_AIInfoComponent info = chimera.m_InfoComponent;
			if (!info)
				continue;
			if(!agent.GetControlledEntity())
				continue;
			if(agent.GetControlledEntity() == entToHeal)
				continue;
			if(agentsToExclude.Contains(agent))
				continue;
			if (info.HasUnitState(EUnitState.UNCONSCIOUS))
				continue;
			if (excludeDrivers)
			{
				if (info.HasUnitState(EUnitState.PILOT))
					continue;
			}
			if (info.HasRole(EUnitRole.MEDIC) && info.GetAIState() == EUnitAIState.AVAILABLE)
			{
				float dist = vector.DistanceSq(agent.GetControlledEntity().GetOrigin(), entToHeal.GetOrigin());
	
				if (dist < closestMedicDistance)
				{
					closestMedicDistance = dist;
					closestMedic = agent;
				}
			}
		}
		
		return closestMedic;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_GROUP_MEMBER_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_REQUIREMENTS_IN,
		PORT_ENTITY_IN,
		PORT_AGENTS_EXCLUDE_ARRAY
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Finds group member available for medic role";
	}
}