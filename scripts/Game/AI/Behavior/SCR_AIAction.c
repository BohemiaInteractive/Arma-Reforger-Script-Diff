//! Fail reasons of actions. They can be generic or specific to some behavior.
enum EAIActionFailReason
{
	UNKNOWN,			// Unknown or unspecified
	CANCELLED,			// Cancelled by SCR_AIMessage_Cancel or something else
	TARGET_UNREACHABLE,
	NO_AMMO,
	ENTITY_DELETED		// Some crucial related entity got deleted
}

class SCR_AIActionBase : AIActionBase 
{
	// Priority levels
	const static float PRIORITY_LEVEL_NORMAL					= 0;
	const static float PRIORITY_LEVEL_PLAYER					= 1000;
	const static float PRIORITY_LEVEL_GAMEMASTER				= 2000;
	// Unit behaviors
	const static float PRIORITY_BEHAVIOR_RETREAT_MELEE					= 190 + PRIORITY_LEVEL_PLAYER;
	const static float PRIORITY_BEHAVIOR_AVOID_CHARACTER				= 180 + PRIORITY_LEVEL_PLAYER;
	const static float PRIORITY_BEHAVIOR_GET_OUT_VEHICLE_HIGH_PRIORITY 	= 162 + PRIORITY_LEVEL_PLAYER; // High priority get out for evacuations of vehicles
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_DANGER				= 160 + PRIORITY_LEVEL_PLAYER;
	const static float PRIORITY_BEHAVIOR_ATTACK_HIGH_PRIORITY			= 120 + PRIORITY_LEVEL_PLAYER;	// Attack high priority
	const static float PRIORITY_BEHAVIOR_HEAL_HIGH_PRIORITY				= 115 + PRIORITY_LEVEL_PLAYER; // Heal yourself in critical situation
	const static float PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE_HIGH_PRIORITY = 113 + PRIORITY_LEVEL_PLAYER; // Unknown fire at close range
	const static float PRIORITY_BEHAVIOR_GET_IN_VEHICLE					= 130;
	const static float PRIORITY_BEHAVIOR_GET_OUT_VEHICLE				= 125;
	const static float PRIORITY_BEHAVIOR_PICKUP_INVENTORY_ITEMS 		= 118;
	const static float PRIORITY_BEHAVIOR_STATIC_ARTILLERY  				= 114;
	const static float PRIORITY_BEHAVIOR_FIRE_ILLUM_FLARE				= 113;
	const static float PRIORITY_BEHAVIOR_THROW_GRENADE					= 112;
	const static float PRIORITY_BEHAVIOR_MEDIC_HEAL						= 111;
	const static float PRIORITY_BEHAVIOR_RETREAT_FROM_TARGET			= 110;	//		(when attack exists, retreat does not, and the other way)
	const static float PRIORITY_BEHAVIOR_PROVIDE_AMMO					= 100;
	const static float PRIORITY_BEHAVIOR_ATTACK_SELECTED				= 90;	// Attack selected
	const static float PRIORITY_BEHAVIOR_HEAL_WAIT						= 83;
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_VEHICLE_HORN			= 72;	
	const static float PRIORITY_BEHAVIOR_ATTACK_NOT_SELECTED			= 70;	// Attack not selected
	const static float PRIORITY_BEHAVIOR_OBSERVE_THREATS_HIGH_PRIORITY	= 69;	// Look at threats (gunshots), before safety is reached. !!! Priority of this must be higher than move and investigate!
	const static float PRIORITY_BEHAVIOR_MOVE_FROM_UNKNOWN_FIRE			= 68;
	const static float PRIORITY_BEHAVIOR_OBSERVE_UNKNOWN_FIRE 			= 66;	// ! Obsolete ! Stare at gunfire origin. !!! Priority of this must be higher than move and investigate!
	const static float PRIORITY_BEHAVIOR_HEAL							= 65;
	const static float PRIORITY_BEHAVIOR_MOVE_AND_INVESTIGATE			= 64;
	const static float PRIORITY_BEHAVIOR_SUPPRESS						= 63;
	const static float PRIORITY_BEHAVIOR_DEFEND							= 61;	// Defend selected waypoint	
	const static float PRIORITY_BEHAVIOR_FIND_FIRE_POSITION				= 60;
	const static float PRIORITY_BEHAVIOR_OBSERVE_LOW_PRIORITY			= 59;	// ! Obsolete ! After observe executed looking once 
	const static float PRIORITY_BEHAVIOR_MOVE_INDIVIDUALLY				= 58;
	const static float PRIORITY_BEHAVIOR_OPEN_NAVLINK_DOOR				= 54;	// Handling of door within group movement
	const static float PRIORITY_BEHAVIOR_PERFORM_ACTION					= 30;
	const static float PRIORITY_BEHAVIOR_MOVE							= 30;
	const static float PRIORITY_BEHAVIOR_MOVE_IN_FORMATION				= 30;
	const static float PRIORITY_BEHAVIOR_ATTACK_DISREGARD_THREATS		= 20;	// Attack, but when we are forbidden to shoot
	const static float PRIORITY_BEHAVIOR_WAIT							= 10;
	const static float PRIORITY_BEHAVIOR_OBSERVE_THREATS_LOW_PRIORITY	= 4;
	const static float PRIORITY_BEHAVIOR_MOVE_IN_FORMATION_LOW_PRIORITY = 3;	// When we're close to leader who isn't moving
	const static float PRIORITY_BEHAVIOR_IDLE_DRIVER					= 2;
	const static float PRIORITY_BEHAVIOR_ANIMATE						= 1.5;
	const static float PRIORITY_BEHAVIOR_IDLE							= 1;
	//const static float PRIORITY_BEHAVIOR_
	
