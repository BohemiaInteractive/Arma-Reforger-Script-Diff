class SCR_AIAllocateActionsForDefendActivity: AITaskScripted
{
	static const string PORT_WAYPOINT				= "WaypointIn";
	static const string PORT_WAYPOINT_RELATED		= "WaypointRelated";
	static const string PORT_PRIORITY_LEVEL			= "PriorityLevel";
	
	protected SCR_AIGroup m_groupOwner;
	protected bool m_bWaypointRelated;
	protected float m_fPriorityLevel;
	protected SCR_DefendWaypoint m_RelatedWaypoint;
	protected SCR_AIGroupUtilityComponent m_Utility;
	protected SCR_MailboxComponent m_Mailbox;
	protected SCR_AIDefendActivity m_RelatedActivity;
	
	//------------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
	
	//------------------------------------------------------------------------------------------------
	override void OnInit(AIAgent owner)
	{
		m_groupOwner = SCR_AIGroup.Cast(owner);
		if (!m_groupOwner)
		{
			m_groupOwner = SCR_AIGroup.Cast(owner.GetParentGroup());
			if (!m_groupOwner)
			{
				SCR_AgentMustBeAIGroup(this, owner);
				return;
			}	
		}
		m_Utility = SCR_AIGroupUtilityComponent.Cast(m_groupOwner.FindComponent(SCR_AIGroupUtilityComponent));
		m_Mailbox = m_Utility.m_Mailbox;
		m_RelatedActivity = SCR_AIDefendActivity.Cast(m_Utility.GetCurrentAction());
	}
	
	//------------------------------------------------------------------------------------------------
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_groupOwner)
			return ENodeResult.FAIL;
		IEntity waypointEnt;
		
		if (!m_Mailbox || !m_Utility || !m_RelatedActivity)
			return ENodeResult.FAIL;
		
		GetVariableIn(PORT_WAYPOINT,waypointEnt);
		GetVariableIn(PORT_WAYPOINT_RELATED,m_bWaypointRelated);
		GetVariableIn(PORT_PRIORITY_LEVEL,m_fPriorityLevel);
		
		array<AIAgent> groupMembers = {};		
		m_groupOwner.GetAgents(groupMembers);
		if (groupMembers.IsEmpty())
			return ENodeResult.FAIL;

		int numSAToOccupy;
		
		// Preparation of turrets and actions array
		m_RelatedWaypoint = SCR_DefendWaypoint.Cast(waypointEnt);
		if (!m_RelatedWaypoint)
			return NodeError(this, owner, "Defend Waypoint not provided to the node!");
		
		SCR_DefendWaypointPreset defendPreset = m_RelatedWaypoint.GetCurrentDefendPreset();
		if (!defendPreset)
			return SCR_AIErrorMessages.NodeErrorDefendPreset(this, owner);
		
		bool useTurrets = defendPreset.GetUseTurrets();
		bool teleport = m_RelatedWaypoint.GetFastInit();
		
		ref array<BaseCompartmentSlot> compartments = {};
		ref array<AISmartActionComponent> smartActions = {};
		
		if (useTurrets)
		{
			m_groupOwner.GetAllocatedCompartments(compartments);
			for (int i = compartments.Count() - 1; i >= 0; i--)
			{
				if (!TurretCompartmentSlot.Cast(compartments[i]))
					compartments.Remove(i);			
			}	
		}
		
		float fraction = defendPreset.GetFractionOfSA();
		if (fraction > 0)
		{
			m_groupOwner.GetAllocatedSmartActions(smartActions);
			numSAToOccupy = Math.Round(fraction * smartActions.Count());		
		}		
			
		// Distribution of turrets, actions and sector defends (randomized)
		for (int i = 0, max = groupMembers.Count(); i < max; i++)
		{
			int j = groupMembers.GetRandomIndex();
			if (useTurrets && compartments.Count() > 0)
			{
				OccupyTurret(groupMembers[j], compartments[0], teleport);
				compartments.Remove(0);
				groupMembers.Remove(j);
				continue;
			}
			if (numSAToOccupy > 0)
			{
				OccupySA(groupMembers[j], smartActions[0], teleport);
				smartActions.Remove(0);
				groupMembers.Remove(j);
				numSAToOccupy--;
				continue;
			}
			m_RelatedActivity.AddAgentToRadialCover(groupMembers[j]);
			groupMembers.Remove(j);
		}
		
		// Remaining agents to defend sectors of Waypoint
		if (m_RelatedActivity.GetRadialCoverAgentsCount() > 0)
		{
			m_RelatedActivity.AllocateAgentsToRadialCover();
			m_RelatedActivity.ClearRadialCoverAgents();
		};
		
		return ENodeResult.SUCCESS;
	}	
	
	//Sends order to agent, reserves compartment
	protected bool OccupyTurret(AIAgent who, BaseCompartmentSlot slot, bool teleport)
	{
		IEntity vehicle = slot.GetVehicle();
		if (!vehicle)
			return false;
		
		SCR_AIVehicleUsageComponent vehicleUsageComp = SCR_AIVehicleUsageComponent.FindOnNearestParent(vehicle, vehicle);
		if (!vehicleUsageComp)
			return false;
		
		if (teleport)
		{
			IEntity entityToTeleport = who.GetControlledEntity();
			if (!entityToTeleport)
				return false;
			CompartmentAccessComponent CAComponent = CompartmentAccessComponent.Cast(entityToTeleport.FindComponent(CompartmentAccessComponent));
			if(!CAComponent.GetInVehicle(vehicle, slot, true, -1, ECloseDoorAfterActions.INVALID, false))
				Print("Unable to teleport to Turret", LogLevel.WARNING); // might be still in the process of teleporting...
		}		
		SCR_AIMessage_GetIn getInMessage = SCR_AIMessage_GetIn.Create(vehicle, null, EAICompartmentType.Turret, m_bWaypointRelated, m_fPriorityLevel, m_RelatedWaypoint, m_RelatedActivity, slot);
		m_Mailbox.RequestBroadcast(getInMessage, who);
		m_groupOwner.GetGroupUtilityComponent().AddUsableVehicle(vehicleUsageComp);
		
		return true;
	}
	
	//Sends order to agent, reserves smart action
	protected bool OccupySA(AIAgent who, AISmartActionComponent smartAction, bool teleport)
	{
		if (teleport)
		{
			vector originOfObject, positionToTeleport;
			vector mat[4];
			
			originOfObject = smartAction.GetOwner().GetOrigin();
			positionToTeleport = originOfObject + smartAction.GetActionOffset();
			BaseGameEntity entityToTeleport = BaseGameEntity.Cast(who.GetControlledEntity());
			if (!entityToTeleport)
				return false;
			Math3D.MatrixIdentity4(mat);
			mat[3] = positionToTeleport;
			
			entityToTeleport.Teleport(mat); // teleporting entity
		}
		
		SCR_AISmartActionComponent smartActionComponent = SCR_AISmartActionComponent.Cast(smartAction);
		if (!smartActionComponent)
			return false;
		smartActionComponent.ReserveAction(who);
		SCR_AIMessage_PerformAction actionMessage = SCR_AIMessage_PerformAction.Create(null, "", smartActionComponent, m_bWaypointRelated, m_fPriorityLevel, m_RelatedWaypoint, m_RelatedActivity);
		m_Mailbox.RequestBroadcast(actionMessage, who);
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected static ref TStringArray s_aVarsIn = {
		PORT_WAYPOINT,
		PORT_WAYPOINT_RELATED,
		PORT_PRIORITY_LEVEL		
	};
	override TStringArray GetVariablesIn()
	{
		return s_aVarsIn;
	}
	
	//------------------------------------------------------------------------------------------------
	static override string GetOnHoverDescription()
	{
		return "AllocateActionsForDefendActivity: Goes over all group members and alocates them either turret, smart action or sector defend.\n Works only inside defend activity under defend waypoint.";
	}
}