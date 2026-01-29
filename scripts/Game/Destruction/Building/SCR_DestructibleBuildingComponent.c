//------------------------------------------------------------------------------------------------
class SCR_DestructibleBuildingComponentClass : SCR_DamageManagerComponentClass
{
	[Attribute()]
	ref array<ref SCR_TimedEffect> m_aEffects;

	[Attribute(desc: "Useful for complex buildings. Leave empty to use entity's bounding box.")]
	ref array<ref SCR_InteriorBoundingBox> m_aInteriorQueryBoundingBoxes;

	[Attribute(defvalue: "0", desc: "Delay in seconds between damage and the beginning of the building transition")]
	float m_fDelay;

	[Attribute(defvalue: "1", desc: "Meters per second")]
	float m_fSinkingSpeed;

	[Attribute(defvalue: "150", desc: "This speeds up the sinking gradually % per second.", params: "0 10000 1")]
	float m_fSinkingSpeedGradualMultiplier;

	[Attribute(defvalue: "0.1", desc: "Degrees per second")]
	float m_fRotationSpeed;

	[Attribute(defvalue: "0.8", desc: "Time between rotation changes when building is collapsing in seconds")]
	float m_fRotationTime;

	[Attribute(defvalue: "50", desc: "Rotation time randomizer in % - can both shorten/prolong the time", params: "0 10000 1")]
	float m_fRotationTimeRandom;

	[Attribute(defvalue: "1", desc: "Max rotations count while sinking")]
	int m_iMaxRotations;

	[Attribute(defvalue : vector.Zero.ToString(), desc: "This vector defines offset for the final position after destruction.")]
	vector m_vSinkVector;

	[Attribute("", UIWidgets.Auto, desc: "Slow down event audio source configuration")]
	ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	[Attribute(uiwidget: UIWidgets.CurveDialog)]
	ref Curve m_CameraShakeCurve;

	[Attribute(uiwidget: UIWidgets.Flags, enums: ParamEnumArray.FromEnum(SCR_EDestructionRotationEnum))]
	SCR_EDestructionRotationEnum m_eAllowedRotations;

	[Attribute()]
	ref DestructionHeatmapEntry m_EntryData;

};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_CAN_HAVE_NAME)]
class SCR_InteriorBoundingBox : Managed
{
	[Attribute()]
	protected ref PointInfo m_vCenter;

	[Attribute(defvalue: "1 1 1")]
	protected vector m_vScale;

	[Attribute(defvalue: "0 1 1 1")]
	protected ref Color m_Color;

	//------------------------------------------------------------------------------------------------
	bool GetCenter(out vector center)
	{
		if (!m_vCenter)
			return false;

		vector centerTransform[4];
		m_vCenter.GetTransform(centerTransform);

		center = centerTransform[3];
		return true;
	}

	//------------------------------------------------------------------------------------------------
	vector GetScale()
	{
		return m_vScale;
	}

	//------------------------------------------------------------------------------------------------
	void GetBounds(out vector mins, out vector maxs)
	{
		vector center;
		GetCenter(center);

		mins = {center[0] - m_vScale[0] * 0.5, center[1] - m_vScale[1] * 0.5, center[2] - m_vScale[2] * 0.5};
		maxs = {center[0] + m_vScale[0] * 0.5, center[1] + m_vScale[1] * 0.5, center[2] + m_vScale[2] * 0.5};
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	void DrawDebug(vector ownerTransform[4])
	{
		vector center;
		GetCenter(center);
		vector halfScale = m_vScale * 0.5;

		vector points[8] = {{-halfScale[0], -halfScale[1], -halfScale[2]} + center,
							{halfScale[0], halfScale[1], halfScale[2]} + center,
							{halfScale[0], -halfScale[1], -halfScale[2]} + center,
							{-halfScale[0], halfScale[1], halfScale[2]} + center,
							{-halfScale[0], halfScale[1], -halfScale[2]} + center,
							{halfScale[0], -halfScale[1], halfScale[2]} + center,
							{-halfScale[0], -halfScale[1], halfScale[2]} + center,
							{halfScale[0], halfScale[1], -halfScale[2]} + center};

		// Before you freak out, this is all just workbench debug visuals :)
		vector p0[] = {points[0].Multiply4(ownerTransform), points[4].Multiply4(ownerTransform)};
		vector p1[] = {points[2].Multiply4(ownerTransform), points[7].Multiply4(ownerTransform)};
		vector p2[] = {points[5].Multiply4(ownerTransform), points[1].Multiply4(ownerTransform)};
		vector p3[] = {points[6].Multiply4(ownerTransform), points[3].Multiply4(ownerTransform)};

		vector p4[] = {points[0].Multiply4(ownerTransform), points[2].Multiply4(ownerTransform)};
		vector p5[] = {points[2].Multiply4(ownerTransform), points[5].Multiply4(ownerTransform)};
		vector p6[] = {points[5].Multiply4(ownerTransform), points[6].Multiply4(ownerTransform)};
		vector p7[] = {points[6].Multiply4(ownerTransform), points[0].Multiply4(ownerTransform)};

		vector p8[] = {points[4].Multiply4(ownerTransform), points[7].Multiply4(ownerTransform)};
		vector p9[] = {points[7].Multiply4(ownerTransform), points[1].Multiply4(ownerTransform)};
		vector p10[] = {points[1].Multiply4(ownerTransform), points[3].Multiply4(ownerTransform)};
		vector p11[] = {points[3].Multiply4(ownerTransform), points[4].Multiply4(ownerTransform)};

		ShapeFlags shapeFlags = ShapeFlags.ONCE | ShapeFlags.NOZBUFFER | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE | ShapeFlags.NOOUTLINE;

		if (!m_Color)
			m_Color = Color.Blue;
		int color = m_Color.PackToInt();

		// Bounding Box markers
		Shape.CreateLines(color, shapeFlags, p0, 2);
		Shape.CreateLines(color, shapeFlags, p1, 2);
		Shape.CreateLines(color, shapeFlags, p2, 2);
		Shape.CreateLines(color, shapeFlags, p3, 2);

		Shape.CreateLines(color, shapeFlags, p4, 2);
		Shape.CreateLines(color, shapeFlags, p5, 2);
		Shape.CreateLines(color, shapeFlags, p6, 2);
		Shape.CreateLines(color, shapeFlags, p7, 2);

		Shape.CreateLines(color, shapeFlags, p8, 2);
		Shape.CreateLines(color, shapeFlags, p9, 2);
		Shape.CreateLines(color, shapeFlags, p10, 2);
		Shape.CreateLines(color, shapeFlags, p11, 2);

		// Calculate min and max points for the box
		vector mins = {center[0] - halfScale[0], center[1] - halfScale[1], center[2] - halfScale[2]};
		vector maxs = {center[0] + halfScale[0], center[1] + halfScale[1], center[2] + halfScale[2]};

		// Create a semi-transparent box using our new AddBoxWithTransform method that transforms all points by owner transform
		color = Color(m_Color.R(), m_Color.G(), m_Color.B(), 0.25).PackToInt();
		SCR_DebugShapeHelperComponent.AddBoxWithTransform(mins, maxs, ownerTransform, color, ShapeFlags.ONCE | ShapeFlags.TRANSP | ShapeFlags.DOUBLESIDE);
	}
#endif
}

//------------------------------------------------------------------------------------------------
enum SCR_EDestructionRotationEnum
{
	NONE = 0,
	ROTATION_X = 1,
	ROTATION_Y = 2,
	ROTATION_Z = 4
}

//------------------------------------------------------------------------------------------------
class SCR_BuildingDestructionCameraShakeProgress : SCR_NoisyCameraShakeProgress
{
	protected const float MAX_MULTIPLIER = 1.5;
	protected float m_fMaxDistance = 50;
	protected float m_fSizeMultiplier = 1;

