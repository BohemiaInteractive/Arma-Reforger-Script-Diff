[EntityEditorProps(category: "GameScripted/ScenarioFramework/Slot", description: "")]
class SCR_ScenarioFrameworkSlotAIClass : SCR_ScenarioFrameworkSlotBaseClass
{
}

// SCR_ScenarioFrameworkSlotAI Spawned Invoker
void ScriptInvokerScenarioFrameworkSlotAIMethod(SCR_ScenarioFrameworkLayerBase layer, IEntity entity);
typedef func ScriptInvokerScenarioFrameworkSlotAIMethod;
typedef ScriptInvokerBase<ScriptInvokerScenarioFrameworkSlotAIMethod> ScriptInvokerScenarioFrameworkSlotAI;

class SCR_ScenarioFrameworkSlotAI : SCR_ScenarioFrameworkSlotBase
{
	[Attribute(desc: "Waypoint Groups if applicable", category: "Waypoints")]
	ref SCR_ScenarioFrameworkWaypointSet m_WaypointSet;

	[Attribute(desc: "Spawn AI on the first WP Slot", defvalue: "1", category: "Waypoints")]
	bool m_bSpawnAIOnWPPos;

	[Attribute(desc: "Default waypoint if any WP group is defined", "{93291E72AC23930F}Prefabs/AI/Waypoints/AIWaypoint_Defend.et", category: "Waypoints")]
	ResourceName m_sWPToSpawn;

	[Attribute(defvalue: "0", desc: "Balancing group sizes based on number of players", category: "Balance")]
	bool m_bBalanceOnPlayersCount;

	[Attribute(defvalue: "1", desc: "Least amount of AIs in the group after balancing occurs. Will not exceed maximum number of units defined in the group prefab.", category: "Balance")]
	int m_iMinUnitsInGroup;
	
	[Attribute(defvalue: "1", desc: "When enabled and AI enters a vehicle, whole AI group won't be despawned by the Dynamic Despawn feature until the whole group is outside the vehicle", category: "Activation")]
	bool m_bExcludeFromDynamicDespawnOnVehicleEntered;

	[Attribute(defvalue: SCR_EAIGroupFormation.Wedge.ToString(), UIWidgets.ComboBox, "AI group formation", "", ParamEnumArray.FromEnum(SCR_EAIGroupFormation), category: "Common")]
	SCR_EAIGroupFormation m_eAIGroupFormation;

