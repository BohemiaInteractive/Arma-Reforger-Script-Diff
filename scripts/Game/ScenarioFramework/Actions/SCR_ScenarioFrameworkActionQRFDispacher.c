enum SCR_EQRFThresholdType
{
	DEAD_UNITS = 0,
	REMAINING_UNITS,
	PROC_LOST_UNITS,
}

[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionQRFDispacher : SCR_ScenarioFrameworkActionBase
{
	[Attribute("X_QRF", UIWidgets.Auto, "Name of the QRF layer that contains pool of spawn points")]
	protected string m_sQRFLayerName;

	[Attribute(SCR_EQRFThresholdType.DEAD_UNITS.ToString(), UIWidgets.ComboBox, desc: "Type of the threshold threshold that will be used to determine if it should spawn QRF", "", ParamEnumArray.FromEnum(SCR_EQRFThresholdType))]
	protected SCR_EQRFThresholdType m_eThresholdType;// <= REMAINING_UNITS, DEAD_UNITS, PROC_LOST_UNITS

	[Attribute("5", UIWidgets.Auto, "Numeric value which interpretation depends on the m_eThresholdType f.e. if Threshold Type == REMAINING_UNITS then if Threshold >= (number of units alive) then send QRF")]
	protected int m_iThreshold;

	[Attribute("5", UIWidgets.Auto, "How many times this Area can send QRF where -1 == unlimited")]
	protected int m_iNumberOfAvailableQRFWaves;// - even if we have more QRF groups but we are out of waves then there wont be new QRF sent

	[Attribute("1", UIWidgets.Auto, "How much threat level has to be increased each time that threshold is reached")]
	protected float m_fThreatLevelEscalation;

	[Attribute("0", UIWidgets.Auto, "How long (in seconds) game should wait when threshold is reached before spawning QRF where 0 == imminently", "0 inf 1")]
	protected float m_fQRFSpawnDelay;

	[Attribute("300", UIWidgets.Auto, "How long (in seconds) game should wait until next QRF will be possible to be requested where 0 == no delay", "0 inf 1")]
	protected float m_fQRFNextWaveDelay;

	[Attribute(desc: "List of possible QRF groups to spawn")]
	protected ref array<ref SCR_QRFGroupConfig> m_aGroupList;

	[Attribute(desc: "List defining maximal distance for QRF unit of given type")]
	protected ref array<ref SCR_QRFTypeMaxDistance> m_aQRFMaxDistanceConfig;

	[Attribute(desc: "List of waypoints for QRF that will be applied in order (to aplicable group type)")]
	protected ref array<ref SCR_QRFWaypointConfig> m_aWPConfig;

	[Attribute(desc: "Sound event name that will be played on dead soldier entity when QRF is requested")]
	protected string m_sQRFRequestedSoundEventName;

	[Attribute(desc: "Sound event name that will be played on dead soldier entity when QRF is spawned")]
	protected string m_sQRFSentSoundEventName;

	[Attribute(desc: "'*.acp' file which contains desired sound events", params: "acp")]
	protected ResourceName m_sSoundProjectFile;
	
	[Attribute(desc: "Actions that will be triggered once QRF are dispatched.")]
	ref array<ref SCR_ScenarioFrameworkActionBase>	m_aActions;

	protected ref array<SCR_ScenarioFrameworkQRFSlotAI> m_aQRFSpawnPoints = {};
	protected SCR_ScenarioFrameworkArea m_AreaFramework;
	protected SCR_ScenarioFrameworkLayerBase m_QRFLayer;
	protected int m_iNumberOfSoldiersInTheArea;
	protected int m_iRemovedSoldier;
	protected float m_fNextWaveDelayClock;
	protected float m_fThreatLevel = 1;
	protected int m_iSpawnTickets;
	protected bool m_bWaitingForDelayedSpawn;
	protected bool m_bWaitingForNextWave;
	protected vector m_vTargetPosition;
	protected ref array<ref SCR_QRFVehicleSpawnConfig> m_aVehicleSpawnQueueConfig = {};

	//------------------------------------------------------------------------------------------------
	//! Will tap into AI groups and monitor their numbers
	//! Will return true if there was at least one grou00p which we now observe
	protected bool WatchAIGroup(IEntity entities)
	{
		return WatchAIGroup({entities});
	}

	//------------------------------------------------------------------------------------------------
	//! Will tap into AI groups and monitor their numbers
	//! Will return true if there was at least one group which we now observe
	protected bool WatchAIGroup(array<IEntity> entities)
	{
		if (!entities)
			return false;

		bool result;
		SCR_AIGroup aiGroup;
		foreach (IEntity entity : entities)
		{
			aiGroup = GetAIGroup(entity);
			if (!aiGroup)
				continue;

			aiGroup.GetOnAgentRemoved().Remove(OnGroupCompositionChanged);
			aiGroup.GetOnAgentRemoved().Insert(OnGroupCompositionChanged);
			aiGroup.GetOnAllDelayedEntitySpawned().Remove(OnDelayedGroupMembersSpawned);
			aiGroup.GetOnAllDelayedEntitySpawned().Insert(OnDelayedGroupMembersSpawned);
			if (aiGroup.GetAgentsCount() == 0)
				m_iNumberOfSoldiersInTheArea++;
			else
				m_iNumberOfSoldiersInTheArea += aiGroup.GetAgentsCount();

			result = true;
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Retrieves AI group from entity even when it is an AI agent or SCR_ChimeraCharacter
	SCR_AIGroup GetAIGroup(IEntity entity)
	{
		if (!entity)
			return null;

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(entity);
		if (aiGroup)
			return aiGroup;

		AIAgent agent = AIAgent.Cast(entity);
		if (agent)
			return SCR_AIGroup.Cast(agent.GetParentGroup());

		if (!SCR_ChimeraCharacter.Cast(entity))
			return null;

		AIControlComponent control = AIControlComponent.Cast(entity.FindComponent(AIControlComponent));
		if (!control)
			return null;

		agent = control.GetControlAIAgent();
		if (!agent)
			return null;

		return SCR_AIGroup.Cast(agent.GetParentGroup());
	}

	//------------------------------------------------------------------------------------------------
	protected void OnDelayedGroupMembersSpawned(SCR_AIGroup group)
	{
		m_iNumberOfSoldiersInTheArea++;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnGroupCompositionChanged(SCR_AIGroup group, AIAgent agent)
	{
		if (!agent)
			return;

		if (!agent.GetControlledEntity())
			return;

		SCR_ChimeraCharacter char = SCR_ChimeraCharacter.Cast(agent.GetControlledEntity());
		if (!char)
			return;

		SCR_CharacterControllerComponent controller = SCR_CharacterControllerComponent.Cast(char.GetCharacterController());
		if (!controller || controller.GetLifeState() != ECharacterLifeState.DEAD)
			return;

		vector tmpVec = agent.GetControlledEntity().GetOrigin();
		if (m_vTargetPosition == vector.Zero)
			m_vTargetPosition = tmpVec;
		else
			m_vTargetPosition = vector.Lerp(m_vTargetPosition, tmpVec, 0.95);

		m_iRemovedSoldier++;
		ThresholdValidation(char)
	}

	//------------------------------------------------------------------------------------------------
	protected void ThresholdValidation(IEntity killedEntity = null)
	{
		bool startQRFProcedure;
		switch (m_eThresholdType)
		{
			case SCR_EQRFThresholdType.DEAD_UNITS :
			{
				if (m_iRemovedSoldier >= m_iThreshold)
					startQRFProcedure = true;

			} break;

			case SCR_EQRFThresholdType.REMAINING_UNITS :
			{
				if (m_iNumberOfSoldiersInTheArea - m_iRemovedSoldier <= m_iThreshold)
					startQRFProcedure = true;

			} break;

			case SCR_EQRFThresholdType.PROC_LOST_UNITS :
			{
				if (m_iNumberOfSoldiersInTheArea > 0 && (m_iNumberOfSoldiersInTheArea - m_iRemovedSoldier) * 100 / m_iNumberOfSoldiersInTheArea <= m_iThreshold)
					startQRFProcedure = true;

			} break;
		}

		if (!startQRFProcedure)
			return;
		
		if (m_bWaitingForDelayedSpawn)
		{
			if (!m_bWaitingForNextWave && m_iNumberOfAvailableQRFWaves - 1 > 0 && m_fQRFSpawnDelay > 0)
			{
				float callTime = m_fNextWaveDelayClock - GetGame().GetWorld().GetWorldTime();
				if (callTime < 0)
					callTime = 0;

				m_bWaitingForNextWave = true;
				SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(StartQRFProcedure, callTime, param1: killedEntity);
			}
			return;
		}

		if (m_bWaitingForNextWave)
			return;

		StartQRFProcedure(killedEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void ThresholdHandling()
	{
		switch (m_eThresholdType)
		{
			case SCR_EQRFThresholdType.DEAD_UNITS :
			{
				m_iRemovedSoldier = 0;
			} break;

			case SCR_EQRFThresholdType.REMAINING_UNITS :
			{
				m_iNumberOfSoldiersInTheArea -= m_iRemovedSoldier;
			} break;
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void StartQRFProcedure(IEntity killedEntity)
	{
		if (m_iNumberOfAvailableQRFWaves == 0)
			return;

		float time = GetGame().GetWorld().GetWorldTime();
		if (m_fNextWaveDelayClock > time && !m_bWaitingForDelayedSpawn && !m_bWaitingForNextWave)
			return;

		if (m_fQRFSpawnDelay > 0)
		{
			if (!m_bWaitingForDelayedSpawn)
			{
				if (m_bWaitingForNextWave)
					m_bWaitingForNextWave = false;

				ThresholdHandling();
				m_fNextWaveDelayClock = time + m_fQRFNextWaveDelay * 1000;
				m_bWaitingForDelayedSpawn = true;
				SCR_ScenarioFrameworkSystem.GetCallQueuePausable().CallLater(StartQRFProcedure, m_fQRFSpawnDelay * 1000, param1: killedEntity);
				if (killedEntity)
					DoPlaySoundOnEntityPosition(killedEntity, m_sSoundProjectFile, m_sQRFRequestedSoundEventName);

				return;
			}
			else
			{
				m_bWaitingForDelayedSpawn = false;
			}
		}
		else
		{
			ThresholdHandling();
			m_fNextWaveDelayClock = time + m_fQRFNextWaveDelay * 1000;
		}

		if (killedEntity)
			DoPlaySoundOnEntityPosition(killedEntity, m_sSoundProjectFile, m_sQRFSentSoundEventName);

		m_iNumberOfSoldiersInTheArea -= m_iRemovedSoldier;
		m_iSpawnTickets += m_fThreatLevel;
		m_fThreatLevel += m_fThreatLevelEscalation;
		if (m_aQRFSpawnPoints.IsEmpty())
			return;

		while (m_iSpawnTickets > 0)
		{
			SCR_QRFGroupConfig selectedGroup = SelectRandomGroup(m_iSpawnTickets);
			if (!selectedGroup)
			{
				m_iSpawnTickets--;
				continue;
			}

			m_iSpawnTickets -= selectedGroup.GetSpawnCost();
			SCR_ScenarioFrameworkQRFSlotAI selectedSpawnPoint = SelectRandomSpawnpoint(m_aQRFSpawnPoints, selectedGroup.GetGroupType());
			if (!selectedSpawnPoint)
			{
				m_iSpawnTickets--;
				continue;
			}

			if (selectedGroup.GetNumberOfAvailableGroups() > 0)
				selectedGroup.SetNumberOfAvailableGroups(selectedGroup.GetNumberOfAvailableGroups() - 1);

			selectedSpawnPoint.SetObjectToSpawn(selectedGroup.GetGroupPrefabName());
			selectedSpawnPoint.SetEnableRepeatedSpawn(true);
			selectedSpawnPoint.SetIsTerminated(false);
			if (selectedSpawnPoint.GetNumberOfExistingWaypoints())
				selectedSpawnPoint.ClearWaypoints();

			selectedSpawnPoint.Init(m_AreaFramework, SCR_ScenarioFrameworkEActivationType.SAME_AS_PARENT);
			IEntity spawnedEntity = selectedSpawnPoint.GetSpawnedEntity();
			SCR_AIGroup aiGroup = GetAIGroup(spawnedEntity);
			if (aiGroup)
			{
				if (WatchAIGroup(aiGroup))
				{
					vector wpPosition;
					foreach (SCR_QRFWaypointConfig wp : m_aWPConfig)
					{
						if (wp.GetOrderType() != SCR_EQRFGroupOrderType.ANY && wp.GetOrderType() != selectedGroup.GetGroupType())
							continue;

						if (wp.GetDistanceOffsetToTargetLocation())
							wpPosition = SCR_Math3D.MoveTowards(m_vTargetPosition, selectedSpawnPoint.GetOwner().GetOrigin(), wp.GetDistanceOffsetToTargetLocation());
						else
							wpPosition = m_vTargetPosition;

						if (float.AlmostEqual(vector.DistanceXZ(wpPosition, selectedSpawnPoint.GetOwner().GetOrigin()), 0, 1))
							continue;

						wpPosition[1] = GetGame().GetWorld().GetSurfaceY(wpPosition[0], wpPosition[2]);
						AIWaypoint aiWP = selectedSpawnPoint.CreateWaypoint(wpPosition, wp.GetWaypointPrefabName());
						if (aiWP)
							aiGroup.AddWaypoint(aiWP);
					}
				}
			}
			else
			{
				if (Vehicle.Cast(spawnedEntity))
				{
					SCR_BaseCompartmentManagerComponent compartmentManager = SCR_BaseCompartmentManagerComponent.Cast(spawnedEntity.FindComponent(SCR_BaseCompartmentManagerComponent));
					if (compartmentManager)
					{
						compartmentManager.GetOnDoneSpawningDefaultOccupants().Insert(OnFinishedSpawningVehicleOccupants);
						array<ECompartmentType> compartmentTypes = {ECompartmentType.PILOT, ECompartmentType.TURRET};
						if (selectedGroup.GetGroupType() == SCR_EQRFGroupType.MOUNTED_INFANTRY)
							compartmentTypes.Insert(ECompartmentType.CARGO);

						m_aVehicleSpawnQueueConfig.Insert(new SCR_QRFVehicleSpawnConfig(compartmentManager, selectedGroup.GetGroupType(), m_vTargetPosition, selectedSpawnPoint));
						compartmentManager.SpawnDefaultOccupants(compartmentTypes);
					}
				}
			}
		}
		if (m_iNumberOfAvailableQRFWaves > 0)
			m_iNumberOfAvailableQRFWaves--;

		m_vTargetPosition = vector.Zero;
		
		foreach (SCR_ScenarioFrameworkActionBase actions : m_aActions)
		{
			actions.OnActivate(m_Entity);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_QRFGroupConfig SelectRandomGroup(int maxCost)
	{
		//first check if there even are groups that we can afford
		bool availableGroupAtFullPrice;
		array<SCR_QRFGroupConfig> verifiedList = {};
		foreach (SCR_QRFGroupConfig group : m_aGroupList)
		{
			if (!group)
				continue;

			if (group.GetNumberOfAvailableGroups() == -1 || group.GetNumberOfAvailableGroups() > 0)
			{
				if (group.GetSpawnCost() == maxCost)
				{
					if (!availableGroupAtFullPrice)
					{
						availableGroupAtFullPrice = true;
						verifiedList.Clear();
					}
					verifiedList.Insert(group);
				}
				else if (group.GetSpawnCost() < maxCost && !availableGroupAtFullPrice)
				{
					verifiedList.Insert(group);
				}
			}
		}

		if (availableGroupAtFullPrice)
			return verifiedList.GetRandomElement();

		SCR_QRFGroupConfig returnedGroup;
		if (!verifiedList.IsEmpty())
		{
			int i, numOfTries, count = verifiedList.Count();
			while (numOfTries < 10)
			{
				i = Math.RandomInt(0, count);
				if (!returnedGroup || returnedGroup && returnedGroup.GetSpawnCost() < verifiedList[i].GetSpawnCost())
					returnedGroup = verifiedList[i];
					
				numOfTries++;
			}
		}

		return returnedGroup;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_ScenarioFrameworkQRFSlotAI SelectRandomSpawnpoint(array<SCR_ScenarioFrameworkQRFSlotAI> aListOfSpawnPoints, SCR_EQRFGroupType searchedType)
	{
		float maxSpawnDistance = GetMaxDistanceForUnitType(searchedType);
		array<SCR_ScenarioFrameworkQRFSlotAI> verifiedList = {};
		foreach (SCR_ScenarioFrameworkQRFSlotAI spawn : aListOfSpawnPoints)
		{
			if (!(spawn.GetGroupType() & searchedType))
				continue;

			if (!CheckSpawnPointSafeZones(spawn.GetOwner().GetOrigin(), spawn.GetSpawnSafeZones(), searchedType))
				continue;

			if (maxSpawnDistance > -1 && vector.Distance(m_vTargetPosition, spawn.GetOwner().GetOrigin()) > maxSpawnDistance)
				continue;

			verifiedList.Insert(spawn);
		}

		int index = -1;
		if (verifiedList.Count() > 0)
			return verifiedList.GetRandomElement();

		float distanceToTarget, cloasestPosDistance = float.MAX;
		foreach (int i, SCR_ScenarioFrameworkQRFSlotAI spawn : aListOfSpawnPoints) //int i = aListOfSpawnPoints.Count() - 1; i >= 0; i--)
		{
			if ( !(spawn.GetGroupType() & searchedType) )
				continue;

			if (!CheckSpawnPointSafeZones(spawn.GetOwner().GetOrigin(), spawn.GetSpawnSafeZones(), searchedType))
				continue;

			distanceToTarget = vector.Distance(m_vTargetPosition, spawn.GetOwner().GetOrigin());
			if (distanceToTarget < cloasestPosDistance)
			{
				cloasestPosDistance = distanceToTarget;
				index = i;
			}
		}

		if (index == -1)
			return null;

		return aListOfSpawnPoints[index];
	}

	//------------------------------------------------------------------------------------------------
	//! Returns true when there are no observers (players) within specified distance for given group type
	protected bool CheckSpawnPointSafeZones(vector spawnPointPosition, array<ref SCR_QRFSpawnSafeZone> spawnSafeZones, SCR_EQRFGroupType searchedType)
	{
		if (spawnSafeZones.IsEmpty())
			return true;

		array<vector> aObserversPositions = {};
		array<int> playerIds = {};
		PlayerManager playerManager = GetGame().GetPlayerManager();
		IEntity player;
		SCR_DamageManagerComponent damageManager;
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			player = playerManager.GetPlayerControlledEntity(playerId);
			if (!player)
				continue;

			damageManager = SCR_DamageManagerComponent.GetDamageManager(player);
			if (damageManager && damageManager.GetState() != EDamageState.DESTROYED)
				aObserversPositions.Insert(player.GetOrigin());
		}

		foreach (SCR_QRFSpawnSafeZone safeZone : spawnSafeZones)
		{
			if (safeZone.GetGroupType() != searchedType)
				continue;

			foreach (vector observerPos : aObserversPositions)
			{
				if (vector.Distance(observerPos, spawnPointPosition) < safeZone.GetMinDistanceToClosestObserver())
					return false;
			}
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetMaxDistanceForUnitType(SCR_EQRFGroupType unitType)
	{
		foreach (SCR_QRFTypeMaxDistance conf : m_aQRFMaxDistanceConfig)
		{
			if (conf.GetGroupType() == unitType)
				return conf.GetMaxSpawnDistance();
		}
		return -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Meant to be invked when vehicle finishes spawning its occupants so we can get the group of those occupants
	protected void OnFinishedSpawningVehicleOccupants(SCR_BaseCompartmentManagerComponent compartmentManager, array<IEntity> occupants, bool wasCanceled)
	{
		compartmentManager.GetOnDoneSpawningDefaultOccupants().Remove(OnFinishedSpawningVehicleOccupants);
		if (wasCanceled)
			return;

		if (occupants.IsEmpty())
			return;

		SCR_ChimeraCharacter occupant = SCR_ChimeraCharacter.Cast(occupants[0]);
		if (!occupant)
			return;

		AIControlComponent control = AIControlComponent.Cast(occupant.FindComponent(AIControlComponent));
		if (!control)
			return;

		AIAgent agent = control.GetControlAIAgent();
		if (!agent)
			return;

		SCR_AIGroup aiGroup = SCR_AIGroup.Cast(agent.GetParentGroup());
		if (!aiGroup)
			return;

		if (!WatchAIGroup(aiGroup))
			return;

		vector wpPosition;
		for (int i, count = m_aVehicleSpawnQueueConfig.Count(); i < count; i++)
		{
			if (m_aVehicleSpawnQueueConfig[i].m_VehicleCompartmentMGR != compartmentManager)
				continue;

			foreach (SCR_QRFWaypointConfig wp : m_aWPConfig)
			{
				if (wp.GetOrderType() != SCR_EQRFGroupOrderType.ANY && wp.GetOrderType() != m_aVehicleSpawnQueueConfig[i].m_eGroupType)
					continue;

				if (wp.GetDistanceOffsetToTargetLocation())
					wpPosition = SCR_Math3D.MoveTowards(m_aVehicleSpawnQueueConfig[i].m_vTargetPosition, occupant.GetOrigin(), wp.GetDistanceOffsetToTargetLocation());
				else
					wpPosition = m_aVehicleSpawnQueueConfig[i].m_vTargetPosition;

				if (float.AlmostEqual(vector.DistanceXZ(wpPosition, occupant.GetOrigin()), 0, 1))
					continue;

				AIWaypoint aiWP = m_aVehicleSpawnQueueConfig[i].m_Slot.CreateWaypoint(wpPosition, wp.GetWaypointPrefabName());
				if (aiWP)
					aiGroup.AddWaypoint(aiWP);
			}
			m_aVehicleSpawnQueueConfig.Remove(i);
			break;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Requests game mode to broadcast sound event on the position of provided entity
	protected void DoPlaySoundOnEntityPosition(IEntity entity, string soundFileName, string soundEventName)
	{
		if (!entity)
			return;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundFileName))
			return;

		if (SCR_StringHelper.IsEmptyOrWhiteSpace(soundEventName))
			return;

		SCR_ScenarioFrameworkSystem scenarioFrameworkSystem = SCR_ScenarioFrameworkSystem.GetInstance();
		if (!scenarioFrameworkSystem)
			return;

		SCR_GadgetManagerComponent gadgetManager = SCR_GadgetManagerComponent.Cast(entity.FindComponent(SCR_GadgetManagerComponent));
		if (!gadgetManager)
			return;

		if (!gadgetManager.GetGadgetByType(EGadgetType.RADIO))
			return;

		scenarioFrameworkSystem.PlaySoundOnEntityPosition(entity, soundFileName, soundEventName);
	}

	//------------------------------------------------------------------------------------------------
	override void Init(IEntity entity)
	{
		SCR_ScenarioFrameworkLayerBase thisLayer = SCR_ScenarioFrameworkLayerBase.Cast(entity.FindComponent(SCR_ScenarioFrameworkLayerBase));
		if (!thisLayer)
			return;

		m_AreaFramework = thisLayer.GetParentArea();
		if (!m_AreaFramework)
			return;

		super.Init(entity);
	}

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		array<SCR_ScenarioFrameworkLayerBase> childLayers = m_AreaFramework.GetChildrenEntities();
		if (childLayers.IsEmpty())
		{
			IEntity child = m_AreaFramework.GetOwner().GetChildren();
			SCR_ScenarioFrameworkLayerBase layer;
			while (child)
			{
				layer = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
				if (layer)
					childLayers.Insert(layer);

				child = child.GetSibling();
			}
			if (childLayers.IsEmpty())
				return;
		}

		m_iNumberOfSoldiersInTheArea = 0;
		foreach (SCR_ScenarioFrameworkLayerBase layer : childLayers)
		{
			if (!layer)
				continue;

			if (layer.GetName() == m_sQRFLayerName)
			{
				m_QRFLayer = layer;
				IEntity child = layer.GetOwner().GetChildren();
				SCR_ScenarioFrameworkQRFSlotAI qrfSlot;
				while (child)
				{
					qrfSlot = SCR_ScenarioFrameworkQRFSlotAI.Cast(child.FindComponent(SCR_ScenarioFrameworkQRFSlotAI));
					if (qrfSlot && !m_aQRFSpawnPoints.Contains(qrfSlot))
						m_aQRFSpawnPoints.Insert(qrfSlot);

					child = child.GetSibling();
				}
				continue;
			}
		}

		if (m_aQRFSpawnPoints && m_aQRFSpawnPoints.IsEmpty())
			Print("SCR_ScenarioFrameworkActionQRFDispacher.OnActivate: Layer " + m_AreaFramework.GetName() + " doesn't have any spawn points for its QRF dispatcher!", LogLevel.ERROR);

		CheckForAIToWatch(m_AreaFramework.GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! Recursive check for AI slot withing entity hierarchy
	protected void CheckForAIToWatch(IEntity entity)
	{
		IEntity child = entity.GetChildren();
		SCR_ScenarioFrameworkLayerBase layer;
		while (child)
		{
			layer = SCR_ScenarioFrameworkLayerBase.Cast(child.FindComponent(SCR_ScenarioFrameworkLayerBase));
			if (SCR_ScenarioFrameworkSlotAI.Cast(layer) || SCR_ScenarioFrameworkSlotTaskAI.Cast(layer))
				WatchAIGroup(layer.GetSpawnedEntities());
			else if (layer && layer.GetName() != m_sQRFLayerName)
				CheckForAIToWatch(child);

			child = child.GetSibling();
		}
	}

	//------------------------------------------------------------------------------------------------
	override array<ref SCR_ScenarioFrameworkActionBase> GetSubActions()
	{
		return m_aActions;
	}
}
