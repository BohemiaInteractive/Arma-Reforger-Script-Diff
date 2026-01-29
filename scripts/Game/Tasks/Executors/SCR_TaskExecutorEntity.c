class SCR_TaskExecutorEntity : SCR_TaskExecutor
{
	[Attribute()]
	protected RplId m_EntityRplID;
	
	//------------------------------------------------------------------------------------------------
	override bool IsClonedBy(SCR_TaskExecutor executor)
	{
		if (!super.IsClonedBy(executor))
			return false;
		
		SCR_TaskExecutorEntity executorEntity = SCR_TaskExecutorEntity.Cast(executor);
		if (!executorEntity)
			return false;
		
		RplComponent rpl = RplComponent.Cast(GetEntity().FindComponent(RplComponent));
		if (!rpl)
			return false;
				
		RplId rplID = rpl.Id();
		if (!rplID.IsValid())
			return false;
				
		rpl = RplComponent.Cast(executorEntity.GetEntity().FindComponent(RplComponent));
		if (!rpl)
			return false;
				
		RplId otherRplID = rpl.Id();
		if (!otherRplID.IsValid())
			return false;
				
		if (rplID != otherRplID)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		RplComponent rpl = RplComponent.Cast(Replication.FindItem(m_EntityRplID));
		if (!rpl)
			return null;
			
		return rpl.GetEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntity(IEntity ent)
	{
		RplComponent rpl = RplComponent.Cast(ent.FindComponent(RplComponent));
		if (!rpl)
			return;
		
		RplId rplID = rpl.Id();
		if (!rplID)
			return;
		
		m_EntityRplID = rplID;
	}
	
	//------------------------------------------------------------------------------------------------
	override FactionKey GetFactionKey()
	{
		IEntity entity = GetEntity();
		if (!entity)
			return FactionKey.Empty;
			
		FactionAffiliationComponent factionAffiliationComponent = FactionAffiliationComponent.Cast(entity.FindComponent(FactionAffiliationComponent));
		if (!factionAffiliationComponent)
			return FactionKey.Empty;
		
		return factionAffiliationComponent.GetAffiliatedFactionKey();
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetGroupID()
	{
		IEntity entity = GetEntity();
		if (!entity)
			return -1;
			
		SCR_AICombatComponent aiCombatComponent = SCR_AICombatComponent.Cast(entity.FindComponent(SCR_AICombatComponent));
		if (!aiCombatComponent)
			return -1;
			
		SCR_ChimeraAIAgent agent = aiCombatComponent.GetAiAgent();
		if (!agent)
			return -1;
			
		SCR_AIGroup group = SCR_AIGroup.Cast(agent.GetParentGroup());
		if (!group)
			return -1;
		
		return group.GetGroupID();
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_TaskExecutorEntity instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeBytes(instance.m_EntityRplID, 4);		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_TaskExecutorEntity instance)
	{
		snapshot.SerializeBytes(instance.m_EntityRplID, 4);		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		RplId entityRplID;
		snapshot.SerializeBytes(entityRplID, 4);
		packet.Serialize(entityRplID, 32);
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		RplId entityRplID;
		packet.Serialize(entityRplID, 32);
		snapshot.SerializeBytes(entityRplID, 4);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4);
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_TaskExecutorEntity instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{		
		return snapshot.Compare(instance.m_EntityRplID, 4);
	}
}