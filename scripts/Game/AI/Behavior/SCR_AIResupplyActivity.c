class SCR_AIResupplyActivity : SCR_AIActivityBase
{
	ref SCR_BTParam<IEntity> m_EntityToResupply = new SCR_BTParam<IEntity>(SCR_AIActionTask.ENTITY_PORT);
	ref SCR_BTParam<typename> m_MagazineWell = new SCR_BTParam<typename>(SCR_AIActionTask.MAGAZINE_WELL_PORT);
	
	//-------------------------------------------------------------------------------------------------------------
	void InitParameters(IEntity entityToResupply, typename magazineWell, float priorityLevel)
	{
		m_EntityToResupply.Init(this, entityToResupply);
		m_MagazineWell.Init(this, magazineWell);
		m_fPriorityLevel.Init(this, priorityLevel);
	}
	
	//-------------------------------------------------------------------------------------------------------------
	void SCR_AIResupplyActivity(SCR_AIGroupUtilityComponent utility, AIWaypoint relatedWaypoint, IEntity ent, typename magazineWell, float priority = PRIORITY_ACTIVITY_RESUPPLY, float priorityLevel = PRIORITY_LEVEL_NORMAL)
	{
		InitParameters(ent, magazineWell, priorityLevel);
		m_sBehaviorTree = "AI/BehaviorTrees/Chimera/Group/ActivityResupply.bt";
		SetPriority(priority);
	}
	
	override string GetActionDebugInfo()
	{
		return this.ToString() + " resupplying unit " + m_EntityToResupply.m_Value.ToString() + " with ammo type " + m_MagazineWell.m_Value.ToString();
	}
};