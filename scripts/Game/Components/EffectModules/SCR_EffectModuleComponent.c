[ComponentEditorProps(category: "GameScripted/Editor/EffectsModule", description: "Holds and executes various effects such as artillery and smoke the editor can place")]
class SCR_EffectsModuleComponentClass : ScriptComponentClass
{
};

void EffectsModuleOnChangedMethod(SCR_EffectsModuleComponent effectModule, SCR_EffectsModule effectConfig);
typedef func EffectsModuleOnChangedMethod;
typedef ScriptInvokerBase<EffectsModuleOnChangedMethod> EffectsModuleOnChanged;

class SCR_EffectsModuleComponent : ScriptComponent
{
	[Attribute("1", desc: "If true will execute as soon as it is placed. If false will only execute manually")]
	protected bool m_bExecuteOnInit;

	[Attribute(desc: "Holds the effect module config with all the required variables and logic to execute")]
	protected ref SCR_EffectsModule m_EffectConfig;
	
	protected bool m_bDoneExecuting;
	
	protected ref EffectsModuleOnChanged m_OnDeleted = new EffectsModuleOnChanged();
	protected ref EffectsModuleOnChanged m_OnEditorOnRemovedFromParent= new EffectsModuleOnChanged();
	protected ref EffectsModuleOnChanged m_OnTransformChanged = new EffectsModuleOnChanged();
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Radius of Effect Module
	*/
	SCR_EffectsModule GetEffectsModuleConfig()
	{
		return m_EffectConfig;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Radius of Effect Module
	*/
	bool EditorInstantDelete()
	{
		return !m_EffectConfig || m_EffectConfig.GetType() != EEffectsModuleType.PARTICLE;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get on transfrom changed event
	*/
	EffectsModuleOnChanged GetOnTransformChanged()
	{
		return m_OnTransformChanged;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get on delete event
	*/
	EffectsModuleOnChanged GetOnDelete()
	{
		return m_OnDeleted;
	}	
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Get on delete event
	*/
	EffectsModuleOnChanged GetOnEditorOnRemovedFromParent()
	{
		return m_OnEditorOnRemovedFromParent;
	}
	
	//======================================== TRANSFORM ========================================\\
	void OnTransformChanged()
	{
		if (!m_EffectConfig)
			return;
		
		//~ Send out on transform changed event
		m_OnTransformChanged.Invoke(this, m_EffectConfig);

		GenericEntity child = GenericEntity.Cast(GetOwner().GetChildren());
		SCR_EffectsModuleChildComponent childComponent;

		while (child)
		{	
			if (!child.FindComponent(SCR_EffectsModuleChildComponent))
			{
				child.OnTransformReset();
			}
			else
			{
				if (m_EffectConfig.m_bSnapToTerrain)
				{
					child.SetAngles(vector.Up);
					vector transform[4];
					Math3D.MatrixIdentity3(transform);

					vector snapPosition;
					vector normal;
					vector pos = child.GetOrigin() + "0 3 0";

					TraceParam trace = new TraceParam();
					{
						trace.Start = pos;

						pos[1] = GetOwner().GetWorld().GetSurfaceY(pos[0], pos[2]);
						pos[1] = SCR_TerrainHelper.GetTerrainY(pos, GetOwner().GetWorld(), true);
						trace.End = pos;
				
						trace.Exclude = child;
						trace.TargetLayers = EPhysicsLayerDefs.FireGeometry;
						trace.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
					}

					SCR_TerrainHelper.SnapToGeometry(snapPosition, pos, null, GetOwner().GetWorld(), traceParam: trace, surfaceNormal: normal);
					if (normal != vector.Zero)
						Math3D.MatrixFromUpVec(normal, transform);

					vector ownerTransform[3];
					GetOwner().GetTransform(ownerTransform);
					Math3D.MatrixInvMultiply3(ownerTransform, transform, transform);

					transform[3] = snapPosition;
					child.SetTransform(transform);
				}
			}
			
			//~ Get next child
			child = GenericEntity.Cast(child.GetSibling());
		}
	}
	
	//======================================== CHILDREN ========================================\\
	bool HasEffectsModuleChildren()
	{
		GenericEntity child = GenericEntity.Cast(GetOwner().GetChildren());

		while (child)
		{	
			if (!child.FindComponent(SCR_EffectsModuleChildComponent))
			{
				child = GenericEntity.Cast(child.GetSibling());
				continue;
			}
				
			return true;
		}
		
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetEffectsModuleChildren(notnull out array<SCR_EffectsModuleChildComponent> effectModuleChildren)
	{
		effectModuleChildren.Clear();
		
		GenericEntity child = GenericEntity.Cast(GetOwner().GetChildren());
		SCR_EffectsModuleChildComponent effectModuleChild;
		
		while (child)
		{	
			effectModuleChild = SCR_EffectsModuleChildComponent.Cast(child.FindComponent(SCR_EffectsModuleChildComponent));
			if (effectModuleChild)
				effectModuleChildren.Insert(effectModuleChild);
			
			child = GenericEntity.Cast(child.GetSibling());
		}
		
		return effectModuleChildren.Count();
	}

	//======================================== SPAWNING ========================================\\
	//------------------------------------------------------------------------------------------------
	void SpawnEffectEntity()
	{		
		if (!m_EffectConfig)
			return;
		
		//~ Create spawn params
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		vector targetPosition;
		m_EffectConfig.SetSpawnParams(spawnParams, targetPosition);
		
		if (!spawnParams)
		{
			Print("SCR_EffectsModuleComponent.SpawnEffectEntity: Failed to create spawn params. Entity defined in Effect Config will not be spawned.", LogLevel.WARNING);
			return;
		}
		
		//~ Spawn entity
		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(Resource.Load(m_EffectConfig.GetModuleEntityPrefab()), GetOwner().GetWorld(), spawnParams);		
		if (!spawnedEntity)
		{
			Print("'SCR_EffectsModuleComponent' failed to spawn entity", LogLevel.ERROR);
			return;
		}

		if (spawnParams.Parent && m_EffectConfig.GetType() != EEffectsModuleType.PROJECTILE)
		{
			RplComponent spawnedRplComp = SCR_EntityHelper.GetEntityRplComponent(spawnedEntity);
			RplComponent parentRplComp = SCR_EntityHelper.GetEntityRplComponent(spawnParams.Parent);
			if (spawnedRplComp && parentRplComp)
			{
				RplNode spawnedNode = spawnedRplComp.GetNode();
				RplNode parentNode = parentRplComp.GetNode();
				if (spawnedNode && parentNode && spawnedNode.GetParent() != parentNode)
					spawnedNode.SetParent(parentNode);
			}
		}

		SCR_EffectsModuleChildComponent effectModuleChild = SCR_EffectsModuleChildComponent.Cast(spawnedEntity.FindComponent(SCR_EffectsModuleChildComponent));
		if (!effectModuleChild)
		{
			Debug.Error2("SCR_EffectsModuleComponent", "Spawned effect module entity does not have 'SCR_EffectsModuleChildComponent'!");
			delete spawnedEntity;
			return;
		}
		
		//~ Execute Post spawn logic after entity is spawned.
		PostEntitySpawned(effectModuleChild, targetPosition);
	}
	
	//~ Called a frame after entity Is Spawned (Server Only)
	protected void PostEntitySpawned(notnull SCR_EffectsModuleChildComponent spawnedEntity, vector targetPosition)
	{		
		//~ Get rpl comp
		RplComponent rplComp = RplComponent.Cast(spawnedEntity.GetOwner().FindComponent(RplComponent));
		if (!rplComp)
		{
			Print("'SCR_EffectsModuleComponent' spawned entity has no 'RplComponent'!", LogLevel.ERROR);
			delete spawnedEntity.GetOwner();
			return;
		}
		
		spawnedEntity.InitChildServer(this, m_EffectConfig);
		m_EffectConfig.OnEntitySpawned(spawnedEntity, targetPosition);
		
		//~ On Spawn broadcast and logic
		Rpc(OnEntitySpawnedBroadcast, rplComp.Id(), targetPosition);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnEntitySpawnedBroadcast(RplId id, vector targetPosition)
	{
		if (!m_EffectConfig)
			return;

		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(id));
		if (!rplComp)
			return;
		
		IEntity spawnedEntity = rplComp.GetEntity();
		if (!spawnedEntity)
			return;
		
		SCR_EffectsModuleChildComponent effectModuleChild = SCR_EffectsModuleChildComponent.Cast(spawnedEntity.FindComponent(SCR_EffectsModuleChildComponent));
		if (!effectModuleChild)
			return;
		
		effectModuleChild.InitChild(this, GetEffectsModuleConfig());
		m_EffectConfig.OnEntitySpawnedBroadcast(effectModuleChild, targetPosition);
	}
	
	//======================================== DONE EXECUTING ========================================\\
	void DoneExecutingModule()
	{
		m_bDoneExecuting = true;
		OnEffectsModuleChildDeleted();
	}
	
	//------------------------------------------------------------------------------------------------
	void OnEffectsModuleChildDeleted()
	{
		//~ Return if not done executing or if still has effect module children
		if (!m_bDoneExecuting || HasEffectsModuleChildren())
			return;
		
		//~ Cancel module
		CancelModuleServer();
		
		IEntity owner = GetOwner();
		if(owner != null && !owner.IsDeleted())
		{
			SCR_EditableEffectsModuleComponent editableEffectsModule = SCR_EditableEffectsModuleComponent.Cast(owner.FindComponent(SCR_EditableEffectsModuleComponent));
			if(editableEffectsModule != null)
			{
				editableEffectsModule.Delete();
			}
		}
		if (owner && !owner.IsDeleted())
			delete owner;
	}
	
	//======================================== CANCEL ========================================\\
	void CancelModuleServer()
	{
		if (!m_EffectConfig)
			return;
		
		m_EffectConfig.CancelModule();
		
		m_EffectConfig.CancelModuleBroadcast();
		Rpc(CancelModuleBroadcast);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void CancelModuleBroadcast()
	{
		if (!m_EffectConfig)
			return;
		
		m_EffectConfig.CancelModuleBroadcast();
	}

	//======================================== EFFECT ========================================\\
	//---------------------------------------- Pause ----------------------------------------\\
	bool CanPause()
	{
		return m_EffectConfig.CanPause();
	}

	//------------------------------------------------------------------------------------------------
	bool IsPaused()
	{
		return m_EffectConfig.IsPaused();
	}

	//------------------------------------------------------------------------------------------------
	void SetPausedServer(bool paused)
	{
		if (!CanPause() || IsPaused() == paused)
			return;

		SetPausedBroadcast(paused);
		Rpc(SetPausedBroadcast, paused);
	}

	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetPausedBroadcast(bool paused)
	{
		m_EffectConfig.SetPaused(paused);
	}

	//---------------------------------------- Looping ----------------------------------------\\
	bool CanLoop()
	{
		return m_EffectConfig.CanLoop();
	}

	//------------------------------------------------------------------------------------------------
	bool IsLooping()
	{
		return m_EffectConfig.IsLooping();
	}

	//------------------------------------------------------------------------------------------------
	void SetLoopingServer(bool loop)
	{
		if (!CanLoop() || IsLooping() == loop)
			return;

		SetLoopingBroadcast(loop);
		Rpc(SetLoopingBroadcast, loop);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void SetLoopingBroadcast(bool loop)
	{
		m_EffectConfig.SetLooping(loop);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		//~ Init on server and client
		m_EffectConfig.Init(GetOwner(), this);

		//~ Execution is Server only
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode || !gameMode.IsMaster())
			return;

		if (!m_bExecuteOnInit)
			return;

		m_EffectConfig.OnModulePreExecute();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		if (!m_EffectConfig)
		{
			Print("'SCR_EffectsModuleComponent' has no effect config assigned!", LogLevel.ERROR);
			return;
		}
		else if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_EffectConfig.GetModuleEntityPrefab()))
		{
			Print("'SCR_EffectsModuleComponent' effect config is invalid as no prefab assigned!", LogLevel.ERROR);
			return;
		}
		else if (m_EffectConfig.GetType() == EEffectsModuleType.NONE)
		{
			Print("'SCR_EffectsModuleComponent' effect config is invalid as type is NONE!", LogLevel.ERROR);
			return;
		}
		else if (!m_EffectConfig.GetEffectsModuleZoneData())
		{
			Print("'SCR_EffectsModuleComponent' effect config has no Zone Data assigned!", LogLevel.ERROR);
			return;
		}
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------
	void EditorOnRemovedFromParent()
	{
		/*array<SCR_EffectsModuleChildComponent> effectModuleChildren = {};
		GetEffectsModuleChildren(effectModuleChildren);
		
		foreach (SCR_EffectsModuleChildComponent child : effectModuleChildren)
		{
			child.EditorOnParentRemoved(this, GetEffectsModuleConfig());
		}*/
		
		m_OnEditorOnRemovedFromParent.Invoke(this, m_EffectConfig);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		m_bDoneExecuting = false;
		
		//~ If editable Effect module then ignore as OnOwnerDeleted will already be executed
		if (owner.FindComponent(SCR_EditableEffectsModuleComponent))
		{
			return;
		}
		
		//~ Execution is Server only
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (gameMode && gameMode.IsMaster())
			CancelModuleServer();
		
		m_OnDeleted.Invoke(this, GetEffectsModuleConfig());
	
	}

	//======================================== RPL ========================================\\
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!m_EffectConfig)
			return false;

		writer.WriteBool(IsPaused());
		writer.WriteBool(IsLooping());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		if (!m_EffectConfig)
			return false;

		bool isPaused, isLooping;

		reader.ReadBool(isPaused);
		reader.ReadBool(isLooping);

		SetPausedBroadcast(isPaused);
		SetLoopingBroadcast(isLooping);

		return true;
	}
};

enum EEffectsModuleType
{
	NONE 			= 0,
	PROJECTILE 		= 10, ///< Projectiles that fly in such as Mortars
	PARTICLE 		= 20, ///< Particles that are directly spawned
	MODULE_ENTITY 	= 30, ///< Entities that are placed such as Mines
};
