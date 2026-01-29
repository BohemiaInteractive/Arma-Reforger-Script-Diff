[EntityEditorProps(category: "GameScripted/Entities", description: "This entity will create airstrikes on the map")]

class SCR_AirstrikePrototypeClass: GenericEntityClass
{
};

class SCR_AirstrikePrototype: GenericEntity
{
	[Attribute("", UIWidgets.ResourceAssignArray, "Generated projectiles will loop through the projectiles on this array", "et")]
	private ref array<ResourceName> m_ProjectilesToTrigger;
	
	//Initial values
	[Attribute("0", UIWidgets.EditBox, "Number of projectiles to create. 0 = until time ends")]
	int m_NumProjectiles;
	
	[Attribute("0", UIWidgets.EditBox, "Time until this entity creates a new projectile.")]
	float m_TimeBetweenProjectiles;
			
	[Attribute("0", UIWidgets.EditBox, "Size of zone around the point where the airstrike will take place")]
	float m_AirstrikeRadius;
	
	[Attribute("0", UIWidgets.CheckBox, "If true, even if there is a random radius it will have a bias towards the original airstrike target")]
	bool m_BiasTowardsCenter;
	
	[Attribute("0", UIWidgets.Range, "Random delay [0,X] applied to the time between projectiles to make it more believable. Ignore Z parameter, 2D vectors are not supported by script")]
	vector m_RandomDelayBetweenRockets;
	
	[Attribute("0", UIWidgets.Range, "Initial delay for the beginning of the airstrike")]
	float m_InitialDelay;
	
	// current values
	private ref array<ref Resource> m_LoadedPrefabs = new array<ref Resource>();

	
	int m_RemainingProjectiles = 0;
	float m_TimeUntilNextProjectile = 0;

	//keeps track of what projectile prefab will be used
	private int m_CurrentProjectilePrefab = 0;
	private int m_PreviousProjectilePrefab = 0;

	private ref RandomGenerator m_RandomGenerator = new RandomGenerator;

	//audio variables
	private BaseSoundComponent m_SoundComponent;
	private int m_SoundShotIndex = 0;
	
	private vector m_direction;
	private vector m_spawnPos;
	private IEntity m_lastSpawnedProjectile = null;
		
	
	private IEntity m_owner = null;
	private RplComponent m_RplComponent = null;
	//------------------------------------------------------------------------------------------------
	void SCR_AirstrikePrototype(IEntitySource src, IEntity parent)
	{
		SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(EntityEvent.FRAME | EntityEvent.INIT);
		
		for(int i = 0; i < m_ProjectilesToTrigger.Count(); i++)
		{
			if(!m_ProjectilesToTrigger[i])
				continue;
		
			Resource projectile = Resource.Load(m_ProjectilesToTrigger[i]);
			if(projectile)
				m_LoadedPrefabs.Insert(projectile);
		}
		
		m_RemainingProjectiles = m_NumProjectiles;
		m_TimeUntilNextProjectile = m_TimeBetweenProjectiles;

		AddRandomDelayToNextProjectile();
	}		