	// Sequence of actions specific for dismounting turret and getting back
	const static float PRIORITY_BEHAVIOR_DISMOUNT_TURRET		= 300;
	const static float PRIORITY_BEHAVIOR_DISMOUNT_TURRET_INVESTIGATE = 21;
	const static float PRIORITY_BEHAVIOR_DISMOUNT_TURRET_GET_IN = 20;
	
	// Group activities
	const static float PRIORITY_ACTIVITY_GET_IN					= 130;
	const static float PRIORITY_ACTIVITY_FOLLOW					= 130;
	const static float PRIORITY_ACTIVITY_RESUPPLY				= 100;
	const static float PRIORITY_ACTIVITY_COMBAT_WITH_VEHICLES 	= 85;
	const static float PRIORITY_ACTIVITY_HEAL					= 80;
	const static float PRIORITY_ACTIVITY_ARTILLERY_SUPPORT		= 75;
	const static float PRIORITY_ACTIVITY_ATTACK_CLUSTER			= 70;
	const static float PRIORITY_ACTIVITY_SEEK_AND_DESTROY 		= 60;
	const static float PRIORITY_ACTIVITY_INVESTIGATE_CLUSTER	= 55;
	const static float PRIORITY_ACTIVITY_DEFEND_FROM_CLUSTER	= 55;
	const static float PRIORITY_ACTIVITY_MOVE					= 50;
	const static float PRIORITY_ACTIVITY_PERFORM_ACTION			= 50;
	const static float PRIORITY_ACTIVITY_DEFEND					= 50;
	const static float PRIORITY_ACTIVITY_GET_OUT				= 50;
	const static float PRIORITY_ACTIVITY_SUPPRESS				= 40;
	const static float PRIORITY_ACTIVITY_ANIMATE				= 10;
	
	
	// TODO: Get rid of flags we don't neccesarily need	
	bool m_bIsInterruptable = true;
	protected ref SCR_BTParam<float> m_fPriorityLevel = new SCR_BTParam<float>(SCR_AIActionTask.PRIORITY_LEVEL_PORT); // used when utility component calls Evaluate() on action, adds level of priority
	
	ResourceName m_sBehaviorTree;
	
	ref ScriptInvoker m_OnActionCompleted = new ScriptInvoker();
	ref ScriptInvoker m_OnActionFailed = new ScriptInvoker();
	
	// Array with parameters which must be exposed to scripts.
	// For example see how m_bPrioritize is used here.
	ref array<SCR_BTParamBase> m_aParams = {};	
	
	protected EAIActionFailReason m_eFailReason;
	