	protected ref Curve m_CameraShakeCurve;
	protected vector m_vStartOrigin;

	//------------------------------------------------------------------------------------------------
	override void Update(IEntity owner, float timeSlice)
	{
		if (!IsRunning())
			return;

		super.Update(owner, timeSlice);

		CameraBase camera = GetGame().GetCameraManager().CurrentCamera();
		if (!camera)
			return;

		float distanceSq = vector.DistanceSqXZ(m_vStartOrigin, camera.GetOrigin());
		float multiplier = 1 - Math.Clamp(distanceSq / (m_fMaxDistance * m_fMaxDistance * m_fSizeMultiplier), 0, 1);
		float curveMultiplier = LegacyCurve.Curve(ECurveType.CatmullRom, 1 - multiplier, m_CameraShakeCurve)[1];

		m_vTranslation *= Math.Min(multiplier * m_fSizeMultiplier * curveMultiplier, MAX_MULTIPLIER);
		m_vRotation *= Math.Min(multiplier * m_fSizeMultiplier * curveMultiplier, MAX_MULTIPLIER);
	}

	//------------------------------------------------------------------------------------------------
	void SetStartOrigin(vector startOrigin)
	{
		m_vStartOrigin = startOrigin
	}

	//------------------------------------------------------------------------------------------------
	void SetCurve(Curve curve)
	{
		m_CameraShakeCurve = curve;
	}

