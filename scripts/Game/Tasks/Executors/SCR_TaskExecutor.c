class SCR_TaskExecutor : ScriptAndConfig
{
	//------------------------------------------------------------------------------------------------
	static SCR_TaskExecutor FromLocalPlayer()
	{
		return FromPlayerID(SCR_PlayerController.GetLocalPlayerId());
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_TaskExecutor FromPlayerID(int playerID)
	{
		SCR_TaskExecutorPlayer executor = new SCR_TaskExecutorPlayer();
		executor.SetPlayerID(playerID);
		
		return executor;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_TaskExecutor FromEntity(IEntity ent)
	{
		if (!ent)
			return null;
		
		SCR_TaskExecutorEntity executor = new SCR_TaskExecutorEntity();
		executor.SetEntity(ent);
		
		return executor;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_TaskExecutor FromGroup(int groupID)
	{
		SCR_TaskExecutorGroup executor = new SCR_TaskExecutorGroup();
		executor.SetGroupID(groupID);
		
		return executor;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_TaskExecutor FindMatchingTaskExecutor(notnull array<ref SCR_TaskExecutor> container, out notnull SCR_TaskExecutor template, bool recursive = false)
	{
		SCR_GroupsManagerComponent groupsManagerComponent = SCR_GroupsManagerComponent.GetInstance();
		if (recursive && !groupsManagerComponent)
			return null;
		
		SCR_AIGroup group;
		SCR_TaskExecutor match;
		foreach (SCR_TaskExecutor e : container)
		{
			if (!e)
				continue;
			
			if (e.IsClonedBy(template))
				return e;
			
			if (recursive && SCR_TaskExecutorGroup.Cast(e))
			{			
				group = groupsManagerComponent.FindGroup(e.GetGroupID());
				if (!group)
					continue;
				
				array<int> groupMemberIDs = group.GetPlayerIDs();
				if (!groupMemberIDs || groupMemberIDs.IsEmpty())
					continue;
				
				array<ref SCR_TaskExecutor> groupMemberExecutors = {};
				SCR_TaskExecutor groupMemberExecutor;
				foreach (int groupMemberID : groupMemberIDs)
				{
					groupMemberExecutor = SCR_TaskExecutor.FromPlayerID(groupMemberID);
					if (groupMemberExecutor)
						groupMemberExecutors.Insert(groupMemberExecutor);
				}
				
				match = SCR_TaskExecutor.FindMatchingTaskExecutor(groupMemberExecutors, template);
				if (match)
					return match;
			}
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsClonedBy(SCR_TaskExecutor executor)
	{
		if (Type() != executor.Type())
			return false;
					
		return true;
	}
		
	//------------------------------------------------------------------------------------------------
	FactionKey GetFactionKey() 
	{ 
		return FactionKey.Empty; 
	}
	
	//------------------------------------------------------------------------------------------------
	int GetGroupID()
	{
		return -1;
	}
}