	//-------------------------------------------------------------------------------------
	override float EvaluatePriorityLevel()
	{
		return m_fPriorityLevel.m_Value;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	void SetPriorityLevel(int priority)
	{
		m_fPriorityLevel.m_Value = priority;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	EAIActionFailReason GetFailReason()
	{
		return m_eFailReason;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	//! Fail reason is an optional value which can be used to figure out why action failed.
	void SetFailReason(EAIActionFailReason failReason)
	{
		m_eFailReason = failReason;
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	//! Returns cause value, used for SCR_AISetting.
	//! The return value should tell for which reason this action is being executed,
	//! to decide if certain SCR_AISetting can be applied when this action runs, or not.
	int GetCause()
	{
		return 0;
	}
	
	//-------------------------------------------------------------------------------------
	
	#ifdef AI_DEBUG
	protected void AddDebugMessage(string str);
	#endif
	
	//-------------------------------------------------------------------------------------
	override void OnSetActionState(EAIActionState state)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetActionState: %1, %2", typename.EnumToString(EAIActionState, state), GetActionDebugInfo()));
		#endif
	}
	
	//-------------------------------------------------------------------------------------
	override void OnSetSuspended(bool suspended)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("SetSuspended: %1, %2", suspended, GetActionDebugInfo()));
		#endif
	}
	
	//-------------------------------------------------------------------------------------
	override void OnComplete()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Complete: %1", GetActionDebugInfo()));
		#endif
		OnActionCompleted();
		m_OnActionCompleted.Invoke(this);
	}
	
	//-------------------------------------------------------------------------------------
	override void OnFail()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Fail: Reason: %1, %2", m_eFailReason, GetActionDebugInfo()));
		#endif
		OnActionFailed();		
		m_OnActionFailed.Invoke(this);
	}
	
	//-------------------------------------------------------------------------------------
	override void OnActionRemoved()
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("Remove: %1", GetActionDebugInfo()));
		#endif
	}
	
	// Called for basic info to print out for specific behaviors - to override
	string GetActionDebugInfo()
	{
		return this.ToString();
	}
	
	string GetDebugPanelText() { return string.Empty; }
	
	// Called after behavior was selected after different behavior
	override void OnActionSelected() {SetActionState(EAIActionState.RUNNING);}

	// Called after behavior different behavior was selected instead of this one
	override void OnActionDeselected() {SetActionState(EAIActionState.EVALUATED);}
	
	// Called each frame behavior was selected, before it's executed in the behavior tree
	void OnActionExecuted() { }
	
	// Called when utility component recognizes completion of the behavior
	void OnActionCompleted() {SetActionState(EAIActionState.COMPLETED);}
	
	// Called when utility component recognizes failure of the behavior
	void OnActionFailed() {SetActionState(EAIActionState.FAILED);}
		
	//-------------------------------------------------------------------------------------
	bool IsActionInterruptable()
	{
		return m_bIsInterruptable || GetActionState() != EAIActionState.RUNNING;
	}
	
	//-------------------------------------------------------------------------------------
	void SetActionInterruptable(bool IsInterruptable)
	{
		m_bIsInterruptable = IsInterruptable;
	}
	
	//---------------------------------------------------------------------------------------------------
	// These methods are used by SCR_AIGetActionParameters and SCR_AISetActionParameters.
	// They help move data between action and behavior tree variables.
	
	void SetParametersToBTVariables(SCR_AIActionTask node)
	{
		foreach (SCR_BTParamBase param : m_aParams)
			param.SetVariableOut(node);
	}
	
	//-------------------------------------------------------------------------------------
	void GetParametersFromBTVariables(SCR_AIActionTask node)
	{
		foreach (SCR_BTParamBase param : m_aParams)
			param.GetVariableIn(node);
	}
	
	//-------------------------------------------------------------------------------------
	// Returns array with port names of all parameters of this action.
	TStringArray GetPortNames()
	{
		TStringArray namesOut = {};
		foreach (SCR_BTParamBase p : m_aParams)
			namesOut.Insert(p.m_sPortName);
		return namesOut;
	}
	
	//-------------------------------------------------------------------------------------
	//! Limits priority level for actions such that those are performed in "NORMAL" and "PLAYER" priority level but not in "GAMEMASTER" 
	float GetRestrictedPriorityLevel(float minimumLevel = PRIORITY_LEVEL_NORMAL)
	{
		return Math.Clamp(Math.Max(EvaluatePriorityLevel(), minimumLevel), PRIORITY_LEVEL_NORMAL, PRIORITY_LEVEL_PLAYER);
	}
};