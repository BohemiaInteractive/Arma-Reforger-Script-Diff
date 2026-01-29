#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
[EntityEditorProps(category: "GameScripted/Destruction", description: "Manager for destructibles", color: "0 0 255 255")]
class SCR_MPDestructionManagerClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_DestructibleIdentificator
{
	protected EntityID m_EntityID = EntityID.INVALID;
	protected RplId m_RplId = RplId.Invalid();
	protected int m_iIndex = -1;
	
	//------------------------------------------------------------------------------------------------
	void SetIndex(int index)
	{
		m_iIndex = index;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRplId(RplId rplId)
	{
		m_RplId = rplId;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetEntityID(EntityID entityID)
	{
		m_EntityID = entityID;
	}
	
	//------------------------------------------------------------------------------------------------
	RplId GetRplId(out int index)
	{
		index = m_iIndex;
		return m_RplId;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetEntity()
	{
		if (m_EntityID != EntityID.INVALID)
		{
			BaseWorld world = GetGame().GetWorld();
			if (world)
				return world.FindEntityByID(m_EntityID);
		}
		
		return null;
	}
	
	//################################################################################################
	//! Codec methods
	//------------------------------------------------------------------------------------------------
	static void Encode(SSnapSerializerBase snapshot, ScriptCtx ctx, ScriptBitSerializer packet) 
	{
		snapshot.Serialize(packet, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Decode(ScriptBitSerializer packet, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		return snapshot.Serialize(packet, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool SnapCompare(SSnapSerializerBase lhs, SSnapSerializerBase rhs, ScriptCtx ctx) 
	{	
		return lhs.CompareSnapshots(rhs, 16);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool PropCompare(SCR_DestructibleIdentificator prop, SSnapSerializerBase snapshot, ScriptCtx ctx) 
	{
		return snapshot.Compare(prop.m_EntityID, 8)
			&& snapshot.Compare(prop.m_RplId, 4)
			&& snapshot.Compare(prop.m_iIndex, 4);
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Extract(SCR_DestructibleIdentificator prop, ScriptCtx ctx, SSnapSerializerBase snapshot) 
	{
		snapshot.SerializeBytes(prop.m_EntityID, 8);
		snapshot.SerializeBytes(prop.m_RplId, 4);
		snapshot.SerializeBytes(prop.m_iIndex, 4);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	static bool Inject(SSnapSerializerBase snapshot, ScriptCtx ctx, SCR_DestructibleIdentificator prop) 
	{
		snapshot.SerializeBytes(prop.m_EntityID, 8);
		snapshot.SerializeBytes(prop.m_RplId, 4);
		snapshot.SerializeBytes(prop.m_iIndex, 4);
		
		return true;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_MPDestructionManager : GenericEntity
{
#ifdef ENABLE_BASE_DESTRUCTION
	[Attribute("", UIWidgets.Auto, desc: "AudioSourceConfiguration shared by all MPDDestruction sounds")]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;
	
	static const string ENTITY_SIZE_SIGNAL_NAME = "EntitySize";
	static const string PHASES_TO_DESTROYED_PHASE_SIGNAL_NAME = "PhasesToDestroyed";
	static const string COLLISIONDV_SIGNAL_NAME = "CollisionDV";
	static const ResourceName DESTRUCTION_MANAGER_PREFAB = "{9BB369F2803C6F71}Prefabs/MP/MPDestructionManager.et";
	static private SCR_MPDestructionManager s_Instance = null;
	static bool s_bInitialized = false;
	
	protected ref array<EntityID> m_ChangedDestructibles = new array<EntityID>();
	protected ref array<EntityID> m_DeletedDestructibles = new array<EntityID>();
	protected ref map<RplId, ref array<SCR_DestructionDamageManagerComponent>> m_mDynamicallySpawnedDestructibles = new map<RplId, ref array<SCR_DestructionDamageManagerComponent>>();
	protected ref map<RplId, ref array<int>> m_mChangedDynamicallySpawnedDestructibles = new map<RplId, ref array<int>>();
	protected ref map<RplId, ref array<int>> m_mDeletedDynamicallySpawnedDestructibles = new map<RplId, ref array<int>>();
	protected ref array<SCR_DestructionBaseHandler> m_aDestroyInFrame = {};
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	static bool IsInitialized()
	{
		return s_bInitialized;
	}
	
	//------------------------------------------------------------------------------------------------
	static notnull SCR_MPDestructionManager InitializeDestructionManager()
	{
		if (!s_Instance)
		{
			Resource resource = Resource.Load(DESTRUCTION_MANAGER_PREFAB);
			if (!resource)
			{
				Print("SCR_MPDestructionManager::CreateInstance(): Failed to create instance! Destruction of objects will not be synched!", LogLevel.ERROR);
				return SCR_MPDestructionManager.Cast(GetGame().SpawnEntity(SCR_MPDestructionManager));
			}
			
			GetGame().SpawnEntityPrefab(resource);
		}
		
		s_bInitialized = true;
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the instance of the destruction manager
	static SCR_MPDestructionManager GetInstance()
	{
		return s_Instance;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Checks if this entity is locally owned
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DestroyDelayed()
	{
		for (int i = m_aDestroyInFrame.Count() - 1; i >= 0; i--)
		{
			SCR_DestructionBaseHandler handler = m_aDestroyInFrame[i];
			if (handler)
				handler.HandleDestruction();

			m_aDestroyInFrame.Remove(i);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DestroyInFrame(SCR_DestructionBaseHandler handler)
	{
		m_aDestroyInFrame.Insert(handler);
		SetEventMask(EntityEvent.FRAME);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when Item is initialized from replication stream. Carries the data from Master.
	override bool RplLoad(ScriptBitReader reader)
	{
		SCR_DestructionDamageManagerComponent.SetReadingInit(true);
		
		// First read IDs for deleted destructibles on the server and delete them locally
		int numDeleted;
		reader.ReadInt(numDeleted); // Read num deleted
		for (int i = 0; i < numDeleted; i++)
		{
			EntityID entID;
			reader.ReadEntityId(entID); // Read entity ID of each deleted destructible
			
			IEntity entity = GetWorld().FindEntityByID(entID);
			if (!entity)
				continue;
			
			SCR_DestructionDamageManagerComponent destructible = SCR_DestructionDamageManagerComponent.Cast(entity.FindComponent(SCR_DestructionDamageManagerComponent));
			if (!destructible)
				continue;
			
			destructible.DeleteDestructibleDelayed();
		}
		
		// Now pass data from server destructibles to local ones
		int numDestructibles;
		reader.ReadInt(numDestructibles); // Read num destructibles sent
		for (int i = 0; i < numDestructibles; i++)
		{
			EntityID entID;
			reader.ReadEntityId(entID); // Read entity ID of each destructible to be read
			
			IEntity entity = GetWorld().FindEntityByID(entID);
			if (!entity)
				continue;
			
			SCR_DestructionDamageManagerComponent destructible = SCR_DestructionDamageManagerComponent.Cast(entity.FindComponent(SCR_DestructionDamageManagerComponent));
			if (!destructible)
				continue;
			
			destructible.NetReadInit(reader);
		}
		
		SCR_DestructionDamageManagerComponent.SetReadingInit(false);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Called when Item is getting replicated from Master to Slave connection. The data will be delivered to Slave using RplLoad method.
	override bool RplSave(ScriptBitWriter writer)
	{
		// First send all the destructibles that have been deleted
		int numDeleted = m_DeletedDestructibles.Count();
		writer.WriteInt(numDeleted); // Write num deleted
		for (int i = 0; i < numDeleted; i++)
		{
			writer.WriteEntityId(m_DeletedDestructibles[i]); // Write entity ID of each deleted destructible
		}
		
		// Send destructibles' data
		int numChanged = m_ChangedDestructibles.Count();
		writer.WriteInt(numChanged); // Write num destructibles
		for (int i = 0; i < numChanged; i++)
		{
			EntityID entID = m_ChangedDestructibles[i];
			IEntity entity = GetWorld().FindEntityByID(entID);
			if (!entity)
				continue;
			
			SCR_DestructionDamageManagerComponent destructible = SCR_DestructionDamageManagerComponent.Cast(entity.FindComponent(SCR_DestructionDamageManagerComponent));
			if (!destructible)
				continue;
			
			writer.WriteEntityId(entID); // Write entity ID of each destructible to be sent
			destructible.NetWriteInit(writer); // Tell destructible to write its data
		}
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_AudioSourceConfiguration GetAudioSourceConfiguration()
	{
		return m_AudioSourceConfiguration;
	}
			
	//------------------------------------------------------------------------------------------------
	protected override void EOnFrame(IEntity owner, float timeSlice)
	{
		ClearEventMask(EntityEvent.FRAME);
		DestroyDelayed();
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_RplComponent = RplComponent.Cast(FindComponent(RplComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	int RegisterDynamicallySpawnedDestructible(notnull SCR_DestructionDamageManagerComponent destructible, RplId rplId)
	{
		array<SCR_DestructionDamageManagerComponent> destructibles = m_mDynamicallySpawnedDestructibles.Get(rplId);
		if (!destructibles)
		{
			destructibles = new array<SCR_DestructionDamageManagerComponent>();
			m_mDynamicallySpawnedDestructibles.Insert(rplId, destructibles);
		}
		
		return destructibles.Insert(destructible);
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DestructionDamageManagerComponent FindDynamicallySpawnedDestructibleByIndex(RplId rplId, int index)
	{
		array<SCR_DestructionDamageManagerComponent> destructibles = m_mDynamicallySpawnedDestructibles.Get(rplId);
		if (!destructibles)
			return null;
		
		if (index >= 0 && index < destructibles.Count())
			return destructibles[index];
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	int FindDynamicallySpawnedDestructibleIndex(RplId rplId, SCR_DestructionDamageManagerComponent destructible)
	{
		array<SCR_DestructionDamageManagerComponent> destructibles = m_mDynamicallySpawnedDestructibles.Get(rplId);
		if (!destructibles)
			return -1;
		
		return destructibles.Find(destructible);
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterDeletedDynamicallySpawnedDestructible(notnull SCR_DestructionDamageManagerComponent destructible, RplId rplId)
	{
		int index = FindDynamicallySpawnedDestructibleIndex(rplId, destructible);
		if (index != -1)
		{
			array<int> deletedDestructibles = m_mDeletedDynamicallySpawnedDestructibles.Get(rplId);
			if (!deletedDestructibles)
			{
				deletedDestructibles = new array<int>();
				m_mDeletedDynamicallySpawnedDestructibles.Insert(rplId, deletedDestructibles);
			}
			
			deletedDestructibles.Insert(index);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterChangedDynamicallySpawnedDestructible(notnull SCR_DestructionDamageManagerComponent destructible, RplId rplId)
	{
		int index = FindDynamicallySpawnedDestructibleIndex(rplId, destructible);
		if (index != -1)
		{
			array<int> changedDestructibles = m_mChangedDynamicallySpawnedDestructibles.Get(rplId);
			if (!changedDestructibles)
			{
				changedDestructibles = new array<int>();
				m_mChangedDynamicallySpawnedDestructibles.Insert(rplId, changedDestructibles);
			}
			
			changedDestructibles.Insert(index);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MPDestructionManager(IEntitySource src, IEntity parent)
	{
		s_Instance = this;
		
		SetEventMask(EntityEvent.INIT);
		
		#ifdef ENABLE_DIAG 
			DiagMenu.RegisterBool(SCR_DebugMenuID.DEBUGUI_SOUNDS_MPDESTRUCTION_SHOW_IMPULSEVALUES, "", "Show MPD Impulse Values", "Sounds"); 
		#endif
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MPDestructionManager()
	{
		#ifdef ENABLE_DIAG 
			DiagMenu.Unregister(SCR_DebugMenuID.DEBUGUI_SOUNDS_MPDESTRUCTION_SHOW_IMPULSEVALUES); 
		#endif
	}
#endif
};
