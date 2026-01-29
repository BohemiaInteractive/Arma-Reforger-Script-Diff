[EntityEditorProps(category: "GameScripted/Debris", description: "Entity used to represent a large piece of debris from a collapsing building.", dynamicBox: true)]
class SCR_DebrisBuildingEntityClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_DebrisBuildingEntity : GenericEntity
{
	const static float RANDOM_INITIAL_LINEAR_VELOCITY_XZ = 0.5;
	const static float RANDOM_INITIAL_LINEAR_VELOCITY_Y = 1;
	const static float RANDOM_INITIAL_ANGULAR_VELOCITY = 3;
	const static float RANDOM_BUMP_DELAY_MIN = 0.5;
	const static float RANDOM_BUMP_DELAY_MAX = 2;
	const static float RANDOM_BUMP_HEIGHT_MAX = 15;
	const static float ANGULAR_VELOCITY_MAX = 30;
	
	protected SCR_BuildingSetup m_ParentBuildingSetup;
	protected int m_iBuildingRegion = 0;
	protected vector m_vLinearVelocity = vector.Zero;
	protected vector m_vAngularVelocity = vector.Zero;
	protected vector m_vCenter = vector.Zero;
	protected float m_fImpactPosY = 0;
	protected float m_fDebrisPosY = 0;
	protected float m_fDelayDeleteTime = 0;
	protected float m_fRandomBumpDelay = 0;
	protected bool m_bSpawnedDebris = false;
	