	//------------------------------------------------------------------------------------------------
	void SetSizeMultiplier(float sizeMultiplier)
	{
		m_fSizeMultiplier = Math.Min(sizeMultiplier, MAX_MULTIPLIER);
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedDebris : SCR_TimedEffect
{
	[Attribute("0 0 0", UIWidgets.Coords, desc: "Positional offset (in local space to the destructible)")]
	protected vector m_vOffsetPosition;

	[Attribute("0 0 0", UIWidgets.Coords, desc: "Yaw, pitch & roll offset (in local space to the destructible)")]
	protected vector m_vOffsetRotation;

	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Debris model prefabs to spawn (spawns ALL of them)", "et xob")]
	ref array<ResourceName> m_ModelPrefabs;

	[Attribute("10", UIWidgets.Slider, "Mass of the debris", "0.01 1000 0.01")]
	float m_fMass;

	[Attribute("5", UIWidgets.Slider, "Minimum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMin;

	[Attribute("10", UIWidgets.Slider, "Maximum lifetime value for the debris (in s)", "0 3600 0.5")]
	float m_fLifetimeMax;

	[Attribute("200", UIWidgets.Slider, "Maximum distance from camera above which the debris is not spawned (in m)", "0 3600 0.5")]
	float m_fDistanceMax;

	[Attribute("0", UIWidgets.Slider, "Higher priority overrides lower priority if at or over debris limit", "0 100 1")]
	int m_fPriority;

	[Attribute("0.1", UIWidgets.Slider, "Damage received to physics impulse (speed / mass) multiplier", "0 10000 0.01")]
	float m_fDamageToImpulse;

	[Attribute("2", UIWidgets.Slider, "Damage to speed multiplier, used when objects get too much damage to impulse", "0 10000 0.01")]
	float m_fMaxDamageToSpeedMultiplier;

	[Attribute("0.5", UIWidgets.Slider, "Random linear velocity multiplier (m/s)", "0 200 0.1")]
	float m_fRandomVelocityLinear;

	[Attribute("180", UIWidgets.Slider, "Random angular velocity multiplier (deg/s)", "0 3600 0.1")]
	float m_fRandomVelocityAngular;

	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for debris sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeDebris))]
	SCR_EMaterialSoundTypeDebris m_eMaterialSoundType;

	//------------------------------------------------------------------------------------------------
	//! Calculates the spawn tranformation matrix for the object
	void GetSpawnTransform(IEntity owner, out vector outMat[4], bool localCoords = false)
	{
		if (localCoords)
		{
			Math3D.AnglesToMatrix(m_vOffsetRotation, outMat);
			// TODO: Remove hotfix for sleeping/static object
			if (m_vOffsetPosition == vector.Zero)
				outMat[3] = vector.Up * 0.001;
			else
				outMat[3] = m_vOffsetPosition;
		}
		else
		{
			vector localMat[4], parentMat[4];
			owner.GetWorldTransform(parentMat);
			Math3D.AnglesToMatrix(m_vOffsetRotation, localMat);
			localMat[3] = m_vOffsetPosition;

			Math3D.MatrixMultiply4(parentMat, localMat, outMat);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Spawns the object
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		int numModelPrefabs = 0;
		if (m_ModelPrefabs)
			numModelPrefabs = m_ModelPrefabs.Count();

		for (int i = 0; i < numModelPrefabs; i++)
		{
			ResourceName prefabPath = m_ModelPrefabs[i];

			ResourceName modelPath;
			string remap;
			SCR_Global.GetModelAndRemapFromResource(prefabPath, modelPath, remap);

			if (modelPath == ResourceName.Empty)
				continue;

			vector spawnMat[4];
			GetSpawnTransform(owner, spawnMat);

			SCR_DestructionDamageManagerComponent destructionComponent = SCR_DestructionDamageManagerComponent.Cast(owner.FindComponent(SCR_DestructionDamageManagerComponent));

			float dmgSpeed = Math.Clamp(hitInfo.m_HitDamage * m_fDamageToImpulse / m_fMass, 0, m_fMaxDamageToSpeedMultiplier);

			vector linearVelocity = hitInfo.m_HitDirection * Math.RandomFloat(0, 1);
			linearVelocity += Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * m_fRandomVelocityLinear;
			linearVelocity *= dmgSpeed;
			vector angularVelocity = Vector(Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1), Math.RandomFloat(-1, 1)) * Math.RandomFloat(0.25, 4) * m_fRandomVelocityAngular;
			angularVelocity *= dmgSpeed;

			SCR_DebrisSmallEntity.SpawnDebris(owner.GetWorld(), spawnMat, modelPath, m_fMass, Math.RandomFloat(m_fLifetimeMin, m_fLifetimeMax), m_fDistanceMax, m_fPriority, linearVelocity, angularVelocity, remap, false, m_eMaterialSoundType);
		}
	}
}

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedEffect : Managed
{
	[Attribute("0", UIWidgets.Slider, "Set time in % of sinking the building. 0 = Immediately, can happen before the sinking starts if delay is used", "0 1 0.01")]
	float m_fSpawnTime;

	[Attribute()]
	bool m_bSnapToTerrain;

	[Attribute()]
	bool m_bAttachToParent;

	[Attribute("0", desc: "Does this effect remain after destruction? F. E. ruins.")]
	bool m_bPersistent;

	//------------------------------------------------------------------------------------------------
	//! Snaps the origin to terrain
	void SnapToTerrain(inout vector origin, IEntity owner)
	{
		if (!m_bSnapToTerrain)
			return;

		float y = SCR_TerrainHelper.GetTerrainY(origin, owner.GetWorld());
		origin[1] = y;
	}

	//------------------------------------------------------------------------------------------------
	//! To be overridden by inherited classes
	void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedSound : SCR_TimedEffect
{
	[Attribute("", UIWidgets.Auto, desc: "Audio Source Configuration")]
	protected ref SCR_AudioSourceConfiguration m_AudioSourceConfiguration;

	//------------------------------------------------------------------------------------------------
	//! Plays effects
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		super.ExecuteEffect(owner, hitInfo, data);

		if (!System.IsConsoleApp())
		{
			SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
			if (!soundManager || !m_AudioSourceConfiguration || !m_AudioSourceConfiguration.IsValid())
				return;

			SCR_DestructibleBuildingComponent destructibleComponent = SCR_DestructibleBuildingComponent.Cast(owner.FindComponent(SCR_DestructibleBuildingComponent));
			if (!destructibleComponent)
				return;

			SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, m_AudioSourceConfiguration, data.m_vStartMatrix[3]);
			if (!audioSource)
				return;

			destructibleComponent.SetAudioSource(audioSource);

			soundManager.PlayAudioSource(audioSource);
		}
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	SCR_AudioSourceConfiguration GetAudioSourceConfiguration()
	{
		return m_AudioSourceConfiguration;
	}
#endif
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedPrefab : SCR_TimedEffect
{
	[Attribute(desc: "Defines what remains after the building is destroyed.", params: "et")]
	protected ResourceName m_sRuinsPrefab;

	[Attribute("1", desc: "If this prefab has an RplComponent then it should be only spawned by the server")]
	protected bool m_bSpawnedByTheServer;

	//------------------------------------------------------------------------------------------------
	//! Spawns prefab
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		super.ExecuteEffect(owner, hitInfo, data);

		RplComponent rplComp = SCR_EntityHelper.GetEntityRplComponent(owner);
		if (m_bSpawnedByTheServer && (!rplComp || rplComp.IsProxy()))
			return;

		Resource resource = Resource.Load(m_sRuinsPrefab);
		if (!resource || !resource.IsValid())
			return;

		vector mat[4];
		mat = data.m_vStartMatrix;

		if (m_bSnapToTerrain)
		{
			vector origin = mat[3];
			SnapToTerrain(origin, owner);
			mat[3] = origin;
		}

		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform = mat;
		params.TransformMode = ETransformMode.WORLD;

		if (m_bAttachToParent)
			params.Parent = owner;

		IEntity spawnedEntity = GetGame().SpawnEntityPrefab(resource, owner.GetWorld(), params);
		if (!spawnedEntity)
		{
			Debug.Error(string.Format("Could not spawn prefab %1 in SCR_TimedPrefab.ExecuteEffect()", m_sRuinsPrefab));
			return;
		}

		auto persistence = PersistenceSystem.GetInstance();
		if (persistence)
			persistence.StartTracking(spawnedEntity);

		data.m_aExcludeList.Insert(spawnedEntity);
	}
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_TimedParticle : SCR_TimedEffect
{
	[Attribute()]
	protected ref SCR_ParticleSpawnable m_Particle;

	[Attribute("1", desc: "Overall multiplier for particle effect intensity. Higher values multiplies particle count (Birt Rate + Birth Rate RND), size (Size Multiplier and Size RND) , and velocity (Velocity and Velocity RND).")]
	protected float m_fParticlesMultiplier;

	void SCR_TimedParticle()
	{
		if (!m_Particle)
		{
			Print("No SCR_ParticleSpawnable defined on SCR_TimedParticle", LogLevel.WARNING);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Plays particles
	override void ExecuteEffect(IEntity owner, SCR_HitInfo hitInfo, inout notnull SCR_BuildingDestructionData data)
	{
		super.ExecuteEffect(owner, hitInfo, data);

		if (m_Particle == null)
			return;

		ParticleEffectEntity emitter;
		if (m_bAttachToParent)
			emitter = m_Particle.SpawnAsChild(owner, hitInfo, m_bSnapToTerrain);
		else
			emitter = ParticleEffectEntity.Cast(m_Particle.Spawn(owner, owner.GetPhysics(), hitInfo, m_bSnapToTerrain));

		if (!emitter)
		{
			Debug.Error("No emitter was spawned in SCR_TimedParticle.ExecuteEffect()");
			return;
		}

		SetParticleParams(emitter, data);

		data.m_aExcludeList.Insert(emitter);
	}

	//------------------------------------------------------------------------------------------------
	void SetParticleParams(ParticleEffectEntity emitter, inout notnull SCR_BuildingDestructionData data)
	{
		Particles particles = emitter.GetParticles();
		if (!particles)
			return;

		particles.MultParam(-1, EmitterParam.BIRTH_RATE, Math.Clamp(data.m_fSizeMultiplier, 0.5, 2) * m_fParticlesMultiplier);
		particles.MultParam(-1, EmitterParam.BIRTH_RATE_RND, Math.Clamp(data.m_fSizeMultiplier, 0.5, 2) * m_fParticlesMultiplier);
		particles.MultParam(-1, EmitterParam.SIZE, Math.Clamp(data.m_fSizeMultiplier, 0.5, 1) * Math.Clamp(m_fParticlesMultiplier, 0.1, 2));
		particles.MultParam(-1, EmitterParam.VELOCITY, Math.Clamp(data.m_fSizeMultiplier, 1, 2) * m_fParticlesMultiplier);
		particles.MultParam(-1, EmitterParam.VELOCITY_RND, Math.Clamp(data.m_fSizeMultiplier, 1, 2) * m_fParticlesMultiplier);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	//! Get m_Particles - Workbench only for SCR_BuildingDestructionSetupTool purpose
	SCR_ParticleSpawnable GetParticle()
	{
		return m_Particle;
	}
	//------------------------------------------------------------------------------------------------
	//! Get m_fParticlesMultiplier - Workbench only for SCR_BuildingDestructionSetupTool purpose
	float GetParticlesMultiplier()
	{
		return m_fParticlesMultiplier;
	}
#endif
};

//------------------------------------------------------------------------------------------------
class SCR_DestructibleBuildingComponent : SCR_DamageManagerComponent
{
	// In project settings you can see the physics response indices & their interactions matrix
	protected const int NO_COLLISION_RESPONSE_INDEX = 11;
	protected const int MAX_CHECKS_PER_FRAME = 20;
	protected const float BUILDING_SIZE = 5000;
	protected const vector TRACE_DIRECTIONS[3] = {vector.Right, vector.Up, vector.Forward};

	private int m_iDataIndex = -1;

	protected bool m_bDestroyed = false;

	protected ref array<vector> m_aPowerPolePositions = {};

	//------------------------------------------------------------------------------------------------
	//! Returns centrally stored data from building destruction manager
	protected SCR_BuildingDestructionData GetData()
	{
		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		if (!manager)
		{
			Print("SCR_BuildingDestructionManagerComponent not found! Building destruction won't work.", LogLevel.ERROR);
			return null;
		}

		return manager.GetData(m_iDataIndex);
	}

	//------------------------------------------------------------------------------------------------
	//! Frees the data stored in building destruction manager
	protected void FreeData()
	{
		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		if (!manager)
		{
			Print("SCR_BuildingDestructionManagerComponent not found! Building destruction won't work.", LogLevel.ERROR);
			return;
		}

		manager.FreeData(m_iDataIndex);
		m_iDataIndex = -1;
	}

	//------------------------------------------------------------------------------------------------
	//! Destroys all direct child entities that have SCR_DestructibleBuildingComponent
	//! Each child will handle its own sub-children during its destruction process
	//! \param[in] owner The parent entity whose children should be checked and destroyed
	protected void DestroyChildDestructibles(notnull IEntity owner)
	{	
		// Pre-allocate shared resources to avoid repeated allocations
		Instigator sharedInstigator = GetInstigator();
		if (!sharedInstigator)
			sharedInstigator = Instigator.CreateInstigator(null);
		
		HitZone defaultHitZone;
		vector hitPosDirNorm[3]; // Empty for TRUE damage type
		// Trigger destruction - child will handle its own sub-children
		SCR_DamageContext damageContext = new SCR_DamageContext(
			EDamageType.TRUE,
			1,
			hitPosDirNorm,
			null,
			defaultHitZone,
			sharedInstigator,
			null,
			-1,
			-1
		);
		
		// Iterate through direct children and destroy those with destructible components
		IEntity child = owner.GetChildren();
		IEntity nextChild;
		while (child)
		{
			// Store next sibling BEFORE any modifications to be safe
			nextChild = child.GetSibling();
			
			SCR_DestructibleBuildingComponent childDestructibleComp = SCR_DestructibleBuildingComponent.Cast(
				child.FindComponent(SCR_DestructibleBuildingComponent)
			);
			
			if (childDestructibleComp)
			{
				defaultHitZone = childDestructibleComp.GetDefaultHitZone();
				if (defaultHitZone)
				{
					// Detach from parent before triggering destruction
					owner.RemoveChild(child, true);
					
					damageContext.damageValue = defaultHitZone.GetMaxHealth();
					damageContext.struckHitZone = defaultHitZone;
					damageContext.hitEntity = child;

					childDestructibleComp.HandleDamage(damageContext);
				}
			}
			
			child = nextChild;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns prefab data stored speed gradual multiplier
	protected float GetSpeedGradualMultiplier()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fSinkingSpeedGradualMultiplier;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetRotationTimeRandomizer()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fRotationTimeRandom;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetRotationSpeed()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fRotationSpeed;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetRotationTime()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fRotationTime;
	}

	//------------------------------------------------------------------------------------------------
	protected int GetMaxRotations()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_iMaxRotations;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns prefab data stored speed
	protected float GetSpeed()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fSinkingSpeed;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetSinkVector()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_vSinkVector;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns prefab data stored delay
	protected float GetDelay()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_fDelay;
	}

	//------------------------------------------------------------------------------------------------
	protected ref array<ref SCR_InteriorBoundingBox> GetInteriorBoundingBoxes()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_aInteriorQueryBoundingBoxes;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetAllowRotationX()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_eAllowedRotations & SCR_EDestructionRotationEnum.ROTATION_X;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetAllowRotationY()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_eAllowedRotations & SCR_EDestructionRotationEnum.ROTATION_Y;
	}

	//------------------------------------------------------------------------------------------------
	protected bool GetAllowRotationZ()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_eAllowedRotations & SCR_EDestructionRotationEnum.ROTATION_Z;
	}

	//------------------------------------------------------------------------------------------------
	protected Curve GetCameraShakeCurve()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_CameraShakeCurve;
	}

	//------------------------------------------------------------------------------------------------
	//! Returns pointer to prefab data stored array of effects, do not modify the array!
	protected array<ref SCR_TimedEffect> GetEffects()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_aEffects;
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_AudioSourceConfiguration GetSlowDownAudioSourceConfiguration()
	{
		return SCR_DestructibleBuildingComponentClass.Cast(GetComponentData(GetOwner())).m_AudioSourceConfiguration;
	}

	//------------------------------------------------------------------------------------------------
	void SetAudioSource(SCR_AudioSource audioSource)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		if (data.m_AudioSource)
		{
			// Kill previous sound
			SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(GetOwner().GetWorld());
			if (soundManager)
				soundManager.TerminateAudioSource(data.m_AudioSource);
		}

		data.m_AudioSource = audioSource;

		if (audioSource)
			data.m_AudioSource.SetSignalValue(SCR_AudioSource.ENTITY_SIZE_SIGNAL_NAME, data.m_fBuildingVolume);
	}

	//------------------------------------------------------------------------------------------------
	SCR_AudioSource GetAudioSource()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return null;

		return data.m_AudioSource;
	}

