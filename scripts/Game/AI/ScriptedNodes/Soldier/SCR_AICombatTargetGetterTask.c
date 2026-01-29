//! Base class for tasks which return some target from combat component
class SCR_AICombatTargetGetterTask : AITaskScripted
{
	protected SCR_AICombatComponent m_CombatComponent;
	
	protected static const string ENEMY_ENTITY_PORT 		= "EntityOut";
	protected static const string ENEMY_LAST_SEEN_POS_PORT 	= "EntityLastSeenPos";
	protected static const string ENEMY_LAST_SEEN_TIME_PORT = "EntityLastSeenTime";
	protected static const string ENEMY_TARGET_INFO_PORT 	= "TargetInfoOut";
	
	ref SCR_AITargetInfo m_TargetInfo;
	
	//-------------------------------------------------------------------------------------------
	protected override void OnInit(AIAgent owner)
	{
		m_CombatComponent = SCR_AICombatComponent.Cast(owner.GetControlledEntity().FindComponent(SCR_AICombatComponent));
	}
	
	//-------------------------------------------------------------------------------------------
	//! Must be overridden in child classes
	protected BaseTarget GetEnemy(SCR_AICombatComponent combatComp);
	
	
	//-------------------------------------------------------------------------------------------
	protected override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!m_CombatComponent)
			return ENodeResult.FAIL;
		
		BaseTarget targetEnemy = GetEnemy(m_CombatComponent);
		
		if (!targetEnemy)
			return Fail();
		
		IEntity entEnemy = targetEnemy.GetTargetEntity();
		
		if (!entEnemy)
			return Fail();
		
		m_TargetInfo = new SCR_AITargetInfo();
		m_TargetInfo.Init(entEnemy, worldPos: targetEnemy.GetLastSeenPosition(), timestamp: targetEnemy.GetTimeLastSeen());
		
		SetVariableOut(ENEMY_ENTITY_PORT, entEnemy);
		SetVariableOut(ENEMY_LAST_SEEN_POS_PORT, m_TargetInfo.m_vWorldPos);
		SetVariableOut(ENEMY_LAST_SEEN_TIME_PORT, m_TargetInfo.m_fTimestamp);
		SetVariableOut(ENEMY_TARGET_INFO_PORT, m_TargetInfo);
		return ENodeResult.SUCCESS;
	}
	
	//-------------------------------------------------------------------------------------------
	protected ENodeResult Fail()
	{
		IEntity _enemy;
		SetVariableOut(ENEMY_ENTITY_PORT, _enemy); // must be null to make deco test NotNull work <- unassigned variable does not 
		ClearVariable(ENEMY_LAST_SEEN_POS_PORT);
		ClearVariable(ENEMY_LAST_SEEN_TIME_PORT);
		ClearVariable(ENEMY_TARGET_INFO_PORT);
		return ENodeResult.FAIL;
	}
	
	//-------------------------------------------------------------------------------------------
	static ref TStringArray s_aVarsOut =
	{
		ENEMY_ENTITY_PORT,
		ENEMY_LAST_SEEN_POS_PORT,
		ENEMY_LAST_SEEN_TIME_PORT,
		ENEMY_TARGET_INFO_PORT
	};
	protected override TStringArray GetVariablesOut()
	{
		return s_aVarsOut;
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return false;}
};


class SCR_AIGetCurrentEnemy: SCR_AICombatTargetGetterTask
{
	//-------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns current enemy from SCR_AICombatComponent.GetCurrentTarget()";
	}
	
	//-------------------------------------------------------------------------------------------
	override protected BaseTarget GetEnemy(SCR_AICombatComponent combatComp)
	{
		return combatComp.GetCurrentTarget();
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
};


class SCR_AIGetLastSeenEnemy: SCR_AICombatTargetGetterTask
{
	//-------------------------------------------------------------------------------------------
	protected static override string GetOnHoverDescription()
	{
		return "Returns current enemy from SCR_AICombatComponent.GetLastSeenEnemy()";
	}
	
	//-------------------------------------------------------------------------------------------
	override protected BaseTarget GetEnemy(SCR_AICombatComponent combatComp)
	{
		return combatComp.GetLastSeenEnemy();
	}
	
	//-------------------------------------------------------------------------------------------
	static override bool VisibleInPalette() {return true;}
};