/*!
@ingroup Editor_Containers_Backend

Saved data for editable entity.
*/
/*
[Obsolete("Only used for backwards compatiblity GM saves. Will be removed entirely.")]
class SCR_EditableEntityStruct: JsonApiStruct
{
	//--- Constants
	protected static const int TARGET_NONE = -1;
	protected static const int TARGET_SLOT = -2;
	
	//--- Serialized (names shortened to save memory)
	protected ResourceName pf; //--- Prefab
	protected bool hy; //--- Was hierarchy changed by user
	protected EEditableEntityFlag ef; //--- EEditableEntityFlag
	protected int pi = -1; //--- Parent ID
	protected int ti = TARGET_NONE; //--- Target ID
	protected int tv = -1; //--- Target value
	protected float px; //--- Pos X
	protected float py; //--- Pos Y
	protected float pz; //--- Pos Y
	protected float qx; //--- Quaternion X
	protected float qy; //--- Quaternion Y
	protected float qz; //--- Quaternion Z
	protected float qw; //--- Quaternion W
	protected float sc; //--- Scale
	protected string au; //--- Author UID
	protected string ap; //--- Author PlatformID
	protected int as; //--- Author Platform (system)
	protected int ut; //--- Last Time of Update
	protected ref array<ref SCR_EditorAttributeStruct> at = {}; //--- Attributes
	
	//--- Non-serialized
	protected SCR_EditableEntityComponent m_Entity;
	protected SCR_EditableEntityComponent m_Target;
	protected static SCR_CompositionSlotManagerComponent m_SlotManager;
	protected static IEntity m_PlayerEntity;

	static void SerializeEntities(out notnull array<ref SCR_EditableEntityStruct> outEntries, SCR_EditorAttributeList attributeList, EEditableEntityFlag requiredFlags)
	{
		//--- Clear existing array
		outEntries.Clear();
		
		//--- Get root entities
		set<SCR_EditableEntityComponent> children;
		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (core)
		{
			children = new set<SCR_EditableEntityComponent>();
			core.GetAllEntities(children, true);
		}
		m_SlotManager = SCR_CompositionSlotManagerComponent.GetInstance();
		
		//--- Process root entities
		array<int> entriesWithTarget = {};
		foreach (SCR_EditableEntityComponent child: children)
		{
			SerializeEntity(child, -1, outEntries, attributeList, requiredFlags, entriesWithTarget, false);
		}

		//--- Link attached entities together (only after all of them were registered)
		int entriesCount = outEntries.Count();
		SCR_EditableEntityStruct entry;
		for (int i = 0, count = entriesWithTarget.Count(); i < count; i++)
		{
			entry = outEntries[entriesWithTarget[i]];
			for (int e = 0; e < entriesCount; e++)
			{
				if (entry.m_Target == outEntries[e].m_Entity)
				{
					entry.ti = e;
					break;
				}
			}
			if (entry.ti == TARGET_NONE)
				entry.m_Entity.Log("Error when serializing attach link!", true, LogLevel.WARNING);
		}
		m_SlotManager = null;
	}
	protected static void SerializeEntity(SCR_EditableEntityComponent entity, int parentID, out notnull array<ref SCR_EditableEntityStruct> outEntries, SCR_EditorAttributeList attributeList, EEditableEntityFlag requiredFlags, out array<int> entriesWithTarget, bool isParentDirty)
	{
		if (!entity || (requiredFlags != 0 && !entity.HasEntityFlag(requiredFlags)))
			return;
		
		SCR_EditableEntityComponent target;
		int targetValue = TARGET_NONE;
		EEditableEntitySaveFlag saveFlags = 0;
		if (!entity.Serialize(target, targetValue, saveFlags))
			return;
		
		//--- Only placeable entities can have dirty hierarchy, artifically created ones (e.g., custom layer) are exempted
		bool canBeDirty = entity.HasEntityFlag(EEditableEntityFlag.PLACEABLE);
		
		SCR_EditableEntityStruct entry = new SCR_EditableEntityStruct();
		entry.m_Entity = entity;
		entry.pi = parentID;
		parentID = outEntries.Insert(entry);
		
		entry.pf = entity.GetPrefab(true);
		
		if (saveFlags != 0)
			entry.pf += saveFlags.ToString();
		
		entry.sc = entity.GetOwner().GetScale();
		entry.hy = isParentDirty || entity.HasEntityFlag(EEditableEntityFlag.INDIVIDUAL_CHILDREN) || entity.HasEntityFlag(EEditableEntityFlag.DIRTY_HIERARCHY);
		entry.ef = entity.GetEntityFlags();
		
		vector transform[4];
		entity.GetOwner().GetWorldTransform(transform);
		entry.px = transform[3][0];
		entry.py = transform[3][1];
		entry.pz = transform[3][2];
		
		float quat[4];
		Math3D.MatrixToQuat(transform, quat);
		entry.qx = quat[0];
		entry.qy = quat[1];
		entry.qz = quat[2];
		entry.qw = quat[3];
		
		entry.tv = targetValue; //--- Assigned even when target is not, some entities may use it for other purposes (e.g., task faction)
		if (target)
		{
			entry.m_Target = target;
			entriesWithTarget.Insert(parentID);
		}
		else if (m_SlotManager)
		{
			SCR_EditableEntityUIInfo info = SCR_EditableEntityUIInfo.Cast(entity.GetInfo());
			if (info && info.GetSlotPrefab() && m_SlotManager.IsInSlot(entity.GetOwner()))
				entry.ti = TARGET_SLOT;
		}
		
		entry.as = entity.GetAuthorPlatform();
		entry.au = entity.GetAuthorUID();
		entry.ap = entity.GetAuthorPlatformID();
		entry.ut = entity.GetAuthorLastUpdated();
		
		SCR_EditorAttributeStruct.SerializeAttributes(entry.at, attributeList, entity);
		
		//--- Process children if the composition is dirty or artificially created (i.e., non-placeable)
		if (entry.hy || !canBeDirty)
		{
			for (int c = 0, count = entity.GetChildrenCount(true); c < count; c++)
			{
				SerializeEntity(entity.GetChild(c), parentID, outEntries, attributeList, requiredFlags, entriesWithTarget, entry.hy && canBeDirty);
			}
		}
	}

	static void DeserializeEntities(notnull array<ref SCR_EditableEntityStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		SCR_CompositionSlotManagerComponent slotManager = SCR_CompositionSlotManagerComponent.GetInstance();
		
		SCR_EditableEntityComponent parent;
		map<int, SCR_EditableEntityComponent> entities = new map<int, SCR_EditableEntityComponent>();
		array<int> entriesWithTarget = {};
		foreach (int id, SCR_EditableEntityStruct entry: entries)
		{
			//--- Extract optional params (encoded in prefab name, so they don't need a variable in every struct)
			ResourceName prefab = entry.pf;
			EEditableEntitySaveFlag saveFlags = 0;
			"{"; // fix indent
			int guidIndex = prefab.LastIndexOf("}") + 1;
			int prefabParamsCount = prefab.Length() - guidIndex;
			if (prefabParamsCount > 0)
			{
				string prefabParams = prefab.Substring(guidIndex, prefabParamsCount);
				saveFlags = prefabParams.ToInt();
				prefab = prefab.Substring(0, guidIndex);
			}	
			
			//--- Don't spawn SP player entity in MP
			if (Replication.IsRunning() && (saveFlags & EEditableEntitySaveFlag.PLAYER))
				continue;
			
			if (entry.pi != -1)
			{
				if (!entities.Find(entry.pi, parent))
				{
					Print(string.Format("SCR_EditableEntityStruct: Error when spawning entity @\"%1\", parent with ID %2 not found!", entry.pf, entry.pi), LogLevel.WARNING);
					continue;
				}
			}
			else
			{
				parent = null;
			}
			
			EntitySpawnParams spawnParams = new EntitySpawnParams();
			
			float quat[4];
			quat[0] = entry.qx;
			quat[1] = entry.qy;
			quat[2] = entry.qz;
			quat[3] = entry.qw;
			Math3D.QuatToMatrix(quat, spawnParams.Transform);
			spawnParams.Transform[3] = Vector(entry.px, entry.py, entry.pz);// + Vector(10, 0, 0); //--- DEBUG OFFSET
			spawnParams.TransformMode = ETransformMode.WORLD;
			
			SCR_EditorLinkComponent.IgnoreSpawning(SCR_Enum.HasFlag(entry.ef, EEditableEntityFlag.SPAWN_UNFINISHED));
			SCR_AIGroup.IgnoreSpawning(true);
			
			if (saveFlags & EEditableEntitySaveFlag.NOT_SPAWNED)
				SCR_EditorLinkComponent.IgnoreSpawning(true);
			
			IEntity rawEntity = GetGame().SpawnEntityPrefab(Resource.Load(prefab), GetGame().GetWorld(), spawnParams);
			entry.m_Entity = SCR_EditableEntityComponent.GetEditableEntity(rawEntity);
			if (entry.m_Entity)
			{
				entities.Insert(id, entry.m_Entity);
				
				rawEntity.SetScale(entry.sc);
				entry.m_Entity.EOnEditorSessionLoad(parent);
				entry.m_Entity.SetParentEntity(parent);
				if (entry.hy)
					entry.m_Entity.SetHierarchyAsDirty();

				entry.m_Entity.CopyEntityFlags(entry.ef);
				
				SCR_EditorAttributeStruct.DeserializeAttributes(entry.at, attributeList, entry.m_Entity);
				
				if (entry.ti != TARGET_NONE)
				{
					if (entry.ti == TARGET_SLOT)
						slotManager.SetOccupant(spawnParams.Transform[3], rawEntity);
					else
						entriesWithTarget.Insert(id);
				}
			
				if (!Replication.IsRunning() && (saveFlags & EEditableEntitySaveFlag.PLAYER))
				{
					m_PlayerEntity = rawEntity;
					RequestLocalPlayerSpawn(SCR_PlayerController.GetLocalPlayerId());
				}
			
				if (saveFlags & EEditableEntitySaveFlag.DESTROYED)
				{
					entry.m_Entity.Destroy();
				}
				
				SCR_EditableEntityAuthor author = new SCR_EditableEntityAuthor();
				author.Initialize(entry.au, entry.ap, entry.as, -1);
				entry.m_Entity.SetAuthor(author);
				entry.m_Entity.SetAuthorUpdatedTime(entry.ut);
				
				Print(string.Format("SCR_EditableEntityStruct: Entity @\"%1\" spawned at %2 as a child of %3", entry.pf, spawnParams.Transform, parent), LogLevel.VERBOSE);
			}
			else
			{
				SCR_EntityHelper.DeleteEntityAndChildren(rawEntity);
				Print(string.Format("SCR_EditableEntityStruct: Error when spawning entity @\"%1\" at %2, SCR_EditableEntityComponent not found!", entry.pf, spawnParams.Transform, parent), LogLevel.WARNING);
			}
		}
		
		//--- Once all entities were spawned, call Deserialize() on them
		foreach (int id, SCR_EditableEntityStruct entry: entries)
		{
			if (entriesWithTarget.Contains(id))
				entry.m_Entity.Deserialize(entries[entry.ti].m_Entity, entry.tv); //--- Link attached entities together (e.g., crew in vehicles)
			else
				entry.m_Entity.Deserialize(null, entry.tv);
		}
		
		SCR_EditorLinkComponent.IgnoreSpawning(false);
		SCR_AIGroup.IgnoreSpawning(false);
	}
	protected static void RequestLocalPlayerSpawn(int playerId)
	{
		SCR_RespawnComponent respawnComponent = SCR_RespawnComponent.Cast(GetGame().GetPlayerManager().GetPlayerRespawnComponent(playerId));
		if (respawnComponent)
		{
			//--- Assign player comtrol using respawn system
			SCR_PossessSpawnData spawnData = SCR_PossessSpawnData.FromEntity(m_PlayerEntity);
			respawnComponent.RequestSpawn(spawnData);
		}
		else
		{
			//--- Player not initialized yet, do so once that happens
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			if (gameMode)
				gameMode.GetOnPlayerRegistered().Insert(RequestLocalPlayerSpawn);
		}
	}

	static void ClearEntities(notnull array<ref SCR_EditableEntityStruct> entries)
	{
		for (int i = 0, count = entries.Count(); i < count; i++)
		{
			if (entries[i].m_Entity)
				entries[i].m_Entity.Delete();
		}
	}

	static void LogEntities(notnull array<ref SCR_EditableEntityStruct> entries, SCR_EditorAttributeList attributeList = null)
	{
		Print("  SCR_EditableEntityStruct: " + entries.Count());
		//string textDefault = "    %1: %2 | file: %3 | pos: %4 | ang: %5 | scl: %6 | drt: %9 | tgt: N/A (#%8)";
		//string textTarget = "    %1: %2 | file: %3 | pos: %4 | ang: %5 | scl: %6 | drt: %9 | tgt: %7 (#%8)";
		float quat[4];
		vector angles;
		Resource resource;
		string resourceName;
		foreach (int id, SCR_EditableEntityStruct entry: entries)
		{
			quat[0] = entry.qx;
			quat[1] = entry.qy;
			quat[2] = entry.qz;
			quat[3] = entry.qw;
			angles = Math3D.QuatToAngles(quat);
			
			//--- Get flags
			ResourceName prefab = entry.pf;
			EEditableEntitySaveFlag saveFlags = 0;
			"{"; // fix indent
			int guidIndex = prefab.LastIndexOf("}") + 1;
			int prefabParamsCount = prefab.Length() - guidIndex;
			if (prefabParamsCount > 0)
			{
				string prefabParams = prefab.Substring(guidIndex, prefabParamsCount);
				saveFlags = prefabParams.ToInt();
			}	
			
			resource = Resource.Load(entry.pf);
			resourceName = resource.GetResource().GetResourceName();
			
			string result = "    " + id + ": " + entry.pi;
			result += " | " + FilePath.StripPath(resourceName);
			result += " | pos: " + Vector(entry.px, entry.py, entry.pz);
			result += " | ang: " + angles;
			result += " | scl: " + entry.sc;
			result += " | flg: " + SCR_Enum.FlagsToString(EEditableEntitySaveFlag, saveFlags);
			result += " | drt: " + entry.hy;
			
			if (entry.ti == TARGET_NONE)
				result += " | tgt: N/A";
			else
				result += " | tgt: " + entry.ti;
			
			result += " (#" + entry.tv + ")";
			
			Print("" + result);
			
			SCR_EditorAttributeStruct.LogAttributes(entry.at, attributeList, "    ");
		}
	}

	void SCR_EditableEntityStruct()
	{
		RegV("pf");
		RegV("ef");
		RegV("hy");
		RegV("pi");
		RegV("ti");
		RegV("tv");
		RegV("px");
		RegV("py");
		RegV("pz");
		RegV("qx");
		RegV("qy");
		RegV("qz");
		RegV("qw");
		RegV("sc");
		RegV("at");
		RegV("au");
		RegV("as");
		RegV("ap");
		RegV("ut");
	}
};
*/