	//------------------------------------------------------------------------------------------------
	protected void SetSeed(int seed)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		data.m_RandomGenerator.SetSeed(seed);
	}

	//------------------------------------------------------------------------------------------------
	//! Called when damage state is changed
	protected override void OnDamageStateChanged(EDamageState newState, EDamageState previousDamageState, bool isJIP)
	{
		super.OnDamageStateChanged(newState, previousDamageState, isJIP);

		if (IsProxy())
			return;

		// Already destroyed
		if (m_bDestroyed)
			return;

		// Called only on the server
		if (newState == EDamageState.DESTROYED)
		{
			int seed = Math.RandomInt(0, 10000);
			StoreNavmeshData();
			RPC_GoToDestroyedState(seed);
			Rpc(RPC_GoToDestroyedState, seed);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Used to do runtime synchronization of state
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_GoToDestroyedState(int seed)
	{
		SetSeed(seed);
		DestroyInteriorInit(false);
		CalculateAndStoreVolume();
		SpawnEffects(0, GetOwner(), false);
		GetGame().GetCallqueue().CallLater(GoToDestroyedState, 1000 * GetDelay(), param1: false, param2: true);
	}

	//------------------------------------------------------------------------------------------------
	//! Handles destruction of interior by gathering objects using AABB (will probably use OBB) and deleting them according to their type
	protected void DestroyInteriorInit(bool immediate)
	{
		// Verify that both data and destruction manager are present before doing anything
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		if (!manager)
			return;

		//exclude spawned effects
		IEntity owner = GetOwner();
		BaseWorld world = owner.GetWorld();
		vector mins, maxs;

		data.m_aQueriedEntities = {};

		owner.GetWorldTransform(data.m_vStartMatrix);

		// Destroy child entities with SCR_DestructibleBuildingComponent before querying
		DestroyChildDestructibles(owner);

		// We'll query for static and dynamic entities only.
		// Proxies are also excluded so we don't even consider:
		// - items attached to players
		// - players attached to vehicles
		// - anything (most likely buildings parts) using EntitySlots
		EQueryEntitiesFlags queryFlags = EQueryEntitiesFlags.STATIC | EQueryEntitiesFlags.DYNAMIC | EQueryEntitiesFlags.NO_PROXIES;

		// Run the queries and fetch entities that we'll be interacting with.
		// In theory, we could run two separate queries, one for static and and for dynamic entities.
		// However, the logic for both is almost the same so there is little to no gain to doing that.
		array<ref SCR_InteriorBoundingBox> boundingBoxes = GetInteriorBoundingBoxes();
		if (!boundingBoxes || boundingBoxes.IsEmpty())
		{

			owner.GetBounds(mins, maxs);
			world.QueryEntitiesByOBB(mins, maxs, data.m_vStartMatrix, AddEntityCallback, QueryFilterCallback, queryFlags);
			return;
		}

		SCR_DestructionManager destrManager = SCR_DestructionManager.GetDestructionManagerInstance();
		SCR_RegionalDestructionManager regionalManager;
		if (destrManager)
			regionalManager = SCR_RegionalDestructionManager.Cast(destrManager.GetOrCreateRegionalDestructionManager(GetOwner().GetOrigin()));

		RplComponent rplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		ref array<ref vector> boxesPositions = new array<ref vector>;

		foreach(SCR_InteriorBoundingBox boundingBox : boundingBoxes)
		{
			vector center;
			if (!boundingBox || !boundingBox.GetCenter(center) || boundingBox.GetScale() == vector.One)
			{
				Print("SCR_DestructibleBuildingComponent: Interior Bounding Box has been added but not set up!", LogLevel.WARNING);
				continue;
			}

			boundingBox.GetBounds(mins, maxs);
			if (regionalManager && !rplComponent.IsProxy())
				regionalManager.RegisterInteriorBoundingBox(data.m_vStartMatrix, mins, maxs, center);


			//on clients set it handled. We dont check for destrManager because we obtain regionalManager from it
			if (rplComponent.IsProxy() && regionalManager)
			{
				if (destrManager.m_RegionalManagerHandledBoxes.Find(regionalManager.GetRplID(), boxesPositions))
				{
					boxesPositions.Insert(data.m_vStartMatrix[3]+center);
				}
				else
				{
					boxesPositions = new array<ref vector>;
					boxesPositions.Insert(data.m_vStartMatrix[3]+center);
					destrManager.m_RegionalManagerHandledBoxes.Insert(regionalManager.GetRplID(), boxesPositions);
				}
			}

			world.QueryEntitiesByOBB(mins, maxs, data.m_vStartMatrix, AddEntityCallback, QueryFilterCallback, queryFlags);
		}
	}

	//------------------------------------------------------------------------------------------------
	// Uses data queried by DestroyInteriorInit method
	protected void DestroyInterior(bool immediate)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		int count = data.m_aQueriedEntities.Count();
		SCR_DestructibleEntity destructibleEntity;
		SCR_DestructionDamageManagerComponent destructionComponent;
		SCR_EditableEntityComponent editableEntity;
		vector hitPosDirNorm[3];
		RplComponent rplComponent;
		IEntity childEntity;
		array<IEntity> handledEntities = {};
		for (int i = count - 1; i >= 0; i--)
		{

			if (!data.m_aQueriedEntities[i])
			{
				data.m_aQueriedEntities.Remove(i);
				continue;
			}

			//Has children, will be handled later
			childEntity = data.m_aQueriedEntities[i].GetChildren();
			if (childEntity)
			{
				bool wasHandled = handledEntities.Contains(childEntity);

				// Child entity was not queried for some reason && has not been handled yet, add it to the list of queried entities
				if (!wasHandled && !data.m_aQueriedEntities.Contains(childEntity))
				{
					data.m_aQueriedEntities.Insert(childEntity);
					i++;
				}

				// If child was already handled, we can consider this entity a non-parent
				if (!wasHandled)
				{
					continue;
				}
			}

			// Following order of operations is crucial, so we don't end up with null pointers at some point!

			// Any entity that reaches this point is perceived as handled
			handledEntities.Insert(data.m_aQueriedEntities[i]);

			// Ignore entities outside the building
			data.m_iChecksThisFrame++;
			if (data.m_iChecksThisFrame >= MAX_CHECKS_PER_FRAME)
			{
				data.m_iChecksThisFrame = 0;
				// Callqueue used here, because it shouldn't rely on running EOnFrame
				GetGame().GetCallqueue().CallLater(DestroyInterior, param1: immediate);
				return;
			}

			// Interior check for the object
			// Disabled for now, this check now happens under ground, so it's pointless here
			/*if (!IsInside(data.m_aQueriedEntities[i]))
			{
				data.m_aQueriedEntities.Remove(i);
				i--;
				count--;

				continue;
			}*/

			destructibleEntity = SCR_DestructibleEntity.Cast(data.m_aQueriedEntities[i]);
			destructionComponent = SCR_DestructionDamageManagerComponent.Cast(data.m_aQueriedEntities[i].FindComponent(SCR_DestructionDamageManagerComponent));
			editableEntity = SCR_EditableEntityComponent.Cast(data.m_aQueriedEntities[i].FindComponent(SCR_EditableEntityComponent));

			// Non-destructible object, just delete it later, so destruction happens on frame
			if (!destructibleEntity && !destructionComponent && !editableEntity)
				RplComponent.DeleteRplEntity(data.m_aQueriedEntities[i], false);

			// Is destructible entity
			if (destructibleEntity)
				destructibleEntity.HandleDamage(EDamageType.TRUE, destructibleEntity.GetCurrentHealth() * 11, hitPosDirNorm);

			// Uses destruction component
			if (destructionComponent)
				destructionComponent.DeleteDestructibleDelayed();

			// Is editable entity
			if (editableEntity)
				editableEntity.Delete();

			data.m_aQueriedEntities.Remove(i);
		}

		if(!IsProxy())
		{
			Rpc(FinishDestruction);
			FinishDestruction();
		}


		DeleteBuilding();
	}

	//------------------------------------------------------------------------------------------------
	//! Handles the destruction of building itself after interior is handled.
	protected void DeleteBuilding()
	{
		RplComponent.DeleteRplEntity(GetOwner(), false);
	}

	//------------------------------------------------------------------------------------------------
	//! Last method to be called after destruction happens, data is cleared
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void FinishDestruction()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		if (data.m_CameraShake != null)
		{
			//data.m_CameraShake.Stop();
			data.m_CameraShake.SetParams(0.15, 0.15, 0.01, 0.3, 0.24);
			data.m_CameraShake = null;
		}

		data.m_aQueriedEntities = null;
		FreeData();
	}

