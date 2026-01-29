[EntityEditorProps(category: "GameScripted/Buildings", description: "Entity used to represent each region of a building when in the damaged state.", dynamicBox: true)]
class SCR_BuildingRegionEntityClass: BaseBuildingClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_BuildingRegionEntity : BaseBuilding
{
	#ifdef ENABLE_BUILDING_DESTRUCTION
	private SCR_DestructibleBuildingEntity m_OwnerBuilding = null;
	private int m_RegionNumber = -1;
	
	ref map<int, SCR_RegionIntersectDebrisEntity> m_RegionIntersectDebris = null;
	
	//------------------------------------------------------------------------------------------------
	//! Deletes all region intersect debris etc
	protected void Cleanup()
	{
		if (!m_RegionIntersectDebris)
			return;
		
		for (int i = m_RegionIntersectDebris.Count() - 1; i >= 0; i--)
		{
			SCR_RegionIntersectDebrisEntity debris = m_RegionIntersectDebris.GetElement(i);
			if (debris)
				delete debris;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads the region's model file, updating physics etc, returns whether successful
	VObject LoadIntersectDebrisModel(int otherRegionIndex)
	{
		if (!m_OwnerBuilding)
			return null;
		
		if (!m_OwnerBuilding.m_BuildingSetup)
			return null;
		
		ResourceName assetPath = m_OwnerBuilding.m_BuildingSetup.GetRegionIntersectDebrisModel(m_RegionNumber, otherRegionIndex);
		if (!FileIO.FileExist(assetPath.GetPath()))
			return null;
		
		Resource resource = Resource.Load(assetPath);
		if (!resource)
			return null;
		
		BaseResourceObject baseObj = resource.GetResource();
		if (!baseObj)
			return null;
		
		return baseObj.ToVObject();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Deletes the intersection debris to another region
	void DeleteIntersectDebris(int otherRegionIndex)
	{
		if (!m_RegionIntersectDebris)
			return;
		
		SCR_RegionIntersectDebrisEntity debris = null;
		m_RegionIntersectDebris.Find(otherRegionIndex, debris);
		
		if (debris)
			delete debris;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Creates the intersection debris to another region
	void CreateIntersectDebris(int otherRegionIndex)
	{
		SCR_RegionIntersectDebrisEntity debris = null;
		
		if (!m_RegionIntersectDebris)
			m_RegionIntersectDebris = new map<int, SCR_RegionIntersectDebrisEntity>();
		
		// Already created, ignore
		if (m_RegionIntersectDebris.Find(otherRegionIndex, debris))
			return;
		
		// Load the debris model (if exists)
		VObject debrisVObj = LoadIntersectDebrisModel(otherRegionIndex);
		if (!debrisVObj)
			return;
		
		debris = SCR_RegionIntersectDebrisEntity.Cast(GetGame().SpawnEntity(SCR_RegionIntersectDebrisEntity));
		
		AddChild(debris, -1, EAddChildFlags.AUTO_TRANSFORM);
		
		debris.SetObject(debrisVObj, "");
		debris.Update();
		
		m_RegionIntersectDebris.Insert(otherRegionIndex, debris);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Loads the region's model file, updating physics etc, returns whether successful
	bool LoadRegionModel()
	{
		if (!m_OwnerBuilding)
			return false;
		
		if (!m_OwnerBuilding.m_BuildingSetup)
			return false;
		
		Resource resource = Resource.Load(m_OwnerBuilding.m_BuildingSetup.GetDamagedRegionModel(m_RegionNumber));
		if (!resource)
			return false;
		
		BaseResourceObject baseObj = resource.GetResource();
		if (!baseObj)
			return false;
		
		VObject obj = baseObj.ToVObject();
		if (!obj)
			return false;
		
		SetObject(obj, "");
		Physics phys = Physics.CreateStatic(this, -1);
		
		phys.SetInteractionLayer(EPhysicsLayerPresets.BuildingFireView);
		
		//Update();
		SetFlags(EntityFlags.ACTIVE);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	SCR_DestructibleBuildingEntity GetBuildingOwner()
	{
		return m_OwnerBuilding;
	}
	
	//------------------------------------------------------------------------------------------------
	int GetRegionNumber()
	{
		return m_RegionNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetBuildingAndIndex(SCR_DestructibleBuildingEntity building, int regionNumber)
	{
		m_OwnerBuilding = building;
		m_RegionNumber = regionNumber;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(float damage, EDamageType type, IEntity pHitEntity, inout vector outMat[3], IEntity damageSource, notnull Instigator instigator, int colliderID, float speed)
	{
		if (!m_OwnerBuilding)
			return;
		
		damage = SCR_Global.GetScaledStructuralDamage(damage, type);
		if (damage <= 0)
			return;
		
		m_OwnerBuilding.AddRegionDamage(m_RegionNumber, damage);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_BuildingRegionEntity()
	{
		Cleanup();
	}
	#endif
};