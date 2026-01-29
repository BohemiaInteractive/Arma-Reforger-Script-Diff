class SCR_AIFindResupplier: AITaskScripted
{
	static const string PORT_REQUIREMENTS_IN	= "RequirementsIn";
	static const string PORT_ENTITY_IN	= "EntityIn";
	static const string PORT_GROUP_MEMBER_OUT	= "GroupMemberOut";
	static const string PORT_IS_UNIQUE_OUT	= "IsUniqueOut";
	static const string PORT_AGENTS_EXCLUDE_ARRAY = "AgentsExcludeArray";
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));		
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent || !m_GroupUtilityComponent.m_aInfoComponents)
			return ENodeResult.FAIL;
				
		bool notFound = true, isUnique = true, isRolePresent = false;
		SCR_AIInfoComponent aIInfoComponent;
		
		typename magazineWell;
		int magazineMaxCountIndex,magazineCount,magazineMaxCount = 0;
		
		IEntity inEntity;
		GetVariableIn(PORT_ENTITY_IN, inEntity);
		
		if (!GetVariableIn(PORT_REQUIREMENTS_IN, magazineWell))
			Debug.Error("No MagazineWell provided for resupply?!");
		
		array<AIAgent> agentsExclude;
		if (!GetVariableIn(PORT_AGENTS_EXCLUDE_ARRAY, agentsExclude))
			agentsExclude = {};
		
		int selectedIndex = m_GroupUtilityComponent.m_iGetMemberByGoalNextIndex;
		
		int length = m_GroupUtilityComponent.m_aInfoComponents.Count();
		for (int i = 0; i < length; i++)
		{
			aIInfoComponent = m_GroupUtilityComponent.m_aInfoComponents[(i + selectedIndex) % length];
			if( !aIInfoComponent )
				return ENodeResult.FAIL;
			//prevent selecting requesting EntityID
			AIAgent agent = AIAgent.Cast(aIInfoComponent.GetOwner());
			if(!agent)
				continue;
			if(agent.GetControlledEntity() == inEntity)
				continue;
			if(agentsExclude.Contains(agent))
				continue;
			if (aIInfoComponent.HasUnitState(EUnitState.UNCONSCIOUS))
				continue;
			
			
			magazineCount = aIInfoComponent.GetMagazineCountByWellType(magazineWell);
			if (magazineCount > magazineMaxCount)
			{
				if (isRolePresent)
					isUnique = false;
				else
					isRolePresent = true;
				magazineMaxCount = magazineCount;
				magazineMaxCountIndex = i;
				if (aIInfoComponent.GetAIState() == EUnitAIState.AVAILABLE)
				{
					if (notFound) 
					{
						notFound = false;
						selectedIndex = (i + selectedIndex) % length;
					}	
					else
					{ 
						selectedIndex = (i + selectedIndex) % length;
						isUnique = false;
					}
				}
			}
		}
		
		if (length != 0)
			m_GroupUtilityComponent.m_iGetMemberByGoalNextIndex = (selectedIndex + 1) % length; // If we don't do +1, next time we start checking same member
		else
			m_GroupUtilityComponent.m_iGetMemberByGoalNextIndex = 0;
		
		if (!isRolePresent)
		{
			return ENodeResult.FAIL;
		}
		
		if (!notFound)
		{
			AIAgent agent = AIAgent.Cast(m_GroupUtilityComponent.m_aInfoComponents[selectedIndex].GetOwner());
			if (agent)
				SetVariableOut(PORT_GROUP_MEMBER_OUT,agent);
			else 
				return ENodeResult.FAIL;
			
			SetVariableOut(PORT_IS_UNIQUE_OUT,isUnique);
			return ENodeResult.SUCCESS;
		}
		else if (isUnique) // I have only one unit that is not available = unit that requested the resupply 
		{
			ClearVariable(PORT_GROUP_MEMBER_OUT);
			SetVariableOut(PORT_IS_UNIQUE_OUT,isUnique);
			return ENodeResult.FAIL;
		}
		
		ClearVariable(PORT_GROUP_MEMBER_OUT);
		ClearVariable(PORT_IS_UNIQUE_OUT);
		
		
		return ENodeResult.RUNNING;
	}
	
	//------------------------------------------------------------------------------------------------
	static override protected bool CanReturnRunning()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_GROUP_MEMBER_OUT,
		PORT_IS_UNIQUE_OUT
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
		return "Getter returns group member available for ressuply";
	}
};