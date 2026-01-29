/*!
Behavior for one soldier to provide ammunition to another one.
*/
class SCR_AIProvideAmmoBehavior : SCR_AIBehaviorBase
{
	protected float MOVE_COMPLETION_RADIUS = 10;
	
	static const string AMMO_CONSUMER_ENTITY_PORT = "AmmoConsumerEntity";
	static const string MAGAZINE_WELL_TYPE_PORT = "MagazineWellType";
	
	ref SCR_BTParam<IEntity> m_AmmoConsumer = new SCR_BTParam<IEntity>(AMMO_CONSUMER_ENTITY_PORT);
	ref SCR_BTParam<typename> m_MagazineWellType = new SCR_BTParam<typename>(MAGAZINE_WELL_TYPE_PORT);
	
	ref SCR_AIMoveIndividuallyBehavior m_MoveBehavior;
	
	//------------------------------------------------------------------------------------------------------------------------------------
	void SCR_AIProvideAmmoBehavior(SCR_AIUtilityComponent utility, SCR_AIActivityBase groupActivity,
		IEntity ammoConsumer, typename magazineWellType, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		SetPriority(SCR_AIActionBase.PRIORITY_BEHAVIOR_PROVIDE_AMMO);
		m_fPriorityLevel.m_Value = priorityLevel;
		m_sBehaviorTree = "{ADA6E7C5BC3C8C8D}AI/BehaviorTrees/Chimera/Soldier/ProvideAmmo.bt";
		
		m_AmmoConsumer.Init(this, ammoConsumer);
		m_MagazineWellType.Init(this, magazineWellType);
		
		if (!utility)
			return;
		
		float movePriority = GetPriority() + 0.1;
		m_MoveBehavior = new SCR_AIMoveIndividuallyBehavior(utility, groupActivity, vector.Zero, priority : movePriority, priorityLevel: priorityLevel, ent: ammoConsumer, radius: 30.0);
		utility.AddAction(m_MoveBehavior);
	}
	
	//---------------------------------------------------------------------------------------------------------------------------------
	override int GetCause()
	{
		return SCR_EAIBehaviorCause.GROUP_GOAL;
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override float CustomEvaluate()
	{
		// Fail is our ammo consumer is not alive any more
		if(!SCR_AIDamageHandling.IsAlive(m_AmmoConsumer.m_Value))
		{
			Fail();
			if (m_MoveBehavior)
				m_MoveBehavior.Fail();
			return 0;
		}
		
		return GetPriority();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionCompleted()
	{
		super.OnActionCompleted();
		if (m_MoveBehavior)
			m_MoveBehavior.Complete();
	}
	
	//------------------------------------------------------------------------------------------------------------------------------------
	override void OnActionFailed()
	{
		super.OnActionFailed();
		if (m_MoveBehavior)
			m_MoveBehavior.Fail();
	}
}

class SCR_AIGetProvideAmmoBehaviorParameters : SCR_AIGetActionParameters
{
	static ref TStringArray s_aVarsOut = (new SCR_AIProvideAmmoBehavior(null, null, null, BaseMagazineWell)).GetPortNames();
	override TStringArray GetVariablesOut() { return s_aVarsOut; }
	
	static override bool VisibleInPalette() { return true; }
};