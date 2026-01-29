class SCR_AIGetOverwatchMembers: AITaskScripted
{
	static const string PORT_FIRETEAM_IN			= "FireteamIn";
	static const string PORT_TARGET_IN				= "TargetIn";
	static const string PORT_MOVING_MEMBER_OUT		= "MovingMemberOut";
	static const string PORT_COVERING_MEMBER_OUT	= "CoveringMemberOut";
	
	SCR_AIGroupUtilityComponent m_GroupUtilityComponent;
	
	int m_selectedIndexForMovement = 0, m_selectedIndexForCovering = 0;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_GroupUtilityComponent = SCR_AIGroupUtilityComponent.Cast(group.FindComponent(SCR_AIGroupUtilityComponent));
	}	
	
	// We are selecting always a pair of AI. The one who is moving and other one for covering.
	// Node will fails if roles are missing and we can never select a pair.
	// The assumption about selection is that AIs are selected for move till there is only one left who is covering.
	// So at least one member of fire team is covering the rest.
	// Node is running till a pair is selected.
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_GroupUtilityComponent || !m_GroupUtilityComponent.m_aInfoComponents)
			return ENodeResult.FAIL;

		ClearVariable(PORT_MOVING_MEMBER_OUT);
		ClearVariable(PORT_COVERING_MEMBER_OUT);
		
		int teamSelection,length;
		if (!GetVariableIn(PORT_FIRETEAM_IN,teamSelection))
		{
			return NodeError(this, owner, "Missing team selection for this node.");
		}
		
		IEntity targetEntity;
		if (!GetVariableIn(PORT_TARGET_IN, targetEntity))
			return NodeError(this, owner, "Missing target entity for this node");
		
		if (!targetEntity)
			return ENodeResult.FAIL;
		
		AIAgent agent;
		
		bool isCoveringPresent = false, isMovingPresent = false, isCoveringSelected = false, isMovingSelected = false;
		SCR_AIInfoComponent aIInfoComponent;
		length = m_GroupUtilityComponent.m_aInfoComponents.Count();
		if (length <= 1)
			return ENodeResult.FAIL;
		
		// Select moving team member, take furthest member in our fireteam
		int selectedIndexForMovementPrevious = m_selectedIndexForMovement;
		for (int i = 0; i < length; i++)
		{
			aIInfoComponent = m_GroupUtilityComponent.m_aInfoComponents[i];
			if( !aIInfoComponent )
				continue;
			
			// Handeling of fireteams should be reworked so we can have geter for firetam members insted iteration over whole team each time 
			// todo fix this
			/*
			if (aIInfoComponent.GetFireTeam() == teamSelection && aIInfoComponent.HasRole(EUnitRole.RIFLEMAN))
			{
				if (aIInfoComponent.GetAIState() == EUnitAIState.AVAILABLE)
				{
					isMovingPresent = true;
					
					AIAgent memberAgent = AIAgent.Cast(aIInfoComponent.GetOwner());
					IEntity memberEnt = memberAgent.GetControlledEntity();
					float distToTarget = vector.Distance(memberEnt.GetOrigin(), targetEntity.GetOrigin());
					
					if (distToTarget > distMax)
					{
						isMovingSelected = true;
						m_selectedIndexForMovement = i;
						distMax = distToTarget;
					}
				}
			}
			*/
		}
		
		if (!isMovingPresent)
		{
			return ENodeResult.FAIL;
		}
		
		if (!isMovingSelected)
		{
			return ENodeResult.RUNNING;
		}
		
		for (int i = 1; i <= length; i++)
		{
			int indexForCover = (i + m_selectedIndexForCovering) % length;
			
			if (indexForCover == m_selectedIndexForMovement)
				continue;
			
			aIInfoComponent = m_GroupUtilityComponent.m_aInfoComponents[indexForCover];
			if( !aIInfoComponent )
				continue;

			// todo fix this
			/*
			if (aIInfoComponent.GetFireTeam() == teamSelection && 
				(aIInfoComponent.HasRole(EUnitRole.RIFLEMAN) || aIInfoComponent.HasRole(EUnitRole.MACHINEGUNNER) || aIInfoComponent.HasUnitState(EUnitState.IN_TURRET)))
			{
				isCoveringPresent = true;
				//Selection conditions could be improved
				//We doesn't ensure that covering element is actually able to cover e.g. has ammo, isn't somewhere behind the wall or far away.
				if (aIInfoComponent.GetAIState() == EUnitAIState.AVAILABLE)
				{
					isCoveringSelected = true;
					m_selectedIndexForCovering = indexForCover;
					break;
				}
			}
			*/
		}
		
		if (!isCoveringPresent)
		{
			return ENodeResult.FAIL;
		}
		
		if (!isCoveringSelected)
		{
			//we shouldn't iterate when selection wasn't succesfull
			m_selectedIndexForMovement = selectedIndexForMovementPrevious;
			return ENodeResult.RUNNING;
		}
		
		agent = AIAgent.Cast(m_GroupUtilityComponent.m_aInfoComponents[m_selectedIndexForMovement].GetOwner());
		if (agent)
			SetVariableOut(PORT_MOVING_MEMBER_OUT,agent);
		else 
			return ENodeResult.FAIL;
		
		agent = AIAgent.Cast(m_GroupUtilityComponent.m_aInfoComponents[m_selectedIndexForCovering].GetOwner());
		if (agent)
			SetVariableOut(PORT_COVERING_MEMBER_OUT,agent);
		else
		{	
			ClearVariable(PORT_MOVING_MEMBER_OUT);
			return ENodeResult.FAIL;
		}

		return ENodeResult.SUCCESS;
	}
	
	//------------------------------------------------------------------------------------------------
	static override protected bool CanReturnRunning()
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsOut = {
		PORT_MOVING_MEMBER_OUT,
		PORT_COVERING_MEMBER_OUT
	};
	override TStringArray GetVariablesOut()
    {
        return s_aVarsOut;
    }

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_FIRETEAM_IN,
		PORT_TARGET_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }

	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Getter returns unit available for cover and unit available for movement";
	}	
};