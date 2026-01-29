[EntityEditorProps(category: "GameScripted/DeployableItems", description: "", visible: false)]
class SCR_DeployableSpawnPointClass : SCR_SpawnPointClass
{
};

void SpawnPointInvokerDelegate(SCR_DeployableSpawnPoint spawnPoint, int i);
typedef func SpawnPointInvokerDelegate;
typedef ScriptInvokerBase<SpawnPointInvokerDelegate> SCR_DeployableSpawnPoint_Invoker;

//------------------------------------------------------------------------------------------------
class SCR_DeployableSpawnPoint : SCR_SpawnPoint
{		
	protected static ref SCR_DeployableSpawnPoint_Invoker s_OnPlayerSpawned;
	
	protected SCR_BaseDeployableSpawnPointComponent m_DeployableSpawnPointComp;
		
	//------------------------------------------------------------------------------------------------
	override void OnFinalizeSpawnDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		super.OnFinalizeSpawnDone_S(requestComponent, data, entity);
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
				
		int userID = playerManager.GetPlayerIdFromControlledEntity(entity);
		
		// param1: spawnpoint entity from which event was invoked; param2: player who spawned
		if (s_OnPlayerSpawned)
			s_OnPlayerSpawned.Invoke(this, userID);
	}
	
	//------------------------------------------------------------------------------------------------
	void SetDeployableSpawnPointComponent(SCR_BaseDeployableSpawnPointComponent spawnPointComp)
	{
		m_DeployableSpawnPointComp = spawnPointComp;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_BaseDeployableSpawnPointComponent GetDeployableSpawnPointComponent()
	{
		return m_DeployableSpawnPointComp;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_DeployableSpawnPoint_Invoker GetOnPlayerSpawned()
	{
		if (!s_OnPlayerSpawned)
			s_OnPlayerSpawned = new SCR_DeployableSpawnPoint_Invoker();
		
		return s_OnPlayerSpawned;
	}
}