[EntityEditorProps(category: "GameScripted/Test", description: "This is used for explosion stress tests")]
class SCR_ExplosionGeneratorClass: GenericEntityClass
{
};

class SCR_ExplosionGenerator: GenericEntity
{
	[Attribute("", UIWidgets.ResourceAssignArray, "Generated explosions will loop through the projectiles on this array", "et")]
	ref array<ResourceName> m_ProjectilesToTrigger;
	
	//Initial values
	[Attribute("0", UIWidgets.EditBox, "Number of explosions to create. 0 = until time ends")]
	int m_NumExplosions;
	
	[Attribute("0", UIWidgets.EditBox, "Time until this entity creates a new explosion.")]
	float m_TimeBetweenExplosions;
	
	[Attribute("0", UIWidgets.EditBox, "Time (in seconds) until no more explosions are generated. This is not needed if Num Explosions is different to 0")]
	float m_TotalDuration;
			
	// current values
	ref array<ref Resource> m_LoadedPrefabs = new array<ref Resource>();
	
	int m_RemainingExplosions = 0;
	float m_TimeUntilNextExplosion = 0;
	float m_RemainingDuration = 0;


	//keeps track of what explosion prefab will be used
	int m_CurrentExplosionPrefab = 0;
	

	//------------------------------------------------------------------------------------------------
	void SCR_ExplosionGenerator(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.FRAME);

		for(int i = 0; i < m_ProjectilesToTrigger.Count(); i++)
		{
			if(m_ProjectilesToTrigger[i])
				m_LoadedPrefabs.Insert(Resource.Load(m_ProjectilesToTrigger[i]));
		}
		
		m_RemainingExplosions = m_NumExplosions;
		m_TimeUntilNextExplosion = m_TimeBetweenExplosions;
		m_RemainingDuration = m_TotalDuration;
	}		

	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice) //EntityEvent.FRAME
	{
		//end conditions, no need to keep updating the object
		if (m_NumExplosions == 0 && m_RemainingDuration < 0 || m_NumExplosions != 0 && m_RemainingExplosions <= 0)
		{
			ClearEventMask(EntityEvent.FRAME);
			return;
		}

		m_TimeUntilNextExplosion -= timeSlice;
		m_RemainingDuration -= timeSlice;

		//no explosion
		if (m_TimeUntilNextExplosion > 0)
			return;

		//added instead of assigned to ensure accuracy
		m_TimeUntilNextExplosion += m_TimeBetweenExplosions;
		
		CreateExplosion(owner);
	}		

	protected void CreateExplosion(IEntity owner)
	{
		ref Resource prefab = m_LoadedPrefabs[m_CurrentExplosionPrefab];
		
		AdvanceExplosionPrefab();
		
		m_RemainingExplosions--;
				
		if (!prefab)
			return;
		
		ref EntitySpawnParams spawnParams = new EntitySpawnParams();
			
		spawnParams.TransformMode = ETransformMode.WORLD;
		owner.GetTransform(spawnParams.Transform);

		IEntity spawnedProjectile = GetGame().SpawnEntityPrefab(prefab, owner.GetWorld(), spawnParams);
		if (!spawnedProjectile)
			return;
		
		Managed managedComp = spawnedProjectile.FindComponent(BaseTriggerComponent);
	
		BaseTriggerComponent comp = BaseTriggerComponent.Cast(managedComp);

		if(comp)
			comp.OnUserTrigger(spawnedProjectile);
	}
	
	//------------------------------------------------------------------------------------------------

	protected void AdvanceExplosionPrefab()
	{
		++m_CurrentExplosionPrefab;
		
		if (m_CurrentExplosionPrefab >= m_LoadedPrefabs.Count())
			m_CurrentExplosionPrefab = 0;
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_ExplosionGenerator()
	{
	}
};
	