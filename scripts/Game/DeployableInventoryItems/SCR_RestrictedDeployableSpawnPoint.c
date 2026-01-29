[EntityEditorProps(category: "GameScripted/DeployableItems", description: "", visible: false)]
class SCR_RestrictedDeployableSpawnPointClass : SCR_DeployableSpawnPointClass
{
};

//------------------------------------------------------------------------------------------------
//! Basically SCR_SpawnPoint with the ability to limit respawn amount
class SCR_RestrictedDeployableSpawnPoint : SCR_DeployableSpawnPoint
{			
	[RplProp()]
	protected bool m_bAllowAllGroupsToSpawn;
	
	[RplProp()]
	protected int m_iGroupID = -1;
	
	protected SCR_ESpawnPointBudgetType m_eRespawnBudgetType;
	
	protected int m_iMaxRespawns;
	
	protected int m_iRespawnCount;
	
	protected bool m_bLoadoutAllowed;
		
	//------------------------------------------------------------------------------------------------
	protected bool CanSpawn(notnull array<SCR_ChimeraCharacter> characters, Faction spawnPointFaction, vector spawnPointOrigin)
	{
		SCR_RestrictedDeployableSpawnPointComponent restrictedDeployableSpawnPointComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(m_DeployableSpawnPointComp);
		if (!restrictedDeployableSpawnPointComp)
			return false;
		
		if (restrictedDeployableSpawnPointComp.GetIgnoreEnemyCharacters())
			return true;
		
		int friendlyCharactersCount, enemyCharactersCount = 0;
		
		foreach (SCR_ChimeraCharacter character : characters)
		{
			if (character.GetCharacterController().IsDead())
				continue;
			
			float distanceToItemSq = (spawnPointOrigin - character.GetOrigin()).LengthSq();
			float queryRadius = restrictedDeployableSpawnPointComp.GetQueryRadiusCharacters();
			float queryRadiusSq = queryRadius * queryRadius;
			
			if (distanceToItemSq > queryRadiusSq)
				continue;
			
			SCR_Faction faction = SCR_Faction.Cast(SCR_Faction.GetEntityFaction(character));
			
			if (faction.DoCheckIfFactionFriendly(spawnPointFaction))
				friendlyCharactersCount++;
			else
				enemyCharactersCount++;
		}
		
		return friendlyCharactersCount - enemyCharactersCount >= 0;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsSpawnPointVisibleForPlayer(int pid)
	{
		if (m_bAllowAllGroupsToSpawn)
			return true;
		
		SCR_PlayerControllerGroupComponent playerControllerGroupComp = SCR_PlayerControllerGroupComponent.GetPlayerControllerComponent(pid);
		if (!playerControllerGroupComp)
			return false;
		
		int localPlayerGroupID = playerControllerGroupComp.GetGroupID();

		return m_iGroupID == localPlayerGroupID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFinalizeSpawnDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{		
		super.OnFinalizeSpawnDone_S(requestComponent, data, entity);
		
		if (m_eRespawnBudgetType == SCR_ESpawnPointBudgetType.SPAWNTICKET)
		{
			m_iRespawnCount++;
		
			SCR_RestrictedDeployableSpawnPointComponent restrictedDeployableSpawnPointComp = SCR_RestrictedDeployableSpawnPointComponent.Cast(m_DeployableSpawnPointComp);
			if (!restrictedDeployableSpawnPointComp)
				return;
		
		 	restrictedDeployableSpawnPointComp.SetRespawnCount(m_iRespawnCount);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (!super.CanReserveFor_S(playerId, result))
            return false;
		
		// Deny spawning when respawn limit is reached and supply usage is disabled
		if (m_eRespawnBudgetType == SCR_ESpawnPointBudgetType.SPAWNTICKET && m_iRespawnCount >= m_iMaxRespawns)
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_SPAWNPOINT_DISABLED_OUT_OF_RESPAWNS;
			return false;
		}
		
		// Deny spawning if custom loadouts are disabled and player is trying to spawn with one
		if (m_eRespawnBudgetType == SCR_ESpawnPointBudgetType.SUPPLIES && !m_bLoadoutAllowed)
		{
			IEntity playerController = GetGame().GetPlayerManager().GetPlayerController(playerId);
			if (!playerController)
				return false;
			
			SCR_PlayerLoadoutComponent loadoutComp = SCR_PlayerLoadoutComponent.Cast(playerController.FindComponent(SCR_PlayerLoadoutComponent));
			if (!loadoutComp)
				return false;
			
			SCR_PlayerArsenalLoadout loadout = SCR_PlayerArsenalLoadout.Cast(loadoutComp.GetLoadout());
			if (loadout)
			{
				result = SCR_ESpawnResult.NOT_ALLOWED_CUSTOM_LOADOUT;
				return false;
			}	
		}	
		
		// Deny spawning when enemies are near spawnpoint
		array<SCR_ChimeraCharacter> characters = SCR_CharacterRegistrationComponent.GetChimeraCharacters();	
		
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!factionManager)
			return false;
		
		SCR_Faction faction = SCR_Faction.Cast(factionManager.GetFactionByKey(GetFactionKey()));
			
		if (!CanSpawn(characters, faction, GetOrigin())) 
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_SPAWNING_DISABLED_ENEMIES_NEARBY;
			return false;
		}	
		
        return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAllowAllGroupsToSpawn(bool allowAllGroupsToSpawn)
	{
		m_bAllowAllGroupsToSpawn = allowAllGroupsToSpawn;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBudgetType(SCR_ESpawnPointBudgetType budgetType)
	{
		m_eRespawnBudgetType = budgetType;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupID(int groupID)
	{
		m_iGroupID = groupID;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	int GetMaxRespawns()
	{
		return m_iMaxRespawns;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetMaxRespawns(int maxRespawns)
	{
		m_iMaxRespawns = maxRespawns;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRespawnCount()
	{
		return m_iRespawnCount;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRespawnCount(int respawnCount)
	{
		m_iRespawnCount = respawnCount;
	}
	
	//------------------------------------------------------------------------------------------------
	bool IsLoadoutAllowed()
	{
		return m_bLoadoutAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLoadoutAllowed(bool allow)
	{
		m_bLoadoutAllowed = allow;
	}
}