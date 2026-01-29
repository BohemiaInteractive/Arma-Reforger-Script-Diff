[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "THIS IS THE SCRIPT DESCRIPTION.", color: "0 0 255 255")]
class SCR_VehicleSpawnerClass : ScriptComponentClass
{
}

class SCR_VehicleSpawner : ScriptComponent
{
	static const float MIN_DISTANCE = 10;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "", "")]
	private ref array<ResourceName> m_aPrefabs;
	
	private IEntity m_pSpawnedEntity;
	private IEntity m_owner;
	
	private bool m_bSpawned = false;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		m_owner = owner;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] resources
	//! \return
	ResourceName PickRandomResource(array<ResourceName> resources)
	{
		if (!resources)
			return string.Empty;
		
		int count = resources.Count();
		if (count < 1)
			return string.Empty;
		
		int randomIndex = Math.RandomInt(0, count);		
		return resources[randomIndex];
	}

	//------------------------------------------------------------------------------------------------
	//!
	void PerformSpawn()
	{
		if (!m_owner)
			return;
		
		if (m_pSpawnedEntity && vector.Distance(m_pSpawnedEntity.GetOrigin(), m_owner.GetOrigin()) < MIN_DISTANCE)
			return;
		
		ArmaReforgerScripted game = GetGame();
		if (!game)
			return;
		
		if (!game.InPlayMode())
			return;
		
		
		// server only
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		ResourceName randomResource = PickRandomResource(m_aPrefabs);
		if (randomResource == string.Empty)
		{
			Print("Resource is null or empty! Cannot spawn!", LogLevel.ERROR);
			return;
		}
		
		Resource resource = Resource.Load(randomResource);
		if (!resource)
			return;
		
		ref EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		m_owner.GetTransform(params.Transform);
		
		m_pSpawnedEntity = game.SpawnEntityPrefab(resource, m_owner.GetWorld(), params);
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] ent
	//! \param[in] parent
	void SCR_VehicleSpawner(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	// destructor
	void ~SCR_VehicleSpawner()
	{
		// server only
		if (RplSession.Mode() == RplMode.Client)
			return;
		
		if (m_pSpawnedEntity)
			delete m_pSpawnedEntity;

		m_pSpawnedEntity = null;
	}
}
