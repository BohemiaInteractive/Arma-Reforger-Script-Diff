[EntityEditorProps(category: "GameScripted/GameMode", description: "Spawn point entity", visible: false)]
class SCR_SpawnPointClass : SCR_PositionClass
{
	// [Attribute()]
	// protected ref SCR_UIInfo m_Info;

	// SCR_UIInfo GetInfo()
	// {
	// 	return m_Info;
	// }
}

void SpawnPointDelegateMethod(SCR_SpawnPoint spawnPoint);
typedef func SpawnPointDelegateMethod;
typedef ScriptInvokerBase<SpawnPointDelegateMethod> SpawnPointInvoker;

void SpawnPointFinalizeSpawnInvoker(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity);
typedef func SpawnPointFinalizeSpawnInvoker;
typedef ScriptInvokerBase<SpawnPointFinalizeSpawnInvoker> SCR_SpawnPointFinalizeSpawn_Invoker;

void SpawnPointNameChangedInvoker(RplId id, string name);
typedef func SpawnPointNameChangedInvoker;
typedef ScriptInvokerBase<SpawnPointNameChangedInvoker> SCR_SpawnPointNameChanged_Invoker;

//------------------------------------------------------------------------------------------------
//! Spawn point entity defines positions on which players can possibly spawn.
class SCR_SpawnPoint : SCR_Position
{
	protected RplComponent m_RplComponent;
	protected float m_fSpawnRadius = 10;
	protected float m_fPlayerCylinderRadius = 0.5;
	protected float m_fPlayerCylinderHeight = 2;
	protected float m_fColliderWidth = m_fPlayerCylinderRadius * 1.73205;
	protected float m_fColliderHeight = m_fPlayerCylinderRadius * 2;
	protected vector m_vCylinderVectorOffset = Vector(0, m_fPlayerCylinderHeight * 0.5, 0);
	
	[Attribute("0", UIWidgets.EditBox, "If bigger than 0, defines radius in which spawn will be randomized")]
	float m_fRandomSpawnRadius;

	[Attribute("", UIWidgets.EditBox, "Determines which faction can spawn on this spawn point."), RplProp(onRplName: "OnSetFactionKey")]
	protected string m_sFaction;

	[Attribute("0")]
	protected bool m_bShowInDeployMapOnly;

	[Attribute("0", desc: "Use custom timer when deploying on this spawn point. Takes the remaining respawn time from SCR_TimedSpawnPointComponent")]
	protected bool m_bTimedSpawnPoint;

	[RplProp(), SortAttribute(), Attribute("0", desc: "Priority of spawn point to be selected as default in spawn menu")]
	protected int m_iPriority;

	protected SCR_UIInfo m_LinkedInfo;
	protected SCR_FactionAffiliationComponent m_FactionAffiliationComponent;

	[Attribute()]
	protected ref SCR_UIInfo m_Info;

	[Attribute("0", desc: "Allow usage of Spawn Positions in range")]
	protected bool m_bUseNearbySpawnPositions;

	[Attribute("100", desc: "Spawn position detection radius, in metres")]
	protected float m_fSpawnPositionUsageRange;

	[Attribute("0", desc: "Additional respawn time (in seconds) when spawning on this spawn point"), RplProp()]
	protected float m_fRespawnTime;
	
	[Attribute("0", desc: "Spawn at a random place on the map")]
	protected bool m_bRandomizedSpawn;
	
	[Attribute("500", desc: "Radius around enemy bases for the random spawn to avoid")]
	protected float m_fRandomSpawnSafeRange;
	
	[Attribute("20", desc: "Random position attempt count")]
	protected int m_iRandomSpawnMaxAttempts;

	// List of all spawn points
	private static ref array<SCR_SpawnPoint> m_aSpawnPoints = new array<SCR_SpawnPoint>();

