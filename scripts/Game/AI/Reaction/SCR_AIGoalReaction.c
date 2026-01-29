//------------------------------------------------------------------------------------------------
// GOAL REACTION BASE
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIGoalReaction
{
	// Don't use, it's here for backwards compatibility, not guaranteed to work in all classes
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Don't use, it's here for backwards compatibility, not guaranteed to work in all classes", params: "bt")]
	string m_OverrideBehaviorTree;
	
	[Attribute("0", UIWidgets.ComboBox, "Type of event activating the reaction", "", ParamEnumArray.FromEnum(EMessageType_Goal) )]
	EMessageType_Goal m_eType;
	
	void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message) {}
	void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message) {}
};


//------------------------------------------------------------------------------------------------
// GOAL REACTIONS - Reactions on different orders
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class SCR_AIGoalReaction_Attack : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Attack msg = SCR_AIMessage_Attack.Cast(message); // new approach: m_Target can be null!
		if (!msg)
			return;
		
		utility.m_CombatComponent.UpdateLastSeenPosition(msg.m_TargetInfo.m_Entity, msg.m_TargetInfo);
		utility.m_CombatComponent.SetAssignedTargets({msg.m_TargetInfo.m_Entity}, null);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackCluster : SCR_AIGoalReaction
{
	// Duration of investigation if target is not seen
	const float INVESTIGATION_DURATION_S = 120.0;
	
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_AttackCluster msg = SCR_AIMessage_AttackCluster.Cast(message);
		if (!msg)
			return;
		
		// Does it still exist?
		if (!msg.m_TargetClusterState || !msg.m_TargetClusterState.m_Cluster)
			return;
		
		// ------ Set assigned targets
		utility.m_CombatComponent.SetAssignedTargets(null, msg.m_TargetClusterState);
			
		
		// ------ Should we investigate there?
		
		// Check if we have identified at least one target
		if (msg.m_bAllowInvestigate)
		{
			bool targetIdentified = false;
			foreach (IEntity targetEntity : msg.m_TargetClusterState.m_Cluster.m_aEntities)
			{
				if (utility.m_PerceptionComponent.GetTargetPerceptionObject(targetEntity, ETargetCategory.ENEMY))
				{
					targetIdentified = true;	// Found in 'Enemy' category - we have direct sight on it
					break;
				}
				else
					continue;
			}
			
			// If we haven't identified any target, go there
			if (!targetIdentified && msg.m_bAllowInvestigate)
			{
				// Cancel all other investigations
				utility.SetStateAllActionsOfType(SCR_AIMoveAndInvestigateBehavior, EAIActionState.FAILED);
				
				vector investigatePos;
				float investigateRadius;
				SCR_AIInvestigateClusterActivity.CalculateInvestigationArea(msg.m_TargetClusterState, investigatePos, investigateRadius);
				
				auto investigateBehavior = new SCR_AIMoveAndInvestigateBehavior(utility, msg.m_RelatedGroupActivity,
					investigatePos, radius: investigateRadius,
					priorityLevel: msg.m_fPriorityLevel, isDangerous: true,
					duration: INVESTIGATION_DURATION_S);
				utility.AddAction(investigateBehavior);
			}
		}
		else
		{
			// Report 'Covering' if we are not supposed to investigate there
			SCR_AICommsHandler commsHandler = utility.m_CommsHandler;
			if (!commsHandler.CanBypass())
			{
				SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_COVERING, null, vector.Zero, 0, false, false, SCR_EAITalkRequestPreset.MEDIUM);
				commsHandler.AddRequest(rq);
			}
		}
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_AttackClusterDone : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_AttackClusterDone msg = SCR_AIMessage_AttackClusterDone.Cast(message);
		if (!msg)
			return;
		
		utility.m_CombatComponent.ResetAssignedTargets();
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_CoverCluster : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		// Now we don't have a behavior to cover those who are attacking/ivnestigating a cluster yet
		// We just use it for voice line
		SCR_AICommsHandler commsHandler = utility.m_CommsHandler;
		if (!commsHandler.CanBypass())
		{
			SCR_AITalkRequest rq = new SCR_AITalkRequest(ECommunicationType.REPORT_COVERING, null, vector.Zero, 0, false, false, SCR_EAITalkRequestPreset.MEDIUM);
			commsHandler.AddRequest(rq);
		}
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_Move : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Move.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIMoveIndividuallyBehavior(utility, msg.m_RelatedGroupActivity,
			msg.m_MovePosition, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY, msg.m_fPriorityLevel, msg.m_FollowEntity);

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Move.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIMoveActivity(utility, msg.m_RelatedWaypoint, msg.m_MovePosition,
			msg.m_FollowEntity, msg.m_eMovementType, msg.m_bUseVehicles, SCR_AIActionBase.PRIORITY_ACTIVITY_MOVE, priorityLevel: msg.m_fPriorityLevel);
		
		utility.SetStateAllActionsOfType(SCR_AISearchAndDestroyActivity,EAIActionState.FAILED); // move fails seek and destroy
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Follow : SCR_AIGoalReaction
{	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{		
		auto msg = SCR_AIMessage_Follow.Cast(message);
		if (!msg || !msg.m_FollowEntity)
			return;
		
		auto activity = new SCR_AIFollowActivity(utility, msg.m_RelatedWaypoint, vector.Zero, msg.m_FollowEntity,
			msg.m_eMovementType, false, SCR_AIActionBase.PRIORITY_ACTIVITY_FOLLOW, priorityLevel: msg.m_fPriorityLevel, distance: msg.m_fDistance);
		utility.SetStateAllActionsOfType(SCR_AISearchAndDestroyActivity, EAIActionState.FAILED); // move fails seek and destroy
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Investigate : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Investigate.Cast(message);
		if (!msg)
			return;
		
		// Cancel previous investigations
		utility.SetStateAllActionsOfType(SCR_AIMoveAndInvestigateBehavior, EAIActionState.FAILED);
			
		auto behavior = new SCR_AIMoveAndInvestigateBehavior(utility, msg.m_RelatedGroupActivity, msg.m_vMovePosition,
			SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE, msg.m_fPriorityLevel, isDangerous: msg.m_bIsDangerous, radius: msg.m_fRadius, targetUnitType: msg.m_eTargetUnitType, duration: msg.m_fDuration);
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_MoveInFormation : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessageGoal msg = SCR_AIMessageGoal.Cast(message);
		
		auto behavior = new SCR_AIMoveInFormationBehavior(utility, msg.m_RelatedGroupActivity, msg.m_fPriorityLevel);
			
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GetInVehicle : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetIn.Cast(message);
		if (!msg)
			return;
		if (!msg.m_Vehicle)
			return;
		// from msg.m_eRoleInVehicle and msg.m_Vehicle we deduce empty compartment to get in, if none found, we dont add behavior get in
		BaseCompartmentSlot compartmentSlot;
		EAICompartmentType wantedCompartmentType = msg.m_eRoleInVehicle;
		if (msg.m_CompartmentSlot)
		{
			compartmentSlot = msg.m_CompartmentSlot;
			wantedCompartmentType = SCR_AICompartmentHandling.CompartmentClassToType(compartmentSlot.Type());
		}
		else
		{
			BaseCompartmentManagerComponent compMan = BaseCompartmentManagerComponent.Cast(msg.m_Vehicle.FindComponent(BaseCompartmentManagerComponent));
			if (!compMan)
				return;
			array<BaseCompartmentSlot> compartments = {};
			compMan.GetCompartments(compartments);
			foreach (BaseCompartmentSlot comp: compartments)
			{
				if (msg.m_eRoleInVehicle == EAICompartmentType.None)
					wantedCompartmentType = SCR_AICompartmentHandling.CompartmentClassToType(comp.Type());
				
				if (comp.IsReserved() || (SCR_AICompartmentHandling.CompartmentClassToType(comp.Type()) != wantedCompartmentType))
					continue;
				
				IEntity occupant = comp.GetOccupant();
				if (occupant && occupant == utility.m_OwnerEntity)
					// we are already inside correct compartment type! we are done -> do not add new behavior
					return;
				else if (occupant && SCR_AIDamageHandling.IsConscious(occupant))
					continue;
				comp.SetReserved(utility.m_OwnerEntity);
				compartmentSlot = comp;
				break;
			}
		}
		if (!compartmentSlot)
		{
#ifdef AI_DEBUG
			PrintFormat("Warning: unit %1 got to get in %2 but there is no room for it in comp type %3!", utility.m_OwnerEntity, msg.m_Vehicle, msg.m_eRoleInVehicle);
#endif
			return;
		}
		auto behavior = new SCR_AIGetInVehicle(utility, msg.m_RelatedGroupActivity, msg.m_Vehicle, compartmentSlot, wantedCompartmentType, SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_IN_VEHICLE, msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetIn.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIGetInActivity(utility, msg.m_RelatedWaypoint, msg.m_Vehicle, msg.m_BoardingParams, msg.m_eRoleInVehicle, priorityLevel: msg.m_fPriorityLevel);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_GetOutVehicle : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetOut.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIGetOutVehicle(utility, msg.m_RelatedGroupActivity, msg.m_Vehicle, delay_s: msg.m_fDelay_s, priority: SCR_AIActionBase.PRIORITY_BEHAVIOR_GET_OUT_VEHICLE, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_GetOut.Cast(message);
		if (!msg)
			return;
		
		// if current activity is follow, make get out a priority so AI disembarks vehicle first
		float customPriorityLevel;
		SCR_AIFollowActivity follow = SCR_AIFollowActivity.Cast(utility.GetCurrentAction());
		if (follow)
			customPriorityLevel = SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER;
		else
			customPriorityLevel = msg.m_fPriorityLevel;
		
		auto activity = new SCR_AIGetOutActivity(utility, msg.m_RelatedWaypoint, msg.m_Vehicle, msg.m_BoardingParams, priorityLevel: customPriorityLevel);
		
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_SeekAndDestroy : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
  		auto msg = SCR_AIMessage_SeekAndDestroy.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AISearchAndDestroyActivity(utility, msg.m_RelatedWaypoint, msg.m_MovePosition, ent: msg.m_FollowEntity, useVehicles: msg.m_bUseVehicles, priorityLevel: msg.m_fPriorityLevel);
		
		utility.SetStateAllActionsOfType(SCR_AIMoveActivity,EAIActionState.FAILED);
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Heal : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Heal.Cast(message);
		if (!msg)
			return;

		auto behavior = new SCR_AIMedicHealBehavior(utility, msg.m_RelatedGroupActivity, msg.m_EntityToHeal, false, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.WrapBehaviorOutsideOfVehicle(behavior);
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Heal.Cast(message);
		if (!msg)
			return;
		
		SCR_AIActionBase currentAction = SCR_AIActionBase.Cast(utility.GetCurrentAction());
		if (!currentAction)
			return;
		
		float priorityLevelClamped = currentAction.GetRestrictedPriorityLevel(msg.m_fPriorityLevel);		

		// Check if we already have an activity to heal this soldier 
		array <ref AIActionBase> actions = {};
		utility.FindActionsOfType(SCR_AIHealActivity, actions);
		foreach (AIActionBase action : actions)
		{
			SCR_AIHealActivity healActivity = SCR_AIHealActivity.Cast(action);
			
			//Return if we are already healing this soldier
			if (healActivity.m_EntityToHeal.m_Value == msg.m_EntityToHeal)
			{
				healActivity.SetPriorityLevel(priorityLevelClamped);
				return;
			}
		}


		auto activity = new SCR_AIHealActivity(utility, msg.m_RelatedWaypoint, msg.m_EntityToHeal, priorityLevel: priorityLevelClamped);

		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_HealWait : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_HealWait.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIHealWaitBehavior(utility, msg.m_RelatedGroupActivity, msg.m_HealProvider, msg.m_fPriorityLevel);
		
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Defend : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Defend.Cast(message);
		if (!msg || !msg.m_RelatedWaypoint)
			return;
		
		auto behavior = new SCR_AIDefendBehavior(utility, msg.m_RelatedGroupActivity, msg.m_RelatedWaypoint,
			msg.m_vDefendDirection, msg.m_fDefendAngularRange, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;

		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Defend.Cast(message);
		if (!msg || !msg.m_RelatedWaypoint)
			return;
		
		auto activity = new SCR_AIDefendActivity(utility, msg.m_RelatedWaypoint, msg.m_vDefendDirection,
			priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_PerformAction : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_PerformAction.Cast(message);
		if (!msg)
			return;
		
		auto behavior = new SCR_AIPerformActionBehavior(utility, msg.m_RelatedGroupActivity, msg.m_SmartActionComponent, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_PerformAction.Cast(message);
		if (!msg)
			return;
		
		auto activity = new SCR_AIPerformActionActivity(utility, msg.m_RelatedWaypoint, msg.m_SmartActionEntity, msg.m_SmartActionTag, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_OpenNavlinkDoor : SCR_AIGoalReaction
{
	static const string SMART_ACTION_TAG = "OpenGate";
	
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_OpenNavlinkDoor.Cast(message);
		if (!msg)
			return;
		
		if (!msg.m_DoorEntity)
			return;
		
		SCR_AISmartActionComponent smartActionComp = SCR_AISmartActionHandling.FindSmartAction(msg.m_DoorEntity, SMART_ACTION_TAG, false); 
		
		if (!smartActionComp)
			return;
		
		SCR_AIPerformActionBehavior performActionBehavior = new SCR_AIPerformActionBehavior(utility,
			msg.m_RelatedGroupActivity, smartActionComp,
			priority: SCR_AIActionBase.PRIORITY_BEHAVIOR_OPEN_NAVLINK_DOOR,
			priorityLevel: msg.m_fPriorityLevel);
				
		utility.WrapBehaviorOutsideOfVehicle(performActionBehavior);
		utility.AddAction(performActionBehavior);
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_Cancel : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_Cancel msg = SCR_AIMessage_Cancel.Cast(message);
		if (!msg || !msg.m_RelatedGroupActivity)
			return;
		//utility.SetStateOfRelatedAction(msg.m_RelatedGroupActivity, EAIActionState.FAILED); TODO: fix code, this is not working right
		array<ref AIActionBase> actions = {};
		utility.GetActions(actions);
		foreach(AIActionBase action: actions)
		{
			if (action.GetRelatedGroupActivity() == msg.m_RelatedGroupActivity)
			{
				SCR_AIActionBase actionBase = SCR_AIActionBase.Cast(action);
				if (actionBase)
					actionBase.SetFailReason(EAIActionFailReason.CANCELLED);
				action.Fail();
			}
		};
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ThrowGrenadeTo : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ThrowGrenadeTo msg = SCR_AIMessage_ThrowGrenadeTo.Cast(message);
		if (!msg)
			return;
		
		SCR_AIThrowGrenadeToBehavior behavior = new SCR_AIThrowGrenadeToBehavior(utility, msg.m_RelatedGroupActivity, msg.m_vTargetPosition, msg.e_WeaponType, msg.m_fDelay, msg.m_fPriorityLevel);
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_FireIllumFlareAt : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_FireIllumFlareAt msg = SCR_AIMessage_FireIllumFlareAt.Cast(message);
		if (!msg)
			return;
		
		BaseWeaponComponent weaponComponent;
		BaseMagazineComponent magazineComponent;
		int muzzleId;
		
		// Don't react if we can't fire a flare
		if (!SCR_AIActivityIllumFlareFeature.GetAgentIllumWeaponAndMuzzle(utility.m_AIInfo, weaponComponent, magazineComponent, muzzleId))
			return;
		
		SCR_AIFireIllumFlareBehavior behavior = new SCR_AIFireIllumFlareBehavior(utility, msg.m_RelatedGroupActivity, msg.m_vTargetPosition, weaponComponent, magazineComponent, muzzleId, msg.m_fPriorityLevel);
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ProvideAmmo : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ProvideAmmo msg = SCR_AIMessage_ProvideAmmo.Cast(message);
		if (!msg)
			return;
		
		SCR_AIProvideAmmoBehavior behavior = new SCR_AIProvideAmmoBehavior(utility, msg.m_RelatedGroupActivity,
			msg.m_AmmoConsumer, msg.m_MagazineWell, msg.m_fPriorityLevel);
		
		utility.AddAction(behavior);
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_PickupInventoryItems : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_PickupInventoryItems msg = SCR_AIMessage_PickupInventoryItems.Cast(message);
		if (!msg)
			return;
		
		SCR_AIPickupInventoryItemsBehavior behavior = new SCR_AIPickupInventoryItemsBehavior(utility, msg.m_RelatedGroupActivity,
			msg.m_vPickupPosition, msg.m_MagazineWellType, msg.m_fPriorityLevel);
		
		utility.AddAction(behavior);
	}
}

[BaseContainerProps()]
class SCR_AIGoalReaction_Suppress : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{		
		SCR_AIMessage_Suppress msg = SCR_AIMessage_Suppress.Cast(message);
		if (!msg || !msg.m_Volume)
			return;
				
		SCR_AISuppressBehavior behavior = new SCR_AISuppressBehavior(utility, msg.m_RelatedGroupActivity, msg.m_Volume, msg.m_fDuration_s, fireRate: msg.m_fFireRate, priorityLevel: msg.m_fPriorityLevel);
		utility.AddAction(behavior);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_Animate : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Animate.Cast(message);
		if (!msg || !msg.m_RootEntity)
			return;
		
		auto behavior = new SCR_AIAnimateBehavior(utility, msg.m_RelatedGroupActivity, rootEntity: msg.m_RootEntity, priorityLevel: msg.m_fPriorityLevel, scriptForAgent: msg.m_AgentScript, relatedInvoker:  msg.m_RelatedInvoker);
		if (m_OverrideBehaviorTree != string.Empty)
			behavior.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(behavior);
	}
	
	override void PerformReaction(notnull SCR_AIGroupUtilityComponent utility, SCR_AIMessageBase message)
	{
		auto msg = SCR_AIMessage_Animate.Cast(message);
		if (!msg || !msg.m_RelatedWaypoint)
			return;
		
		auto activity = new SCR_AIAnimateActivity(utility, msg.m_RelatedWaypoint, priorityLevel: msg.m_fPriorityLevel);
		if (m_OverrideBehaviorTree != string.Empty)
			activity.m_sBehaviorTree = m_OverrideBehaviorTree;
		
		utility.AddAction(activity);
	}
};

[BaseContainerProps()]
class SCR_AIGoalReaction_ArtillerySupport : SCR_AIGoalReaction
{
	override void PerformReaction(notnull SCR_AIUtilityComponent utility, SCR_AIMessageBase message)
	{
		SCR_AIMessage_ArtillerySupport msg = SCR_AIMessage_ArtillerySupport.Cast(message);
		if (!msg)
			return;
		
		array<ref AIActionBase> actions = {};
		utility.FindActionsOfType(SCR_AIStaticArtilleryBehavior, actions);
		foreach (auto a : actions)
			a.Fail();
		
		SCR_AIStaticArtilleryBehavior b = new SCR_AIStaticArtilleryBehavior(utility, msg.m_RelatedGroupActivity, msg.m_ArtilleryEntity, msg.m_vTargetPos, msg.m_eAmmoType, priorityLevel: msg.m_fPriorityLevel);
		utility.AddAction(b);
	}
};
