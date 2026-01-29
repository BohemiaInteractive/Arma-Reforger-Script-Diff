class SCR_AIFollowActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_Entity = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<float> m_fCompletionDistance = new SCR_BTParam<float>(SCR_AIActionTask.DESIREDDISTANCE_PORT);
	protected CharacterControllerComponent m_TargetCharacterController;
	protected bool m_bTargetEntityIsCharacter;
	
	//------------------------------------------------------------------------------------------------
	void InitParameters(IEntity entity,float priorityLevel)
	{
		m_Entity.Init(this, entity);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	//------------------------------------------------------------------------------------------------
	// SCR_AIBaseUtilityComponent utility, bool isWaypointRelated, vector pos, float priority = PRIORITY_ACTIVITY_MOVE, IEntity ent = null, bool useVehicles = true
	void SCR_AIFollowActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, vector pos, IEntity ent, EMovementType movementType = EMovementType.RUN, bool useVehicles = false, float priority = PRIORITY_ACTIVITY_FOLLOW, float priorityLevel = PRIORITY_LEVEL_NORMAL, float distance = 1.0)
	{
		InitParameters(ent, priorityLevel);
		m_fCompletionDistance.Init(this, distance);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityFollow.bt";
		SetPriority(priority);

		if (!utility)
			return;
		
		ChimeraCharacter character = ChimeraCharacter.Cast(ent);
		if (!character)
			return;
		else
		{
			m_bTargetEntityIsCharacter = true;
			m_TargetCharacterController = character.GetCharacterController();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override string GetActionDebugInfo()
	{
		return this.ToString() + " following entity " + m_Entity.m_Value.ToString();
	}
	
	//------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		// Check if followed entity is character (not vehicle)
		if (m_bTargetEntityIsCharacter) 
		{
			// Fail if target character to follow is null or unconscious
			if (!m_TargetCharacterController || m_TargetCharacterController.IsUnconscious())
				Fail();
		}
		
		return GetPriority();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		SendCancelMessagesToAllAgents();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnActionDeselected()
	{
		super.OnActionDeselected();
		SendCancelMessagesToAllAgents();
	}
};
