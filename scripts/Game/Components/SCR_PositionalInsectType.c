//! Base class that handles different insects based on their type
[BaseContainerProps(configRoot: true)]
class SCR_PositionalInsectType
{
	[Attribute()]
	protected ref array<ref SCR_InsectDef> m_aInsectDef;

	[Attribute("", UIWidgets.ResourceNamePicker, desc: "", params: "conf")]
	ResourceName m_sSpawnDef;

	[Attribute(defvalue: "25", UIWidgets.Slider, params: "0 100 1", desc: "When suitable entity is found, with what chance is Insect spawned there")]
	protected int m_iSpawnChance;

	[Attribute(defvalue: "0 0 0", desc: "Offset from parent entity")];
	protected vector m_vOffset;

	protected SCR_AmbientSoundsComponent m_AmbientSoundsComponent;
	protected SignalsManagerComponent m_LocalSignalsManager;

	protected ref array<IEntity> m_aClosestEntities = {};
	protected ref array<IEntity> m_aActiveEntities = {};
	protected ref array<ref SCR_InsectParticle> m_aParticles = {};

	protected ref set<ref Resource> m_ResourceSet = new set<ref Resource>();
	protected ref set<BaseContainer> m_PrefabContainerSet = new set<BaseContainer>();
	
	ref SCR_InsectSpawnDef m_SpawnDef;
	float m_fDayTimeCurve = 1;
	float m_fRainIntensityCurve = 1;

	protected static const int INVALID = -1;