	static ref ScriptInvoker Event_OnSpawnPointCountChanged = new ScriptInvoker();
	static ref ScriptInvoker Event_SpawnPointFactionAssigned = new ScriptInvoker();
	static ref SpawnPointInvoker Event_SpawnPointAdded = new SpawnPointInvoker();
	static ref SpawnPointInvoker Event_SpawnPointRemoved = new SpawnPointInvoker();
	static ref SCR_SpawnPointFinalizeSpawn_Invoker s_OnSpawnPointFinalizeSpawn;

	[RplProp()]
	protected string m_sSpawnPointName;
	
	static ref SCR_SpawnPointNameChanged_Invoker OnSpawnPointNameChanged = new SCR_SpawnPointNameChanged_Invoker();
	
	// spawn point will work as a spawn point group if it has any SCR_Position as its children
	protected ref array<SCR_Position> m_aChildren = {};

	/*!
		Authority:
			Set of all pending players that have a reservation for this spawn point.
	*/
	protected ref set<int> m_ReservationLocks = new set<int>();
	
	protected ref ScriptInvokerBool  m_OnSetSpawnPointEnabled;
	
	[Attribute("1"), RplProp(onRplName: "OnSetEnabled")]
	protected bool m_bSpawnPointEnabled;

	//------------------------------------------------------------------------------------------------
	bool IsSpawnPointVisibleForPlayer(int pid)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpawnPointEnabled()
	{
		return m_bSpawnPointEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnPointEnabled_S(bool enabled)
	{
		if (enabled == m_bSpawnPointEnabled)
			return;
		
		m_bSpawnPointEnabled = enabled;
		OnSetEnabled();
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnSetEnabled()
	{
		if (m_OnSetSpawnPointEnabled)
			m_OnSetSpawnPointEnabled.Invoke(m_bSpawnPointEnabled);
	}
	
	//------------------------------------------------------------------------------------------------
	ScriptInvokerBool GetOnSpawnPointEnabled()
	{
		if (!m_OnSetSpawnPointEnabled)
			m_OnSetSpawnPointEnabled = new ScriptInvokerBool();
		
		return m_OnSetSpawnPointEnabled;
	}
	
	//------------------------------------------------------------------------------------------------
	static SCR_SpawnPointFinalizeSpawn_Invoker GetOnSpawnPointFinalizeSpawn()
	{
		if (!s_OnSpawnPointFinalizeSpawn)
			s_OnSpawnPointFinalizeSpawn = new SCR_SpawnPointFinalizeSpawn_Invoker();
		
		return s_OnSpawnPointFinalizeSpawn;
	}

	//------------------------------------------------------------------------------------------------
	float GetRespawnTime()
	{
		return m_fRespawnTime;
	}

	//------------------------------------------------------------------------------------------------
	void SetRespawnTime(float time)
	{
		m_fRespawnTime = time;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	int GetPriority()
	{
		return m_iPriority;
	}

	//------------------------------------------------------------------------------------------------
	void SetPriority(int priority)
	{
		if (priority == m_iPriority)
			return;

		m_iPriority = priority;
		Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Returns whether this point can be reserved for provided player.
			Derived logic can e.g. check amount of pending locks versus available compartments
			for vehicle spawn points and similar.
			\param playerId PlayerId of player who wants to reserve this point
			\param[out] result Reason why respawn is disabled. Note that if returns true the reason will always be OK
			\return Return true if reservation can proceed, false otherwise.
	*/
	bool CanReserveFor_S(int playerId, out SCR_ESpawnResult result = SCR_ESpawnResult.SPAWN_NOT_ALLOWED)
	{
		if (IsSpawnPointEnabled())
		{
			return true;
		}
		else 
		{
			result = SCR_ESpawnResult.NOT_ALLOWED_SPAWNPOINT_DISABLED;
			return false;
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Returns whether this point is currently reserved for provided player.
			\param playerId PlayerId of player to check reservation for.
			\return Return true if reservation can proceed, false otherwise.
	*/
	bool IsReservedFor_S(int playerId)
	{
		return m_ReservationLocks.Contains(playerId);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Returns whether this point is currently reserved for provided player.
			\param playerId PlayerId of player to check reservation for.
			\return Return true if reservation can proceed, false otherwise.
	*/
	bool ReserveFor_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ReserveFor_S(playerId: %2, pointId: %3)", Type().ToString(), playerId, GetRplId());
		#endif

		if (!m_ReservationLocks.Insert(playerId))
		{
			Debug.Error(string.Format("SCR_SpawnPoint reservation for playerId: %1 failed!", playerId));
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Clears reservation for provided player.
			\param playerId PlayerId of player to check reservation for.
	*/
	void ClearReservationFor_S(int playerId)
	{
		#ifdef _ENABLE_RESPAWN_LOGS
		PrintFormat("%1::ClearReservationFor_S(playerId: %2, pointId: %3)", Type().ToString(), playerId, GetRplId());
		#endif

		int index = m_ReservationLocks.Find(playerId);
		if (index != -1)
		{
			m_ReservationLocks.Remove(index);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Radius in which players can be spawned on empty position.
	*/
	float GetSpawnRadius()
	{
		return m_fSpawnRadius;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetSpawnRadius(float radius)
	{
		m_fSpawnRadius = radius;
	}
	
	//------------------------------------------------------------------------------------------------
	static void ShowSpawnPointDescriptors(bool show, Faction faction)
	{
		if (!m_aSpawnPoints)
			return;

		string factionKey = string.Empty;
		if (faction)
			factionKey = faction.GetFactionKey();

		foreach (SCR_SpawnPoint spawnPoint : m_aSpawnPoints)
		{
			if (!spawnPoint)
				continue;

			auto mapDescriptor = SCR_MapDescriptorComponent.Cast(spawnPoint.FindComponent(SCR_MapDescriptorComponent));
			if (!mapDescriptor)
				continue;

			bool visible = show && !factionKey.IsEmpty() && spawnPoint.GetFactionKey() == factionKey;
			if (mapDescriptor.Item())
				mapDescriptor.Item().SetVisible(visible);
		}
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Returns RplId of this spawn point.
	*/
	RplId GetRplId()
	{
		if (!m_RplComponent)
			return RplId.Invalid();

		return m_RplComponent.Id();
	}

	//------------------------------------------------------------------------------------------------
	static SCR_SpawnPoint GetSpawnPointByRplId(RplId id)
	{
		if (!id.IsValid())
			return null;

		Managed instance = Replication.FindItem(id);
		if (!instance)
			return null;

		RplComponent rplComponent = RplComponent.Cast(instance);
		if (rplComponent)
			return SCR_SpawnPoint.Cast(rplComponent.GetEntity());

		return SCR_SpawnPoint.Cast(instance);
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetEmptyPositionAndRotationInRange(out vector pos, out vector rot)
	{
		SCR_SpawnPositionComponentManager spawnPosManagerComponent = SCR_SpawnPositionComponentManager.GetInstance();
		if (!spawnPosManagerComponent)
			return false;

		array<SCR_SpawnPositionComponent> positions = {};
		int count = spawnPosManagerComponent.GetSpawnPositionsInRange(GetOrigin(), m_fSpawnPositionUsageRange, positions);
		if (count < 0)
			return false;

		SCR_SpawnPositionComponent position;
		
		while (!positions.IsEmpty())
		{
			position = positions.GetRandomElement();

			if (position.IsFree())
			{
				pos = position.GetOwner().GetOrigin();
				rot = position.GetOwner().GetAngles();
				return true;
			}
			else
			{
				positions.RemoveItem(position);
			}
		}

		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	void SetUseNearbySpawnPositions(bool use)
	{
		m_bUseNearbySpawnPositions = use;
	}

	//------------------------------------------------------------------------------------------------
	void GetPositionAndRotation(out vector pos, out vector rot)
	{
		if (m_bRandomizedSpawn && GetRandomPositionAndRotation(pos,rot))
				return;

		if (m_bUseNearbySpawnPositions)
		{
			if (GetEmptyPositionAndRotationInRange(pos, rot))
				return;
		}

		vector rand = Vector(0, 0, 0);
		if (m_fRandomSpawnRadius > 0)
			rand = Vector(Math.RandomFloat01() * m_fRandomSpawnRadius, 0, Math.RandomFloat01() * m_fRandomSpawnRadius);
		
		if (m_aChildren.Count() > 1)
		{
			int id = m_aChildren.GetRandomIndex();
			SCR_WorldTools.FindEmptyTerrainPosition(pos, m_aChildren[id].GetOrigin() + rand, GetSpawnRadius());
			rot = m_aChildren[id].GetAngles();
		}
		else
		{
			SCR_WorldTools.FindEmptyTerrainPosition(pos, GetOrigin() + rand, GetSpawnRadius());
			rot = GetAngles();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Return spawn point or null if out of bounds
	static SCR_SpawnPoint GetSpawnPointByIndex(int spawnPointIndex)
	{
		if (spawnPointIndex >= 0 && spawnPointIndex < m_aSpawnPoints.Count())
			return m_aSpawnPoints[spawnPointIndex];

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Return spawn point index or -1 if not existant
	static int GetSpawnPointIndex(SCR_SpawnPoint spawnPoint)
	{
		return m_aSpawnPoints.Find(spawnPoint);
	}

	//------------------------------------------------------------------------------------------------
	bool IsSpawnPointActive()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
	\return Number of spawn points in the world
	*/
	static int CountSpawnPoints()
	{
		return m_aSpawnPoints.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool GetVisibleInDeployMapOnly()
	{
		return m_bShowInDeployMapOnly;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetVisibleInDeployMapOnly(bool visible)
	{
		m_bShowInDeployMapOnly = visible;
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyFactionChange(FactionAffiliationComponent owner, Faction previousFaction, Faction newFaction)
	{
		SetFaction(newFaction);
	}

	//------------------------------------------------------------------------------------------------
	void SetFaction(Faction faction)
	{
		if (!faction)
			SetFactionKey(string.Empty);
		else
			SetFactionKey(faction.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	void SetFactionKey(string factionKey)
	{
		if (factionKey == m_sFaction)
			return;

		m_sFaction = factionKey;
		OnSetFactionKey();
		Replication.BumpMe();
	}
	protected void OnSetFactionKey()
	{
		Event_SpawnPointFactionAssigned.Invoke(this);
	}

	//------------------------------------------------------------------------------------------------
	void SetSpawnPositionRange(float range)
	{
		m_fSpawnPositionUsageRange = range;
	}

	//------------------------------------------------------------------------------------------------
	float GetSpawnPositionRange()
	{
		return m_fSpawnPositionUsageRange;
	}

	//------------------------------------------------------------------------------------------------
	array<SCR_Position> GetChildSpawnPoints()
	{
		return m_aChildren;
	}

	//------------------------------------------------------------------------------------------------
	//! \return string name of the faction of the spawn point.
	string GetFactionKey()
	{
		return m_sFaction;
	}

	//------------------------------------------------------------------------------------------------
	//! \param character determines which entity is being checked.
	//! \param range determines the max range.
	//! \return whether is in range or not.
	private bool GetIsInRange(ChimeraCharacter character, float range)
	{
		return character && vector.DistanceSq(character.GetOrigin(), GetOrigin()) < range*range;
	}

	//------------------------------------------------------------------------------------------------
	//! \return an array of all spawn point entities.
	static array<SCR_SpawnPoint> GetSpawnPoints()
	{
		return m_aSpawnPoints;
	}

	//------------------------------------------------------------------------------------------------
	//! \return random spawn point without limitations.
	static SCR_SpawnPoint GetRandomSpawnPointDeathmatch()
	{
		if (!m_aSpawnPoints || m_aSpawnPoints.IsEmpty())
			return null;

		return m_aSpawnPoints.GetRandomElement();
	}

	//------------------------------------------------------------------------------------------------
	//! \return random spawn point for a character.
	static SCR_SpawnPoint GetRandomSpawnPoint(SCR_ChimeraCharacter character)
	{
		if (!character)
			return null;

		return GetRandomSpawnPointForFaction(character.GetFactionKey());
	}

	//------------------------------------------------------------------------------------------------
	//! \return spawn points valid for for local player.
	static array<SCR_SpawnPoint> GetSpawnPointsForPlayer(SCR_ChimeraCharacter character)
	{
		string factionKey = string.Empty;
		if (character)
			factionKey = character.GetFactionKey();

		return GetSpawnPointsForFaction(factionKey);
	}

	//------------------------------------------------------------------------------------------------
	//! Get spawn points valid for given faction
	//! \param factionKey Valid faction key
	static array<SCR_SpawnPoint> GetSpawnPointsForFaction(string factionKey)
	{
		array<SCR_SpawnPoint> factionSpawnPoints = new array<SCR_SpawnPoint>();
		if (factionKey.IsEmpty())
			return factionSpawnPoints;

		array<SCR_SpawnPoint> spawnPoints = GetSpawnPoints();
		foreach (SCR_SpawnPoint spawnPoint : spawnPoints)
		{
			if (spawnPoint && spawnPoint.GetFactionKey() == factionKey)
				factionSpawnPoints.Insert(spawnPoint);
		}
		return factionSpawnPoints;
	}
	//------------------------------------------------------------------------------------------------
	//! Get count of spawn points belonging to given faction.
	//! \param factionKey Valid faction key
	//! \return Number of spawn points
	static int GetSpawnPointCountForFaction(string factionKey)
	{
		if (factionKey.IsEmpty())
			return 0;

		int count = 0;
		foreach (SCR_SpawnPoint spawnPoint : GetSpawnPoints())
		{
			if (!spawnPoint) continue;
			if (spawnPoint.GetFactionKey() == factionKey)
				count++;
		}
		return count;
	}

	//------------------------------------------------------------------------------------------------
	//! \return random spawn point valid for a specific faction.
	static SCR_SpawnPoint GetRandomSpawnPointForFaction(string factionKey)
	{
		array<SCR_SpawnPoint> spawnPoints = GetSpawnPointsForFaction(factionKey);
		if (!spawnPoints.IsEmpty())
			return spawnPoints.GetRandomElement();

		return null;
	}

	//------------------------------------------------------------------------------------------------
	// todo(koudelkaluk): get back to this

	//------------------------------------------------------------------------------------------------
	// SCR_UIInfo GetInfo()
	// {
	// 	if (m_LinkedInfo)
	// 		return m_LinkedInfo;
	// 	else
	// 	    return GetInfoFromPrefab();
	// }

	// protected SCR_UIInfo GetInfoFromPrefab()
	// {

		// SCR_SpawnPointClass prefabData = SCR_SpawnPointClass.Cast(GetPrefabData());
		// if (!prefabData)
		// 	return null;

		// return prefabData.GetInfo();
	// }

	//------------------------------------------------------------------------------------------------
	SCR_UIInfo GetInfo()
	{
		if (m_LinkedInfo)
			return m_LinkedInfo;
		else
			return m_Info;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetInfo(SCR_UIInfo info)
	{
		m_Info = info;
	}

	//------------------------------------------------------------------------------------------------
	string GetSpawnPointName()
	{
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sSpawnPointName) && GetInfo())
			return GetInfo().GetName();
		else
			return m_sSpawnPointName;

		return string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSpawnPointName(string name)
	{
		m_sSpawnPointName = name;
		Replication.BumpMe();
		OnSpawnPointNameChanged.Invoke(GetRplId(), m_sSpawnPointName);
	}

	//------------------------------------------------------------------------------------------------
	bool IsTimed()
	{
		return m_bTimedSpawnPoint;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetIsTimed(bool isTimed)
	{
		m_bTimedSpawnPoint = isTimed;
	}

	//------------------------------------------------------------------------------------------------
	void LinkInfo(SCR_UIInfo info)
	{
		m_LinkedInfo = info;
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override void SetColorAndText()
	{
		m_sText = m_sFaction;

		// Fetch faction data
		FactionManager factionManager = GetGame().GetFactionManager();
		if (factionManager)
		{
			Faction faction = factionManager.GetFactionByKey(m_sFaction);
			if (faction)
			{
				m_iColor = faction.GetFactionColor().PackToInt();
			}
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	/*
		Authority:
			During the spawn process, prior to passing the ownership to the remote project spawned entity
			can be prepared (e.g. moved to position, seated in vehicle, items spawned in inventory).

			This method is the place to do so, but at this point the spawning process can still fail and
			terminate if preparation fails (returns false). Player is then informed about spawn.

			Following a successful preparation is CanSpawnFinalize_S, and SpawnFinalize_S after which
			the process is sucessfully ended.

			\param requestComponent Player request component
			\param data Data received for this request
			\param entity Spawned entity (to prepare)

			\return True if successful (move to finalizing the request), false (to terminate the process).
	*/
	bool PrepareSpawnedEntity_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (!IsSpawnPointEnabled())
			return false;
		
		// WS target position, pitch yaw roll angles in degrees
		vector position, rotation;
		GetPositionAndRotation(position, rotation);

		// apply transformation
		entity.SetOrigin(position);
		entity.SetAngles(rotation);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			The PrepareEntity_S step might start doing an operation which is not performed immediately, for such cases
			we can await the finalization by returning 'false', until spawned entity is in desired state.
			(E.g. upon seating a character we can await until character is properly seated)

			Following a successful preparation is CanSpawnFinalize_S, and SpawnFinalize_S after which
			the process is sucessfully ended.

			\param requestComponent Player request component
			\param data Data received for this request
			\param entity Spawned entity (to await)

			\return True if ready, false (to await a frame).
	*/
	bool CanFinalizeSpawn_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		return IsSpawnPointEnabled();
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Authority:
			Callback for when finalization is done, e.g. the ownership is passed to the client and
			the spawn process is deemed complete.
	*/
	void OnFinalizeSpawnDone_S(SCR_SpawnRequestComponent requestComponent, SCR_SpawnData data, IEntity entity)
	{
		if (s_OnSpawnPointFinalizeSpawn)
			s_OnSpawnPointFinalizeSpawn.Invoke(requestComponent, data, entity);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		if (!GetGame().GetWorldEntity())
			return;

		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));

		IEntity child = GetChildren();
		while (child)
		{
			if (SCR_Position.Cast(child))
				m_aChildren.Insert(SCR_Position.Cast(child));
			child = child.GetSibling();
		}

		InitFactionAffiliation(owner);

		// Add to list of all points
		m_aSpawnPoints.Insert(this);
		Event_OnSpawnPointCountChanged.Invoke(m_sFaction);
		Event_SpawnPointAdded.Invoke(this);

		ClearFlags(EntityFlags.ACTIVE);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void InitFactionAffiliation(IEntity owner)
	{
		m_FactionAffiliationComponent = SCR_FactionAffiliationComponent.Cast(owner.FindComponent(SCR_FactionAffiliationComponent));
		if (m_FactionAffiliationComponent)
		{
			m_FactionAffiliationComponent.GetOnFactionChanged().Insert(ApplyFactionChange);
			Faction faction = m_FactionAffiliationComponent.GetAffiliatedFaction();
			if (faction)
				m_sFaction = faction.GetFactionKey();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteString(m_sSpawnPointName);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override protected bool RplLoad(ScriptBitReader reader)
	{
		// Raise callback about adding of this point
		Event_SpawnPointAdded.Invoke(this);

		// Update faction related stats
		OnSetFactionKey();
		Event_OnSpawnPointCountChanged.Invoke(m_sFaction);
		
		reader.ReadString(m_sSpawnPointName);
		
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_SpawnPoint(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, true);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_SpawnPoint()
	{
		// Remove from list of all points
		if (m_aSpawnPoints)
		{
			m_aSpawnPoints.RemoveItem(this);
		}

		//~ Raise events
		Event_OnSpawnPointCountChanged.Invoke(m_sFaction);
		Event_SpawnPointRemoved.Invoke(this);

		// Remove callbacks
		if (m_FactionAffiliationComponent)
		{
			m_FactionAffiliationComponent.GetOnFactionChanged().Remove(ApplyFactionChange);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetRandomPositionAndRotation(out vector vOutPosition, out vector vOutRotation)
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world) 
			return false;

		int zMin, zMax, zStep;
		string spawnPointFaction;
		float posX, posZ, traceCoef, distance;
		vector vRandomSpawnCenter, vTestPosition;
		
		TraceFlags flags = TraceFlags.ENTS | TraceFlags.OCEAN;
		TraceParam trace = new TraceParam();
		trace.Flags = flags | TraceFlags.WORLD;
		vector traceOffset = Vector(0, 10, 0);
		bool bPointIsSafe = true;
		
		for (int i; i < m_iRandomSpawnMaxAttempts; i++)
		{	
			vRandomSpawnCenter = GetRandomWorldPosition();
			bPointIsSafe = true;
			
			if (m_fRandomSpawnSafeRange > 0)
			{
				foreach (SCR_SpawnPoint spawnPoint : m_aSpawnPoints) 
				{
					if (!spawnPoint)
						continue;
					
					spawnPointFaction = spawnPoint.GetFactionKey();
					if (spawnPointFaction != m_sFaction)
					{
						distance = vector.DistanceXZ(vRandomSpawnCenter, spawnPoint.GetOrigin());
	    				if (distance <= m_fRandomSpawnSafeRange)
						{
							bPointIsSafe = false;
							break;
						}
					}
				}
			}
			
			if (!bPointIsSafe)
				continue;
			
			for (int r; r <= m_fRandomSpawnRadius; r++)
			{	
				for (int x = -r; x <= r; x++)
				{
					posX = m_fColliderWidth * x;
					posZ = m_fColliderHeight * (x - SCR_Math.fmod(x, 1)) * 0.5;
					zMin = Math.Max(-r - x, -r);
					zMax = Math.Min(r - x, r);
					if (Math.AbsInt(x) == r)
						zStep = 1;
					else
						zStep = zMax - zMin;

					for (int z = zMin; z <= zMax; z += zStep)
					{
						vTestPosition = vRandomSpawnCenter + Vector(posX, 0, posZ + m_fColliderHeight * z);
						trace.Start = vTestPosition;
						trace.End = vTestPosition - traceOffset;
						traceCoef = world.TraceMove(trace, null);
						vTestPosition[1] = Math.Max(trace.Start[1] - traceCoef * traceOffset[1] + 0.01, world.GetSurfaceY(vTestPosition[0], vOutPosition[2]));
		
						if (!SCR_WorldTools.TraceCylinder(vTestPosition + m_vCylinderVectorOffset, m_fPlayerCylinderRadius, m_fPlayerCylinderHeight, flags, world))
							continue;
							
						vOutRotation = GetAngles();
						vOutPosition = vTestPosition;
						return true;
					}
				}
			}
		}
		
		return false;
	}

	//------------------------------------------------------------------------------------------------
	vector GetRandomWorldPosition()
	{
		BaseWorld world = GetGame().GetWorld();
		if (!world) 
			return GetOrigin();
		
		vector mins, maxs;
		world.GetBoundBox(mins, maxs);
		vector vRandomizedValue = Vector(Math.RandomFloat(mins[0], maxs[0]), 0, Math.RandomFloat(mins[2], maxs[2]));
		vRandomizedValue[1] = world.GetSurfaceY(vRandomizedValue[0], vRandomizedValue[2]);
	    return vRandomizedValue;
	}

}
