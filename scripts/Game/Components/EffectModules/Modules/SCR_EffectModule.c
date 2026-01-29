[BaseContainerProps(configRoot: true)]
class SCR_EffectsModule
{	
	[Attribute(desc: "Prefab spawned when Module is executed", category: "General", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "et")]
	protected ResourceName m_sModuleEntityPrefab;
	
	[Attribute(desc: "This holds the position data allowing things like owner (simply spawned in center of owner), Radius, rectangles", category: "Zone Data")]
	protected ref SCR_BaseEffectsModulePositionData m_ModuleZoneData;
	
	[Attribute(desc: "Type of module, PROJECTILE will spawn a projectile that flies towards the place location. PARTICLE will instantly execute on spawn eg smoke", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EEffectsModuleType), category: "General")]
	protected EEffectsModuleType m_eEffectsModuleType;
	
	[Attribute("0 0 0", desc: "PROJECTILE uses this value to spawn where as other types will either ignore this value (if m_bSnapToTerrain is true) or only use the Y value", uiwidget: UIWidgets.Coords, params: "inf inf 0 purpose=coords space=entity", category: "General")]
	protected vector m_vLocalSpawnOffset;
	
	[Attribute("0 0 0", desc: "PROJECTILE uses this value to aim to hit the target correctly using local space. Do not fill in the Y value", uiwidget: UIWidgets.Coords, params: "inf inf 0 purpose=coords space=entity", category: "General")]
	protected vector m_vLocalTargetOffset;
	
	[Attribute("1", desc: "Delay between when start is called and when the module will actually start executing. In seconds", params: "0 inf 0", category: "General")]
	protected float m_fStartDelay;
	
	[Attribute(desc: "If true it will ignore the Y coord and snap to the terrain. Ignored if Type PROJECTILE", category: "General")]
	bool m_bSnapToTerrain;
	
	//~ If currently Executing
	protected bool m_bExecuting;
	
	//~ The system is activly checking if it should delete itself when the module is done
	//~ For PARTICLE it will check if all particles are done executing as it subscribes to the particle state change event
	//~ For PROJECTILE if all of them are spawned it will run an update and check if the entity has any children. If none it will delete itself
	//~ For MODULE_ENTITY It subscribes to the OnDelete SCR_EffectsModuleChildComponent ScriptInvoker and checks on every child deleted if there are any children left
	protected bool m_bCheckForDelete;
	
	//~ States
	protected bool m_bIsPaused;
	
	//~ Refs
	IEntity m_Owner;
	SCR_EffectsModuleComponent m_EffectsModuleParent;
	
	//~ Time in seconds to check if all children are gone and delete itself
	protected const float CHECK_FOR_DELETE_DELAY = 0.5;
	
	//======================================== GENERAL GETTERS ========================================\\
	//------------------------------------------------------------------------------------------------
	/*!
	Get Module prefab to spawn
	\return Module prefab
	*/
	ResourceName GetModuleEntityPrefab()
	{
		return m_sModuleEntityPrefab;
	}
	
	SCR_BaseEffectsModulePositionData GetEffectsModuleZoneData()
	{
		return m_ModuleZoneData;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Module local spawn position vars
	\return Local spawn postion
	*/
	vector GetLocalSpawnOffset()
	{
		return m_vLocalSpawnOffset;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Module Type. Types dictate how the entity is spawned and what happens after spawning
	\return Module type
	*/
	EEffectsModuleType GetType()
	{
		return m_eEffectsModuleType;
	} 
	
	//======================================== INIT ========================================\\
	//------------------------------------------------------------------------------------------------
	/*!
	Init the Module. Called by SCR_EffectsModuleComponent both Server and Client
	\param owner Owner of the SCR_EffectsModuleComponent this Effect Module is attached to
	\param effectModuleParent SCR_EffectsModuleComponent of the owner this Effect Module is attached to
	*/
	void Init(notnull IEntity owner, notnull SCR_EffectsModuleComponent effectModuleParent)
	{
		m_Owner = owner;
		m_EffectsModuleParent = effectModuleParent;
		
		if (!m_ModuleZoneData)
			Debug.Error2("SCR_EffectsModule", "Has no m_ModuleZoneData!");
	}
	
	//======================================== EXECUTING ========================================\\
	//------------------------------------------------------------------------------------------------
	/*!
	Start the execution of the Effect module (Server Only)
	Will execute after a delay if any is set
	*/
	sealed void OnModulePreExecute()
	{		
		if (m_bExecuting)
			return;
		
		m_bExecuting = true;
		
		if (m_fStartDelay <= 0)
			OnModuleExecute();
		else
			GetGame().GetCallqueue().CallLater(OnModuleExecute, m_fStartDelay * 1000);
	}
	
	//------------------------------------------------------------------------------------------------
	//~ The actual execution of the Module logic (Server Only)
	protected void OnModuleExecute()
	{
		m_EffectsModuleParent.SpawnEffectEntity();
		
		//~ Only spawn one so check if can delete
		m_EffectsModuleParent.DoneExecutingModule();
	}
	
	//======================================== SPAWNING ========================================\\
	//------------------------------------------------------------------------------------------------
	/*!
	Set the spawn params for effect module (Server Only)
	\param[out] spawnParams Spawn params
	*/
	void SetSpawnParams(notnull out EntitySpawnParams spawnParams, out vector targetPosition)
	{
		if (!m_Owner || m_Owner.IsDeleted())
		{
			Print("SCR_EffectsModule.SetSpawnParams: Effect module owner no longer exists. Spawning of " + m_sModuleEntityPrefab + " aborted.", LogLevel.WARNING);
			spawnParams = null;
			return;
		}

		spawnParams.TransformMode = ETransformMode.LOCAL;
		spawnParams.Parent = m_Owner;

		targetPosition = m_ModuleZoneData.GetNewPosition(this);

		vector spawnTransform[4];
		Math3D.MatrixIdentity4(spawnTransform);

		//~ Is spawned at a specific height
		if (GetType() == EEffectsModuleType.PROJECTILE)
		{
			spawnTransform[3] = GetLocalSpawnOffset();
		}
		//~ If not projectile use target position instead
		else
		{
			//~ Snap to terrain
			if (m_bSnapToTerrain)
			{
				vector snapPosition;
				vector normal;

				SCR_TerrainHelper.SnapToGeometry(snapPosition, m_Owner.CoordToParent(targetPosition), null, m_Owner.GetWorld(), surfaceNormal: normal);
				if (normal != vector.Zero)
					Math3D.MatrixFromUpVec(normal, spawnTransform);

				vector ownerTransform[3];
				m_Owner.GetTransform(ownerTransform);
				Math3D.MatrixInvMultiply3(ownerTransform, spawnTransform, spawnTransform);

				spawnTransform[3] = m_Owner.CoordToLocal(snapPosition);	
			}
			else
			{
				spawnTransform[3] = targetPosition;
				spawnTransform[3][1] = GetLocalSpawnOffset()[1];
			}
		}

		//~ Set transform data
		spawnParams.Transform = spawnTransform;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called by SCR_EffectsModuleComponent when the Module prefab is spawned (Server Only)
	*/
	void OnEntitySpawned(notnull SCR_EffectsModuleChildComponent spawnedChild, vector targetPosition)
	{
		IEntity spawnedEntity = spawnedChild.GetOwner();
		
		//~ Projectiles
		if (m_eEffectsModuleType == EEffectsModuleType.PROJECTILE)
		{			
			ProjectileMoveComponent moveComponent = ProjectileMoveComponent.Cast(spawnedEntity.FindComponent(ProjectileMoveComponent));
			if (!moveComponent)
			{
				Debug.Error2("SCR_EffectsModule", "Module is set to type 'PROJECTILE' but spawned prefab has no 'ProjectileMoveComponent'!");	
				return;
			}
			
			vector transform[4];
			spawnedEntity.GetTransform(transform);
			
			//float time;
			vector targetPositionWorld = m_EffectsModuleParent.GetOwner().CoordToParent(targetPosition);
			targetPositionWorld[1] = targetPositionWorld[1];// + BallisticTable.GetHeightFromProjectile(vector.Distance(transform[3], targetPositionWorld), time, spawnedEntity);
			SCR_Math3D.LookAt(transform[3], targetPositionWorld, vector.Up, transform);
			spawnedEntity.SetTransform(transform);
			
			//~ Get launch Direction
			vector launchDirection = vector.Direction(transform[3], targetPositionWorld);
			launchDirection.Normalize();
			
			//~ Launch projectile
			moveComponent.Launch(launchDirection, vector.Zero, 1, spawnedEntity, null, m_EffectsModuleParent.GetOwner(), null, null);				
		}
		//~ Entity (Needs SCR_EffectsModuleChildComponent)
		else if (m_eEffectsModuleType == EEffectsModuleType.MODULE_ENTITY)
		{
			//~ Mine logics
			SCR_PressureTriggerComponent pressureTriggerComponent = SCR_PressureTriggerComponent.Cast(spawnedEntity.FindComponent(SCR_PressureTriggerComponent));
			if (pressureTriggerComponent)
			{
				pressureTriggerComponent.ActivateTrigger();
				if (pressureTriggerComponent.GetArmingTime() > 0)
					pressureTriggerComponent.ResetTimeout();
			}
		}
		OnEntitySpawnedBroadcast(spawnedChild, targetPosition);
	}	
	
	//------------------------------------------------------------------------------------------------
	/*!
	Called by SCR_EffectsModuleComponent and OnEntitySpawned() when the Module prefab is spawned. On Server and Client. Has one frame delay before called
	*/
	void OnEntitySpawnedBroadcast(notnull SCR_EffectsModuleChildComponent spawnedChild, vector targetPosition);

	//======================================== FINISHED AND CANCELED ========================================\\
	//------------------------------------------------------------------------------------------------
	/*!
	Cancel executing of the effect module (Server Only)
	*/
	void CancelModule();
	
	//------------------------------------------------------------------------------------------------
	/*!
	Cancel executing of the effect module Server and Client
	*/
	void CancelModuleBroadcast();
	
	//======================================== STATE GETTERS AND SETTERS ========================================\\
	//------------------------------------------------------------------------------------------------
	/*!
	Get Can pause the effect module 
	\return Can pause 
	*/
	bool CanPause()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get If effect module is paused
	\return Is paused
	*/
	bool IsPaused()
	{
		return m_bIsPaused;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Pause the effect module
	\param paused. If true then it is paused
	*/
	void SetPaused(bool paused)
	{
		if (!CanPause() || paused == IsPaused())
			return;
		
		m_bIsPaused = paused;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Can loop the effect module
	\return Can loop
	*/
	bool CanLoop()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get If effect module is looping
	\return Is looping
	*/
	bool IsLooping()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	/*!
	Set looping the effect module
	\param loop If true than it will start looping
	*/
	void SetLooping(bool loop);
	
	//------------------------------------------------------------------------------------------------
	/*!
	/return X and Y offset where the entity is spawned
	*/
	vector GetLocalTargetOffset()
	{
		return m_vLocalTargetOffset;
	}
};