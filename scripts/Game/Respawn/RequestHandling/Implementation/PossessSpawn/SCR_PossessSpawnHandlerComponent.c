[ComponentEditorProps(category: "GameScripted/Respawn/Handlers", description: "Allows the respawn system to utilize spawning on AI(s). Requires a SCR_OnAIRespawnComponent attached to PlayerController.")]
class SCR_PossessSpawnHandlerComponentClass : SCR_SpawnHandlerComponentClass
{
}

class SCR_PossessSpawnHandlerComponent : SCR_SpawnHandlerComponent
{
	[Attribute("1", desc: "When enabled, conditions like respawn time will not be checked.")]
	protected bool m_bIgnoreConditions;
	
	//------------------------------------------------------------------------------------------------
	protected override SCR_ESpawnResult SpawnEntity_S(SCR_SpawnRequestComponent requestComponent, notnull SCR_SpawnData data, out IEntity spawnedEntity)
	{
		int playerId = requestComponent.GetPlayerController().GetPlayerId();
		#ifdef _ENABLE_RESPAWN_LOGS
		Print(string.Format("%1::SpawnEntity(playerId: %2, data: %3)", Type().ToString(),
					playerId,
					data), LogLevel.NORMAL);
		#endif

		// We simply use the entity without spawning it
		spawnedEntity = GetEntity(data);
		if (!spawnedEntity)
			return SCR_ESpawnResult.CANNOT_VALIDATE;

		if (!PrepareEntity_S(requestComponent, spawnedEntity, data))
			return SCR_ESpawnResult.CANNOT_PREPARE;

		return SCR_ESpawnResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	override SCR_ESpawnResult CanHandleRequest_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data)
	{
		SCR_ESpawnResult result = super.CanHandleRequest_S(requestComponent, data);
		if (result != SCR_ESpawnResult.OK)
			return result;

		IEntity entity = GetEntity(data);
		if (!entity)
			return SCR_ESpawnResult.CANNOT_VALIDATE;

		// Also for now allow only characters, for.. reasons
		ChimeraCharacter character = ChimeraCharacter.Cast(entity);
		if (!character)
			return SCR_ESpawnResult.CANNOT_POSSES;

		// And alive, if possible
		if (character.GetCharacterController().IsDead())
			return SCR_ESpawnResult.CANNOT_POSSES;

		// Ensure that entity to be possessed is not already possessed
		int controllingPlayerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(entity);
		if (controllingPlayerId != 0)
			return SCR_ESpawnResult.CANNOT_POSSES;

		return SCR_ESpawnResult.OK;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity GetEntity(SCR_SpawnData data)
	{
		SCR_PossessSpawnData possessData = SCR_PossessSpawnData.Cast(data);
		if (!possessData)
			return null;

		RplComponent rplComponent = RplComponent.Cast(Replication.FindItem(possessData.GetRplId()));
		if (!rplComponent)
			return null;

		return rplComponent.GetEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	override protected bool ShouldDeleteEntityOnSpawnFailure_S(SCR_SpawnRequestComponent requestComponent, IEntity entity, SCR_SpawnData data, SCR_ESpawnResult reason)
	{
		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Ensures that entity to possess is valid, if anything.
	//! \param[in] requestComponent
	//! \param[in] data
	protected override bool ValidateData_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data)
	{
		if (!super.ValidateData_S(requestComponent, data))
			return false;

		SCR_PossessSpawnData possessData = SCR_PossessSpawnData.Cast(data);
		if (!possessData || !possessData.GetRplId().IsValid())
			return false;

		Managed rplComponent = Replication.FindItem(possessData.GetRplId());
		return rplComponent != null;
	}

	//------------------------------------------------------------------------------------------------
	override bool CanRequestSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, out SCR_ESpawnResult result)
	{
		return m_bIgnoreConditions || super.CanRequestSpawn_S(requestComponent, data, result);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnFinalizeDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		// Assign possessed entity's faction to the player
		PlayerController pc = requestComponent.GetPlayerController();
		SCR_PlayerFactionAffiliationComponent playerFactionComp = SCR_PlayerFactionAffiliationComponent.Cast(pc.FindComponent(SCR_PlayerFactionAffiliationComponent));
		Faction targetFaction = GetFactionFromPrefab(entity.GetPrefabData().GetPrefabName());
		if (!targetFaction || targetFaction == playerFactionComp.GetAffiliatedFaction())
			return;

		playerFactionComp.SetAffiliatedFaction(targetFaction);

		// Notify faction manager
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		factionManager.UpdatePlayerFaction_S(playerFactionComp);

		// Notify game mode
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.OnPlayerFactionSet_S(playerFactionComp, targetFaction);	
	}

	//------------------------------------------------------------------------------------------------
	protected Faction GetFactionFromPrefab(ResourceName prefab)
	{
		Resource res = Resource.Load(prefab);
		IEntityComponentSource src = SCR_BaseContainerTools.FindComponentSource(res, FactionAffiliationComponent);
		if (!src)
			return null;

		string factionKey;
		src.Get("faction affiliation", factionKey);

		return GetGame().GetFactionManager().GetFactionByKey(factionKey);
	}	
}
