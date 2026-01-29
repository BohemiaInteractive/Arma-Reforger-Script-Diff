// TODO:
// Better calculation of actual threat - can he see me, does he aim at me, etc.
// Grenade landed event

enum EAIThreatState
{
	SAFE,
	VIGILANT,
	ALERTED,
	THREATENED
}

typedef func SCR_AIThreatStateChangedCallback;
void SCR_AIThreatStateChangedCallback(EAIThreatState prevState, EAIThreatState newState);
typedef ScriptInvokerBase<SCR_AIThreatStateChangedCallback> SCR_AIThreatStateChangedInvoker;

class SCR_AIThreatSystem
{	
	private static const float THREAT_SHOT_DROP_RATE = 	0.11 * 0.001;	//!< Falloff (percentual drop per milisecond)
	private static const float THREAT_SUPPRESSION_DROP_RATE = 0.1 * 0.001;
	private static const float THREAT_ENDANGERED_DROP_RATE = 	0.2 * 0.001;
	
	private static const float BLEEDING_FIXED_INCREMENT = 0.3;
	private static const float SUPPRESSION_BULLET_INCREMENT = 0.10;
	private static const float ZERO_DISTANCE_SHOT_INCREMENT = 0.008;
	private static const float DISTANT_SHOT_INCREMENT = 0.002;
	// Decrased from 0.6 until friendly explosions can be filtered out on game code level
	private static const float EXPLOSION_MAX_INCREMENT = 0.2;   
	
	private static const float EXPLOSION_CLOSE_DISTANCE = 12;	//!< What distance in m is considered close - max increment is used
	
	// Decrased from 700 until friendly explosions can be filtered out on game code level
	static const float EXPLOSION_MAX_DISTANCE = 100;			//!< Maxmial distance from explosion to have any influence on threat level
	
	private static const float ENDANGERED_INCREMENT = 0.2;
	
	static const float VIGILANT_THRESHOLD = 0.05;
	protected static const float ALERTED_THRESHOLD = 0.33;
	static const float THREATENED_THRESHOLD = 0.66;
	
	//! When threat is below this level, our attack against enemy is delayed
	//! count about 36 seconds until m_fThreatIsEndangered drops to this value
	static const float ATTACK_DELAYED_THRESHOLD = 0.001;

	//range between <0,1>
	private float m_fThreatTotal;
	private float m_fThreatSuppression;
	private float m_fThreatShotsFired;
	private float m_fThreatIsEndangered; //!< Endangered mean somebody is aiming at me
	private float m_fThreatInjury;

	private SCR_AIUtilityComponent				m_Utility;
	private SCR_AIConfigComponent				m_Config;
	private SCR_AICombatComponent				m_Combat;
	private SCR_ExtendedDamageManagerComponent	m_DamageManager;
	
	private SCR_ChimeraAIAgent m_Agent;
	
	private EAIThreatState m_State;
		