	[Attribute(defvalue: EAISkill.REGULAR.ToString(), UIWidgets.ComboBox, "AI skill in combat", "", ParamEnumArray.FromEnum(EAISkill), category: "Common")]
	EAISkill m_eAISkill;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.EditBox, desc: "Sets perception ability. Affects speed at which perception detects targets. Bigger value means proportionally faster detection.", params: "0 100 0.001", category: "Common")]
	float m_fPerceptionFactor;

	[Attribute(defvalue: "{000CD338713F2B5A}Prefabs/AI/Groups/Group_Base.et", category: "Common")]
	ResourceName m_sGroupPrefab;

	ref array<AIWaypoint> m_aWaypoints = {};
	SCR_AIGroup m_AIGroup;
	ref array<ResourceName> m_aAIPrefabsForRemoval = {};

	ref array<SCR_ScenarioFrameworkSlotWaypoint> m_aSlotWaypoints = {};
	int m_iCurrentlySpawnedWaypoints;

	// Temporary solution as we don't know if invoker or a different method will initialize waypoints
	bool m_bWaypointsInitialized;
	bool m_bGroupWasNull;

	//------------------------------------------------------------------------------------------------
	//! \param[in] arrayForRemoval Array of resource names for AI prefab removal.
	void SetAIPrefabsForRemoval(array<ResourceName> arrayForRemoval)
	{
		m_aAIPrefabsForRemoval = arrayForRemoval;
	}

	//------------------------------------------------------------------------------------------------
	//! \return Array of AI prefab names for removal.
	array<ResourceName> GetAIPrefabsForRemoval()
	{
		return m_aAIPrefabsForRemoval;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Restores default settings, clears waypoints, AI group, prefabs, slots, and initializes randomization.
	//! \param[in] includeChildren Includes restoring default settings for child objects as well.
	//! \param[in] reinitAfterRestoration Resets object state after restoration, allowing for fresh initialization.
	//! \param[in] affectRandomization Affects whether randomization is reset during default restoration.
	override void RestoreToDefault(bool includeChildren = false, bool reinitAfterRestoration = false, bool affectRandomization = true)
	{
		m_aWaypoints.Clear();
		m_AIGroup = null;
		m_aAIPrefabsForRemoval.Clear();
		m_aSlotWaypoints.Clear();
		m_iCurrentlySpawnedWaypoints = 0;
		m_bWaypointsInitialized = false;
		
		super.RestoreToDefault(includeChildren, reinitAfterRestoration, affectRandomization);
	}

	//------------------------------------------------------------------------------------------------
	//! Dynamically despawns this layer.
	//! \param[in] layer Layer represents the scenario framework layer for dynamic despawning entities in the method.
	override void DynamicDespawn(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(DynamicDespawn);
		if (!m_Entity && !SCR_StringHelper.IsEmptyOrWhiteSpace(m_sObjectToSpawn))
		{
			GetOnAllChildrenSpawned().Insert(DynamicDespawn);
			return;
		}
		
		GetOnAllChildrenSpawned().Remove(ProcessWaypoints);
		GetOnAllChildrenSpawned().Remove(SetWaypointToAI);

		if (!m_bInitiated || m_bExcludeFromDynamicDespawn)
			return;

		m_bInitiated = false;
		m_bDynamicallyDespawned = true;
		if (m_Entity)
			m_vPosition = m_Entity.GetOrigin();

		if (m_AIGroup)
			m_AIGroup.GetOnAgentRemoved().Remove(DecreaseAIGroupMemberCount);

		m_AIGroup = null;

		m_aSpawnedEntities.RemoveItem(null);
		foreach (IEntity entity : m_aSpawnedEntities)
		{
			m_vPosition = entity.GetOrigin();
			SCR_EntityHelper.DeleteEntityAndChildren(entity);
		}

		m_aSpawnedEntities.Clear();
		m_aWaypoints.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAgentCompartmentEntered(AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (m_bExcludeFromDynamicDespawn)
			return;
		
		if (!agent || !Vehicle.Cast(targetEntity))
			return;
		
		m_bExcludeFromDynamicDespawn = true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnAgentCompartmentLeft(AIAgent agent, IEntity targetEntity, BaseCompartmentManagerComponent manager, int mgrID, int slotID, bool move)
	{
		if (!m_bExcludeFromDynamicDespawn)
			return;
		
		if (!agent || !Vehicle.Cast(targetEntity))
			return;
		
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);
		
		bool isInVehicle;
		foreach (AIAgent groupAgent : agents)
		{
			ChimeraCharacter character = ChimeraCharacter.Cast(groupAgent.GetControlledEntity());
			if (character && character.IsInVehicle())
			{
				isInVehicle = true;
				break;
			}
		}
		
		if (!isInVehicle)
			m_bExcludeFromDynamicDespawn = false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Clears cached values and handles Waypoint Sets
	//! \return true if all initialization steps succeed, false otherwise.
	override bool InitOtherThings()
	{
		m_aWaypoints.Clear();
		m_AIGroup = null;
		m_iCurrentlySpawnedWaypoints = 0;
		m_bWaypointsInitialized = false;
		SCR_AIGroup.IgnoreSpawning(true);

		if (m_eActivationType == SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT && m_WaypointSet && !m_WaypointSet.m_aLayerName.IsEmpty())
			InitWaypoints();
		
		return super.InitOtherThings();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes AI after all children spawned, removes event handler.
	//! \param[in] layer for which this is called
	override void AfterAllChildrenSpawned(SCR_ScenarioFrameworkLayerBase layer)
	{
		m_bInitiated = true;

		if (m_Entity)
			ActivateAI();
		else
			AfterAllAgentsSpawned();

		GetOnAllChildrenSpawned().Remove(AfterAllChildrenSpawned);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes plugins, actions, and checks for parent layer spawn completion, then sets waypoints for AI if not initialized.
	void AfterAllAgentsSpawned()
	{
		if (!m_bInitiated)
			return;
		
		foreach (SCR_ScenarioFrameworkPlugin plugin : m_aPlugins)
		{
			plugin.Init(this);
		}

		InitActivationActions();

		if (m_ParentLayer)
			m_ParentLayer.CheckAllChildrenSpawned(this);

		if (!m_bWaypointsInitialized)
			SetWaypointToAI(this);
		
		SCR_ScenarioFrameworkSystem.InvokeSlotAISpawned(this, m_Entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Activates AI group, removes unwanted prefabs, balances units count, sets on agent remove and add events,
	void ActivateAI()
	{
		if (m_AIGroup) // Group alraedy setup from e.g. save-game
		{
			AfterAllAgentsSpawned();
			return;
		}
		
		m_bGroupWasNull = false;
		m_AIGroup = SCR_AIGroup.Cast(m_Entity);
		if (!m_AIGroup)
			m_bGroupWasNull = true;

		if (!m_AIGroup && !CreateAIGroup())
		{
			AfterAllAgentsSpawned();
			return;
		}

		if (!m_aAIPrefabsForRemoval.IsEmpty())
		{
			foreach (ResourceName prefabToRemove : m_aAIPrefabsForRemoval)
			{
				for (int i = m_AIGroup.m_aUnitPrefabSlots.Count() - 1; i >= 0; i--)
				{
					if (m_AIGroup.m_aUnitPrefabSlots[i] != prefabToRemove)
						continue;

					m_AIGroup.m_aUnitPrefabSlots.Remove(i);
					break;
				}
			}
		}
		else if (m_bBalanceOnPlayersCount)
		{
			int iMaxUnitsInGroup = m_AIGroup.m_aUnitPrefabSlots.Count();
			float iUnitsToSpawn = Math.Map(GetPlayersCount(), 1, GetMaxPlayersForGameMode(), Math.RandomInt(1, 3), iMaxUnitsInGroup);

			if (iUnitsToSpawn < m_iMinUnitsInGroup)
				iUnitsToSpawn = m_iMinUnitsInGroup;

			m_AIGroup.SetMaxUnitsToSpawn(iUnitsToSpawn);
		}

		m_AIGroup.GetOnAgentRemoved().Insert(DecreaseAIGroupMemberCount);

		if (m_bGroupWasNull)
		{
			m_AIGroup.SetNumberOfMembersToSpawn(1);
			OnAgentAdded(null);
			return;
		}

		m_AIGroup.SetMemberSpawnDelay(200);
		m_AIGroup.GetOnAgentAdded().Insert(OnAgentAdded);
		m_AIGroup.SpawnUnits();
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] child Child is an AI Agent added to the group, triggers group component initialization and waypoint setting if necessary.
	void OnAgentAdded(AIAgent child)
	{
		if (!m_AIGroup)
		{
			AfterAllAgentsSpawned();
			return;
		}
		
		if (m_AIGroup.GetNumberOfMembersToSpawn() != m_AIGroup.GetAgentsCount())
			return;

		m_AIGroup.GetOnAgentAdded().Remove(OnAgentAdded);

		InitGroupComponents();

		if (m_aSlotWaypoints.IsEmpty())
			SetWaypointToAI(this);

		AfterAllAgentsSpawned();
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes AI group components, sets formation, and combat parameters for agents in the group.
	protected void InitGroupComponents()
	{
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);

		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (formComp)
		{
			formComp.SetFormation(SCR_Enum.GetEnumName(SCR_EAIGroupFormation, m_eAIGroupFormation));

			AIFormationDefinition formDef = formComp.GetFormation();
			if (formDef)
			{
				foreach (int i, AIAgent agent : agents)
				{
					IEntity agentEntity = agent.GetControlledEntity();
					if (!agentEntity)
						continue;

					vector basePosition = m_vPosition;
					if (basePosition == vector.Zero)
					{
						basePosition = m_AIGroup.GetOrigin();
						// Set the Y-Level to the Slot height to allow mission makers to tune the spawn height.
						basePosition[1] = GetOwner().GetOrigin()[1];
					}
					vector formationPosition = basePosition + formDef.GetOffsetPosition(i);
					
					World world = agentEntity.GetWorld();
					if (world)
					{
						float surfaceY = world.GetSurfaceY(formationPosition[0], formationPosition[2]);
						float oceanY = world.GetOceanHeight(formationPosition[0], formationPosition[2]);
						formationPosition[1] = Math.Max(Math.Max(oceanY, surfaceY), formationPosition[1]);
						vector floorPosition;
						if (SCR_EmptyPositionHelper.TryFindNearbyFloorPositionForEntity(world, agentEntity, formationPosition, floorPosition))
							formationPosition = floorPosition;
					}
					agentEntity.SetOrigin(formationPosition);
					agentEntity.Update();
					
					SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
					if (combatComponent)
					{
						combatComponent.SetAISkill(m_eAISkill);
						combatComponent.SetPerceptionFactor(m_fPerceptionFactor);
					}
					
					if (!m_bExcludeFromDynamicDespawnOnVehicleEntered)
						continue;
					
					SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
					if (!chimeraAgent)
						continue;
					
					SCR_AIInfoComponent info = chimeraAgent.m_InfoComponent;
					if (!info)
						continue;
					
					info.m_OnCompartmentEntered.Insert(OnAgentCompartmentEntered);
					info.m_OnCompartmentLeft.Insert(OnAgentCompartmentLeft);
				}
			}
		}
		else
		{
			foreach (AIAgent agent : agents)
			{
				IEntity agentEntity = agent.GetControlledEntity();
				if (!agentEntity)
					continue;

				SCR_AICombatComponent combatComponent = SCR_AICombatComponent.Cast(agentEntity.FindComponent(SCR_AICombatComponent));
				if (combatComponent)
				{
					combatComponent.SetAISkill(m_eAISkill);
					combatComponent.SetPerceptionFactor(m_fPerceptionFactor);
				}
				
				if (!m_bExcludeFromDynamicDespawnOnVehicleEntered)
					continue;
				
				SCR_ChimeraAIAgent chimeraAgent = SCR_ChimeraAIAgent.Cast(agent);
				if (!chimeraAgent)
					continue;
					
				SCR_AIInfoComponent info = chimeraAgent.m_InfoComponent;
				if (!info)
					continue;
					
				info.m_OnCompartmentEntered.Insert(OnAgentCompartmentEntered);
				info.m_OnCompartmentLeft.Insert(OnAgentCompartmentLeft);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes waypoints by finding them in specified layers, adding them to list, and processing them after initialization.
	protected void InitWaypoints()
	{
		array<string> WaypointSetLayers = {};
		WaypointSetLayers = m_WaypointSet.m_aLayerName;
		array<SCR_ScenarioFrameworkLayerBase> layerBases = {};
		array<SCR_ScenarioFrameworkLayerBase> layerChildren = {};
		SCR_ScenarioFrameworkLayerBase layerBase;
		SCR_ScenarioFrameworkSlotWaypoint slotWaypoint;
		
		BaseWorld world = GetGame().GetWorld();
		if (!world)
			return;

		foreach (string waypointSetLayer : WaypointSetLayers)
		{
			IEntity layerEntity = world.FindEntityByName(waypointSetLayer);
			if (!layerEntity)
				continue;

			layerBase = SCR_ScenarioFrameworkLayerBase.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (layerBase && !SCR_ScenarioFrameworkSlotWaypoint.Cast(layerBase))
			{
				if (layerBases.Contains(layerBase))
					continue;

				layerChildren = layerBase.GetChildrenEntities();
				foreach (SCR_ScenarioFrameworkLayerBase child : layerChildren)
				{
					slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(child);
					if (slotWaypoint && !m_aSlotWaypoints.Contains(slotWaypoint))
						m_aSlotWaypoints.Insert(slotWaypoint);
				}
			}
			else
			{
				slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(layerEntity.FindComponent(SCR_ScenarioFrameworkSlotWaypoint));
				if (slotWaypoint && !m_aSlotWaypoints.Contains(slotWaypoint))
						m_aSlotWaypoints.Insert(slotWaypoint);
			}
		}

		int waypointCount = m_aSlotWaypoints.Count();
		foreach (SCR_ScenarioFrameworkSlotWaypoint waypoint : m_aSlotWaypoints)
		{
			if (waypoint.GetIsInitiated())
				m_iCurrentlySpawnedWaypoints++;
			else
				waypoint.GetOnAllChildrenSpawned().Insert(CheckWaypointsAfterInit);

			if (waypointCount == m_iCurrentlySpawnedWaypoints)
				GetOnAllChildrenSpawned().Insert(ProcessWaypoints);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Checks waypoints after init, adds/removes event handler for all waypoints spawned, increments spawned way
	//! \param[in] layer for which this is called
	protected void CheckWaypointsAfterInit(SCR_ScenarioFrameworkLayerBase layer)
	{
		SCR_ScenarioFrameworkSlotWaypoint slotWaypoint = SCR_ScenarioFrameworkSlotWaypoint.Cast(layer);
		if (slotWaypoint)
		{
			SCR_ScenarioFrameworkWaypointCycle cycleWP = SCR_ScenarioFrameworkWaypointCycle.Cast(slotWaypoint.m_Waypoint);
			if (cycleWP)
			{
				if (!cycleWP.m_bInitiated)
				{
					cycleWP.GetOnAllWaypointsSpawned().Insert(CheckWaypointsAfterInit);
					return;
				}
				else if (cycleWP.m_OnAllWaypointsSpawned)
				{
					cycleWP.GetOnAllWaypointsSpawned().Remove(CheckWaypointsAfterInit);
				}
			}
		}

		m_iCurrentlySpawnedWaypoints++;
		if (m_iCurrentlySpawnedWaypoints == m_aSlotWaypoints.Count())
			ProcessWaypoints(this);

		layer.GetOnAllChildrenSpawned().Remove(CheckWaypointsAfterInit);
	}

	//------------------------------------------------------------------------------------------------
	//! Initializes waypoints for AI if scenario is initiated, otherwise waits for all children to spawn before setting waypoints.
	//! \param[in] layer for which this is called
	protected void ProcessWaypoints(SCR_ScenarioFrameworkLayerBase layer)
	{
		if (m_bInitiated && !m_bWaypointsInitialized)
			SetWaypointToAI(this);
		else if (!m_bGroupWasNull)
			GetOnAllChildrenSpawned().Insert(SetWaypointToAI);
	}

	//------------------------------------------------------------------------------------------------
	//! \param[in] layer Initializes waypoints for AI group in scenario framework layer.
	protected void SetWaypointToAI(SCR_ScenarioFrameworkLayerBase layer)
	{
		GetOnAllChildrenSpawned().Remove(SetWaypointToAI);
		m_bWaypointsInitialized = true;

		if (!m_Entity)
		{
			Print(string.Format("ScenarioFramework: Trying to add waypoints to non existing entity! Did you select the object to spawn for %1?", GetOwner().GetName()), LogLevel.ERROR);
			SCR_AIGroup.IgnoreSpawning(false);
			return;
		}

		if (m_aSlotWaypoints.IsEmpty())
		{
			CreateDefaultWaypoint();
		}
		else
		{
			AIWaypoint waypoint;
			foreach (SCR_ScenarioFrameworkSlotWaypoint waypointSlot : m_aSlotWaypoints)
			{
				waypoint = AIWaypoint.Cast(waypointSlot.GetSpawnedEntity());
				if (waypoint)
					m_aWaypoints.Insert(waypoint);
			}
		}

		if (!m_AIGroup)
			m_AIGroup = SCR_AIGroup.Cast(m_Entity);

		if (!m_AIGroup)
		{
			if (!CreateAIGroup())
			{
				m_bWaypointsInitialized = true;
				return;
			}
		}

		m_aWaypoints.RemoveItemOrdered(null);

		foreach (AIWaypoint waypoint : m_aWaypoints)
		{
			m_AIGroup.AddWaypoint(waypoint);
		}
		
		if (m_aWaypoints.IsEmpty())
		{
			m_bWaypointsInitialized = false;
			return;
		}
		
		if (m_vPosition != vector.Zero || !m_bSpawnAIOnWPPos || m_aWaypoints.IsEmpty())
			return;
		
		AIWaypoint waypoint = m_aWaypoints[0];
		AIWaypointCycle cycleWaypoint = AIWaypointCycle.Cast(waypoint);
		if (cycleWaypoint)
		{
			array<AIWaypoint> cycleWaypoints = {};
			cycleWaypoint.GetWaypoints(cycleWaypoints);
			if (!cycleWaypoints.IsEmpty() && cycleWaypoints[0] != null)
			{
				waypoint = cycleWaypoints[0];
			}
			else
			{
				m_bWaypointsInitialized = false;
				return;
			}
		}
			
		AIFormationComponent formComp = AIFormationComponent.Cast(m_AIGroup.FindComponent(AIFormationComponent));
		if (!formComp)
			return;
			
		AIFormationDefinition formDef = formComp.GetFormation();
		if (!formDef)
			return;
			
		array<AIAgent> agents = {};
		m_AIGroup.GetAgents(agents);

		foreach (int i, AIAgent agent : agents)
		{
			IEntity agentEntity = agent.GetControlledEntity();
			if (!agentEntity)
				continue;
	
			agentEntity.SetOrigin(waypoint.GetOrigin() + formDef.GetOffsetPosition(i));
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Creates default waypoint for AI entity, spawns it in world space, adds it to list.
	//! \return an AIWaypoint object if successful, otherwise null.
	protected AIWaypoint CreateDefaultWaypoint()
	{
		if (!m_Entity)
			return null;

		EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = m_Entity.GetOrigin();

		Resource resWP = Resource.Load(m_sWPToSpawn);
		if (!resWP || !resWP.IsValid())
			return null;

		AIWaypoint waypoint = AIWaypoint.Cast(GetGame().SpawnEntityPrefab(resWP, GetGame().GetWorld(), spawnParams));
		if (!waypoint)
			return null;

		m_aWaypoints.Insert(waypoint);
		return waypoint;
	}

	//------------------------------------------------------------------------------------------------
	//! Decreases AI group member count, handles termination if group empty, adds removed AI entity prefab to removal list.
	//! \param[in] group Decreases AI group member count, checks for termination conditions, adds removed AI entity prefab to removal list.
	//! \param[in] agent Agent represents an AI entity in the game world, controlled by AI group, which is decreased from the group count in this
	void DecreaseAIGroupMemberCount(SCR_AIGroup group, AIAgent agent)
	{
		if (group.GetAgentsCount() == 0)
		{
			if (m_bEnableRepeatedSpawn)
			{
				if (m_iRepeatedSpawnNumber != -1 && m_iRepeatedSpawnNumber <= 0)
					SetIsTerminated(true);
			}
			else
			{
				SetIsTerminated(true);
			}
		}

		IEntity agentEntity = agent.GetControlledEntity();
		if (!agentEntity)
		{
			Print("ScenarioFramework: Decreasing AI member count failed due to not getting AI entity!", LogLevel.ERROR);
			return;
		}

		EntityPrefabData prefabData = agentEntity.GetPrefabData();
		if (!prefabData)
		{
			Print("ScenarioFramework: Decreasing AI member count failed due to not getting entity prefab data", LogLevel.ERROR);
			return;
		}

		ResourceName resource = prefabData.GetPrefabName();
		if (resource)
			m_aAIPrefabsForRemoval.Insert(resource);
	}

	//------------------------------------------------------------------------------------------------
	//! Creates an AI group, sets its faction, adds entity to group, and optionally sets entity's position.
	//! \return true if AI group creation is successful, false otherwise.
	protected bool CreateAIGroup()
	{
		EntitySpawnParams paramsPatrol = new EntitySpawnParams();
		paramsPatrol.TransformMode = ETransformMode.WORLD;

		paramsPatrol.Transform[3] = m_Entity.GetOrigin();
		Resource groupResource = Resource.Load(m_sGroupPrefab);
		if (!groupResource.IsValid())
			return false;

		m_AIGroup = SCR_AIGroup.Cast(GetGame().SpawnEntityPrefab(groupResource, GetGame().GetWorld(), paramsPatrol));
		if (!m_AIGroup)
			return false;

		FactionAffiliationComponent facComp = FactionAffiliationComponent.Cast(m_Entity.FindComponent(FactionAffiliationComponent));
		if (!facComp)
			return false;

		m_AIGroup.SetFaction(facComp.GetAffiliatedFaction());
		m_AIGroup.AddAIEntityToGroup(m_Entity);

		if (m_vPosition != vector.Zero)
			m_Entity.SetOrigin(m_vPosition);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_ScenarioFrameworkSlotAI(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_iDebugShapeColor = ARGB(100, 0x00, 0x10, 0xFF);
	}
}