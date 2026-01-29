class SCR_AIGetSmartActionsState : AITaskScripted
{
	static const string WAYPOINT_ENTITY_IN = "WaypointEntityIn";

	private EAIWaypointCompletionType m_eCompletionType;
	private int m_iNumberOfSALeft = 0;
	private ENodeResult m_eSACompletedResult = ENodeResult.RUNNING;
	private bool m_bLeaderWasSuccessful;
	private bool m_bIsRegisteredTags = false;

	private SCR_SmartActionWaypoint m_prevWaypoint;
	private ref array<string> m_arrayOfTags;
	private AIAgent m_leaderAgent;

	private IEntity m_SmartActionEntity;
	private SCR_SmartActionWaypoint m_waypoint;
	private string m_smartActionTag;
	private ref array<Managed> m_smartActionComponents;

	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		WAYPOINT_ENTITY_IN
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }	

	//------------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_smartActionComponents = new array<Managed>;
		m_arrayOfTags = new array<string>;
	}

	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		AIWaypoint targetWaypoint;
		GetVariableIn("WaypointEntityIn", targetWaypoint);

		if (!targetWaypoint)
		{
			NodeError(this, owner, "Waypoint parameter is not of correct type");
			return ENodeResult.FAIL;
		};

		AIGroup group = AIGroup.Cast(owner);
		if (group)
			m_leaderAgent = group.GetLeaderAgent();
		else m_leaderAgent = owner;

		m_waypoint =  SCR_SmartActionWaypoint.Cast(targetWaypoint);
	// is the waypoint new or same as in prev. check?
		if (m_prevWaypoint != m_waypoint)
		{
			if (m_waypoint)
			{
				if (m_prevWaypoint && m_smartActionComponents)
					UnregisterSmartActionsWithTag(m_smartActionTag,m_smartActionComponents);
				
				m_eCompletionType = targetWaypoint.GetCompletionType();
				m_waypoint.GetSmartActionEntity(m_SmartActionEntity, m_smartActionTag);
				if (!m_SmartActionEntity || m_smartActionTag.IsEmpty())
					NodeError(this, owner, "Waypoint is not set properly");

				FindAllSmartActionComponents(m_SmartActionEntity,m_smartActionComponents);
				m_iNumberOfSALeft = RegisterSmartActionsWithTag(m_smartActionTag, m_smartActionComponents);
				m_prevWaypoint = m_waypoint;			
			}
		};

		switch (m_eCompletionType)
		{
			case EAIWaypointCompletionType.All:
			{
				// we want all of the SA to be completed, not all were yet reported back but one has failed now
				if (m_eSACompletedResult == ENodeResult.FAIL)
				{
					ResetVariables();
					return ENodeResult.FAIL;
				}
				else if (m_iNumberOfSALeft == 0)
				{
					ResetVariables();
					return ENodeResult.SUCCESS;
				}
				break;
			}
			case EAIWaypointCompletionType.Any:
			{
				// we succeed the node the first time the SA completes
				if (m_eSACompletedResult == ENodeResult.SUCCESS)
				{
					ResetVariables();
					return ENodeResult.SUCCESS;
				}
				else if (m_iNumberOfSALeft == 0)
				{
					ResetVariables();
					return ENodeResult.FAIL;
				}
				break;
			}
			case EAIWaypointCompletionType.Leader:
			{
				if (m_bLeaderWasSuccessful)
				{
					ResetVariables();
					return ENodeResult.SUCCESS;
				}
				else if (m_iNumberOfSALeft == 0)
				{
					ResetVariables();
					return ENodeResult.FAIL;
				}
				break;
			}
		}
		return ENodeResult.RUNNING;	
	}

	//------------------------------------------------------------------------------------------------
	override void OnAbort(AIAgent owner, Node nodeCausingAbort)
	{
		ResetVariables();
	}

	//------------------------------------------------------------------------------------------------
	int FindAllSmartActionComponents(IEntity entityContainingSmartActions, out array<Managed> smartActionComponents)
	{
		entityContainingSmartActions.FindComponents(SCR_AISmartActionComponent,smartActionComponents);
		IEntity child = entityContainingSmartActions.GetChildren();
		while (child)
		{
			array<Managed> smartActionComponentsOfOneEntity = {};
			child.FindComponents(SCR_AISmartActionComponent,smartActionComponentsOfOneEntity);
			smartActionComponents.InsertAll(smartActionComponentsOfOneEntity);
			child = child.GetSibling();
		}
		return smartActionComponents.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	int RegisterSmartActionsWithTag(string tag, array<Managed> smartActionComponents)
	{
		int numberResult;
		foreach (Managed managedComp : smartActionComponents)
		{
			SCR_AISmartActionComponent comp = SCR_AISmartActionComponent.Cast(managedComp);
			comp.GetTags(m_arrayOfTags);
			if (m_arrayOfTags.Contains(tag))
			{
				numberResult ++;
				comp.GetOnActionEnd().Insert(OnSmartActionEnd);
				comp.GetOnActionFailed().Insert(OnSmartActionFailed);
			}
		}
		m_bIsRegisteredTags = true;
		return numberResult;
	}

	//------------------------------------------------------------------------------------------------
	void UnregisterSmartActionsWithTag(string tag, array<Managed> smartActionComponents)
	{
		if (!m_bIsRegisteredTags)
			return;	

		foreach (Managed managedComp : smartActionComponents)
		{
			if (managedComp)
			{
				SCR_AISmartActionComponent comp = SCR_AISmartActionComponent.Cast(managedComp);
				comp.GetTags(m_arrayOfTags);
				if (m_arrayOfTags.Contains(tag))
				{
					comp.GetOnActionEnd().Remove(OnSmartActionEnd);
					comp.GetOnActionFailed().Remove(OnSmartActionFailed);
				}
			}	
		}
		m_arrayOfTags.Clear();
		m_smartActionComponents = null;
		m_bIsRegisteredTags = false;
	}

	//------------------------------------------------------------------------------------------------
	void OnSmartActionEnd(AIAgent user)
	{
		m_iNumberOfSALeft = Math.Max(m_iNumberOfSALeft - 1,0);
		if (user == m_leaderAgent)	
			m_bLeaderWasSuccessful = true;
		m_eSACompletedResult = ENodeResult.SUCCESS;
	}

	//------------------------------------------------------------------------------------------------
	void OnSmartActionFailed(AIAgent user)
	{
		m_iNumberOfSALeft = Math.Max(m_iNumberOfSALeft - 1,0);
		m_eSACompletedResult = ENodeResult.FAIL;
	}

	//------------------------------------------------------------------------------------------------
	private void ResetVariables()
	{
		UnregisterSmartActionsWithTag(m_smartActionTag, m_smartActionComponents);
		m_iNumberOfSALeft = 0;
		m_eSACompletedResult = ENodeResult.RUNNING;
		m_bLeaderWasSuccessful = false;
		m_leaderAgent = null;
		m_waypoint = null;
	}

	//------------------------------------------------------------------------------------------------
	protected static override bool VisibleInPalette()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static override bool CanReturnRunning()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Node that checks if smart actions from given waypoint have been completed depending on settings of the waypoint, returns SUCCESS if they were.";
	}	
};