	private ref SCR_AIThreatStateChangedInvoker m_OnThreatStateChanged = new SCR_AIThreatStateChangedInvoker();
	
	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] utility
	void SCR_AIThreatSystem(SCR_AIUtilityComponent utility)
	{
		m_Utility = utility;
		m_Config = utility.m_ConfigComponent;	
		m_Combat = utility.m_CombatComponent;
		m_DamageManager = SCR_ExtendedDamageManagerComponent.Cast(utility.m_OwnerEntity.FindComponent(SCR_ExtendedDamageManagerComponent));
		SCR_ChimeraAIAgent agent = SCR_ChimeraAIAgent.Cast(utility.GetOwner());
		if (!agent)
			return;
		m_Agent = agent;
		
		// AI threat system is owned by Utility Component, therefore we don't unsubscribe from the event
		if (m_DamageManager)
		{
			m_DamageManager.GetOnDamageEffectAdded().Insert(OnDamageEffectAdded);
			m_DamageManager.GetOnDamageEffectRemoved().Insert(OnDamageEffectRemoved);
		}
			
		m_State = EAIThreatState.SAFE;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_AIThreatStateChangedInvoker GetOnThreatStateChanged()
	{
		return m_OnThreatStateChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	EAIThreatState GetState()
	{
		return m_State;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return Sum of all threats without the effect of injuries - used for deciding to patch oneself
	float GetThreatMeasureWithoutInjuryFactor()
	{
		return m_fThreatTotal - m_fThreatInjury;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetSuppressionMeasure()
	{
		return m_fThreatSuppression;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetThreatMeasure()
	{
		return m_fThreatTotal;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//!
	void ShowDebug()
	{
		// Show message above AI's head

		Color color;
		
		switch (m_State)
		{
			case EAIThreatState.SAFE:
			{
				color = Color.FromInt(Color.GREEN);
				break;
			}
			case EAIThreatState.VIGILANT:
			{
				color = Color.FromInt(Color.DARK_GREEN);
				break;
			}
			case EAIThreatState.ALERTED:
			{
				color = Color.FromInt(Color.DARK_YELLOW);
				break;
			}
			case EAIThreatState.THREATENED:
			{
				color = Color.FromInt(Color.DARK_RED);
				break;
			}
		}
		
		SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, typename.EnumToString(EAIThreatState, m_State), EAIDebugCategory.THREAT, 1.4, color);	
	}
#endif // WORKBENCH
	
	//------------------------------------------------------------------------------------------------
	private void StateTransition(EAIThreatState newState)
	{
		if (newState == m_State)
			return;
		
		m_OnThreatStateChanged.Invoke(m_State, newState);
		
		m_State = newState;
	}
	
	//------------------------------------------------------------------------------------------------
	private void UpdateState()
	{
		EAIThreatState newState = EAIThreatState.SAFE;
		
		if (m_fThreatTotal > THREATENED_THRESHOLD)
			newState = EAIThreatState.THREATENED;
		else if (m_fThreatTotal > ALERTED_THRESHOLD)
			newState = EAIThreatState.ALERTED;
		else if (m_fThreatTotal > VIGILANT_THRESHOLD)
			newState = EAIThreatState.VIGILANT;
		
		StateTransition(newState);
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! Called by utilityComponent each EvaluateBehavior call
	//! \param[in] utility
	//! \param[in] timeSlice
	void Update(SCR_AIUtilityComponent utility, float timeSlice)
	{
		// Threat falloff
		m_fThreatSuppression -= m_fThreatSuppression * THREAT_SUPPRESSION_DROP_RATE * timeSlice;
		m_fThreatShotsFired -= m_fThreatShotsFired * THREAT_SHOT_DROP_RATE * timeSlice;
		
		if (m_Combat)
		{
			if (m_Combat.GetCurrentTarget())
				m_fThreatIsEndangered = ENDANGERED_INCREMENT;
			else
				m_fThreatIsEndangered -= m_fThreatIsEndangered * THREAT_ENDANGERED_DROP_RATE * timeSlice;
		}

		// Process all danger events and clear the array
		if (m_Agent && m_Config.m_EnableDangerEvents)
		{
			int i;
			AIDangerEvent dangerEvent;
			
#ifdef AI_DEBUG
			if (m_Agent.GetDangerEventsCount() != 0)
				AddDebugMessage(string.Format("Processing danger events: %1 in the queue", m_Agent.GetDangerEventsCount()));
#endif
			
			for (int max = m_Agent.GetDangerEventsCount(); i < max; i++)
			{
				int eventAggregationCount;
				dangerEvent = m_Agent.GetDangerEvent(i, eventAggregationCount);
				
				
				#ifdef AI_DEBUG
				AddDebugMessage(string.Format("PerformDangerReaction: %1x %2", eventAggregationCount, dangerEvent));
				#endif
				
				if (dangerEvent)
				{
					if (m_Config.PerformDangerReaction(m_Utility, dangerEvent, eventAggregationCount))
					{
#ifdef WORKBENCH
						string message = typename.EnumToString(EAIDangerEventType, dangerEvent.GetDangerType());
						SCR_AIDebugVisualization.VisualizeMessage(m_Utility.m_OwnerEntity, message, EAIDebugCategory.DANGER, 2);	// Show message above AI's head
#endif
					}
				}
			}

			m_Agent.ClearDangerEvents(i + 1);
		}		

		// Add threat value from current behavior
		float threatFromBehavior;
		if (utility.m_CurrentBehavior)
			threatFromBehavior = utility.m_CurrentBehavior.m_fThreat;
		
		m_fThreatTotal = Math.Clamp(threatFromBehavior + m_fThreatSuppression + m_fThreatInjury + m_fThreatShotsFired + m_fThreatIsEndangered, 0, 1);
		
		UpdateState();
#ifdef WORKBENCH
		ShowDebug();
#endif
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectAdded(notnull SCR_DamageEffect dmgEffect)
	{
		if (dmgEffect.GetDamageType() != EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
			return;
		
		m_fThreatInjury = BLEEDING_FIXED_INCREMENT;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnDamageEffectRemoved(notnull SCR_DamageEffect dmgEffect)
	{
		if (dmgEffect.GetDamageType() != EDamageType.BLEEDING || !DotDamageEffect.Cast(dmgEffect))
			return;
		
		if (!m_DamageManager.GetAllPersistentEffectsOfType(SCR_BleedingDamageEffect))
			m_fThreatInjury = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] count
	void ThreatBulletImpact(int count)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("ThreatBulletImpact: %1", count));
		#endif
		
		m_fThreatSuppression = Math.Clamp(m_fThreatSuppression + count*SUPPRESSION_BULLET_INCREMENT, 0, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] distance
	void ThreatExplosion(float distance)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("ThreatExplosion: %1", distance));
		#endif
		
		// Max increment is default as we assume that explosion happened cloaser than EXPLOSION_CLOSE_DISTANCE
		float increment = EXPLOSION_MAX_INCREMENT;
		
		// If distance is more than EXPLOSION_CLOSE_DISTANCE perform reversed linear mapping of threat increment
		if (distance > EXPLOSION_CLOSE_DISTANCE)
			increment = Math.Map(distance, EXPLOSION_CLOSE_DISTANCE, EXPLOSION_MAX_DISTANCE, EXPLOSION_MAX_INCREMENT, 0);
		
		// Increment has to be positive or 0
		if (increment < 0)
			return;
		
		m_fThreatSuppression += increment;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] distance
	//! \param[in] count
	void ThreatShotFired(float distance, int count)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("ThreatShotFired: %1, %2", distance, count));
		#endif
		
		// google can show you the increment function if you write it in
		m_fThreatShotsFired = Math.Clamp(m_fThreatShotsFired + count*(DISTANT_SHOT_INCREMENT + ZERO_DISTANCE_SHOT_INCREMENT/(distance + 1)), 0, THREATENED_THRESHOLD);
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] count
	void ThreatProjectileFlyby(int count)
	{
		#ifdef AI_DEBUG
		AddDebugMessage(string.Format("ThreatProjectileFlyby"));
		#endif
		
		m_fThreatSuppression = Math.Clamp(m_fThreatSuppression + count * SUPPRESSION_BULLET_INCREMENT, 0, 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Retrieve all threat values. primarily used for save/load logic
	void GetThreatValues(out float suppression, out float shotsFired, out float injury, out float endangered)
	{
		suppression = m_fThreatSuppression;
		shotsFired = m_fThreatShotsFired;
		injury = m_fThreatInjury;
		endangered = m_fThreatIsEndangered;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Apply threat values obtained through GetThreatValues. primarily used for save/load logic
	void SetThreatValues(float suppression, float shotsFired, float injury, float endangered)
	{
		m_fThreatSuppression = suppression;
		m_fThreatShotsFired = shotsFired;
		m_fThreatInjury = injury;
		m_fThreatIsEndangered = endangered;
	}

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] w
	void DebugPrintToWidget(TextWidget w)
	{
		w.SetText(
			typename.EnumToString(EAIThreatState, m_State) + "\n "
			+ m_fThreatTotal.ToString(1,4) + "\n "
			+ m_fThreatSuppression.ToString(1,4) + "\n "
			+ m_fThreatShotsFired.ToString(1,4) + "\n "
			+ m_fThreatInjury.ToString(1,4) + "\n "
			+ m_fThreatIsEndangered.ToString(1,4));
		;
	}
	
	#ifdef AI_DEBUG
	//--------------------------------------------------------------------------------------------
	protected void AddDebugMessage(string str)
	{
		m_Utility.m_AIInfo.AddDebugMessage(str, msgType: EAIDebugMsgType.THREAT);
	}
	#endif
}