	//------------------------------------------------------------------------------------------------
	//! Initialises necessary things for Insect type
	//! \param[in] ambientSoundsComponent
	//! \param[in] objectPositionalInsects
	//! \param[in] ambientInsectsComponent
	void Init(notnull SCR_AmbientSoundsComponent ambientSoundsComponent, notnull SCR_ObjectPositionalInsects objectPositionalInsects, notnull SCR_AmbientInsectsComponent ambientInsectsComponent)
	{
		m_AmbientSoundsComponent = ambientSoundsComponent;
		
		if (!m_sSpawnDef.IsEmpty())
		{
			Resource holder = BaseContainerTools.LoadContainer(m_sSpawnDef);
			if (holder)
				m_SpawnDef = SCR_InsectSpawnDef.Cast(BaseContainerTools.CreateInstanceFromContainer(holder.GetResource().ToBaseContainer()));
		}

		if (!m_SpawnDef)
		{
			Print("AMBIENT LIFE: SCR_AmbientInsectsComponent: Missing Spawn Definition Config", LogLevel.WARNING);
			ambientInsectsComponent.RemoveInsectEffect(objectPositionalInsects);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	//! \param[in] cameraPos
	void Update(vector cameraPos);
	
	//------------------------------------------------------------------------------------------------
	//! Randomises animation for Insect
	protected void RandomizeAnimation(IEntity entity)
	{
		AnimationPlayerComponent animComponent = AnimationPlayerComponent.Cast(entity.FindComponent(AnimationPlayerComponent));
		if (!animComponent)
			return;
		
		ResourceName animationResource = animComponent.GetAnimation();
		if (!animationResource || SCR_StringHelper.IsEmptyOrWhiteSpace(animationResource))
			return;
			
		animComponent.Prepare(animationResource, Math.RandomFloatInclusive(1, 5), 1, true);
		animComponent.Play();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns particle with sound based on spawnParams and randomized chance
	//! \param[in] spawnParams
	//! \param[in] chance
	protected void SpawnParticle(ParticleEffectEntitySpawnParams spawnParams, int chance = 100)
	{
		SCR_InsectDef insectDef = m_aInsectDef.GetRandomElement();
		SCR_InsectParticle particle = new SCR_InsectParticle();
		if (Math.RandomIntInclusive(0, 100) <= chance)
		{
			string soundName = insectDef.m_sSoundName;
			if (!soundName.IsEmpty())
				particle.m_AudioHandle = m_AmbientSoundsComponent.SoundEventLooped(soundName, spawnParams.Transform);

			ResourceName prefabName = insectDef.m_sPrefabName;
			if (!prefabName.IsEmpty())
			{
				Resource resource = Resource.Load(prefabName);
				if (resource)
				{
					EntitySpawnParams entitySpawnParams = new EntitySpawnParams;
					entitySpawnParams.Transform = spawnParams.Transform;
					entitySpawnParams.TransformMode = spawnParams.TransformMode;
					
					particle.m_InsectEntity = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), entitySpawnParams);
					if (particle.m_InsectEntity)
					{
						particle.m_InsectEntity.SetVComponentFlags(VCFlags.DYNAMICBBOX);
						RandomizeAnimation(particle.m_InsectEntity);
					}
				}
			}
			else if (!insectDef.m_sParticleEffect.IsEmpty())
			{
				particle.m_ParticleEffect = ParticleEffectEntity.SpawnParticleEffect(insectDef.m_sParticleEffect, spawnParams);
			}

#ifdef ENABLE_DIAG
			if (DiagMenu.GetBool(SCR_DebugMenuID.DEBUGUI_AMBIENT_LIFE))
			{
				if (!soundName.IsEmpty())
					particle.m_DebugShape = Shape.CreateSphere(COLOR_GREEN, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, spawnParams.Transform[3], 0.2);
				else
					particle.m_DebugShape = Shape.CreateSphere(COLOR_BLUE, ShapeFlags.NOZWRITE | ShapeFlags.WIREFRAME, spawnParams.Transform[3], 0.2);
			}
#endif
		}

		m_aParticles.Insert(particle);
	}

	//------------------------------------------------------------------------------------------------
	//! Deletes Insects that are too far away
	protected void RemoveDistantInsects()
	{
		for (int i = m_aActiveEntities.Count() - 1; i >= 0; i--)
		{
			int index = m_aClosestEntities.Find(m_aActiveEntities[i]);
			if (index == INVALID)
			{
				if (m_aParticles[i].m_AudioHandle)
					m_AmbientSoundsComponent.TerminateLooped(m_aParticles[i].m_AudioHandle);
				
				m_aActiveEntities.Remove(i);
				m_aParticles.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Get prefab container set
	//! \return prefab container set
	set<BaseContainer> GetPrefabContainerSet()
	{
		return m_PrefabContainerSet;
	}

	//------------------------------------------------------------------------------------------------
	//! Adds entity to the closest entities array for selected insect type
	//! \param[in] entity
	void AddClosestEntity(IEntity entity)
	{
		m_aClosestEntities.Insert(entity);
	}

	//------------------------------------------------------------------------------------------------
	//! Clears closest entities array for selected insect type
	void ClearClosestEntities()
	{
		m_aClosestEntities.Clear();
	}
}

//! Handles insects that are supposed to be spawned around light sources defined in prefab names array
[BaseContainerProps(configRoot: true)]
class SCR_PositionalLightSourceInsect : SCR_PositionalInsectType
{
	[Attribute(desc: "Prefab names of entities that correspond with this insect type")]
	protected ref array<ResourceName> m_aPrefabNames;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises necessary things for Insect type
	override void Init(notnull SCR_AmbientSoundsComponent ambientSoundsComponent, notnull SCR_ObjectPositionalInsects objectPositionalInsects, notnull SCR_AmbientInsectsComponent ambientInsectsComponent)
	{
		super.Init(ambientSoundsComponent, objectPositionalInsects, ambientInsectsComponent);

		Resource resource;		
		foreach (ResourceName prefab : m_aPrefabNames)
		{
			resource = Resource.Load(prefab);
			if (!resource.IsValid())
				continue;

			m_ResourceSet.Insert(resource);
			m_PrefabContainerSet.Insert(resource.GetResource().ToBaseContainer());
		}

		m_aPrefabNames = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	override void Update(vector cameraPos)
	{
		// Removes insects that are no longer among closest entities
		RemoveDistantInsects();

		// Play sounds on entities, that become closest
		ProcessLightsSources();
	}
	
	//------------------------------------------------------------------------------------------------
	//! \param[in] state
	void OnObjectDamage(EDamageState state)
	{
		if (state != EDamageState.DESTROYED)
			return;
		
		IEntity entity;
		IEntity childEntity;
		SCR_DamageManagerComponent objectDmgManager;
		for (int i = m_aActiveEntities.Count() - 1; i >= 0; i--)
		{
			entity = m_aActiveEntities[i];
			childEntity = entity.GetChildren();
			if (!childEntity)
				continue;
			
			objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(childEntity);
			if (objectDmgManager)
			{
				if (objectDmgManager.GetState() != EDamageState.DESTROYED)
					continue;
					
				objectDmgManager.GetOnDamageStateChanged().Remove(OnObjectDamage);
				
				if (m_aParticles[i].m_AudioHandle)
					m_AmbientSoundsComponent.TerminateLooped(m_aParticles[i].m_AudioHandle);
				
				m_aActiveEntities.Remove(i);
				m_aParticles.Remove(i);
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Processes closest light sources and spawns Insects on correct positions
	protected void ProcessLightsSources()
	{
		IEntity childEntity;
		SCR_DamageManagerComponent objectDmgManager;
		foreach (IEntity entity : m_aClosestEntities)
		{
			if (m_aActiveEntities.Contains(entity))
				continue;
			
			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			vector mat[4];
			entity.GetWorldTransform(mat);
			vector position = mat[3];
			
			//Lamps have lights as their children
			childEntity = entity.GetChildren();
			if (!childEntity)
				continue;
			
			position = 	childEntity.CoordToParent(m_vOffset);
			
			objectDmgManager = SCR_DamageManagerComponent.GetDamageManager(childEntity);
			if (objectDmgManager)
			{
				if (objectDmgManager.GetState() == EDamageState.DESTROYED)
					continue;
				
				objectDmgManager.GetOnDamageStateChanged().Insert(OnObjectDamage);
			}
			
			mat[3] = position;
			
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			//Apply rotation
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);

			m_aActiveEntities.Insert(entity);
			SpawnParticle(spawnParams, m_iSpawnChance);
		}
	}
}

//! Handles insects that are supposed to be spawned trees
[BaseContainerProps(configRoot: true)]
class SCR_PositionalAmbientLeafParticles : SCR_PositionalInsectType
{
	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	override void Update(vector cameraPos)
	{
		//Get closest entities array
		m_AmbientSoundsComponent.GetClosestEntities(EQueryType.TreeLeafy, 10, m_aClosestEntities);

		// Removes insects that are no longer among closest entities
		RemoveDistantInsects();

		// Play sounds on entities, that become closest
		ProcessTrees();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes closest trees and spawns Insects on correct positions
	protected void ProcessTrees()
	{
		foreach (IEntity entity : m_aClosestEntities)
		{
			if (m_aActiveEntities.Contains(entity))
				continue;

			//Get tree entity
			Tree tree = Tree.Cast(entity);
			if (!tree)
				continue;

			//Get prefab data
			TreeClass treeClass = TreeClass.Cast(tree.GetPrefabData());
			if (!treeClass)
				continue;

			int foliageHeight = treeClass.m_iFoliageHeight * entity.GetScale();

			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			vector mat[4];
			entity.GetWorldTransform(mat);
			
			vector mins, maxs;
			entity.GetWorldBounds(mins, maxs);
			float minY = mins[1] + foliageHeight;
			float maxY = maxs[1];
			
			vector position = mat[3];
			position[1] = (minY + maxY) * 0.5;
			position += m_vOffset;
			mat[3] = position;
			
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			//Apply rotation
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);

			m_aActiveEntities.Insert(entity);
			SpawnParticle(spawnParams, m_iSpawnChance);
		}
	}
}

//! Handles insects that are supposed to be spawned around selected prefabs defined in prefab names array
[BaseContainerProps(configRoot: true)]
class SCR_PositionalGenericInsect : SCR_PositionalInsectType
{
	[Attribute(desc: "Prefab names of entities that correspond with this insect type")]
	protected ref array<ResourceName> m_aPrefabNames;
	
	//------------------------------------------------------------------------------------------------
	//! Initialises necessary things for Insect type
	override void Init(notnull SCR_AmbientSoundsComponent ambientSoundsComponent, notnull SCR_ObjectPositionalInsects objectPositionalInsects, notnull SCR_AmbientInsectsComponent ambientInsectsComponent)
	{
		super.Init(ambientSoundsComponent, objectPositionalInsects, ambientInsectsComponent);

		Resource resource;		
		foreach (ResourceName prefab : m_aPrefabNames)
		{
			resource = Resource.Load(prefab);
			if (!resource.IsValid())
				continue;

			m_ResourceSet.Insert(resource);
			m_PrefabContainerSet.Insert(resource.GetResource().ToBaseContainer());
		}

		m_aPrefabNames = null;
	}

	//------------------------------------------------------------------------------------------------
	//! Performs updates to the Insect type
	override void Update(vector cameraPos)
	{
		// Removes insects that are no longer among closest entities
		RemoveDistantInsects();

		// Play sounds on entities, that become closest
		ProcessEntitySources();
	}

	//------------------------------------------------------------------------------------------------
	//! Processes closest entities and spawns Insects on correct positions
	protected void ProcessEntitySources()
	{
		foreach (IEntity entity : m_aClosestEntities)
		{
			if (m_aActiveEntities.Contains(entity))
				continue;
			
			ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
			vector mat[4];
			entity.GetWorldTransform(mat);
			
			vector position = mat[3];
			position += m_vOffset;
			mat[3] = position;
			
			spawnParams.TransformMode = ETransformMode.WORLD;
			spawnParams.Transform = mat;
			
			//Apply rotation
			vector angles = Math3D.MatrixToAngles(spawnParams.Transform);
			Math3D.AnglesToMatrix(angles, spawnParams.Transform);

			m_aActiveEntities.Insert(entity);
			SpawnParticle(spawnParams, m_iSpawnChance);
		}
	}
}
