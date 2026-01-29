[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component to be used with barrack compositions, handing unit spawning.", color: "0 0 255 255")]
class SCR_DefenderSpawnerComponentClass : SCR_SlotServiceComponentClass
{
	[Attribute(defvalue: "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", UIWidgets.ResourceNamePicker, desc: "Default waypoint prefab", "et", category: "Defender Spawner")]
	protected ResourceName m_sDefaultWaypointPrefab;

	[Attribute(defvalue: "75", params: "0 inf", desc: "Radius of default waypoint", "et", category: "Defender Spawner")]
	protected float m_fDefaultWaypointCompletionRadius;
	
	[Attribute(defvalue :"Wedge", UIWidgets.Auto, desc: "Group formation created on rally point. Available formations are found on SCR_AIWorld entity", category: "Defender Spawner")]
	protected string m_sExitFormation;
	
	//------------------------------------------------------------------------------------------------
	//! \return
	ResourceName GetDefaultWaypointPrefab()
	{
		return m_sDefaultWaypointPrefab;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	float GetDefaultWaypointCompletionRadius()
	{
		return m_fDefaultWaypointCompletionRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	string GetExitFormation()
	{
		return m_sExitFormation;
	}
}

void OnDefenderGroupSpawnedDelegate(SCR_DefenderSpawnerComponent spawner, SCR_AIGroup group);
typedef func OnDefenderGroupSpawnedDelegate;
typedef ScriptInvokerBase<OnDefenderGroupSpawnedDelegate> OnDefenderGroupSpawnedInvoker;

//! Service providing group of defenders defined in faction. Requires SCR_EnableDefenderAction on ActionManager for players to manage its functionality.
class SCR_DefenderSpawnerComponent : SCR_SlotServiceComponent
{
	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Defender Spawner", desc: "Allowed labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aAllowedLabels;

	[Attribute(uiwidget: UIWidgets.SearchComboBox, category: "Defender Spawner", desc: "Ignored labels.", enums: ParamEnumArray.FromEnum(EEditableEntityLabel))]
	protected ref array<EEditableEntityLabel> m_aIgnoredLabels;

	[Attribute(defvalue: "300", uiwidget: UIWidgets.Auto, category: "Defender Spawner", desc: "Time in seconds between unit respawns")]
	protected float m_fRespawnDelay;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, category: "Defender Spawner", desc: "Enable spawning"), RplProp()]
	protected bool m_bEnableSpawning;
	
	[Attribute(defvalue: "30", params: "0 inf", desc: "Minimum distance to hostile players for spawner to work", "et")]
	protected float m_fMinHostilePlayerDistance;
	
	[Attribute(desc: "Enables supplies usage.", category: "Supplies")]
	protected bool m_bSuppliesConsumptionEnabled;
	
	[Attribute(params: "0 inf", desc: "Custom supplies value.", category: "Supplies")]
	protected int m_iCustomSupplies;
	
	[Attribute(SCR_ECharacterRank.CORPORAL.ToString(), uiwidget: UIWidgets.ComboBox, enums: ParamEnumArray.FromEnum(SCR_ECharacterRank))]
	protected SCR_ECharacterRank m_eMinimumRankForEnabling;
	
	protected RplComponent m_RplComponent;
	protected SCR_CampaignSuppliesComponent m_SupplyComponent; //TODO: Temporary until supply sandbox rework
	protected SCR_ResourceComponent m_ResourceComponent;
	protected SCR_SpawnerAIGroupManagerComponent m_GroupSpawningManager;
	protected WorldTimestamp m_fNextRespawnTime;
	protected SCR_EntityCatalogEntry m_GroupEntry;
	protected SCR_AIGroup m_AIgroup;
	protected AIWaypoint m_Waypoint;
	protected int m_iDespawnedGroupMembers;
	protected ref array<ref Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp>> m_aUnitsOnMove = {};
	protected ref array<ResourceName> m_aRefillQueue;
	protected ref array<SCR_AIGroup> m_aPreviousGroups;
	protected ref array<IEntity> m_aOldUnits;
	
	protected ref OnDefenderGroupSpawnedInvoker m_OnDefenderGroupSpawned;

	protected static const int SPAWN_CHECK_INTERVAL = 1000;
	protected static const int SPAWN_GROUP_JOIN_TIMEOUT = 20; //Maximum time in seconds for spawned unit to reach waypoint, before it is assigned to group automatically
	protected static const int SPAWN_RADIUS_MIN = Math.Pow(500, 2);
	protected static const int SPAWN_RADIUS_MAX = Math.Pow(1000, 2);
	
	//------------------------------------------------------------------------------------------------
	//! \return
	SCR_ECharacterRank GetMinimumRank()
	{
		return m_eMinimumRankForEnabling;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] user
	//! \return
	bool UserRankCheck(IEntity user)
	{
		SCR_GameModeCampaign campaign = SCR_GameModeCampaign.GetInstance();
		if (!campaign)
			return true;
		
		SCR_ECharacterRank rank = SCR_CharacterRankComponent.GetCharacterRank(user);
		
		return rank >= m_eMinimumRankForEnabling;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	OnDefenderGroupSpawnedInvoker GetOnDefenderGroupSpawned()
	{
		if (!m_OnDefenderGroupSpawned)
			m_OnDefenderGroupSpawned = new OnDefenderGroupSpawnedInvoker();

		return m_OnDefenderGroupSpawned;
	}

	//------------------------------------------------------------------------------------------------
	//! Assign custom waypoint to be used by defender group
	//! \param[in] wp
	void SetWaypoint(SCR_AIWaypoint wp)
	{
		if (wp)
			m_Waypoint = wp;
	}

	//------------------------------------------------------------------------------------------------
	//! Set Defender group data from Entity catalog on owning faction
	protected void AssignDefenderGroupDataFromOwningFaction()
	{
		SCR_Faction faction = SCR_Faction.Cast(GetFaction());
		if (!faction)
			return;

		SCR_EntityCatalog catalog = faction.GetFactionEntityCatalogOfType(EEntityCatalogType.GROUP);
		if (!catalog)
		{
			Print("SCR_Faction Config lacks catalog of GROUP type. SCR_DefenderSpawnerComponent won't work without it", LogLevel.ERROR);
			return;
		}

		array<SCR_EntityCatalogEntry> entityEntries = {};
		array<typename> includedDataClasses = {};
		includedDataClasses.Insert(SCR_EntityCatalogSpawnerData); //~ The Data the entity must have

		catalog.GetFullFilteredEntityList(entityEntries, m_aAllowedLabels, m_aIgnoredLabels, includedDataClasses, null, false);

		//Only one entry of defender group should be used. Rest will be ditched, at least for now
		if (!entityEntries.IsEmpty())
			m_GroupEntry = entityEntries[0];
	}

	//------------------------------------------------------------------------------------------------
	//! Assign supply component to handle supplies of spawner. Currently uses Campaign specific supplies (temporarily)
	//! \param[in] supplyComp
	void AssignSupplyComponent(notnull SCR_CampaignSuppliesComponent supplyComp)
	{
		m_SupplyComponent = supplyComp;
		
		//Resets timer, so new soldiers are spawned, if spawning is enabled.
		m_fNextRespawnTime = null;
	}

	//------------------------------------------------------------------------------------------------
	//! \return supply component assigned to spawner, temporary
	SCR_CampaignSuppliesComponent GetSpawnerSupplyComponent()
	{
		return m_SupplyComponent;
	}

	//------------------------------------------------------------------------------------------------
	//! Get available supplies
	float GetSpawnerSupplies()
	{
		if (!m_ResourceComponent)
			return m_iCustomSupplies;
		
		SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
			
		if (!consumer)
			return 0.0;
		
		if (m_RplComponent && !m_RplComponent.IsProxy())
			GetGame().GetResourceGrid().UpdateInteractor(consumer);
		
		return consumer.GetAggregatedResourceValue();
	}

	//------------------------------------------------------------------------------------------------
	//! Set the spawner supplies value
	//! \param[in] value
	void AddSupplies(float value)
	{
		if (m_ResourceComponent)
		{
			SCR_ResourceInteractor interactor;
			
			if (value >= 0)
			{
				SCR_ResourceGenerator generator = m_ResourceComponent.GetGenerator(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!generator)
					return;
				
				interactor = generator;
				
				generator.RequestGeneration(value);
			}
			else
			{
				SCR_ResourceConsumer consumer = m_ResourceComponent.GetConsumer(EResourceGeneratorID.DEFAULT, EResourceType.SUPPLIES);
				
				if (!consumer)
					return;
				
				interactor = consumer;
				
				consumer.RequestConsumtion(-value);
			}
		}

		m_iCustomSupplies = m_iCustomSupplies + value;
		if (m_iCustomSupplies < 0)
			m_iCustomSupplies = 0;
				
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true if spawner is currently activated
	bool IsSpawningEnabled()
	{
		return m_bEnableSpawning;
	}

	//------------------------------------------------------------------------------------------------
	//! Enable spawning of defender groups
	//! \param[in] enable bool to enable or disable spawning
	//! \param[in] playerID id of player using this action
	void EnableSpawning(bool enable, int playerID)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (!playerController)
			return;

		SCR_ChimeraCharacter chimeraCharacter = SCR_ChimeraCharacter.Cast(playerController.GetControlledEntity());
		if (!chimeraCharacter || chimeraCharacter.GetFaction() != GetFaction())
			return;

		if (!UserRankCheck(chimeraCharacter))
			return;
		
		m_bEnableSpawning = enable;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! \return
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawn Units with given ResourceName and assign it to group
	//! \param[in] unitResource resource name of unit to be spawned
	//! \param[in] consumeSupplies if false, spawning this unit won't consume supplies. Is used for respawning units, which were previously despawned to save performance
	protected void SpawnUnit(ResourceName unitResource, bool consumeSupplies = true)
	{
		if (IsProxy())
			return;
		
		if (!m_GroupEntry || !m_AIgroup || !m_AIgroup.m_aUnitPrefabSlots || m_AIgroup.m_aUnitPrefabSlots.IsEmpty())
			return;
		
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return;
		
		SCR_EntityCatalogSpawnerData entityData = SCR_EntityCatalogSpawnerData.Cast(m_GroupEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!entityData)
			return;
		
		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return;
		
		// Calculate unit spawn cost from overall group member count
		int spawnCost;
		if (m_AIgroup.m_aUnitPrefabSlots.Count() > 0)
			spawnCost = entityData.GetSupplyCost() / m_AIgroup.m_aUnitPrefabSlots.Count();

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		slot.GetOwner().GetTransform(spawnParams.Transform);
		Resource res = Resource.Load(unitResource);

		IEntity ai = GetGame().SpawnEntityPrefab(res, null, spawnParams);
		if (!ai)
			return;

		if (m_aRefillQueue)
			m_aRefillQueue.RemoveItem(unitResource);
		
		// consume supplies, if their consumption is enabled and revents spawning, if there is not enough supplies available.		
		if (consumeSupplies && m_bSuppliesConsumptionEnabled && GetSpawnerSupplies() >= spawnCost)
			AddSupplies(-spawnCost);

		AIControlComponent control = AIControlComponent.Cast(ai.FindComponent(AIControlComponent));
		if (control)
			control.ActivateAI();
		
		AIAgent agent = control.GetAIAgent();
		if (!agent)
			return;
		
		//Remove group waypoint to reduce performance impact due to new members being added to it.
		if (m_Waypoint)
			m_AIgroup.RemoveWaypoint(m_Waypoint);
		
		SCR_DefenderSpawnerComponentClass prefabData = SCR_DefenderSpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		
		SCR_EntityLabelPointComponent rallyPointComponent = slot.GetRallyPoint();
		if (rallyPointComponent && prefabData)
		{
			IEntity rallyPointEntity = rallyPointComponent.GetOwner();
			
			// Create temporary group for unit and later rally point waypoint
			SCR_AIUtilityComponent utilityComponent = SCR_AIUtilityComponent.Cast(agent.FindComponent(SCR_AIUtilityComponent));
			
			vector rpTransform[4];
			rallyPointEntity.GetWorldTransform(rpTransform);
			
			AIFormationDefinition formationDefiniton = aiWorld.GetFormation(prefabData.GetExitFormation());
			vector posLocal;
			
			if (m_aUnitsOnMove && formationDefiniton)
			{
				vector formationOffset = 0.5 * formationDefiniton.GetOffsetPosition(m_aUnitsOnMove.Count());
				posLocal = posLocal + formationOffset;
			}
			
			vector posWorld = posLocal.Multiply4(rpTransform);
			
			SCR_AIMoveIndividuallyBehavior moveBehavior = new SCR_AIMoveIndividuallyBehavior(utilityComponent, null, posWorld, SCR_AIActionBase.PRIORITY_BEHAVIOR_MOVE, SCR_AIActionBase.PRIORITY_LEVEL_GAMEMASTER, radius: 0.5);
			if (!moveBehavior)
			{
				m_AIgroup.AddAgent(agent);
				return;
			}
			
			utilityComponent.AddAction(moveBehavior);
			
			if (!m_aUnitsOnMove)
				m_aUnitsOnMove = {};
			
			ChimeraWorld world = GetOwner().GetWorld();
			
			m_aUnitsOnMove.Insert(new Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp>(moveBehavior, agent, world.GetServerTimestamp()));
			
			moveBehavior.m_OnActionCompleted.Insert(OnMoveFinished);
			
			SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(ai.FindComponent(SCR_CharacterControllerComponent));
			if (!charController)
				return;
			
			charController.m_OnPlayerDeathWithParam.Insert(OnMovingCharacterDeath);
		}
		else
		{
			// Rally point not found, add spawned ai directly to controlled group
			m_AIgroup.AddAgent(agent);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMovingCharacterDeath(SCR_CharacterControllerComponent characterController, IEntity killerEntity, Instigator killer)
	{
		if (!characterController)
			return;
		
		IEntity character = characterController.GetOwner();
		if (!character)
			return;
		
		foreach (int index, Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp> AIaction : m_aUnitsOnMove)
		{
			if (AIaction.param2.GetControlledEntity() != character)
				continue;
			
			m_aUnitsOnMove.Remove(index);
			break;
		}
		
		if (m_aUnitsOnMove.IsEmpty())
			m_aUnitsOnMove = null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnMoveFinished(SCR_AIActionBase action)
	{
		if (!m_aUnitsOnMove)
			return;
		
		AIAgent agent;
		int groupIndex;
		
		foreach (int i, Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp> AIaction : m_aUnitsOnMove)
		{
			if (AIaction.param1 != action)
				continue;
			
			agent = AIaction.param2;
			groupIndex = i;
			break;
		}
		
		SCR_ChimeraCharacter agentEntity = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (!agentEntity)
			return;
		
		SCR_CharacterControllerComponent charController = SCR_CharacterControllerComponent.Cast(agentEntity.FindComponent(SCR_CharacterControllerComponent));
		if (charController)
			charController.m_OnPlayerDeathWithParam.Remove(OnMovingCharacterDeath);
		
		if (!agent || !m_AIgroup)
		{
			if (!m_aOldUnits)
				m_aOldUnits = {};
			
			m_aOldUnits.Insert(agentEntity);
			
			return;
		}
		
		if (agentEntity.GetFaction() == m_AIgroup.GetFaction())
			m_AIgroup.AddAgent(agent);
		
		m_aUnitsOnMove.Remove(groupIndex);
		if (m_aUnitsOnMove.IsEmpty())
			m_aUnitsOnMove = null;
		
		if ((m_AIgroup.GetAgentsCount() == m_AIgroup.m_aUnitPrefabSlots.Count()) || (!m_aUnitsOnMove))
			m_AIgroup.AddWaypoint(GetWaypoint());
	}

	//------------------------------------------------------------------------------------------------
	//! \return free slot or null if not found
	SCR_EntitySpawnerSlotComponent GetFreeSlot()
	{
		foreach (SCR_EntitySpawnerSlotComponent slot : m_aChildSlots)
		{
			if (!slot.IsOccupied())
				return slot;
		}

		foreach (SCR_EntitySpawnerSlotComponent slot : m_aNearSlots)
		{
			if (!slot.IsOccupied())
				return slot;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Refills AI's in defender group
	protected void ReinforceGroup()
	{
		array<ResourceName> unitPrefabs = {};
		unitPrefabs.Copy(m_AIgroup.m_aUnitPrefabSlots);

		array<AIAgent> outAgents = {};
		m_AIgroup.GetAgents(outAgents);

		foreach (AIAgent agent : outAgents)
		{
			IEntity controlledEntity = agent.GetControlledEntity();
			if (!controlledEntity)
				return;

			EntityPrefabData prefabData = controlledEntity.GetPrefabData();
			if (!prefabData)
				return;

			ResourceName aiResource = prefabData.GetPrefabName();
			unitPrefabs.RemoveItem(aiResource);
		}

		IEntity ent;
		if (m_aUnitsOnMove)
		{
			foreach (Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp> groups : m_aUnitsOnMove)
			{
				if (groups.param2)
					ent = groups.param2.GetControlledEntity();
				
				if (!ent)
					continue;
				
				EntityPrefabData prefabData = ent.GetPrefabData();
				if (!prefabData)
					continue;
				
				unitPrefabs.RemoveItem(prefabData.GetPrefabName());
			}
		}
		
		m_aRefillQueue = unitPrefabs;
	}

	//------------------------------------------------------------------------------------------------
	//! Spawn defender AI group from faction prefab. Also handles price per spawned unit.
	//! Should some units be despawned because of player distance, spawn price will be reduced by it
	protected void SpawnGroup()
	{
		if (IsProxy())
			return;

		if (!m_GroupEntry)
			return;

		SCR_EntityCatalogSpawnerData entityData = SCR_EntityCatalogSpawnerData.Cast(m_GroupEntry.GetEntityDataOfType(SCR_EntityCatalogSpawnerData));
		if (!entityData)
			return;

		float supplies = GetSpawnerSupplies();
		int spawnCost = entityData.GetSupplyCost();
		if (supplies < entityData.GetSupplyCost() && m_bSuppliesConsumptionEnabled)
			return;

		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		slot.GetOwner().GetTransform(params.Transform);

		Resource res = Resource.Load(m_GroupEntry.GetPrefab());
		if (!res.IsValid())
			return;

		SCR_AIGroup.IgnoreSpawning(true);
		SCR_AIGroup group = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(res, null, params));
		if (!group || group.m_aUnitPrefabSlots.Count() == 0)
		{
			SCR_AIGroup.IgnoreSpawning(false);
			return;
		}

		m_AIgroup = group;
		
		if (m_OnDefenderGroupSpawned)
			m_OnDefenderGroupSpawned.Invoke(this, group);
		
		ReinforceGroup();
	}
	
	//------------------------------------------------------------------------------------------------
	protected AIWaypoint GetWaypoint()
	{
		if (m_Waypoint)
			return m_Waypoint;
		
		SCR_EntitySpawnerSlotComponent slot = GetFreeSlot();
		if (!slot)
			return null;
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		
		SCR_EntityLabelPointComponent rallyPoint = slot.GetRallyPoint();
		//If rally point is available, create waypoint on it, otherwise use slot position
		if (rallyPoint)
			rallyPoint.GetOwner().GetTransform(params.Transform);
		else
			slot.GetOwner().GetTransform(params.Transform);

		SCR_DefenderSpawnerComponentClass prefabData = SCR_DefenderSpawnerComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData)
			return null;

		Resource wpRes = Resource.Load(prefabData.GetDefaultWaypointPrefab());
		if (!wpRes.IsValid())
			return null;

		m_Waypoint = SCR_AIWaypoint.Cast(GetGame().SpawnEntityPrefabLocal(wpRes, null, params));
		if (!m_Waypoint)
			return null;
		
		m_Waypoint.SetCompletionRadius(prefabData.GetDefaultWaypointCompletionRadius());
		
		return m_Waypoint;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return state according to current players vicinities to DefenderSpawner
	protected SCR_EDefenderSpawnerState GetPlayerDistanceState()
	{
		array<int> players = {};
		GetGame().GetPlayerManager().GetPlayers(players);

		float spawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() * 0.5, 2), SPAWN_RADIUS_MAX, SPAWN_RADIUS_MIN);
		float despawnDistanceSq = Math.Clamp(Math.Pow(GetGame().GetViewDistance() * 0.5, 2), SPAWN_RADIUS_MIN, SPAWN_RADIUS_MAX);
		float minSpawnDistanceSq = m_fMinHostilePlayerDistance * m_fMinHostilePlayerDistance; //Minimum range for hostile players in vicinity
		
		SCR_ChimeraCharacter playerEntity;
		float dist;
		vector origin = GetOwner().GetOrigin();
		
		SCR_EDefenderSpawnerState outState = SCR_EDefenderSpawnerState.DEFENDERS_DESPAWN;
		
		foreach (int player : players)
		{
			playerEntity = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerManager().GetPlayerControlledEntity(player));
			if (!playerEntity)
				continue;
			
			dist = vector.DistanceSq(playerEntity.GetOrigin(), origin);
			
			if (outState == SCR_EDefenderSpawnerState.DEFENDERS_DESPAWN) //Do this check only if outstate is set to Despawn, otherwise it is not necessary to repeat it
			{
				if (dist < spawnDistanceSq)		//If player is withing the range, enable defenders
				{
					outState = SCR_EDefenderSpawnerState.DEFENDERS_ENABLED;
				}
				else	//If player is not within range, set to DEFENDER_DESPAWN and skip to next
				{
					outState = SCR_EDefenderSpawnerState.DEFENDERS_DESPAWN;
					continue;
				}
			}
			
			if (dist < minSpawnDistanceSq && playerEntity.GetFaction() != GetFaction())		//if player in vicinity is hostile and is not in minimum distance, pause spawning
			{
				outState = SCR_EDefenderSpawnerState.DEFENDERS_PAUSED_SPAWN;
				break;
			}
		}

		return outState;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Manages automatic group spawning
	protected void HandleGroup()
	{
		ChimeraWorld world = GetOwner().GetWorld();
		WorldTimestamp replicationTime = world.GetServerTimestamp();
		
		AIWorld aiWorld = GetGame().GetAIWorld();
		if (!aiWorld)
			return;
		
		//If another Ai would be over limit, stop requesting and prevent players from enabling it again.
		if ((aiWorld.GetCurrentAmountOfLimitedAIs() + 1) >= aiWorld.GetAILimit())
		{
			if (m_bEnableSpawning)
			{
				m_bEnableSpawning = false; //disable spawner
				Replication.BumpMe();
				
				m_GroupSpawningManager.SetIsAtAILimit(true); //block requesting action 
			}
			
			return;
		}
		
		SCR_EDefenderSpawnerState distanceState = GetPlayerDistanceState();
		
		if (m_bEnableSpawning && (distanceState == SCR_EDefenderSpawnerState.DEFENDERS_ENABLED))
		{
			// Add any stray or stuck units to group
			if (m_aUnitsOnMove)
			{
				int count = m_aUnitsOnMove.Count()-1;
				for (int i = count; i >= 0; i--)
				{
					if (!m_aUnitsOnMove[i].param2)
					{
						m_aUnitsOnMove.Remove(i);
						continue;
					}
					
					if (!m_aUnitsOnMove[i].param3.PlusSeconds(SPAWN_GROUP_JOIN_TIMEOUT).GreaterEqual(replicationTime))
						OnMoveFinished(m_aUnitsOnMove[i].param1);
				}
			}
			
			// Reinforce existing or create new defender group. Also handles respawning of despawned defenders
			if (m_iDespawnedGroupMembers > 0 || replicationTime.Greater(m_fNextRespawnTime))
			{
				m_fNextRespawnTime = replicationTime.PlusSeconds(m_fRespawnDelay);
				if (m_AIgroup && (m_AIgroup.m_aUnitPrefabSlots.Count() != m_AIgroup.GetAgentsCount()))
					ReinforceGroup();

				if (!m_AIgroup)
					SpawnGroup();
			}
			
			//Spawn queued units
			if (m_aRefillQueue && !m_aRefillQueue.IsEmpty())
			{
				if (m_iDespawnedGroupMembers > 0)
				{
					SpawnUnit(m_aRefillQueue[0], false);
					m_iDespawnedGroupMembers--;
				}
				else
				{
					SpawnUnit(m_aRefillQueue[0]);
				}
			}
		}
		
		//Despawn units that are too far away from players
		if (distanceState == SCR_EDefenderSpawnerState.DEFENDERS_DESPAWN)
		{
			if (m_AIgroup)
			{
				m_iDespawnedGroupMembers = m_AIgroup.GetAgentsCount();
				SCR_EntityHelper.DeleteEntityAndChildren(m_AIgroup);
			}
			
			//If leftover groups from previous owners remains alive, they will be despawned
			if (m_aPreviousGroups)
			{
				foreach (SCR_AIGroup aiGroup : m_aPreviousGroups)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(aiGroup);
				}
				
				m_aPreviousGroups = null;
			}
			
			//Delete units from previous owners that were spawning in process of faction change. (edge case, but might happen)
			if (m_aOldUnits)
			{
				int index = m_aOldUnits.Count()-1;
				for (index; index >= 0; index--)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(m_aOldUnits[index]);
				}
				
				m_aOldUnits = null;
			}
			
			//Despawn any units that are currently walking to RP
			if (m_aUnitsOnMove)
			{
				foreach (Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp> groupWaypoint : m_aUnitsOnMove)
				{
					SCR_EntityHelper.DeleteEntityAndChildren(groupWaypoint.param2.GetControlledEntity());
					m_iDespawnedGroupMembers++;
				}
				
				m_aUnitsOnMove = null;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnFactionChanged(FactionAffiliationComponent owner, Faction previousFaction, Faction faction)
	{
		//if there is already a group from current faction, cache it for despawn
		if (m_AIgroup)
		{
			if (!m_aPreviousGroups)
				m_aPreviousGroups = {};
			
			m_aPreviousGroups.Insert(m_AIgroup);
			m_AIgroup = null;
		}
		
		m_GroupEntry = null;
		AssignDefenderGroupDataFromOwningFaction();
	}
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		super.EOnInit(owner);

		m_ResourceComponent = SCR_ResourceComponent.FindResourceComponent(owner);
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return;	
			
		m_GroupSpawningManager = SCR_SpawnerAIGroupManagerComponent.Cast(gameMode.FindComponent(SCR_SpawnerAIGroupManagerComponent));
		if (!m_GroupSpawningManager)
		{
			Print("SCR_DefenderSpawnerComponent requires SCR_SpawnerAIGroupManagerComponent attached to gamemode to work properly!", LogLevel.ERROR);	
			return;
		}	
			
		if (IsProxy())
			return;

		//Setup group handling (spawning and refilling). This delay also prevents potential JIP replication error on clients
		GetGame().GetCallqueue().CallLater(HandleGroup, SPAWN_CHECK_INTERVAL, true);
	}
		
	//------------------------------------------------------------------------------------------------
	protected override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;

		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_DefenderSpawnerComponent()
	{
		if (m_AIgroup)
			SCR_EntityHelper.DeleteEntityAndChildren(m_AIgroup);

		//Delete units in process of spawning
		if (m_aUnitsOnMove)
		{
			foreach (Tuple3<SCR_AIActionBase, AIAgent, WorldTimestamp> groupWaypoint : m_aUnitsOnMove)
			{
				if (!groupWaypoint.param2)
					continue;
				
				SCR_EntityHelper.DeleteEntityAndChildren(groupWaypoint.param2.GetControlledEntity());
			}
		}
		
		//Delete units from previous owners
		if (m_aPreviousGroups)
		{
			int index = m_aPreviousGroups.Count()-1;
			for (index; index >= 0; index--)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(m_aPreviousGroups[index]);
			}
		}
		
		//Delete units from previous owners that were spawning in process of faction change. (edge case, but might happen)
		if (m_aOldUnits)
		{
			int index = m_aOldUnits.Count()-1;
			for (index; index >= 0; index--)
			{
				SCR_EntityHelper.DeleteEntityAndChildren(m_aOldUnits[index]);
			}
		}
		
		GetGame().GetCallqueue().Remove(HandleGroup);
	}
}

enum SCR_EDefenderSpawnerState
{
	DEFENDERS_ENABLED,
	DEFENDERS_PAUSED_SPAWN,
	DEFENDERS_DESPAWN
}
