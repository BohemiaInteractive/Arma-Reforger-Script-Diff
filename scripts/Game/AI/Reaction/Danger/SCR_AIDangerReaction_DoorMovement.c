[BaseContainerProps()]
class SCR_AIDangerReaction_DoorMovement : SCR_AIDangerReaction
{
	static const float AGENT_DEFAULT_RADIUS = 0.3;
	static const float AGENT_DEFAULT_HEIGHT = 1.8;
	static const float REACTION_DIST_SQ = 100.0;
#ifdef WORKBENCH
	ref Shape s1,s2,s3,s4;
#endif
	
	override bool PerformReaction(notnull SCR_AIUtilityComponent utility, notnull SCR_AIThreatSystem threatSystem, AIDangerEvent dangerEvent, int dangerEventCount)
	{
		IEntity door = dangerEvent.GetObject();
		if (!door)
			return false;
		vector agentPos = utility.GetOrigin();
		vector doorPos = door.GetOrigin();
		
		//Initial filter for hearing distance (TODO: optimization spatial grid)
		float distSq = vector.DistanceSq(agentPos, doorPos);
		if (distSq >= REACTION_DIST_SQ)
			return false;
		
		vector min, max;
		float distToDoorY = doorPos[1] - agentPos[1];
		//If agent is below door more than agent height, skip 
		if (distToDoorY > AGENT_DEFAULT_HEIGHT)
			return false;
			
		door.GetBounds(min, max);
		vector doorExtent = max - min;
		//If agent above door more than door height, skip
		if (-distToDoorY > doorExtent[1])
			return false;
		
		float distToDoorXZ = vector.DistanceXZ(doorPos, agentPos) - AGENT_DEFAULT_RADIUS;
		doorExtent[1] = 0.0;
		float doorLength = doorExtent.Length();
		//If the agent isn't in XZ range, skip
		if (distToDoorXZ > doorLength)
			return false;
	
		//Ensure the necessary entities and components are valid
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		if (!agent)
			return false;
		
		//Turn around if the instigator of door is an enemy,
		IEntity actionStarter = dangerEvent.GetVictim();
		if (agent.IsPerceivedEnemy(actionStarter))
		{
			utility.AddAction(new SCR_AIMoveIndividuallyBehavior(utility, null, agentPos, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, 0, null, AGENT_DEFAULT_RADIUS));
			utility.m_LookAction.LookAt(actionStarter.GetOrigin(), utility.m_LookAction.PRIO_DANGER_EVENT);
			return true;
		}
			
		ChimeraCharacter character = ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (character && character.IsInVehicle())
			return false;
		
		DoorComponent doorComponent = DoorComponent.Cast(door.FindComponent(DoorComponent));
		if (!doorComponent)
			return false;
		
		vector doorNormal = door.VectorToParent(vector.Forward);
		bool flipLogic = doorComponent.GetAngleRange() < 0.0; // door that open -90 have the normals in oposite direction
		if (flipLogic) 
			doorNormal = -1.0 * doorNormal;
		if (doorComponent.IsOpening())
			doorNormal = -1.0 * doorNormal;
		
		vector agentDirection = agentPos - doorComponent.GetDoorPivotPointWS();
		agentDirection.Normalize();
		
		vector rightHand = vector.Up * doorNormal; // vector pointing in direction of door revolvment perpend. to doorNormal
		if (flipLogic) 
			rightHand = -1.0 * rightHand;
		if (doorComponent.IsClosing())
			rightHand = -1.0 * rightHand;
		
		IEntity doorFrame = door.GetParent();
		if (!doorFrame)
			doorFrame = door;
		
		vector doorFrameUp = doorFrame.VectorToParent(vector.Up);
		vector fleeingDir;
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
		{
			s1 = Shape.CreateArrow(doorPos, doorPos + agentDirection, 0.1, COLOR_RED, ShapeFlags.NOZBUFFER);
			s2 = Shape.CreateArrow(doorPos, doorPos + doorNormal, 0.1, COLOR_BLUE, ShapeFlags.NOZBUFFER);
			s3 = Shape.CreateArrow(doorPos, doorPos + rightHand, 0.1, COLOR_GREEN, ShapeFlags.NOZBUFFER);
		}
#endif
		float dotP1 = vector.Dot(doorNormal, agentDirection);
		float dotP2 = vector.Dot(rightHand, agentDirection);
		float distanceToMove;
		bool isDoorOpeningUpwards = !float.AlmostEqual(vector.Dot(doorNormal, doorFrameUp),0); // bar gates have rotation planes paralel with up vector of "door frame"
		
		if (isDoorOpeningUpwards)
		{
			if (!doorComponent.IsClosing()) // ignoring opening gate
				return false;
			else 
			{
				distToDoorXZ = vector.DistanceXZ(doorFrame.GetOrigin(), agentPos) - AGENT_DEFAULT_RADIUS;
				doorLength = 2.0; // bit hacky but simple
				if (dotP2 < 0)
					fleeingDir = -1.0 * rightHand;
				else 
					fleeingDir = rightHand;
				distanceToMove = Math.Clamp((doorLength - distToDoorXZ) * 2.0, doorLength, 2.0 * doorLength);
			}
		}
		else
		{
			if (dotP1 < 0) 			// standing in the other halfplane, than the one where the door is revolving
				return false;
			else if (dotP2 > 0) 	// standing in the halfplane of revolving door but not in the quadrant
				return false;
			if  (doorComponent.IsClosing())
				fleeingDir = -1.0 * rightHand;
			else
				fleeingDir = doorNormal;
			distanceToMove = Math.Clamp((doorLength - distToDoorXZ) * 2.0, 2.0 * AGENT_DEFAULT_RADIUS, doorLength);
		}
		
#ifdef WORKBENCH
		if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AI_SHOW_DEBUG_SHAPES))
			s4 = Shape.CreateArrow(agentPos, agentPos + fleeingDir * distanceToMove, 0.5, COLOR_YELLOW, ShapeFlags.NOZBUFFER);
#endif
		vector movePos = agentPos + fleeingDir * distanceToMove;
		
		//Step away and free door space
		utility.AddAction(new SCR_AIMoveIndividuallyBehavior(utility, null, movePos, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, 0, null, AGENT_DEFAULT_RADIUS));
		return true;
	}
};