class SCR_TaskExecutorGroup : SCR_TaskExecutor
{
	[Attribute()]
	protected int m_iGroupID;
	
	//------------------------------------------------------------------------------------------------
	override bool IsClonedBy(SCR_TaskExecutor executor)
	{
		if (!super.IsClonedBy(executor))
			return false;
		
		SCR_TaskExecutorGroup executorGroup = SCR_TaskExecutorGroup.Cast(executor);
		if (!executorGroup)
			return false;

		if (m_iGroupID != executorGroup.GetGroupID())
			return false;
				
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override FactionKey GetFactionKey()
	{	
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManagerComponent)
			return FactionKey.Empty;
			
		SCR_AIGroup group = groupsManagerComponent.FindGroup(m_iGroupID);
		if (!group)
			return FactionKey.Empty;
		
		return group.GetFactionName();
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetGroupID()
	{
		return m_iGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupID(int groupID)
	{
		m_iGroupID = groupID;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_TaskExecutorGroup instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iGroupID);		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_TaskExecutorGroup instance)
	{
		snapshot.SerializeInt(instance.m_iGroupID);	
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet)
	{
		snapshot.EncodeInt(packet);
	}

	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.DecodeInt(packet);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs , ScriptCtx ctx)
	{
		return lhs.CompareSnapshots(rhs, 4);
	}

	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_TaskExecutorGroup instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iGroupID);
	}
}