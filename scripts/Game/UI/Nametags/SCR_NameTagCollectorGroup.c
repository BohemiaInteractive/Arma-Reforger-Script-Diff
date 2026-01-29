//------------------------------------------------------------------------------------------------
//! Group nametag collector
[BaseContainerProps()]
class SCR_NameTagCollectorGroup : SCR_NameTagCollectorBase
{
	protected bool m_bUpdateFlag = true;				// update request flag
	protected int m_iGroupID = -1;						// current group ID of the controlled entity
	protected SCR_GroupsManagerComponent m_GroupsMgr;	
	
	//------------------------------------------------------------------------------------------------
	// SCR_AIGroup event
	protected void OnPlayerAdded(SCR_AIGroup group, int playerID)
	{
		if (m_iGroupID == -1)	// not yet initialized group
		{
			if (GetGame().GetPlayerController().GetPlayerId() == playerID)
			{
				m_iGroupID = group.GetGroupID();
				RefreshEntities();
				//m_bUpdateFlag = true;
				
				return;
			}
		}
		
		if (group.GetGroupID() != m_iGroupID)
			return;
		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (playerMgr)
			m_aEntities.Insert(playerMgr.GetPlayerControlledEntity(playerID));
		
		//m_bUpdateFlag = true;
	}
	
	//------------------------------------------------------------------------------------------------
	// SCR_AIGroup event
	protected  void OnPlayerRemoved(SCR_AIGroup group, int playerID)
	{
		if (group.GetGroupID() != m_iGroupID)
			return;
		
		if (GetGame().GetPlayerController().GetPlayerId() == playerID)	// current player left group
		{
			m_iGroupID = -1;
			m_Display.CleanupAllTags();
			return;
		}
		
		// TODO groupID can be saved in NameTagData and checked within ruleset for generic cleanup
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (playerMgr)
		{
			IEntity ent = playerMgr.GetPlayerControlledEntity(playerID);
			m_aEntities.RemoveItem(playerMgr.GetPlayerControlledEntity(playerID));	
			
			SCR_NameTagData tag = m_Display.GetEntityNameTag(ent);
			if (tag)
				m_Display.CleanupTag(tag);
		}
		
		//m_bUpdateFlag = true;
	}
	
	//------------------------------------------------------------------------------------------------
	// SCR_BaseGameMode event
	/*protected  void OnControllableSpawned(IEntity entity)
	{
		if (m_iGroupID < 0)
			return;
		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (!playerMgr)
			return;
		
		int playerID = playerMgr.GetPlayerIdFromControlledEntity(entity);
	
		SCR_AIGroup group = m_GroupsMgr.FindGroup(m_iGroupID);
		if (group)
		{
			if (group.IsPlayerInGroup(playerID))
			{
				m_aEntities.Insert(entity);
				m_bUpdateFlag = true;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	// SCR_BaseGameMode event
	protected  void OnControllableDestroyed(IEntity entity, IEntity killer)
	{
		if (m_iGroupID < 0)
			return;
		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (!playerMgr)
			return;
		
		int playerID = playerMgr.GetPlayerIdFromControlledEntity(entity);
	
		SCR_AIGroup group = m_GroupsMgr.FindGroup(m_iGroupID);
		if (group)
		{
			if (group.IsPlayerInGroup(playerID))
			{
				m_aEntities.RemoveItem(entity);
				m_bUpdateFlag = true;
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool UpdatePerformed(float timeSlice, SCR_NameTagData controlledTag)
	{		
		if (m_bUpdateFlag)
		{
			m_bUpdateFlag = false;
			return true;
		}
		
		return false;
	}*/
		
	//------------------------------------------------------------------------------------------------
	override void RefreshEntities()
	{
		if (m_iGroupID == -1)
			return;
		
		SCR_AIGroup group = m_GroupsMgr.FindGroup(m_iGroupID);
		if (!group)
		{
			Print("SCR_NameTagCollectorGroup: Failed to retrieve a target group", LogLevel.WARNING);
			m_iGroupID = -1;
			m_aEntities.Clear();
			m_Display.CleanupAllTags();
			return;
		}
		
		ref array<int> playerIDs = group.GetPlayerIDs();
		
		PlayerManager playerMgr = GetGame().GetPlayerManager();
		if (!playerMgr)
			return;
				
		m_aEntities.Clear();
		foreach ( int playerID: playerIDs)
		{		
			m_aEntities.Insert(playerMgr.GetPlayerControlledEntity(playerID));
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void Init(SCR_NameTagConfig cfg, SCR_NameTagDisplay display)
	{
		super.Init(cfg, display);
		
		m_GroupsMgr = SCR_GroupsManagerComponent.GetInstance();
		
		SCR_AIGroup.GetOnPlayerAdded().Insert(OnPlayerAdded);
		SCR_AIGroup.GetOnPlayerRemoved().Insert(OnPlayerRemoved);
		
		SCR_PlayerControllerGroupComponent playerGroupComponent = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();
		if (!playerGroupComponent)
			return;
		
		int groupID = playerGroupComponent.GetGroupID();
		if (groupID != -1)		// if already in group
		{
			m_iGroupID = groupID;
			RefreshEntities();
		}
		
		//SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());	// these events cannot be used atm since playerID cant be fetched from them
		//gameMode.GetOnControllableSpawned().Insert(OnControllableSpawned);		
		//gameMode.GetOnControllableDestroyed().Insert(OnControllableDestroyed);
	}
};