	//------------------------------------------------------------------------------------------------
	override protected void EOnInit(IEntity owner)
	{	
		m_owner = owner;
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
	
		m_TimeUntilNextProjectile += m_InitialDelay;
	}		
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice) //EntityEvent.FRAME
	{
		if(m_lastSpawnedProjectile)
		{
			Do_LaunchMissile(m_direction);
			
			m_lastSpawnedProjectile = null;
		}
		
		//end conditions, no need to keep updating the object
		if (m_RemainingProjectiles <= 0)
		{
			ClearFlags(EntityFlags.ACTIVE, false);
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		m_TimeUntilNextProjectile -= timeSlice;
		
		//no projectile
		if (m_TimeUntilNextProjectile > 0)
			return;

		//added instead of assigned to ensure accuracy
		m_TimeUntilNextProjectile += m_TimeBetweenProjectiles;
		AddRandomDelayToNextProjectile();
		
		if(!m_RplComponent || !m_RplComponent.IsProxy())
			ReadyProjectile();
	}		

	protected void ReadyProjectile()
	{
		ref Resource prefab = m_LoadedPrefabs[m_CurrentProjectilePrefab];
		
		AdvanceProjectilePrefab();
		
		m_RemainingProjectiles--;
				
		if (!prefab)
			return;
		
		ref EntitySpawnParams spawnParams = new EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;

		vector direction;
		vector missileTarget = ComputeMissileTarget();
		
		ComputeProjectileTransform(spawnParams.Transform, direction, missileTarget);
		
		FireProjectile(prefab, spawnParams, direction);
	}			
	
	//------------------------------------------------------------------------------------------------	
	protected void AdvanceProjectilePrefab()
	{
		m_PreviousProjectilePrefab = m_CurrentProjectilePrefab;
		++m_CurrentProjectilePrefab;
		
		if (m_CurrentProjectilePrefab >= m_LoadedPrefabs.Count())
			m_CurrentProjectilePrefab = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	protected vector ComputeMissileTarget()
	{
		vector ownerTransform[4];
		GetTransform(ownerTransform);
		
		if(m_AirstrikeRadius == 0)
			return ownerTransform[3];
		
		vector missileTarget = m_RandomGenerator.GenerateRandomPointInRadius(0, m_AirstrikeRadius, ownerTransform[3], !m_BiasTowardsCenter);
		missileTarget[1] = ownerTransform[3][1];
		
		return missileTarget;
	}
	//------------------------------------------------------------------------------------------------
	protected void ComputeProjectileTransform(out vector transform[4], out vector direction, vector missileTarget)
	{
		vector mat[4];
		vector rotation[4];

		GetTransform(mat);
		
		m_spawnPos = mat[3];
		m_spawnPos[1] = m_spawnPos[1] + 100;
	
		direction = vector.Direction(m_spawnPos, missileTarget);
		direction.Normalize();

		Math3D.MatrixFromForwardVec(direction, transform);
		transform[3] = m_spawnPos;
	}
	
	//------------------------------------------------------------------------------------------------
	void FireProjectile(Resource prefab, EntitySpawnParams spawnParams, vector direction)
	{
		IEntity spawnedProjectile = GetGame().SpawnEntityPrefab(prefab, GetWorld(), spawnParams);

		m_lastSpawnedProjectile = spawnedProjectile;
		m_direction = direction;
	}
	
	//------------------------------------------------------------------------------------------------
	void Do_LaunchMissile(vector direction)
	{
		RplComponent rplComp = RplComponent.Cast(m_lastSpawnedProjectile.FindComponent(RplComponent));
		
		Rpc(RpcLaunchMissile_BC, rplComp.Id(), direction);
		
		LaunchMissile(m_lastSpawnedProjectile, direction);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]	
	void RpcLaunchMissile_BC(RplId id, vector direction)
	{
		RplComponent rocketRplComp = RplComponent.Cast(Replication.FindItem(id));
		
		LaunchMissile(rocketRplComp.GetEntity(), direction);
	}
	
	//------------------------------------------------------------------------------------------------
	void LaunchMissile(IEntity rocket, vector direction)
	{
		if(!rocket)
			return;
		
		ProjectileMoveComponent moveComp = ProjectileMoveComponent.Cast(rocket.FindComponent(ProjectileMoveComponent));

		if(!moveComp)
			return;
		
		moveComp.Launch(direction, vector.Zero, 1, rocket, null, null, null, null);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void AddRandomDelayToNextProjectile()
	{
		if(m_RandomDelayBetweenRockets[0] >= m_RandomDelayBetweenRockets[1])
			return;
		
		float delay = m_RandomGenerator.RandFloatXY(m_RandomDelayBetweenRockets[0], m_RandomDelayBetweenRockets[1]);
	
		m_TimeUntilNextProjectile += delay;
		
	}
	//------------------------------------------------------------------------------------------------
	void ~SCR_AirstrikePrototype()
	{
	}
};