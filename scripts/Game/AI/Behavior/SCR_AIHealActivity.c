class SCR_AIHealActivity : SCR_AIActivityBase
{
	const string GROUP_MEMBERS_EXCLUDE_PORT = "AgentsExclude";
	const string MEDIC_PORT = "MedicEntity";
	
	const int TIMEOUT_FAILED = 5*60*1000;
	
	ref SCR_BTParam<IEntity> m_EntityToHeal = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	
	// Selected medic. Assigned from behavior tree.
	ref SCR_BTParam<IEntity> m_MedicEntity = new SCR_BTParam<IEntity>(MEDIC_PORT);
	
	// BTParam can't hold a ref to object, so we hold a ref to it ourselves through a member variable
	ref array<AIAgent> m_aMedicsExclude = {};
	ref SCR_BTParam<array<AIAgent>> m_aMedicsExcludeParam = new SCR_BTParam<array<AIAgent>>(GROUP_MEMBERS_EXCLUDE_PORT);
	
	protected float m_fTimeCreated_world; // World time when this was created
	
	protected float m_fTimeCheckConditions_world;
	
	// Features
	static ref array<ref SCR_AIActivityFeatureBase> s_ActivityFeatures = {new SCR_AIHealActivitySmokeCoverFeature()};
	override array<ref SCR_AIActivityFeatureBase> GetActivityFeatures() { return s_ActivityFeatures; }
	
	override float CustomEvaluate()
    {
		float worldTime = GetGame().GetWorld().GetWorldTime();
		
		if (worldTime - m_fTimeCheckConditions_world > 2500)
		{
			// Fail if target is null or destroyed
			if (!SCR_AIDamageHandling.IsAlive(m_EntityToHeal.m_Value))
			{
				#ifdef AI_DEBUG
				AddDebugMessage("Target entity is null or destroyed, action failed.");
				#endif
				
				Fail();
				return 0;
			}
			
			// Complete if entity is not wounded any more
			if (!SCR_AIDamageHandling.IsCharacterWounded(m_EntityToHeal.m_Value))
			{
				Complete();
				return 0;
			}
			
			// Replan if medic is null or destroyed
			if (!SCR_AIDamageHandling.IsAlive(m_MedicEntity.m_Value))
			{
				SetActionIsSuspended(false);
			}
			
			m_fTimeCheckConditions_world = worldTime;
		}
		
		// Fail on timeout
		if (worldTime - m_fTimeCreated_world > TIMEOUT_FAILED)
		{
			#ifdef AI_DEBUG
			AddDebugMessage("Timeout, action failed.");
			#endif
			
			Fail();
			return 0;
		}
		
		
		
		return GetPriority();
	}
	
	void InitParameters(IEntity entity, array<AIAgent> medicsToExclude, float priorityLevel)
	{
		IEntity medic = null;
		m_EntityToHeal.Init(this, entity);
		m_aMedicsExcludeParam.Init(this, medicsToExclude);
		m_fPriorityLevel.Init(this,priorityLevel);
		m_MedicEntity.Init(this, medic); // Must use a variable, otherwise null directly doesn't work with templates.
	}	
	
	//-------------------------------------------------------------------------------------------------------
	void SCR_AIHealActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity ent, float priority = PRIORITY_ACTIVITY_HEAL, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityHeal.bt";
		SetPriority(priority);
		InitParameters(ent, m_aMedicsExclude, priorityLevel);
		SetIsUniqueInActionQueue(false);
		
		auto game = GetGame();
		if (game)
		{
			m_fTimeCreated_world = game.GetWorld().GetWorldTime();
		}
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " healing unit " + m_EntityToHeal.m_Value.ToString();
	}
	
	override bool OnMessage(AIMessage msg)
	{
		SCR_AIMessage_HealFailed healFailed = SCR_AIMessage_HealFailed.Cast(msg);
		
		if (healFailed)
		{
			// Did someone fail to heal our entity?
			if (healFailed.m_TargetEntity != m_EntityToHeal.m_Value)
				return false;
			
			// Don't pick this medic any more
			m_aMedicsExclude.Insert(healFailed.GetSender());
			
			#ifdef AI_DEBUG
			AddDebugMessage(string.Format("Medic %1 has failed the action. MedicsExclude: %2", healFailed.GetSender(), m_aMedicsExclude));
			#endif
			
			// Request re-run of this activity
			SetActionIsSuspended(false);
			return true;
		}
		
		return false;
	}
};

class SCR_AIGetHealActivityParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIHealActivity(null, null, null)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};

class SCR_AISetHealActivityParameters : SCR_AISetActionParameters
{
	protected static ref TStringArray s_aVarsIn = (new SCR_AIHealActivity(null, null, null)).GetPortNames();
	override TStringArray GetVariablesIn() { return s_aVarsIn; }
	static override bool VisibleInPalette() { return true; }
};