	//------------------------------------------------------------------------------------------------
	protected bool PerformTrace(notnull TraceParam param, vector start, vector direction, notnull BaseWorld world, float lengthMultiplier = 1)
	{
		param.Start = start - direction * lengthMultiplier;
		param.End = start + direction * lengthMultiplier;
		world.TraceMove(param, TraceFilter);

		return param.TraceEnt != null;
	}

	//------------------------------------------------------------------------------------------------
	//! Checks whether or not an entity is inside of the building, using a trace in each world axis
	protected bool IsInside(notnull IEntity entity)
	{
		IEntity owner = GetOwner();
		BaseWorld world = owner.GetWorld();
		vector start = entity.GetOrigin();

		TraceParam param = new TraceParam();
		param.Flags = TraceFlags.ENTS;
		param.LayerMask = EPhysicsLayerDefs.Projectile;
		param.Include = owner; // Include only the building for performance reasons

		bool result;
		for (int i = 0; i < 3; i++)
		{
			float lengthMultiplier = 1;
			if (i == 1)
				lengthMultiplier = 100; // Vertical traces can and must be long to detect roof, where there is no floor, also they are internally optimized

			result = PerformTrace(param, start, TRACE_DIRECTIONS[i], world, lengthMultiplier);

			if (result)
				return true;
		}

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Filters out unwanted entities
	protected bool TraceFilter(notnull IEntity e, vector start = "0 0 0", vector dir = "0 0 0")
	{
		return e == GetOwner();
	}

	//------------------------------------------------------------------------------------------------
	protected bool HasPhysicalChildren(notnull IEntity e)
	{
		IEntity children = e.GetChildren();
		while (children)
		{
			if (children.GetPhysics())
				return true;

			children = children.GetSibling();
		}

		return false;
	}

	//! Used to filter out entities that are not meant to be handled in AddEntityCallback
	protected bool QueryFilterCallback(notnull IEntity entity)
	{
		IEntity owner = GetOwner();
		IEntity entityParent = entity.GetParent();

		// Exclude the owner && children of other objects
		if (entity == owner || (entityParent && entityParent != owner))
			return false;

		return true;
	}

	//! Used by Query in DestroyInterior
	protected bool AddEntityCallback(notnull IEntity e)
	{
		// Exclude characters and vehicles right away.
		// Note, we query with NO_PROXY flags so characters inside vehicles or inventory
		// items of characters won't even make it here.
		if (ChimeraCharacter.Cast(e))
		{
			// Kill occupants after destruction starts determined by delay set in prefab data, so it appears as if they died from the building falling on top of them
			GetGame().GetCallqueue().CallLater(DamageOccupantsDelayed, GetDelay() * 1000, param1: e);
			return true;
		}
		else if (Vehicle.Cast(e))
		{
			//destroy the vehicle after the destruction delay
			GetGame().GetCallqueue().CallLater(HandleVehicle, GetDelay() * 1000, param1: e);
			return true;
		}

		SCR_BuildingDestructionData data = GetData();

		//write debug here to see where Villa_E_2I01_FIA_01.e is failing to get dragged down

		// Exclude entity if already included.
		// NOTE: This place can be a bottleneck when running a big amount of queries.
		//       The number grows bigger progressively and if there are too many things
		//       to query, checking against e.g. 200 of them might be an issue.
		if (data.m_aQueriedEntities.Contains(e))
			return true;

		// Exclude entities in exclude list
		// Disabled for now, because it might not be necessary
		// Keeping it for later if we decide to use it in some other way than originally planned
		/*if (data.m_aExcludeList.Contains(e))
			return true;*/

		// Exclude static entities such as trees, rocks, ruins etc. and other destructible buildings
		SCR_BuildingDestructionManagerComponent manager = GetGame().GetBuildingDestructionManager();
		foreach (typename typeName : manager.GetExcludedQueryTypes())
		{
			if (e.IsInherited(typeName))
			{
				if (!e.Type().IsInherited(SCR_PowerPole) && !e.Type().IsInherited(SCR_JunctionPowerPole))
					return true;

				// Cast to SCR_PowerPole to access slot data
				SCR_PowerPole powerPole = SCR_PowerPole.Cast(e);
				if (!powerPole)
				{
					// Fallback to entity origin if cast fails
					m_aPowerPolePositions.Insert(e.GetOrigin());
					continue;
				}

				// Get prefab data to access cable slot groups
				SCR_PowerPoleClass prefabData = SCR_PowerPoleClass.Cast(powerPole.GetPrefabData());
				if (!prefabData || !prefabData.m_aCableSlotGroups)
				{
					// Fallback to entity origin if no cable slot groups
					m_aPowerPolePositions.Insert(e.GetOrigin());
					continue;
				}

				// Iterate through cable slot groups and average slot positions per group
				foreach (SCR_PoleCableSlotGroup slotGroup : prefabData.m_aCableSlotGroups)
				{
					if (!slotGroup || !slotGroup.m_aSlots)
						continue;

					vector avgLocalPos = vector.Zero;
					int validSlotCount = 0;

					// Sum all slot positions in this group
					foreach (SCR_PoleCableSlot slot : slotGroup.m_aSlots)
					{
						if (!slot)
							continue;

						avgLocalPos += slot.m_vPosition;
						validSlotCount++;
					}

					// Calculate average and transform to world space
					if (validSlotCount > 0)
					{
						avgLocalPos = avgLocalPos / validSlotCount;
						vector avgWorldPos = powerPole.CoordToParent(avgLocalPos);
						m_aPowerPolePositions.Insert(avgWorldPos);
					}
				}
			}
		}

		// There shouldn't really be any physical hierchies created on buildings and alike,
		// so more than anything, this check is just a precaution.
		// I wish it was just a precaution - Mour
		if (!e.GetPhysics() && !HasPhysicalChildren(e))
			return true;

		SCR_EditorLinkComponent linkComp = SCR_EditorLinkComponent.Cast(e.FindComponent(SCR_EditorLinkComponent));
		if (linkComp)
			return true;

		// In order to sink entities within the building along with it when sinking, we need to make sure
		// they are attached to the building.
		// We already filtered out all entities that have a parent different than this building so it
		// is okay to simply attach the queried entity as a child directly.
		// TODO: On headless, we should probably just delete non-replicated things right away so we don't
		//       waste performance updating the entire building with all of its furniture. Players and
		//       vehicles inside the building are getting destroyed so no collisiton desyncs are probable.
		//       And even if they happened, characters have client-authority collision and vehicles are
		//       server-corrected.
		IEntity owner = GetOwner();
		if(e.GetParent() != owner)
		{
			//e.SetFlags(EntityFlags.USER3);
			owner.AddChild(e, -1, EAddChildFlags.AUTO_TRANSFORM | EAddChildFlags.RECALC_LOCAL_TRANSFORM);
		}

		// If entity is editable, prevent game master interaction
		SCR_EditableEntityComponent editableEntity = SCR_EditableEntityComponent.Cast(e.FindComponent(SCR_EditableEntityComponent));
		if (editableEntity)
			editableEntity.SetEntityState(EEditableEntityState.INTERACTIVE, false);

		data.m_aQueriedEntities.Insert(e);
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void DamageOccupantsDelayed(IEntity targetEntity)
	{
		// The character is outside the building
		if (!IsInside(targetEntity))
			return;

		SCR_CharacterDamageManagerComponent damageManager = SCR_CharacterDamageManagerComponent.Cast(targetEntity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return;

		HitZone headHitzone = damageManager.GetHeadHitZone();
		if (headHitzone)
			headHitzone.HandleDamage(1000, EDamageType.TRUE, null);
	}

	//------------------------------------------------------------------------------------------------
	protected void HandleVehicle(IEntity targetEntity)
	{
		if (!IsInside(targetEntity))
			return;

		SCR_VehicleDamageManagerComponent damageManager = SCR_VehicleDamageManagerComponent.Cast(targetEntity.FindComponent(DamageManagerComponent));
		if (!damageManager)
			return;

		HitZone defaultHitzone = damageManager.GetDefaultHitZone();
		if (defaultHitzone)
			defaultHitzone.HandleDamage(defaultHitzone.GetMaxHealth(), EDamageType.TRUE, null);
	}

	//------------------------------------------------------------------------------------------------
	void HandleConnectedPowerlines()
	{
		BaseWorld world = GetGame().GetWorld();

		foreach (vector polePosition : m_aPowerPolePositions)
		{
			world.QueryEntitiesBySphere(polePosition, 3, ProcessFoundPowerline);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool ProcessFoundPowerline(notnull IEntity e)
	{
		if (e.Type() != PowerlineEntity)
			return true;

		delete e;
		return true;
	}

	//------------------------------------------------------------------------------------------------
	void GoToDestroyedStateLoad(bool spawnEffects = true)
	{
		StoreNavmeshData();
		GoToDestroyedState(true, spawnEffects);
	}

	//------------------------------------------------------------------------------------------------
	void CalculateAndStoreVolume()
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		vector mins, maxs;
		GetOwner().GetBounds(mins, maxs);

		float x = Math.AbsFloat(mins[0]) + maxs[0];
		float y = Math.AbsFloat(mins[1]) + maxs[1];
		float z = Math.AbsFloat(mins[2]) + maxs[2];

		float buildingVolume = x * y * z;

		data.m_fBuildingVolume = buildingVolume;

		data.m_fSizeMultiplier = data.m_fBuildingVolume / BUILDING_SIZE; // BUILDING_SIZE constant is value for the average building size
	}

	//------------------------------------------------------------------------------------------------
	//! Destroys interior
	//! Starts position lerping (Enables frame, activates the entity)
	//! Or moves the building to target position immediately if it's JIP
	protected void GoToDestroyedState(bool immediate, bool spawnEffects = true)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		m_bDestroyed = true;

		if (IsMaster())
			HandleConnectedPowerlines();

		IEntity owner = GetOwner();

		// Ensure registration of destroyed building as they are otherwise not tracked by default
		auto persistence = PersistenceSystem.GetInstance();
		if (persistence)
			persistence.StartTracking(owner);

		vector mins, maxs;
		owner.GetBounds(mins, maxs);

		maxs[0] = 0;
		maxs[2] = 0;

		vector sinkVector = GetSinkVector();
		if (sinkVector == vector.Zero)
			sinkVector = -maxs;

		data.m_vTargetOrigin = owner.GetOrigin() + sinkVector;
		data.m_vStartAngles = owner.GetLocalAngles();

		StaticModelEntity.Cast(owner).DestroyOccluders();

		// Don't animate, JIP happened
		if (immediate)
		{
			DestroyInteriorInit(immediate);
			FinishLerp(owner, immediate, true, spawnEffects);
		}
		else // Animate sinking, play particles, sounds etc...
		{
			owner.GetPhysics().SetResponseIndex(NO_COLLISION_RESPONSE_INDEX);

			auto shake = data.m_CameraShake;
			if (shake != null)
			{
				shake.SetParams(0.15, 0.15, 0.01, 400, 0.24);
				shake.SetCurve(GetCameraShakeCurve());
				shake.SetStartOrigin(data.m_vStartMatrix[3]);
				shake.SetSizeMultiplier(data.m_fSizeMultiplier);

				ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
				if (character && character.IsInVehicle())
				{
					Vehicle vehicle = Vehicle.Cast(character.GetParent());
					if (vehicle)
					{
						VehicleBaseSimulation simulation = VehicleBaseSimulation.Cast(vehicle.FindComponent(VehicleBaseSimulation));
						if (simulation && simulation.HasAnyGroundContact())
							SCR_CameraShakeManagerComponent.AddCameraShake(shake);
					}
				}
				else
				{
					SCR_CameraShakeManagerComponent.AddCameraShake(shake);
				}

				//SCR_CameraShakeManagerComponent.AddCameraShake(shake);
			}

			//SetEventMask(owner, EntityEvent.FRAME);
			EnableDamageSystemOnFrame();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Called in Frame (while building is sinking)
	//! Spawns effects which are supposed to spawn at this time
	//! Immediate destruction = from JIP f. e., only spawn effects that remain after destruction - like ruins prefabs
	protected void SpawnEffects(float percentDone, IEntity owner, bool immediateDestruction)
	{
		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		if (!data.m_aExcludeList)
			data.m_aExcludeList = {};

		SCR_HitInfo hitInfo = new SCR_HitInfo();
		hitInfo.m_DamageType = EDamageType.KINETIC; // Todo properly store damage type

		array<ref SCR_TimedEffect> effects = GetEffects();
		SCR_TimedEffect currentEffect;
		for (int i = effects.Count() - 1; i >= 0; i--)
		{
			if (data.m_aExecutedEffectIndices && data.m_aExecutedEffectIndices.Contains(i))
				continue;

			currentEffect = effects[i]; // Store it, because each effects[i] is effects.Get(i) call internally

			if (immediateDestruction && !currentEffect.m_bPersistent)
				continue; // Skip because destruction happened immediately & effect isn't persistent

			if (currentEffect.m_fSpawnTime <= percentDone)
			{
				currentEffect.ExecuteEffect(owner, hitInfo, data);

				// Create the set if it doesn't exist yet
				if (!data.m_aExecutedEffectIndices)
				{
					data.m_aExecutedEffectIndices = new set<int>();
					// Max size of the set is known beforehand, because m_aEffects is in prefab data
					data.m_aExecutedEffectIndices.Reserve(effects.Count());
				}

				data.m_aExecutedEffectIndices.Insert(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Handles the end of building position lerp
	//! Disables frame, deactivates the entity
	//! Hides the mesh
	//! Plays final effects
	protected void FinishLerp(IEntity owner, bool immediate, bool updateEntity, bool spawnEffects = true)
	{
		owner.SetObject(null, ""); // Hide the building
		//ClearEventMask(owner, EntityEvent.FRAME);
		DisableDamageSystemOnFrame();

		if (spawnEffects)
			SpawnEffects(1, owner, immediate); // Ensure all effects get played

		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		owner.SetOrigin(data.m_vTargetOrigin);

		if (updateEntity)
			owner.Update();
		else
		{
			// Rather than updating entities one by one, let the damage system
			// know it is supposed to perform a parallel update.
			owner.SetFlags(EntityFlags.USER3);
		}

		RegenerateNavmesh();
		DestroyInterior(immediate);
	}

	//------------------------------------------------------------------------------------------------
	//! Regenerates navmesh using previously stored data
	protected void RegenerateNavmesh()
	{
		if (Replication.IsClient())
			return;

		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return;

		aiWorld.RequestNavmeshRebuildAreas(data.m_aNavmeshAreas, data.m_aRedoRoads);
	}

	//------------------------------------------------------------------------------------------------
	//! Stores navmesh data to regenerate navmesh later
	protected void StoreNavmeshData()
	{
		if (Replication.IsClient())
			return;

		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		SCR_AIWorld aiWorld = SCR_AIWorld.Cast(GetGame().GetAIWorld());
		if (!aiWorld)
			return;

		data.m_aNavmeshAreas = {};
		data.m_aRedoRoads = {};
		aiWorld.GetNavmeshRebuildAreas(GetOwner(), data.m_aNavmeshAreas, data.m_aRedoRoads); // Get area with current phase
	}

	//------------------------------------------------------------------------------------------------
	protected void ClampVector(inout vector currentOrigin, vector startOrigin, vector endOrigin)
	{
		bool targetSmaller;
		for (int i = 0; i < 3; i++)
		{
			targetSmaller = endOrigin[i] < startOrigin[i];
			if (targetSmaller)
			{
				if (currentOrigin[i] < endOrigin[i])
					currentOrigin[i] = endOrigin[i];
			}
			else
			{
				if (currentOrigin[i] > endOrigin[i])
					currentOrigin[i] = endOrigin[i];
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void LerpRotation(IEntity owner, float timeSlice)
	{
		if (!owner)
			return;

		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		if (data.m_iRotatedTimes > GetMaxRotations())
			return;

		// When it's time to reset the target rotation
		if (data.m_iRotationStart + data.m_iRotationTime < owner.GetWorld().GetWorldTime() && data.m_iRotatedTimes < GetMaxRotations())
		{
			// Generate next rotation time offset
			float rotationTimeRandomizer = GetRotationTimeRandomizer() * 0.01; // * 0.01 to 0-1 range
			data.m_iRotationTime = GetRotationTime() * (1 + data.m_RandomGenerator.RandFloatXY(0, rotationTimeRandomizer)) * 1000; // * 1000 = to ms

			// Slow down the sinking to make it seem like it crashed into something
			data.m_fSpeedMultiplier *= 0.05;

			// Only call OnSlowDown if it's not initial rotation setting
			if (data.m_iRotatedTimes != 0)
				OnSlowDown();

			bool allowRotationX = GetAllowRotationX();
			bool allowRotationY = GetAllowRotationY();
			bool allowRotationZ = GetAllowRotationZ();

			// Generate new random angles
			vector newTargetAngles = Vector(data.m_RandomGenerator.RandFloatXY(5, 20) * allowRotationX * data.m_iRotationMultiplier + data.m_vStartAngles[0]
											, data.m_RandomGenerator.RandFloatXY(5, 20) * allowRotationY * data.m_iRotationMultiplier + data.m_vStartAngles[1]
											, data.m_RandomGenerator.RandFloatXY(5, 20) * allowRotationZ * data.m_iRotationMultiplier + data.m_vStartAngles[2]);

			// This ensures the rotation will always be going to the opposite side of the previous one
			data.m_iRotationMultiplier *= -1;

			// Reset the speed multiplier
			data.m_fRotationSpeedMultiplier = 0.5;

			// Save the new target angles
			data.m_vTargetAngles = newTargetAngles;

			// Cache world
			BaseWorld world = owner.GetWorld();

			int pauseTime = data.m_RandomGenerator.RandIntInclusive(0, 500);

			// Save the current timestamp as the rotation start
			// Add the pause time to it
			data.m_iRotationStart = world.GetWorldTime() + pauseTime;

			// Set pause time to stop the building for a while
			data.m_iPauseTime = world.GetWorldTime() + pauseTime;

			data.m_iRotatedTimes++;
		}

		// This is the actual lerp
		data.m_fRotationSpeedMultiplier += timeSlice;
		//vector newAngles = vector.Lerp(owner.GetAngles(), data.m_vTargetAngles, timeSlice * Math.Pow(data.m_fRotationSpeedMultiplier, 3));
		vector newAngles = LerpAngles(data.m_vStartAngles, owner.GetLocalAngles(), data.m_vTargetAngles, GetRotationSpeed(), timeSlice, data);
		owner.SetAngles(newAngles);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySlowDownSound()
	{
		const IEntity owner = GetOwner();
		SCR_SoundManagerModule soundManager = SCR_SoundManagerModule.GetInstance(owner.GetWorld());
		if (!soundManager)
			return;

		SCR_AudioSourceConfiguration audioSourceConfiguration = GetSlowDownAudioSourceConfiguration();
		if (!audioSourceConfiguration || !audioSourceConfiguration.IsValid())
			return;

		SCR_AudioSource audioSource = soundManager.CreateAudioSource(owner, audioSourceConfiguration, GetData().m_vStartMatrix[3]);
		if (!audioSource)
			return;

		SetAudioSource(audioSource);

		soundManager.PlayAudioSource(audioSource);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnSlowDown()
	{
		PlaySlowDownSound();
	}

	//------------------------------------------------------------------------------------------------
	protected vector LerpAngles(vector start, vector current, vector target, float rotationSpeed, float timeSlice, notnull SCR_BuildingDestructionData data)
	{
		if (target == vector.Zero)
			return vector.Zero;

		float percent = timeSlice / (data.m_iRotationTime * 0.001);
		vector diff = target - start;
		vector nextRotation = current + percent * diff * rotationSpeed * data.m_fRotationSpeedMultiplier;

		ClampVector(nextRotation, start, target);
		return nextRotation;
	}

	//------------------------------------------------------------------------------------------------
	//! Handles position lerping
	//! Handles calling SpawnEffects, calculates percentDone parameter
	protected void LerpPosition(IEntity owner, float timeSlice)
	{
		if (!owner)
			return;

		SCR_BuildingDestructionData data = GetData();
		if (!data)
			return;

		vector currentOrigin = owner.GetOrigin();
		vector direction = (data.m_vTargetOrigin - currentOrigin).Normalized();

		data.m_fSpeedMultiplier += (GetSpeedGradualMultiplier() * timeSlice) * 0.01;
		vector mat[4];
		owner.GetTransform(mat);
		vector newOrigin = currentOrigin + direction * GetSpeed() * timeSlice * data.m_fSpeedMultiplier;
		ClampVector(newOrigin, data.m_vStartMatrix[3], data.m_vTargetOrigin);

		// No need to clear the parallel update flag.
		// The damage system automatically clears the flag before each entity update.
		// -> owner.ClearFlags(EntityFlags.USER3);

		bool isFinalState =
			float.AlmostEqual(newOrigin[0], data.m_vTargetOrigin[0]) &&
			float.AlmostEqual(newOrigin[1], data.m_vTargetOrigin[1]) &&
			float.AlmostEqual(newOrigin[2], data.m_vTargetOrigin[2]);

		float difY = data.m_vTargetOrigin[1] - data.m_vStartMatrix[3][1];
		float curY = newOrigin[1] - data.m_vStartMatrix[3][1];
		float percentDone = curY/difY;
		SpawnEffects(percentDone, owner, false);

		// Final state
		if (isFinalState)
			FinishLerp(owner, false, false);
		else
		// Getting to the final state
		{
			vector worldMat[4];
			owner.GetWorldTransform(worldMat);
			worldMat[3] = newOrigin;
			owner.SetWorldTransform(worldMat);

			// Rather than updating entities one by one, let the damage system
			// know it is supposed to perform a parallel update.
			owner.SetFlags(EntityFlags.USER3);
		}
	}

	//------------------------------------------------------------------------------------------------
	override bool HijackDamageHandling(notnull BaseDamageContext damageContext)
	{
		if (damageContext.damageEffect && damageContext.damageEffect.Type() == SCR_VehicleFireDamageEffect)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	protected override bool HasDataToReplicate()
	{
		if (m_bDestroyed == true)
			return true;

		return false;
	}

	//------------------------------------------------------------------------------------------------
	//! Serializes state over network
	protected override event bool OnRplSave(ScriptBitWriter writer)
	{
		super.OnRplSave(writer);

		writer.WriteBool(m_bDestroyed);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Loads serialized state on client
	protected override event bool OnRplLoad(ScriptBitReader reader)
	{
		super.OnRplLoad(reader);

		reader.ReadBool(m_bDestroyed);

		if (m_bDestroyed)
		{
			// Need to remove it from callqueue in case it was queued by OnDamageStateChanged
			GetGame().GetCallqueue().Remove(GoToDestroyedState);
			GoToDestroyedState(true);
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Contact to deal damage
	override void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{
		if (IsProxy())
			return;

		if (GetHealth() <= 0)
			return;

		if (other && other.IsInherited(SCR_DebrisSmallEntity)) // Ignore impacts from debris
 			return;

		// Get the physics of the dynamic object (if owner is static, then we use the other object instead)
		Physics physics = contact.Physics1;
		int responseIndex = physics.GetResponseIndex();
		float ownerMass = physics.GetMass();
		float otherMass;
		if (!physics.IsDynamic())
		{
			physics = contact.Physics2;
			if (!physics)
				return; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects

			otherMass = physics.GetMass();
		}
		else
		{
			Physics otherPhysics = other.GetPhysics();
			if (!otherPhysics)
				return; // This only happens with ragdolls, other objects do have physics here, as collision only happens between physical objects

			otherMass = otherPhysics.GetMass();
		}

		float momentum = SCR_DestructionUtility.CalculateMomentum(contact, ownerMass, otherMass);

		vector outMat[3];
		vector relVel = contact.VelocityBefore2 - contact.VelocityBefore1;
		outMat[0] = contact.Position; // Hit position
		outMat[1] = relVel.Normalized(); // Hit direction
		outMat[2] = contact.Normal; // Hit normal

		float damage = momentum * 0.05; // Todo replace with attribute

		// Send damage to damage handling
		Instigator instigator = Instigator.CreateInstigator(other);
		SCR_DamageContext dmgContext = new SCR_DamageContext(EDamageType.COLLISION, damage, outMat, GetOwner(), null, instigator, null, -1, -1);
		HandleDamage(dmgContext);

		return;
	}

	//------------------------------------------------------------------------------------------------
	//! Handles per-frame operations, only enabled while the building is sinking
	protected override void OnFrame(IEntity owner, float timeSlice)
	{
		LerpPosition(owner, timeSlice);
		LerpRotation(owner, timeSlice);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.CONTACT);
	}

#ifdef WORKBENCH
	//------------------------------------------------------------------------------------------------
	override array<ref WB_UIMenuItem> _WB_GetContextMenuItems(IEntity owner)
	{
		array<ref WB_UIMenuItem> items = { new WB_UIMenuItem("Toggle Interior Bounding Box Debug", 0) };

		return items;
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_OnContextMenu(IEntity owner, int id)
	{
		switch (id)
		{
			case 0:
			{
				DiagMenu.SetValue(SCR_DebugMenuID.DEBUGUI_SHOW_INTERIOR_BOUNDING_BOX, !DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SHOW_INTERIOR_BOUNDING_BOX));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void _WB_AfterWorldUpdate(IEntity owner, float timeSlice)
	{
		super._WB_AfterWorldUpdate(owner, timeSlice);

		if (!DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_SHOW_INTERIOR_BOUNDING_BOX))
			return;

		GenericEntity entity = GenericEntity.Cast(owner);
		if (!entity)
			return;

		WorldEditorAPI api = entity._WB_GetEditorAPI();
		if (!api || !api.IsEntitySelected(api.EntityToSource(owner)))
			return;

		// Draw Interior Bounding Box Debug
		vector ownerTransform[4];
		owner.GetWorldTransform(ownerTransform);

		foreach (SCR_InteriorBoundingBox boundingBox : GetInteriorBoundingBoxes())
		{
			if (boundingBox)
				boundingBox.DrawDebug(ownerTransform);
		}
	}
#endif

	//------------------------------------------------------------------------------------------------
	//! Returns true if local instance is proxy (not the authority)
	protected bool IsProxy()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return rplComponent && rplComponent.IsProxy();
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsMaster()
	{
		RplComponent rplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		return rplComponent && rplComponent.IsMaster();
	}
};