	//------------------------------------------------------------------------------------------------
	static SCR_DebrisBuildingEntity SpawnBuildingDebris(vector mat[4], SCR_BuildingSetup buildingSetup, int buildingRegion, IEntity ignoreEnt, string remap = "")
	{
		if (!buildingSetup)
			return null;
		
		ResourceName model = buildingSetup.GetDamagedRegionModel(buildingRegion);
		
		// Check if model is valid
		if (model == string.Empty)
			return null;
		
		Resource resource = Resource.Load(model);
		
		// Check if resource is valid
		if (!resource.IsValid())
			return null;
		
		SCR_DebrisBuildingEntity entity = SCR_DebrisBuildingEntity.Cast(GetGame().SpawnEntity(SCR_DebrisBuildingEntity));
		if (!entity)
			return null;
		
		entity.SetParentBuilding(buildingSetup);
		entity.SetTransform(mat);
		VObject obj = resource.GetResource().ToVObject();
		entity.SetObject(obj, remap);
		entity.m_vLinearVelocity = Vector(Math.RandomFloat(-RANDOM_INITIAL_LINEAR_VELOCITY_XZ, RANDOM_INITIAL_LINEAR_VELOCITY_XZ), Math.RandomFloat(-RANDOM_INITIAL_LINEAR_VELOCITY_Y, RANDOM_INITIAL_LINEAR_VELOCITY_Y), Math.RandomFloat(-RANDOM_INITIAL_LINEAR_VELOCITY_XZ, RANDOM_INITIAL_LINEAR_VELOCITY_XZ));
		entity.m_vAngularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * RANDOM_INITIAL_ANGULAR_VELOCITY;
		entity.SetBuildingRegion(buildingRegion);
		
		vector mins, maxs;
		entity.GetBounds(mins, maxs);
		
		float regionVerticalSize = 3;
		
		entity.m_vCenter = (maxs - mins) * 0.5 + mins;
		regionVerticalSize = maxs[1] - mins[1];
		
		// Trace downward and find the impact pos
		vector center = entity.CoordToParent(entity.m_vCenter);
		autoptr TraceParam param = new TraceParam;
		param.Exclude = ignoreEnt;
		param.Start = center;
		param.End = param.Start - Vector(0, buildingSetup.m_fLargeDebrisDropMax, 0);
		param.Flags = TraceFlags.ENTS | TraceFlags.WORLD;
		entity.m_fImpactPosY = param.End[1] - regionVerticalSize;
		entity.m_fDebrisPosY = entity.m_fImpactPosY;
		
		float traced = entity.GetWorld().TraceMove(param, null);
		if (traced < 1)
		{
			entity.m_fDebrisPosY = (param.End[1] - param.Start[1]) * traced + param.Start[1];
			entity.m_fImpactPosY = entity.m_fDebrisPosY - regionVerticalSize;
		}
		
		return entity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetParentBuilding(SCR_BuildingSetup buildingSetup)
	{
		m_ParentBuildingSetup = buildingSetup;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetLinearVelocity()
	{
		return m_vLinearVelocity;
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetAngularVelocity()
	{
		return m_vAngularVelocity;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetLinearVelocity(vector vel)
	{
		m_vLinearVelocity = vel;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAngularVelocity(vector vel)
	{
		m_vAngularVelocity = vel;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetBuildingRegion(int buildingRegion)
	{
		m_iBuildingRegion = buildingRegion;
	}
	
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice) //EntityEvent.FRAME
	{
		#ifdef BUILDING_DESTRUCTION_DEBUG
			if (Debug.KeyState(KeyCode.KC_SUBTRACT))
				timeSlice *= 0.1;
		#endif
		
		vector curMat[4];
		GetTransform(curMat);
		
		vector rotMat[3];
		Math3D.AnglesToMatrix(m_vAngularVelocity * timeSlice, rotMat);
		
		vector newMat[4];
		Math3D.MatrixMultiply3(curMat, rotMat, newMat);
		newMat[3] = curMat[3] + m_vLinearVelocity * timeSlice;
		
		SetTransform(newMat);
		vector center = CoordToParent(m_vCenter);
		
		float height = center[1];
		float heightAboveImpact = height - m_fImpactPosY;
		
		float gravityMult = 1;
		if (heightAboveImpact < RANDOM_BUMP_HEIGHT_MAX)
			gravityMult = 0.5;
		
		// Add gravity to fall
		m_vLinearVelocity += PhysicsWorld.GetGravity(GetGame().GetWorldEntity()) * timeSlice * gravityMult;
		
		// Add angular velocity, but don't increase if at maximum
		if (m_vAngularVelocity.LengthSq() < Math.Pow(ANGULAR_VELOCITY_MAX, 2))
			m_vAngularVelocity += m_vAngularVelocity * timeSlice;
		
		// Handle random "bumping", so simulate bouncing off surroundings
		m_fRandomBumpDelay -= timeSlice;
		if (m_fRandomBumpDelay <= 0 && heightAboveImpact > RANDOM_BUMP_HEIGHT_MAX)
		{
			m_fRandomBumpDelay = Math.RandomFloat(RANDOM_BUMP_DELAY_MIN, RANDOM_BUMP_DELAY_MAX);
			const float rndPosXZ = 1;
			const float rndPosYPctMin = 0.3;
			const float rndPosYPctMax = 0.9;
			const float rndAng = 2;
			m_vLinearVelocity += Vector(Math.RandomFloat(-rndPosXZ, rndPosXZ), Math.RandomFloat(rndPosYPctMin, rndPosYPctMax) * -m_vLinearVelocity[1], Math.RandomFloat(-rndPosXZ, rndPosXZ));
			m_vAngularVelocity += Vector(Math.RandomFloat(-rndAng, rndAng), Math.RandomFloat(-rndAng, rndAng), Math.RandomFloat(-rndAng, rndAng));
		}
		
		if (!m_bSpawnedDebris && height < m_fDebrisPosY)
		{
			m_bSpawnedDebris = true;
			if (m_ParentBuildingSetup)
				m_ParentBuildingSetup.SpawnDebrisForRegion(this, m_iBuildingRegion, m_vLinearVelocity, m_vAngularVelocity);
		}
		
		if (height > m_fImpactPosY)
			return;
		
		delete this;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DebrisBuildingEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT | EntityEvent.FRAME);
		SetFlags(EntityFlags.ACTIVE, true);
		
		m_fRandomBumpDelay = Math.RandomFloat(RANDOM_BUMP_DELAY_MIN, RANDOM_BUMP_DELAY_MAX);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_DebrisBuildingEntity()
	{
		IEntity child = GetChildren();
		while (child)
		{
			delete child;
			child = GetChildren();
		}
	}
};