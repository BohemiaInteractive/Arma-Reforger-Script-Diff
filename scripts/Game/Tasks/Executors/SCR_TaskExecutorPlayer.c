class SCR_TaskExecutorPlayer : SCR_TaskExecutor
{
	[Attribute()]
	protected int m_iPlayerID;
	
	//------------------------------------------------------------------------------------------------
	override bool IsClonedBy(SCR_TaskExecutor executor)
	{
		if (!super.IsClonedBy(executor))
			return false;
		
		SCR_TaskExecutorPlayer executorPlayer = SCR_TaskExecutorPlayer.Cast(executor);
		if (!executorPlayer)
			return false;

		if (m_iPlayerID != executorPlayer.GetPlayerID())
			return false;
				
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	int GetPlayerID()
	{
		return m_iPlayerID;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetPlayerID(int playerID)
	{
		m_iPlayerID = playerID;
	}
	
	//------------------------------------------------------------------------------------------------
	override FactionKey GetFactionKey()
	{
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		if (!factionManager)
			return FactionKey.Empty;
			
		Faction faction = factionManager.GetPlayerFaction(m_iPlayerID);
		if (!faction)
			return FactionKey.Empty;
		
		return faction.GetFactionKey();
	}
	
	//------------------------------------------------------------------------------------------------
	override int GetGroupID()
	{
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (!groupsManagerComponent)
			return -1;
			
		SCR_AIGroup group = groupsManagerComponent.GetPlayerGroup(m_iPlayerID);
		if (!group)
			return -1;
		
		return group.GetGroupID();
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_TaskExecutorPlayer instance, ScriptCtx ctx, SSnapSerializerBase snapshot)
	{
		snapshot.SerializeInt(instance.m_iPlayerID);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_TaskExecutorPlayer instance)
	{
		snapshot.SerializeInt(instance.m_iPlayerID);	
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
	static bool PropCompare(SCR_TaskExecutorPlayer instance, SSnapSerializerBase snapshot, ScriptCtx ctx)
	{
		return snapshot.CompareInt(instance.m_iPlayerID);
	}
}